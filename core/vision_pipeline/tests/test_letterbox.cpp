// =============================================================================
// test_letterbox.cpp — §4.2 forward + inverse letterbox math + §5.1 stretch.
// =============================================================================

#include "roc/vision/letterbox.hpp"
#include "roc/vision/manifest.hpp"

#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <cstdint>

using namespace roc::vision;
using Catch::Matchers::WithinAbs;

namespace {

InputSpec make_detection_spec() {
    InputSpec s;
    s.layout = TensorLayout::NCHW;
    s.channel_order = ChannelOrder::RGB;
    s.expected_shape = { 1, 3, 640, 640 };
    s.dtype = TensorDType::Float32;
    s.normalization.scale = 1.0 / 255.0;
    s.normalization.mean = { 0.0, 0.0, 0.0 };
    s.normalization.std  = { 1.0, 1.0, 1.0 };
    s.preprocessing.letterbox = true;
    s.preprocessing.pad_value = { 114, 114, 114 };
    s.preprocessing.pad_to    = PadTo::ModelInput;
    return s;
}

InputSpec make_reid_spec() {
    InputSpec s;
    s.layout = TensorLayout::NCHW;
    s.channel_order = ChannelOrder::RGB;
    s.expected_shape = { 1, 3, 256, 128 };
    s.dtype = TensorDType::Float32;
    s.normalization.scale = 1.0 / 255.0;
    s.normalization.mean = { 0.485, 0.456, 0.406 };
    s.normalization.std  = { 0.229, 0.224, 0.225 };
    s.preprocessing.letterbox = false;
    return s;
}

cv::Mat make_synthetic_bgr(int w, int h, std::uint8_t v = 200) {
    return cv::Mat(h, w, CV_8UC3, cv::Scalar(v, v, v));
}

}  // namespace

TEST_CASE("Letterbox: detection 1920x1080 -> 640x640 letterbox math", "[letterbox]") {
    LetterboxPreprocessor pp(make_detection_spec());
    cv::Mat src = make_synthetic_bgr(1920, 1080);

    std::vector<float> dst;
    LetterboxParams params;
    pp.preprocess(src, dst, params);

    CHECK(dst.size() == 3u * 640u * 640u);
    CHECK(params.model_w == 640);
    CHECK(params.model_h == 640);
    CHECK(params.src_w == 1920);
    CHECK(params.src_h == 1080);

    // scale = min(640/1920, 640/1080) = 640/1920 = 0.3333...
    const double expected_scale = 640.0 / 1920.0;
    CHECK_THAT(params.scale, WithinAbs(expected_scale, 1e-9));
    CHECK(params.letterboxed_w == 640);
    CHECK(params.letterboxed_h == static_cast<int>(std::floor(1080.0 * expected_scale)));

    // pad_x = 0 (width is the limiting axis), pad_y = (640 - 360) / 2 = 140
    CHECK(params.pad_x == 0.0);
    CHECK_THAT(params.pad_y, WithinAbs(140.0, 1e-9));
}

TEST_CASE("Letterbox: forward+inverse round-trip preserves bbox to <1 px", "[letterbox]") {
    LetterboxPreprocessor pp(make_detection_spec());
    cv::Mat src = make_synthetic_bgr(1920, 1080);

    std::vector<float> dst;
    LetterboxParams params;
    pp.preprocess(src, dst, params);

    // A synthetic detection centred at (960, 540) in the source image with a
    // 200x200 bbox.  Map forward to letterboxed coordinates, then inverse-
    // letterbox; the round-trip must agree with the original to <1 px.
    const double cx_src = 960.0;
    const double cy_src = 540.0;
    const double w_src  = 200.0;
    const double h_src  = 200.0;

    const double cx_m = cx_src * params.scale + params.pad_x;
    const double cy_m = cy_src * params.scale + params.pad_y;
    const double w_m  = w_src  * params.scale;
    const double h_m  = h_src  * params.scale;

    double cx_r, cy_r, w_r, h_r;
    LetterboxPreprocessor::unletterbox(cx_m, cy_m, w_m, h_m, params,
                                       cx_r, cy_r, w_r, h_r);

    CHECK_THAT(cx_r, WithinAbs(cx_src, 1.0));
    CHECK_THAT(cy_r, WithinAbs(cy_src, 1.0));
    CHECK_THAT(w_r,  WithinAbs(w_src,  1.0));
    CHECK_THAT(h_r,  WithinAbs(h_src,  1.0));
}

