// =============================================================================
// roc::vision::Yolo26DetectPostprocessor — §4.1 NMS-free decoder.
//
// Tensor layout (channel-major, single batch):
//     Y[0, c, n]    where c in [0, 4 + C), n in [0, N).
//
// Implementation notes:
//   * Single linear pass over N anchors.  For each anchor, we read the four
//     bbox channels and then the C class logits, applying sigmoid only to the
//     winning class (saves C-1 sigmoid evaluations per anchor at N ≈ 8400 →
//     ~50K sigmoids saved per frame).  Actually we apply sigmoid to all to
//     pick the max — but only for the *score*, never on the box channels;
//     boxes are in letterboxed pixels directly.
//   * We accept either explicit `[1, 4+C, N]` or `[4+C, N]` (rank-2 export).
//     Rejecting any other rank with an empty return is the §"no exceptions
//     in the hot path" contract.
//   * Channel-major / row-major ambiguity: YOLO26 always exports channel-
//     major (channels stride = N*sizeof(float)).  We honour that contract.
// =============================================================================

#include "roc/vision/postprocess/yolo26_detect.hpp"
#include "roc/vision/letterbox.hpp"

#include <Eigen/Dense>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace roc {
namespace vision {

namespace {

inline float fast_sigmoid(float x) noexcept {
    // 1 / (1 + exp(-x)).  Saturate exponent for numerical stability.
    if (x >= 0.0f) {
        const float z = std::exp(-x);
        return 1.0f / (1.0f + z);
    }
    const float z = std::exp(x);
    return z / (1.0f + z);
}

// Returns a Label given a class id and the manifest's class table; defaults
// to Label::Unknown if the id is out of range (which the manifest validator
// should already reject — defensive belt-and-braces).
Label lookup_label(const ManifestSpec& m, int cls_id) noexcept {
    for (const auto& c : m.classes) {
        if (c.id == cls_id) return c.roc_label;
    }
    return Label::Unknown;
}

}  // namespace

std::vector<Detection>
Yolo26DetectPostprocessor::decode_detections(const std::vector<Ort::Value>& outputs,
                                             const PostprocessContext&      ctx) const {
    std::vector<Detection> dets;
    if (outputs.empty() || !ctx.manifest) return dets;

    const Ort::Value& y = outputs[0];
    if (!y.IsTensor()) return dets;

    auto type_info = y.GetTensorTypeAndShapeInfo();
    auto shape = type_info.GetShape();
    const auto elem = type_info.GetElementType();
    if (elem != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) return dets;

    // Accept [1, F, N] (F = 4 + C) or [F, N]; reject anything else.
    int64_t F = 0;
    int64_t N = 0;
    if (shape.size() == 3 && shape[0] == 1) {
        F = shape[1];
        N = shape[2];
    } else if (shape.size() == 2) {
        F = shape[0];
        N = shape[1];
    } else {
        return dets;
    }
    if (F <= 4 || N <= 0) return dets;

    const int C = static_cast<int>(F) - 4;
    if (C != ctx.manifest->output.num_classes) {
        // The manifest's num_classes must match the live tensor; the loader's
        // §3.4 #2 cross-check normally rejects this before we are reached.
        // We still degrade rather than throw on the hot path.
        return dets;
    }

    const float*  data = y.GetTensorData<float>();
    const std::size_t stride_channel = static_cast<std::size_t>(N);  // channel-major

    const double thresh = ctx.manifest->output.thresholds.score;
    const float  thresh_f = static_cast<float>(thresh);

    dets.reserve(static_cast<std::size_t>(N) / 32);  // generous lower-bound hint

    const LetterboxParams& lp = ctx.letterbox;

    for (int64_t n = 0; n < N; ++n) {
        const float cx_m = data[0 * stride_channel + static_cast<std::size_t>(n)];
        const float cy_m = data[1 * stride_channel + static_cast<std::size_t>(n)];
        const float w_m  = data[2 * stride_channel + static_cast<std::size_t>(n)];
        const float h_m  = data[3 * stride_channel + static_cast<std::size_t>(n)];

        // Argmax + sigmoid only over the class channels.
        float best_logit = -std::numeric_limits<float>::infinity();
        int   best_cls   = -1;
        for (int c = 0; c < C; ++c) {
            const float logit = data[(static_cast<std::size_t>(4 + c)) * stride_channel +
                                     static_cast<std::size_t>(n)];
            if (logit > best_logit) {
                best_logit = logit;
                best_cls   = c;
            }
        }
        const float best_score = fast_sigmoid(best_logit);
        if (best_score < thresh_f || best_cls < 0) continue;

        // §4.2 inverse letterbox — bring boxes back to capture frame pixels.
        double cx_src, cy_src, w_src, h_src;
        LetterboxPreprocessor::unletterbox(static_cast<double>(cx_m),
                                           static_cast<double>(cy_m),
                                           static_cast<double>(w_m),
                                           static_cast<double>(h_m),
                                           lp, cx_src, cy_src, w_src, h_src);

        Detection det;
        det.bbox_center.u = cx_src;
        det.bbox_center.v = cy_src;
        det.bbox_w_px     = w_src;
        det.bbox_h_px     = h_src;
        det.confidence    = static_cast<double>(best_score);
        det.label         = lookup_label(*ctx.manifest, best_cls);
        det.appearance    = Embedding::Zero();   // populated later by the ReID engine
        dets.push_back(det);
    }
    // --- Class-agnostic NMS pass (manifest lacks NMS-in-graph, and YOLO26's
    //     dual assignment reduces but doesn't eliminate overlapping proposals
    //     for COCO-pretrained models).  Hard IoU floor of 0.45 matches the
    //     Ultralytics default; future manifest schema bumps will read this
    //     from `output.thresholds.nms_iou`. ---
    if (dets.size() > 1) {
        constexpr float kNmsIou = 0.45f;
        std::sort(dets.begin(), dets.end(),
            [](const Detection& a, const Detection& b) noexcept {
                return a.confidence > b.confidence;
            });

        std::vector<bool> suppressed(dets.size(), false);
        for (std::size_t i = 0; i < dets.size(); ++i) {
            if (suppressed[i]) continue;
            const double ax1 = dets[i].bbox_center.u - dets[i].bbox_w_px * 0.5;
            const double ay1 = dets[i].bbox_center.v - dets[i].bbox_h_px * 0.5;
            const double ax2 = dets[i].bbox_center.u + dets[i].bbox_w_px * 0.5;
            const double ay2 = dets[i].bbox_center.v + dets[i].bbox_h_px * 0.5;
            const double a_area = dets[i].bbox_w_px * dets[i].bbox_h_px;

            for (std::size_t j = i + 1; j < dets.size(); ++j) {
                if (suppressed[j]) continue;
                const double bx1 = std::max(ax1, dets[j].bbox_center.u - dets[j].bbox_w_px * 0.5);
                const double by1 = std::max(ay1, dets[j].bbox_center.v - dets[j].bbox_h_px * 0.5);
                const double bx2 = std::min(ax2, dets[j].bbox_center.u + dets[j].bbox_w_px * 0.5);
                const double by2 = std::min(ay2, dets[j].bbox_center.v + dets[j].bbox_h_px * 0.5);
                const double iw = bx2 - bx1;
                const double ih = by2 - by1;
                if (iw <= 0.0 || ih <= 0.0) continue;
                const double inter = iw * ih;
                const double b_area = dets[j].bbox_w_px * dets[j].bbox_h_px;
                const double iou = inter / (a_area + b_area - inter + 1e-12);
                if (iou > kNmsIou) suppressed[j] = true;
            }
        }

        std::vector<Detection> kept;
        kept.reserve(dets.size());
        for (std::size_t i = 0; i < dets.size(); ++i) {
            if (!suppressed[i]) kept.push_back(std::move(dets[i]));
        }
        dets = std::move(kept);
    }

    return dets;
}

}  // namespace vision
}  // namespace roc
