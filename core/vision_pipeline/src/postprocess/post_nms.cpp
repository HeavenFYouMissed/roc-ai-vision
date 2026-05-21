// =============================================================================
// roc::vision::PostNmsPostprocessor — end-to-end NMS decoder for [1, 300, 6]
// =============================================================================

#include "roc/vision/postprocess/post_nms.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace roc {
namespace vision {

namespace {

Label lookup_label(const ManifestSpec& m, int cls_id) noexcept {
    for (const auto& c : m.classes) {
        if (c.id == cls_id) return c.roc_label;
    }
    return Label::Unknown;
}

}  // namespace

std::vector<Detection>
PostNmsPostprocessor::decode_detections(const std::vector<Ort::Value>& outputs,
                                         const PostprocessContext&      ctx) const {
    std::vector<Detection> dets;
    if (outputs.empty() || !ctx.manifest) return dets;

    const Ort::Value& y = outputs[0];
    if (!y.IsTensor()) return dets;

    auto type_info = y.GetTensorTypeAndShapeInfo();
    auto shape = type_info.GetShape();
    if (type_info.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) return dets;

    // Accept [1, N, 6] or [N, 6]
    int64_t N = 0;
    int64_t K = 0;
    if (shape.size() == 3 && shape[0] == 1) {
        N = shape[1];
        K = shape[2];
    } else if (shape.size() == 2) {
        N = shape[0];
        K = shape[1];
    } else {
        return dets;
    }
    if (K != 6 || N <= 0) return dets;

    const float* data = y.GetTensorData<float>();
    const double thresh = ctx.manifest->output.thresholds.score;
    const int imgW = ctx.letterbox.dst_w > 0 ? ctx.letterbox.dst_w : ctx.letterbox.src_w;
    const int imgH = ctx.letterbox.dst_h > 0 ? ctx.letterbox.dst_h : ctx.letterbox.src_h;

    const std::size_t stride = static_cast<std::size_t>(N);

    for (int64_t n = 0; n < N; ++n) {
        const float x1   = data[0 * stride + static_cast<std::size_t>(n)];
        const float y1   = data[1 * stride + static_cast<std::size_t>(n)];
        const float x2   = data[2 * stride + static_cast<std::size_t>(n)];
        const float y2   = data[3 * stride + static_cast<std::size_t>(n)];
        const float conf = data[4 * stride + static_cast<std::size_t>(n)];
        const float cls_f = data[5 * stride + static_cast<std::size_t>(n)];

        if (conf < static_cast<float>(thresh)) continue;

        const int cls_id = static_cast<int>(cls_f);
        if (cls_id < 0) continue;

        // Convert normalized coords to source-pixel coords
        const double cx = static_cast<double>((x1 + x2) * 0.5f * static_cast<float>(imgW));
        const double cy = static_cast<double>((y1 + y2) * 0.5f * static_cast<float>(imgH));
        const double w  = static_cast<double>((x2 - x1) * static_cast<float>(imgW));
        const double h  = static_cast<double>((y2 - y1) * static_cast<float>(imgH));

        if (w <= 1.0 || h <= 1.0) continue; // skip degenerate boxes

        Detection det;
        det.bbox_center.u = cx;
        det.bbox_center.v = cy;
        det.bbox_w_px     = w;
        det.bbox_h_px     = h;
        det.confidence    = static_cast<double>(conf);
        det.label         = lookup_label(*ctx.manifest, cls_id);
        det.appearance    = Embedding::Zero();

        dets.push_back(det);
    }

    return dets;
}

}  // namespace vision
}  // namespace roc
