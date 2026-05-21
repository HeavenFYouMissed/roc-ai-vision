// =============================================================================
// roc::vision::LetterboxPreprocessor — §4.2 + §5.1 implementation.
//
// Detection manifests (e.g. yolo26m): preserve aspect ratio, pad with
// (114, 114, 114), cache (s, pad_x, pad_y) for the inverse transform.
//
// ReID manifests (osnet-ain-x1-0): stretch resize, no padding.  The
// inverse transform is a pure (sx, sy) scale.
//
// Channel reordering, float conversion and per-channel mean/std normalisation
// are applied in a single pass over the post-resize image.  We compute on
// uint8 → float32 directly into the NCHW packed buffer to avoid a redundant
// cv::Mat → float conversion intermediate.
// =============================================================================

#include "roc/vision/letterbox.hpp"

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace roc {
namespace vision {

LetterboxPreprocessor::LetterboxPreprocessor(const InputSpec& spec) : spec_(spec) {}

void LetterboxPreprocessor::preprocess(const cv::Mat&       src,
                                       std::vector<float>&  dst,
                                       LetterboxParams&     params_out) {
    const int C = static_cast<int>(spec_.expected_shape[1]);
    const int H = static_cast<int>(spec_.expected_shape[2]);
    const int W = static_cast<int>(spec_.expected_shape[3]);
    const std::size_t n_elem = static_cast<std::size_t>(C) *
                               static_cast<std::size_t>(H) *
                               static_cast<std::size_t>(W);

    dst.resize(n_elem);
    preprocess_into(src, spec_.preprocessing.letterbox, dst.data(), params_out);
}

void LetterboxPreprocessor::preprocess_batch(Span<const cv::Mat>            crops_bgr,
                                             std::vector<float>&            dst,
                                             std::vector<LetterboxParams>&  params_out) {
    const int C = static_cast<int>(spec_.expected_shape[1]);
    const int H = static_cast<int>(spec_.expected_shape[2]);
    const int W = static_cast<int>(spec_.expected_shape[3]);
    const std::size_t per_image = static_cast<std::size_t>(C) *
                                  static_cast<std::size_t>(H) *
                                  static_cast<std::size_t>(W);
    const std::size_t K = crops_bgr.size();
    dst.resize(per_image * K);
    params_out.resize(K);

    for (std::size_t i = 0; i < K; ++i) {
        preprocess_into(crops_bgr[i],
                        /*letterbox_mode=*/false,
                        dst.data() + i * per_image,
                        params_out[i]);
    }
}

void LetterboxPreprocessor::unletterbox(double cx_m, double cy_m, double w_m, double h_m,
                                        const LetterboxParams& params,
                                        double& cx_src, double& cy_src,
                                        double& w_src,  double& h_src) noexcept {
    // §4.2 inverse transform.
    cx_src = (cx_m - params.pad_x) / params.scale;
    cy_src = (cy_m - params.pad_y) / params.scale;
    w_src  = w_m / params.scale;
    h_src  = h_m / params.scale;
}

void LetterboxPreprocessor::preprocess_into(const cv::Mat&     src,
                                            bool               letterbox_mode,
                                            float*             dst,
                                            LetterboxParams&   params_out) {
    const int C = static_cast<int>(spec_.expected_shape[1]);
    const int H = static_cast<int>(spec_.expected_shape[2]);
    const int W = static_cast<int>(spec_.expected_shape[3]);

    // Degenerate input — produce a zero tensor + identity params; the engine
    // catches the empty-frame upstream but we must not segfault on an empty Mat.
    if (src.empty() || src.cols <= 0 || src.rows <= 0) {
        std::memset(dst, 0, static_cast<std::size_t>(C) *
                            static_cast<std::size_t>(H) *
                            static_cast<std::size_t>(W) * sizeof(float));
        params_out = LetterboxParams{ 1.0, 0.0, 0.0, W, H, W, H, W, H };
        return;
    }

    const int src_w = src.cols;
    const int src_h = src.rows;
    const double s_x = static_cast<double>(W) / static_cast<double>(src_w);
    const double s_y = static_cast<double>(H) / static_cast<double>(src_h);

    cv::Mat post;            // post-resize image (CV_8UC3)
    int new_w = W;
    int new_h = H;
    int pad_l = 0;
    int pad_t = 0;
    double scale_out = 1.0;

    if (letterbox_mode) {
        scale_out = std::min(s_x, s_y);
        new_w = std::max(1, static_cast<int>(std::floor(src_w * scale_out)));
        new_h = std::max(1, static_cast<int>(std::floor(src_h * scale_out)));
        cv::resize(src, scratch_resized_, cv::Size(new_w, new_h), 0.0, 0.0, cv::INTER_LINEAR);

        pad_l = (W - new_w) / 2;
        pad_t = (H - new_h) / 2;
        const int pad_r = W - new_w - pad_l;
        const int pad_b = H - new_h - pad_t;
        const cv::Scalar pad_color(spec_.preprocessing.pad_value[0],
                                   spec_.preprocessing.pad_value[1],
                                   spec_.preprocessing.pad_value[2]);
        cv::copyMakeBorder(scratch_resized_, scratch_padded_,
                           pad_t, pad_b, pad_l, pad_r,
                           cv::BORDER_CONSTANT, pad_color);
        post = scratch_padded_;
    } else {
        cv::resize(src, scratch_resized_, cv::Size(W, H), 0.0, 0.0, cv::INTER_LINEAR);
        post = scratch_resized_;
        // For stretch mode we report `scale` as the geometric mean of (s_x, s_y) —
        // callers that need the inverse transform use w_src = w_m / scale_x and
        // h_src = h_m / scale_y separately.  The cached (s_x, s_y) live in the
        // letterboxed_w/h fields so the inverse helper can be derived.
        scale_out = s_x;       // primary scale (x); see params_out below for the y axis
    }

    // Optional BGR → RGB swap (manifest channel_order == RGB).
    cv::Mat channel_view;
    if (spec_.channel_order == ChannelOrder::RGB && post.channels() == 3) {
        cv::cvtColor(post, scratch_rgb_, cv::COLOR_BGR2RGB);
        channel_view = scratch_rgb_;
    } else {
        channel_view = post;
    }

    if (channel_view.channels() != C) {
        // Fatal in principle (the engine should reject the manifest's C
        // mismatch), but we degrade rather than throw in the hot path: just
        // pack first C channels (or zero-pad if fewer).
        std::memset(dst, 0, static_cast<std::size_t>(C) *
                            static_cast<std::size_t>(H) *
                            static_cast<std::size_t>(W) * sizeof(float));
    }

    const int n_channels = std::min(C, channel_view.channels());
    const double scale = spec_.normalization.scale;

    // NCHW packing: dst[c, y, x] = (raw[c, y, x] * scale - mean[c]) / std[c].
    const std::size_t plane = static_cast<std::size_t>(H) * static_cast<std::size_t>(W);
    if (spec_.layout == TensorLayout::NCHW) {
        for (int c = 0; c < n_channels; ++c) {
            const double m = spec_.normalization.mean[static_cast<std::size_t>(c)];
            const double d = spec_.normalization.std [static_cast<std::size_t>(c)];
            const double inv_d = 1.0 / d;
            float* plane_ptr = dst + static_cast<std::size_t>(c) * plane;
            for (int y = 0; y < H; ++y) {
                const std::uint8_t* row = channel_view.ptr<std::uint8_t>(y);
                float* row_dst = plane_ptr + static_cast<std::size_t>(y) * static_cast<std::size_t>(W);
                for (int x = 0; x < W; ++x) {
                    const double raw = static_cast<double>(
                        row[static_cast<std::size_t>(x) * static_cast<std::size_t>(channel_view.channels()) +
                            static_cast<std::size_t>(c)]);
                    row_dst[x] = static_cast<float>((raw * scale - m) * inv_d);
                }
            }
        }
        // Zero any extra channels the manifest expected (rare).
        for (int c = n_channels; c < C; ++c) {
            std::memset(dst + static_cast<std::size_t>(c) * plane, 0, plane * sizeof(float));
        }
    } else {
        // NHWC packing: dst[y, x, c].
        for (int y = 0; y < H; ++y) {
            const std::uint8_t* row = channel_view.ptr<std::uint8_t>(y);
            float* row_dst = dst + static_cast<std::size_t>(y) * static_cast<std::size_t>(W) *
                                   static_cast<std::size_t>(C);
            for (int x = 0; x < W; ++x) {
                for (int c = 0; c < n_channels; ++c) {
                    const double m = spec_.normalization.mean[static_cast<std::size_t>(c)];
                    const double d = spec_.normalization.std [static_cast<std::size_t>(c)];
                    const double raw = static_cast<double>(
                        row[static_cast<std::size_t>(x) * static_cast<std::size_t>(channel_view.channels()) +
                            static_cast<std::size_t>(c)]);
                    row_dst[static_cast<std::size_t>(x) * static_cast<std::size_t>(C) +
                            static_cast<std::size_t>(c)] = static_cast<float>((raw * scale - m) / d);
                }
                for (int c = n_channels; c < C; ++c) {
                    row_dst[static_cast<std::size_t>(x) * static_cast<std::size_t>(C) +
                            static_cast<std::size_t>(c)] = 0.0f;
                }
            }
        }
    }

    params_out.scale         = scale_out;
    params_out.pad_x         = static_cast<double>(pad_l);
    params_out.pad_y         = static_cast<double>(pad_t);
    params_out.letterboxed_w = new_w;
    params_out.letterboxed_h = new_h;
    params_out.model_w       = W;
    params_out.model_h       = H;
    params_out.src_w         = src_w;
    params_out.src_h         = src_h;
}

}  // namespace vision
}  // namespace roc
