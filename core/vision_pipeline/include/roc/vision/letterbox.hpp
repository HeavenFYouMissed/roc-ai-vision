#pragma once

// =============================================================================
// roc::vision::letterbox
//
// Per-manifest preprocessor implementing two preprocessing modes:
//
//   * Letterbox mode (detection — §4.2): preserve aspect ratio with
//     (114, 114, 114) padding to model_w × model_h.  Caches (s, pad_x, pad_y)
//     for the inverse transform used by the YOLO26 postprocessor (§4.1).
//
//   * Stretch mode (ReID — §5.1): cv::resize stretches the crop to model
//     dimensions, ignoring aspect ratio (OSNet is trained this way).  No
//     padding; inverse transform is a pure scale.
//
// Both modes apply the manifest's BGR→RGB conversion (if requested), float
// conversion, scale, and per-channel mean/std normalisation.  Output is
// always packed in `NCHW float32` for the engine — `NHWC float32` is supplied
// when the manifest declares NHWC layout (rare but allowed by §3.3).
//
// Memory: this class owns two reusable scratch buffers (the post-resize cv::Mat
// and the float NCHW packed buffer) so steady-state inference makes zero
// per-frame heap allocations.
// =============================================================================

#include "roc/vision/manifest.hpp"
#include "roc/vision/types.hpp"

#include <opencv2/core.hpp>

#include <cstddef>
#include <vector>

namespace roc {
namespace vision {

class LetterboxPreprocessor {
public:
    explicit LetterboxPreprocessor(const InputSpec& spec);

    // Preprocess a single capture frame.  Writes the packed NCHW (or NHWC)
    // float32 tensor to `dst` (1 × C × H × W contiguous floats); writes the
    // inverse-transform parameters to `params_out`.  `src` may be BGR uint8
    // (the standard OpenCV format) — channel conversion is driven by the spec.
    void preprocess(const cv::Mat&        src,
                    std::vector<float>&   dst,
                    LetterboxParams&      params_out);

    // Batched preprocessing for ReID: K crops → packed [K, C, H, W] float
    // tensor.  Used by §5.3 batched inference.  All crops are stretch-resized
    // to (model_w, model_h); no letterbox padding regardless of the spec
    // (ReID extractor manifests set `preprocessing.letterbox = false`).
    // `params_out` is sized K, one entry per crop (scale-only, no padding).
    void preprocess_batch(Span<const cv::Mat>           crops_bgr,
                          std::vector<float>&           dst,
                          std::vector<LetterboxParams>& params_out);

    // §4.2 inverse letterbox — model-space pixel coords back to source frame.
    static void unletterbox(double cx_m, double cy_m, double w_m, double h_m,
                            const LetterboxParams&     params,
                            double& cx_src, double& cy_src,
                            double& w_src,  double& h_src) noexcept;

    // Read-only views of the configured input spec (test introspection).
    const InputSpec& spec() const noexcept { return spec_; }
    int model_w() const noexcept { return static_cast<int>(spec_.expected_shape[3]); }
    int model_h() const noexcept { return static_cast<int>(spec_.expected_shape[2]); }

private:
    void preprocess_into(const cv::Mat&    src,
                         bool              letterbox_mode,
                         float*            dst,
                         LetterboxParams&  params_out);

    InputSpec   spec_{};
    cv::Mat     scratch_resized_;     // post-resize, pre-pad
    cv::Mat     scratch_padded_;      // padded model-input-sized image
    cv::Mat     scratch_rgb_;         // optional BGR→RGB conversion
};

}  // namespace vision
}  // namespace roc
