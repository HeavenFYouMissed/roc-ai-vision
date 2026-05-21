#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "roc/vision/optical_flow_sampler.hpp"

#include <opencv2/imgproc.hpp>

#include <cmath>

using Catch::Matchers::WithinAbs;

static cv::Mat make_feature_rich_frame(int w, int h) {
    cv::Mat gray(h, w, CV_8UC1, cv::Scalar(40));
    for (int y = 0; y < h; y += 24) {
        for (int x = 0; x < w; x += 24) {
            cv::rectangle(gray, cv::Point(x + 4, y + 4), cv::Point(x + 12, y + 12),
                          cv::Scalar(220), cv::FILLED);
        }
    }
    cv::Mat bgr;
    cv::cvtColor(gray, bgr, cv::COLOR_GRAY2BGR);
    return bgr;
}

TEST_CASE("OpticalFlowSampler recovers known translation", "[optical_flow]") {
    roc::vision::OpticalFlowSampler sampler;
    const int w = 640;
    const int h = 480;
    const float tx = 12.0f;
    const float ty = -7.0f;

    cv::Mat frame0 = make_feature_rich_frame(w, h);
    (void)sampler.sample(frame0, 1, 1'000'000);

    cv::Mat frame1;
    const cv::Mat m = (cv::Mat_<double>(2, 3) << 1, 0, tx, 0, 1, ty);
    cv::warpAffine(frame0, frame1, m, frame0.size(), cv::INTER_LINEAR,
                   cv::BORDER_REFLECT101);

    std::optional<roc::vision::BgOdometrySample> out;
    for (int i = 0; i < 5; ++i) {
        out = sampler.sample(frame1, static_cast<std::uint32_t>(2 + i),
                             2'000'000 + i);
        if (out && out->valid_estimate) break;
    }

    REQUIRE(out.has_value());
    INFO("dx=" << out->dx_px << " dy=" << out->dy_px
               << " valid=" << out->valid_estimate
               << " features=" << out->feature_count);
    const float mag =
        std::sqrt(out->dx_px * out->dx_px + out->dy_px * out->dy_px);
    REQUIRE(mag > 4.0f);
    REQUIRE(out->feature_count >= 8);
}

TEST_CASE("OpticalFlowSampler empty frame returns nullopt path", "[optical_flow]") {
    roc::vision::OpticalFlowSampler sampler;
    cv::Mat empty;
    const auto out = sampler.sample(empty, 0, 0);
    REQUIRE_FALSE(out.has_value());
}
