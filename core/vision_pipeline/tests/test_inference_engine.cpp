// =============================================================================
// test_inference_engine.cpp — end-to-end smoke test.
//
// When the YOLO26n COCO model + manifest are present under
// ROC_VISION_TEST_MODELS_DIR (downloaded at configure time by
// scripts/fetch_yolo26n.py), this test loads the engine, runs a synthetic
// 640×640 noise image through it, and verifies no exceptions escape.
//
// When the model is absent (e.g. CI without network), the test skips with
// an explanatory SUCCEED("...skipped...") message — see the worker spec's
// "gracefully skip if model file is absent" clause.
// =============================================================================

#include "roc/vision/inference_engine.hpp"
#include "roc/vision/postprocessor_registry.hpp"
#include "roc/vision/postprocess/yolo26_detect.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <filesystem>
#include <random>
#include <string>

using namespace roc::vision;

#ifndef ROC_VISION_TEST_MODELS_DIR
#  define ROC_VISION_TEST_MODELS_DIR "."
#endif

namespace {

std::filesystem::path models_dir() {
    return std::filesystem::path(ROC_VISION_TEST_MODELS_DIR);
}

cv::Mat noise_frame(int w, int h) {
    cv::Mat m(h, w, CV_8UC3);
    std::mt19937 g(0xC0FFEE);
    std::uniform_int_distribution<int> dist(0, 255);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            m.at<cv::Vec3b>(y, x) = cv::Vec3b(
                static_cast<std::uint8_t>(dist(g)),
                static_cast<std::uint8_t>(dist(g)),
                static_cast<std::uint8_t>(dist(g)));
        }
    }
    return m;
}

}  // namespace

TEST_CASE("PostprocessorRegistry: bundled heads available", "[engine]") {
    PostprocessorRegistry reg;
    CHECK(reg.contains("yolo26_detect"));
    CHECK(reg.contains("osnet_reid"));
    CHECK(reg.find("yolo26_detect") != nullptr);
    CHECK(reg.find("osnet_reid")    != nullptr);
    CHECK(reg.find("bogus") == nullptr);
    CHECK(reg.size() >= 2u);
}

TEST_CASE("PostprocessorRegistry: duplicate registration rejected", "[engine]") {
    PostprocessorRegistry reg;
    auto p = std::unique_ptr<Postprocessor>(nullptr);
    // Null pointers cannot be registered.
    CHECK_FALSE(reg.register_postprocessor("yolo26_detect", std::move(p)));
    // Duplicate key.
    auto y = std::unique_ptr<Postprocessor>(
        new Yolo26DetectPostprocessor());
    CHECK_FALSE(reg.register_postprocessor("yolo26_detect", std::move(y)));
}

TEST_CASE("InferenceEngine: detection end-to-end on YOLO26n COCO", "[engine][integration]") {
    const auto onnx_path     = models_dir() / "yolo26n-coco.onnx";
    const auto manifest_path = models_dir() / "yolo26n-coco.yaml";
    if (!std::filesystem::exists(onnx_path) || !std::filesystem::exists(manifest_path)) {
        SUCCEED("skipped: yolo26n-coco.onnx / .yaml not present at "
                + onnx_path.string() + " (fetch_yolo26n.py may have failed; tests skip gracefully)");
        return;
    }

    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "roc_vision_engine_test");
    PostprocessorRegistry reg;
    InferenceEngine engine(env, ModelPath(onnx_path), manifest_path, reg);

    cv::Mat frame = noise_frame(640, 640);
    auto dets = engine.run(frame);

    // We make no claim about how many detections appear on pure noise — only
    // that the engine ran, produced a valid vector, and emitted bbox values
    // that fall on the source frame (i.e. inverse-letterbox didn't blow up).
    for (const auto& d : dets) {
        CHECK(d.bbox_center.u >= -50.0);
        CHECK(d.bbox_center.u <= 700.0);
        CHECK(d.bbox_center.v >= -50.0);
        CHECK(d.bbox_center.v <= 700.0);
        CHECK(d.confidence >= 0.0);
        CHECK(d.confidence <= 1.0);
        CHECK(d.appearance.norm() == 0.0f);   // ReID populates later
    }
    INFO("YOLO26n produced " << dets.size() << " detections on noise");
    SUCCEED("engine.run() returned without throwing on a synthetic 640x640 noise frame");
}

TEST_CASE("InferenceEngine: manifest-only path rejects missing model", "[engine]") {
    const auto onnx_path     = models_dir() / "doesnt_exist.onnx";
    const auto manifest_path = models_dir() / "yolo26n-coco.yaml";
    if (!std::filesystem::exists(manifest_path)) {
        SUCCEED("skipped: reference manifest not generated yet");
        return;
    }
    Ort::Env env(ORT_LOGGING_LEVEL_ERROR, "roc_vision_engine_neg_test");
    PostprocessorRegistry reg;
    CHECK_THROWS([&]() {
        InferenceEngine engine(env, ModelPath(onnx_path), manifest_path, reg);
        (void)engine;
    }());
}

TEST_CASE("InferenceEngine::run on empty cv::Mat returns empty + no throw", "[engine]") {
    const auto onnx_path     = models_dir() / "yolo26n-coco.onnx";
    const auto manifest_path = models_dir() / "yolo26n-coco.yaml";
    if (!std::filesystem::exists(onnx_path) || !std::filesystem::exists(manifest_path)) {
        SUCCEED("skipped: yolo26n-coco.onnx not present");
        return;
    }
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "roc_vision_engine_empty_test");
    PostprocessorRegistry reg;
    InferenceEngine engine(env, ModelPath(onnx_path), manifest_path, reg);
    cv::Mat empty;
    auto dets = engine.run(empty);
    CHECK(dets.empty());
}
