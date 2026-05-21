// =============================================================================
// test_yolov8_decode.cpp — coverage for `Yolov8DetectPostprocessor`.
//
// The decoder differs from the YOLO26 head along three axes; we exercise all
// three:
//
//   1. Scores are already sigmoid-activated (no inner sigmoid call in the
//      postprocessor).  We feed probabilities directly and assert the
//      returned confidence matches the input value (no double-sigmoid).
//
//   2. Per-class NMS removes overlapping anchors.  We synthesise three
//      same-class anchors at IoU > 0.45 + one at IoU < 0.45, and assert
//      exactly two boxes survive (the highest-score of the cluster + the
//      isolated one).
//
//   3. Both single-class [1, 5, N] and 80-class [1, 84, N] layouts decode
//      correctly through the same postprocessor implementation.
// =============================================================================

#include "roc/vision/postprocess/yolov8_detect.hpp"
#include "roc/vision/manifest.hpp"
#include "roc/vision/letterbox.hpp"

#include <onnxruntime_cxx_api.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <array>
#include <vector>

using namespace roc::vision;
using Catch::Matchers::WithinAbs;

namespace {

ManifestSpec make_detection_manifest(int num_classes, double iou_thresh = 0.45) {
    ManifestSpec m;
    m.schema_version = 1;
    m.name = "test_yolov8";
    m.purpose = "detection";
    m.output.head = OutputHead::Yolov8Detect;
    m.head_string = "yolov8_detect";
    m.output.format = OutputFormat::CxCyWhSigmoid;
    m.output.num_classes = num_classes;
    m.output.thresholds.score = 0.25;
    m.output.thresholds.iou   = iou_thresh;
    m.input.expected_shape = { 1, 3, 640, 640 };
    for (int i = 0; i < num_classes; ++i) {
        ClassMapping c;
        c.id = i;
        c.name = "class_" + std::to_string(i);
        // Class 0 → Person; everything else → Unknown (mirrors the COCO
        // mapping the user's collection manifests use).
        c.roc_label = (i == 0) ? Label::Person : Label::Unknown;
        m.classes.push_back(c);
    }
    return m;
}

PostprocessContext make_identity_ctx(const ManifestSpec& m) {
    PostprocessContext ctx;
    ctx.letterbox.scale = 1.0;
    ctx.letterbox.pad_x = 0.0;
    ctx.letterbox.pad_y = 0.0;
    ctx.letterbox.model_w = 640;
    ctx.letterbox.model_h = 640;
    ctx.model_input_hw   = { 640, 640 };
    ctx.capture_input_hw = { 640, 640 };
    ctx.manifest = &m;
    return ctx;
}

}  // namespace

TEST_CASE("Yolov8 decode: 5xN single-class — scores not re-sigmoided", "[yolov8]") {
    constexpr int F = 5;
    constexpr int N = 64;
    std::vector<float> tensor(static_cast<std::size_t>(F * N), 0.0f);
    auto idx = [](int ch, int n) {
        return static_cast<std::size_t>(ch) * static_cast<std::size_t>(N) +
               static_cast<std::size_t>(n);
    };

    // Two well-separated peaks far below mutual IoU = 0.45.  Centres at
    // (100, 100) and (500, 500) with 30×30 boxes → IoU = 0.
    tensor[idx(0, 5)]  = 100.0f;
    tensor[idx(1, 5)]  = 100.0f;
    tensor[idx(2, 5)]  = 30.0f;
    tensor[idx(3, 5)]  = 30.0f;
    tensor[idx(4, 5)]  = 0.90f;   // already-sigmoid'd score

    tensor[idx(0, 40)] = 500.0f;
    tensor[idx(1, 40)] = 500.0f;
    tensor[idx(2, 40)] = 30.0f;
    tensor[idx(3, 40)] = 30.0f;
    tensor[idx(4, 40)] = 0.75f;

    // A below-threshold anchor that must be culled.
    tensor[idx(0, 12)] = 250.0f;
    tensor[idx(1, 12)] = 250.0f;
    tensor[idx(2, 12)] = 30.0f;
    tensor[idx(3, 12)] = 30.0f;
    tensor[idx(4, 12)] = 0.10f;

    Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<int64_t> shape{1, F, N};
    Ort::Value v = Ort::Value::CreateTensor<float>(mi, tensor.data(), tensor.size(),
                                                   shape.data(), shape.size());
    std::vector<Ort::Value> outs;
    outs.push_back(std::move(v));

    ManifestSpec m = make_detection_manifest(1);
    PostprocessContext ctx = make_identity_ctx(m);

    Yolov8DetectPostprocessor pp;
    auto dets = pp.decode_detections(outs, ctx);

    REQUIRE(dets.size() == 2u);
    std::sort(dets.begin(), dets.end(),
              [](const Detection& a, const Detection& b) {
                  return a.bbox_center.u < b.bbox_center.u;
              });
    CHECK_THAT(dets[0].bbox_center.u, WithinAbs(100.0, 1e-3));
    CHECK_THAT(dets[0].confidence,   WithinAbs(0.90, 1e-3));
    CHECK(dets[0].label == Label::Person);

    CHECK_THAT(dets[1].bbox_center.u, WithinAbs(500.0, 1e-3));
    CHECK_THAT(dets[1].confidence,   WithinAbs(0.75, 1e-3));
}

