// =============================================================================
// test_yolo26_decode.cpp — §4.1 NMS-free YOLO26 decoder coverage.
//
// We synthesize a [1, 4+C, N] output tensor with three hand-placed peaks
// above threshold and one peak below threshold, then verify the postprocessor
// returns exactly three Detection structs with the expected labels and
// inverse-letterbox coordinates.
// =============================================================================

#include "roc/vision/postprocess/yolo26_detect.hpp"
#include "roc/vision/manifest.hpp"
#include "roc/vision/letterbox.hpp"

#include <onnxruntime_cxx_api.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <array>
#include <cmath>
#include <vector>

using namespace roc::vision;
using Catch::Matchers::WithinAbs;

namespace {

float inv_sigmoid(float p) {
    p = std::min(0.999f, std::max(0.001f, p));
    return std::log(p / (1.0f - p));
}

ManifestSpec make_test_manifest(int num_classes) {
    ManifestSpec m;
    m.schema_version = 1;
    m.name = "test";
    m.purpose = "detection";
    m.output.head = OutputHead::Yolo26Detect;
    m.head_string = "yolo26_detect";
    m.output.format = OutputFormat::CxCyWhLogits;
    m.output.num_classes = num_classes;
    m.output.thresholds.score = 0.25;
    m.input.expected_shape = { 1, 3, 640, 640 };
    for (int i = 0; i < num_classes; ++i) {
        ClassMapping c;
        c.id = i;
        c.name = "class_" + std::to_string(i);
        c.roc_label = (i == 0) ? Label::Person
                     : (i == 1) ? Label::Head
                     : (i == 2) ? Label::UpperTorso
                     : Label::AccessoryBackpack;
        m.classes.push_back(c);
    }
    return m;
}

}  // namespace

TEST_CASE("Yolo26 decode: three hand-placed peaks above threshold", "[yolo26]") {
    constexpr int C = 6;
    constexpr int N = 100;
    constexpr int F = 4 + C;

    // Tensor layout: channel-major [1, F, N].  data[ch * N + n].
    std::vector<float> tensor(static_cast<std::size_t>(1 * F * N), 0.0f);
    auto idx = [](int ch, int n) -> std::size_t {
        return static_cast<std::size_t>(ch) * static_cast<std::size_t>(N) + static_cast<std::size_t>(n);
    };

    // Initialise all logits to a very negative value (post-sigmoid ≈ 0).
    for (int n = 0; n < N; ++n) {
        for (int c = 0; c < C; ++c) {
            tensor[idx(4 + c, n)] = -10.0f;
        }
    }

    // Peak 1: anchor 10, class 0 (Person), bbox center (100, 200), 30x50,
    // sigmoid score 0.95.
    tensor[idx(0, 10)] = 100.0f;
    tensor[idx(1, 10)] = 200.0f;
    tensor[idx(2, 10)] = 30.0f;
    tensor[idx(3, 10)] = 50.0f;
    tensor[idx(4 + 0, 10)] = inv_sigmoid(0.95f);

    // Peak 2: anchor 42, class 1 (Head), (320, 320), 20x20, score 0.80.
    tensor[idx(0, 42)] = 320.0f;
    tensor[idx(1, 42)] = 320.0f;
    tensor[idx(2, 42)] = 20.0f;
    tensor[idx(3, 42)] = 20.0f;
    tensor[idx(4 + 1, 42)] = inv_sigmoid(0.80f);

    // Peak 3: anchor 88, class 2 (UpperTorso), (500, 100), 60x80, score 0.55.
    tensor[idx(0, 88)] = 500.0f;
    tensor[idx(1, 88)] = 100.0f;
    tensor[idx(2, 88)] = 60.0f;
    tensor[idx(3, 88)] = 80.0f;
    tensor[idx(4 + 2, 88)] = inv_sigmoid(0.55f);

    // Below-threshold peak: anchor 7, class 0, score 0.10 — must be culled.
    tensor[idx(0, 7)] = 50.0f;
    tensor[idx(1, 7)] = 50.0f;
    tensor[idx(2, 7)] = 15.0f;
    tensor[idx(3, 7)] = 15.0f;
    tensor[idx(4 + 0, 7)] = inv_sigmoid(0.10f);

    // Wrap as Ort::Value (CPU tensor).
    Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<int64_t> shape{1, F, N};
    Ort::Value v = Ort::Value::CreateTensor<float>(mi, tensor.data(), tensor.size(),
                                                   shape.data(), shape.size());

    std::vector<Ort::Value> outputs;
    outputs.push_back(std::move(v));

    // Identity letterbox (no padding / scale) so model coords == src coords.
    PostprocessContext ctx;
    ctx.letterbox.scale         = 1.0;
    ctx.letterbox.pad_x         = 0.0;
    ctx.letterbox.pad_y         = 0.0;
    ctx.letterbox.model_w       = 640;
    ctx.letterbox.model_h       = 640;
    ctx.model_input_hw          = { 640, 640 };
    ctx.capture_input_hw        = { 640, 640 };
    ManifestSpec m = make_test_manifest(C);
    ctx.manifest = &m;

    Yolo26DetectPostprocessor pp;
    auto dets = pp.decode_detections(outputs, ctx);
    REQUIRE(dets.size() == 3u);

    // Sort by bbox_center.u for deterministic ordering.
    std::sort(dets.begin(), dets.end(), [](const Detection& a, const Detection& b) {
        return a.bbox_center.u < b.bbox_center.u;
    });

    CHECK_THAT(dets[0].bbox_center.u, WithinAbs(100.0, 1e-3));
    CHECK_THAT(dets[0].bbox_center.v, WithinAbs(200.0, 1e-3));
    CHECK_THAT(dets[0].bbox_w_px,    WithinAbs(30.0,  1e-3));
    CHECK_THAT(dets[0].bbox_h_px,    WithinAbs(50.0,  1e-3));
    CHECK(dets[0].label == Label::Person);
    CHECK_THAT(dets[0].confidence,   WithinAbs(0.95, 5e-3));

    CHECK_THAT(dets[1].bbox_center.u, WithinAbs(320.0, 1e-3));
    CHECK_THAT(dets[1].bbox_center.v, WithinAbs(320.0, 1e-3));
    CHECK(dets[1].label == Label::Head);
    CHECK_THAT(dets[1].confidence,   WithinAbs(0.80, 5e-3));

    CHECK_THAT(dets[2].bbox_center.u, WithinAbs(500.0, 1e-3));
    CHECK(dets[2].label == Label::UpperTorso);
    CHECK_THAT(dets[2].confidence,   WithinAbs(0.55, 5e-3));

    // All appearances must be zeroed (ReID populates later).
    for (const auto& d : dets) {
        CHECK(d.appearance.norm() == 0.0f);
    }
}