TEST_CASE("Letterbox: padding gray value lands in the padded rows", "[letterbox]") {
    LetterboxPreprocessor pp(make_detection_spec());
    cv::Mat src = make_synthetic_bgr(1920, 1080, /*v=*/255);

    std::vector<float> dst;
    LetterboxParams params;
    pp.preprocess(src, dst, params);

    // The padded rows (top 140 rows under the default math) should contain
    // the gray normalised value 114 / 255 = 0.447... on every channel.
    const double expected = 114.0 / 255.0;
    const std::size_t plane = 640u * 640u;
    const std::size_t row   = 640u;
    // Row 0, channel R (NCHW: dst[0 * plane + 0 * row + 0])
    for (int c = 0; c < 3; ++c) {
        for (int x = 0; x < 5; ++x) {
            const std::size_t idx = static_cast<std::size_t>(c) * plane + 0u * row + static_cast<std::size_t>(x);
            CHECK_THAT(static_cast<double>(dst[idx]), WithinAbs(expected, 1e-3));
        }
    }
}

TEST_CASE("Letterbox: stretch mode for ReID — exact resize, no padding", "[letterbox]") {
    LetterboxPreprocessor pp(make_reid_spec());
    cv::Mat src = make_synthetic_bgr(320, 480, /*v=*/128);

    std::vector<float> dst;
    LetterboxParams params;
    pp.preprocess(src, dst, params);

    CHECK(dst.size() == 3u * 256u * 128u);
    CHECK(params.pad_x == 0.0);
    CHECK(params.pad_y == 0.0);
    CHECK(params.model_w == 128);
    CHECK(params.model_h == 256);

    // Stretch resize: every pixel of the resized image must be the source
    // colour normalised under ImageNet stats.  Channel ordering after the
    // optional BGR→RGB swap matches manifest channel_order.  The original
    // image is uniform gray (128,128,128) BGR; after BGR→RGB the channels
    // are still uniform 128.  Normalised by ImageNet:
    //   r = (128/255 - 0.485) / 0.229
    //   g = (128/255 - 0.456) / 0.224
    //   b = (128/255 - 0.406) / 0.225
    const double base = 128.0 / 255.0;
    const double exp_r = (base - 0.485) / 0.229;
    const double exp_g = (base - 0.456) / 0.224;
    const double exp_b = (base - 0.406) / 0.225;
    const std::size_t plane = 256u * 128u;
    CHECK_THAT(static_cast<double>(dst[0 * plane + 100]),                 WithinAbs(exp_r, 1e-4));
    CHECK_THAT(static_cast<double>(dst[1 * plane + 100]),                 WithinAbs(exp_g, 1e-4));
    CHECK_THAT(static_cast<double>(dst[2 * plane + 100]),                 WithinAbs(exp_b, 1e-4));
}

TEST_CASE("Letterbox: batched ReID preprocessing produces packed [K,C,H,W]", "[letterbox]") {
    LetterboxPreprocessor pp(make_reid_spec());
    std::vector<cv::Mat> crops;
    for (int i = 0; i < 4; ++i) crops.push_back(make_synthetic_bgr(100 + 10 * i, 200 + 20 * i,
                                                                   static_cast<std::uint8_t>(50 + i * 10)));

    std::vector<float> dst;
    std::vector<LetterboxParams> params;
    pp.preprocess_batch(Span<const cv::Mat>(crops.data(), crops.size()), dst, params);

    CHECK(dst.size() == 4u * 3u * 256u * 128u);
    CHECK(params.size() == 4u);
    for (const auto& p : params) {
        CHECK(p.model_w == 128);
        CHECK(p.model_h == 256);
        CHECK(p.pad_x == 0.0);
        CHECK(p.pad_y == 0.0);
    }
}

TEST_CASE("Letterbox: degenerate empty input yields identity params + zero tensor", "[letterbox]") {
    LetterboxPreprocessor pp(make_detection_spec());
    cv::Mat empty;
    std::vector<float> dst;
    LetterboxParams params;
    pp.preprocess(empty, dst, params);
    CHECK(dst.size() == 3u * 640u * 640u);
    bool all_zero = true;
    for (float v : dst) if (v != 0.0f) { all_zero = false; break; }
    CHECK(all_zero);
    CHECK(params.scale == 1.0);
}