TEST_CASE("Yolov8 decode: per-class NMS suppresses overlapping anchors", "[yolov8]") {
    constexpr int C = 1;
    constexpr int F = 4 + C;
    constexpr int N = 16;
    std::vector<float> tensor(static_cast<std::size_t>(F * N), 0.0f);
    auto idx = [](int ch, int n) {
        return static_cast<std::size_t>(ch) * static_cast<std::size_t>(N) +
               static_cast<std::size_t>(n);
    };

    // Cluster of three same-class, highly-overlapping anchors centred near
    // (200, 200).  Sizes 50×50 with small jitter so IoU > 0.6 between any
    // pair → NMS keeps only the highest-score one (anchor 1).
    tensor[idx(0, 0)] = 198.0f;  tensor[idx(1, 0)] = 202.0f;
    tensor[idx(2, 0)] = 50.0f;   tensor[idx(3, 0)] = 50.0f;
    tensor[idx(4, 0)] = 0.70f;
    tensor[idx(0, 1)] = 200.0f;  tensor[idx(1, 1)] = 200.0f;
    tensor[idx(2, 1)] = 50.0f;   tensor[idx(3, 1)] = 50.0f;
    tensor[idx(4, 1)] = 0.92f;   // winner
    tensor[idx(0, 2)] = 203.0f;  tensor[idx(1, 2)] = 199.0f;
    tensor[idx(2, 2)] = 50.0f;   tensor[idx(3, 2)] = 50.0f;
    tensor[idx(4, 2)] = 0.85f;

    // Isolated anchor far enough that IoU with the cluster is 0.
    tensor[idx(0, 8)] = 500.0f;  tensor[idx(1, 8)] = 500.0f;
    tensor[idx(2, 8)] = 40.0f;   tensor[idx(3, 8)] = 40.0f;
    tensor[idx(4, 8)] = 0.50f;

    Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<int64_t> shape{1, F, N};
    Ort::Value v = Ort::Value::CreateTensor<float>(mi, tensor.data(), tensor.size(),
                                                   shape.data(), shape.size());
    std::vector<Ort::Value> outs;
    outs.push_back(std::move(v));

    ManifestSpec m = make_detection_manifest(1, /*iou_thresh=*/0.45);
    PostprocessContext ctx = make_identity_ctx(m);

    Yolov8DetectPostprocessor pp;
    auto dets = pp.decode_detections(outs, ctx);

    REQUIRE(dets.size() == 2u);
    std::sort(dets.begin(), dets.end(),
              [](const Detection& a, const Detection& b) {
                  return a.confidence > b.confidence;
              });
    CHECK_THAT(dets[0].bbox_center.u, WithinAbs(200.0, 1e-3));
    CHECK_THAT(dets[0].confidence,   WithinAbs(0.92, 1e-3));
    CHECK_THAT(dets[1].bbox_center.u, WithinAbs(500.0, 1e-3));
    CHECK_THAT(dets[1].confidence,   WithinAbs(0.50, 1e-3));
}