TEST_CASE("Yolo26 decode: empty outputs vector yields empty Detection[]", "[yolo26]") {
    Yolo26DetectPostprocessor pp;
    ManifestSpec m = make_test_manifest(6);
    PostprocessContext ctx;
    ctx.manifest = &m;
    auto dets = pp.decode_detections({}, ctx);
    CHECK(dets.empty());
}

TEST_CASE("Yolo26 decode: inverse letterbox applies when scale != 1", "[yolo26]") {
    constexpr int C = 2;
    constexpr int N = 1;
    constexpr int F = 4 + C;
    std::vector<float> tensor(static_cast<std::size_t>(F * N), 0.0f);
    auto idx = [](int ch, int n) { return static_cast<std::size_t>(ch) * static_cast<std::size_t>(N) +
                                          static_cast<std::size_t>(n); };
    tensor[idx(4 + 1, 0)] = -10.0f;
    tensor[idx(0, 0)] = 200.0f;      // cx_m
    tensor[idx(1, 0)] = 300.0f;      // cy_m
    tensor[idx(2, 0)] = 50.0f;       // w_m
    tensor[idx(3, 0)] = 50.0f;       // h_m
    tensor[idx(4 + 0, 0)] = inv_sigmoid(0.90f);

    Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<int64_t> shape{1, F, N};
    Ort::Value v = Ort::Value::CreateTensor<float>(mi, tensor.data(), tensor.size(),
                                                   shape.data(), shape.size());
    std::vector<Ort::Value> outs;
    outs.push_back(std::move(v));

    PostprocessContext ctx;
    ctx.letterbox.scale = 0.5;
    ctx.letterbox.pad_x = 40.0;
    ctx.letterbox.pad_y = 60.0;
    ctx.letterbox.model_w = 640;
    ctx.letterbox.model_h = 640;
    ctx.model_input_hw   = { 640, 640 };
    ctx.capture_input_hw = { 1200, 1200 };
    ManifestSpec m = make_test_manifest(C);
    ctx.manifest = &m;

    Yolo26DetectPostprocessor pp;
    auto dets = pp.decode_detections(outs, ctx);
    REQUIRE(dets.size() == 1u);
    // cx_src = (200 - 40) / 0.5 = 320, cy_src = (300 - 60) / 0.5 = 480
    CHECK_THAT(dets[0].bbox_center.u, WithinAbs(320.0, 1e-3));
    CHECK_THAT(dets[0].bbox_center.v, WithinAbs(480.0, 1e-3));
    CHECK_THAT(dets[0].bbox_w_px,    WithinAbs(100.0, 1e-3));
    CHECK_THAT(dets[0].bbox_h_px,    WithinAbs(100.0, 1e-3));
}

TEST_CASE("Yolo26 decode: malformed tensor rank returns empty (hot-path noexcept)", "[yolo26]") {
    Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<float> data{1.0f, 2.0f, 3.0f};
    std::vector<int64_t> shape{3};         // rank-1, not what yolo26 expects
    Ort::Value v = Ort::Value::CreateTensor<float>(mi, data.data(), data.size(),
                                                   shape.data(), shape.size());
    std::vector<Ort::Value> outs;
    outs.push_back(std::move(v));

    PostprocessContext ctx;
    ManifestSpec m = make_test_manifest(6);
    ctx.manifest = &m;

    Yolo26DetectPostprocessor pp;
    auto dets = pp.decode_detections(outs, ctx);
    CHECK(dets.empty());
}