TEST_CASE("Yolov8 decode: 84xN COCO — argmax picks winning class per anchor", "[yolov8]") {
    constexpr int C = 80;
    constexpr int F = 4 + C;
    constexpr int N = 32;
    std::vector<float> tensor(static_cast<std::size_t>(F * N), 0.0f);
    auto idx = [](int ch, int n) {
        return static_cast<std::size_t>(ch) * static_cast<std::size_t>(N) +
               static_cast<std::size_t>(n);
    };

    // Anchor 4: COCO class 0 (person) dominant at 0.88, all others < 0.10.
    tensor[idx(0, 4)] = 320.0f;  tensor[idx(1, 4)] = 240.0f;
    tensor[idx(2, 4)] = 60.0f;   tensor[idx(3, 4)] = 160.0f;
    tensor[idx(4 + 0, 4)] = 0.88f;
    for (int c = 1; c < C; ++c) tensor[idx(4 + c, 4)] = 0.05f;

    // Anchor 17: COCO class 24 (backpack) dominant at 0.71.
    tensor[idx(0, 17)] = 100.0f; tensor[idx(1, 17)] = 100.0f;
    tensor[idx(2, 17)] = 40.0f;  tensor[idx(3, 17)] = 40.0f;
    for (int c = 0; c < C; ++c) tensor[idx(4 + c, 17)] = 0.02f;
    tensor[idx(4 + 24, 17)] = 0.71f;

    Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<int64_t> shape{1, F, N};
    Ort::Value v = Ort::Value::CreateTensor<float>(mi, tensor.data(), tensor.size(),
                                                   shape.data(), shape.size());
    std::vector<Ort::Value> outs;
    outs.push_back(std::move(v));

    ManifestSpec m = make_detection_manifest(C);
    // Map class 24 (backpack) → AccessoryBackpack, the rest stay Unknown
    // except class 0 (Person), which the helper already mapped above.
    for (auto& cm : m.classes) {
        if (cm.id == 24) cm.roc_label = Label::AccessoryBackpack;
    }
    PostprocessContext ctx = make_identity_ctx(m);

    Yolov8DetectPostprocessor pp;
    auto dets = pp.decode_detections(outs, ctx);

    REQUIRE(dets.size() == 2u);
    std::sort(dets.begin(), dets.end(),
              [](const Detection& a, const Detection& b) {
                  return a.bbox_center.u < b.bbox_center.u;
              });
    CHECK_THAT(dets[0].bbox_center.u, WithinAbs(100.0, 1e-3));
    CHECK(dets[0].label == Label::AccessoryBackpack);
    CHECK_THAT(dets[0].confidence,   WithinAbs(0.71, 1e-3));

    CHECK_THAT(dets[1].bbox_center.u, WithinAbs(320.0, 1e-3));
    CHECK(dets[1].label == Label::Person);
    CHECK_THAT(dets[1].confidence,   WithinAbs(0.88, 1e-3));
}

TEST_CASE("Yolov8 decode: malformed tensor rank returns empty (hot-path noexcept)", "[yolov8]") {
    Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<float> data{1.0f, 2.0f, 3.0f};
    std::vector<int64_t> shape{3};
    Ort::Value v = Ort::Value::CreateTensor<float>(mi, data.data(), data.size(),
                                                   shape.data(), shape.size());
    std::vector<Ort::Value> outs;
    outs.push_back(std::move(v));

    ManifestSpec m = make_detection_manifest(80);
    PostprocessContext ctx;
    ctx.manifest = &m;

    Yolov8DetectPostprocessor pp;
    auto dets = pp.decode_detections(outs, ctx);
    CHECK(dets.empty());
}

TEST_CASE("Yolov8 decode: empty outputs vector yields empty Detection[]", "[yolov8]") {
    Yolov8DetectPostprocessor pp;
    ManifestSpec m = make_detection_manifest(1);
    PostprocessContext ctx;
    ctx.manifest = &m;
    auto dets = pp.decode_detections({}, ctx);
    CHECK(dets.empty());
}

TEST_CASE("Yolov8 decode: inverse letterbox applies after NMS", "[yolov8]") {
    constexpr int C = 1;
    constexpr int F = 4 + C;
    constexpr int N = 4;
    std::vector<float> tensor(static_cast<std::size_t>(F * N), 0.0f);
    auto idx = [](int ch, int n) {
        return static_cast<std::size_t>(ch) * static_cast<std::size_t>(N) +
               static_cast<std::size_t>(n);
    };
    tensor[idx(0, 0)] = 200.0f; tensor[idx(1, 0)] = 300.0f;
    tensor[idx(2, 0)] = 50.0f;  tensor[idx(3, 0)] = 50.0f;
    tensor[idx(4, 0)] = 0.90f;

    Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<int64_t> shape{1, F, N};
    Ort::Value v = Ort::Value::CreateTensor<float>(mi, tensor.data(), tensor.size(),
                                                   shape.data(), shape.size());
    std::vector<Ort::Value> outs;
    outs.push_back(std::move(v));

    ManifestSpec m = make_detection_manifest(1);
    PostprocessContext ctx;
    ctx.letterbox.scale = 0.5;
    ctx.letterbox.pad_x = 40.0;
    ctx.letterbox.pad_y = 60.0;
    ctx.letterbox.model_w = 640;
    ctx.letterbox.model_h = 640;
    ctx.model_input_hw   = { 640, 640 };
    ctx.capture_input_hw = { 1200, 1200 };
    ctx.manifest = &m;

    Yolov8DetectPostprocessor pp;
    auto dets = pp.decode_detections(outs, ctx);
    REQUIRE(dets.size() == 1u);
    // cx_src = (200 - 40) / 0.5 = 320; cy_src = (300 - 60) / 0.5 = 480
    CHECK_THAT(dets[0].bbox_center.u, WithinAbs(320.0, 1e-3));
    CHECK_THAT(dets[0].bbox_center.v, WithinAbs(480.0, 1e-3));
    CHECK_THAT(dets[0].bbox_w_px,    WithinAbs(100.0, 1e-3));
    CHECK_THAT(dets[0].bbox_h_px,    WithinAbs(100.0, 1e-3));
}
