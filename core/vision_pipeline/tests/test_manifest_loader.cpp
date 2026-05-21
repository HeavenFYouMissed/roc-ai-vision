// =============================================================================
// test_manifest_loader.cpp — §3.4 schema validation coverage.
// =============================================================================

#include "roc/vision/manifest_loader.hpp"
#include "roc/vision/manifest.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

using namespace roc::vision;

namespace {

constexpr const char* kValidYolo26n = R"YAML(
schema_version: 1
name:    yolo26n-coco
purpose: detection
trained_on: COCO 2017
notes: smoke
input:
  layout:        NCHW
  channel_order: RGB
  expected_shape: [1, 3, 640, 640]
  dtype: float32
  normalization:
    scale: 0.00392156862745
    mean:  [0.0, 0.0, 0.0]
    std:   [1.0, 1.0, 1.0]
  preprocessing:
    letterbox: true
    pad_value: [114, 114, 114]
    pad_to:    model_input
output:
  head:           yolo26_detect
  format:         cxcywh_logits
  expected_shape: [1, 84, 8400]
  num_classes:    80
  nms_in_graph:   false
  thresholds:
    score: 0.25
runtime_hints:
  preferred_ep:   DirectML
  precision:      fp16
  cache_compiled: true
classes:
  - {id: 0, name: person,   roc_label: Person}
  - {id: 1, name: bicycle,  roc_label: Unknown}
)YAML";

constexpr const char* kValidOsnet = R"YAML(
schema_version: 1
name:    osnet-ain-x1-0
purpose: reid
input:
  layout:        NCHW
  channel_order: RGB
  expected_shape: [1, 3, 256, 128]
  dtype: float32
  normalization:
    scale: 0.00392156862745
    mean:  [0.485, 0.456, 0.406]
    std:   [0.229, 0.224, 0.225]
output:
  head:           osnet_reid
  format:         reid_embedding
  expected_shape: [1, 128, -1]
  num_classes:    0
runtime_hints:
  preferred_ep: DirectML
  precision:    fp16
  batchable:    true
)YAML";

}  // namespace

TEST_CASE("ManifestLoader: valid yolo26n manifest loads cleanly", "[manifest]") {
    std::string err;
    auto spec = ManifestLoader::load_from_string(kValidYolo26n, &err);
    REQUIRE(spec.has_value());
    CHECK(err.empty());

    CHECK(spec->schema_version == 1);
    CHECK(spec->name == "yolo26n-coco");
    CHECK(spec->purpose == "detection");

    CHECK(spec->input.layout == TensorLayout::NCHW);
    CHECK(spec->input.channel_order == ChannelOrder::RGB);
    CHECK(spec->input.expected_shape[0] == 1);
    CHECK(spec->input.expected_shape[1] == 3);
    CHECK(spec->input.expected_shape[2] == 640);
    CHECK(spec->input.expected_shape[3] == 640);
    CHECK(spec->input.dtype == TensorDType::Float32);
    CHECK(spec->input.preprocessing.letterbox == true);
    CHECK(spec->input.preprocessing.pad_value[0] == 114);

    CHECK(spec->output.head == OutputHead::Yolo26Detect);
    CHECK(spec->head_string == "yolo26_detect");
    CHECK(spec->output.format == OutputFormat::CxCyWhLogits);
    CHECK(spec->output.num_classes == 80);
    CHECK(spec->output.thresholds.score == 0.25);

    CHECK(spec->runtime_hints.preferred_ep == EpKind::DirectML);
    CHECK(spec->runtime_hints.precision == Precision::Fp16);

    REQUIRE(spec->classes.size() == 2);
    CHECK(spec->classes[0].id == 0);
    CHECK(spec->classes[0].roc_label == Label::Person);
    CHECK(spec->classes[1].roc_label == Label::Unknown);
}

TEST_CASE("ManifestLoader: valid osnet manifest loads cleanly", "[manifest]") {
    std::string err;
    auto spec = ManifestLoader::load_from_string(kValidOsnet, &err);
    REQUIRE(spec.has_value());
    CHECK(err.empty());
    CHECK(spec->purpose == "reid");
    CHECK(spec->output.head == OutputHead::OsnetReid);
    CHECK(spec->output.format == OutputFormat::ReidEmbedding);
    CHECK(spec->output.num_classes == 0);
    CHECK(spec->runtime_hints.batchable == true);
    CHECK(spec->input.normalization.mean[0] == 0.485);
    CHECK(spec->input.normalization.std[2]  == 0.225);
}

TEST_CASE("ManifestLoader: missing required field 'schema_version' rejects", "[manifest]") {
    std::string yaml = R"YAML(
name: x
purpose: detection
input:
  layout: NCHW
  channel_order: RGB
  expected_shape: [1, 3, 640, 640]
  dtype: float32
  normalization: {scale: 0.00392, mean: [0,0,0], std: [1,1,1]}
output:
  head: yolo26_detect
  format: cxcywh_logits
  expected_shape: [1, 84, 8400]
  num_classes: 80
)YAML";
    std::string err;
    auto spec = ManifestLoader::load_from_string(yaml, &err);
    CHECK_FALSE(spec.has_value());
    CHECK(err.find("schema_version") != std::string::npos);
}

TEST_CASE("ManifestLoader: unknown output.head rejects (§3.4 #6)", "[manifest]") {
    std::string yaml = R"YAML(
schema_version: 1
name: bogus
purpose: detection
input:
  layout: NCHW
  channel_order: RGB
  expected_shape: [1, 3, 640, 640]
  dtype: float32
  normalization: {scale: 0.00392, mean: [0,0,0], std: [1,1,1]}
output:
  head: my_secret_head
  format: cxcywh_logits
  expected_shape: [1, 84, 8400]
  num_classes: 80
)YAML";
    std::string err;
    auto spec = ManifestLoader::load_from_string(yaml, &err);
    CHECK_FALSE(spec.has_value());
    CHECK(err.find("my_secret_head") != std::string::npos);
}

TEST_CASE("ManifestLoader: class id out of range rejects (§3.4 #3)", "[manifest]") {
    std::string yaml = R"YAML(
schema_version: 1
name: yolo26n-coco
purpose: detection
input:
  layout: NCHW
  channel_order: RGB
  expected_shape: [1, 3, 640, 640]
  dtype: float32
  normalization: {scale: 0.00392, mean: [0,0,0], std: [1,1,1]}
output:
  head: yolo26_detect
  format: cxcywh_logits
  expected_shape: [1, 84, 8400]
  num_classes: 6
classes:
  - {id: 0,  name: person, roc_label: Person}
  - {id: 99, name: alien,  roc_label: Person}
)YAML";
    std::string err;
    auto spec = ManifestLoader::load_from_string(yaml, &err);
    CHECK_FALSE(spec.has_value());
    CHECK(err.find("99") != std::string::npos);
}

TEST_CASE("ManifestLoader: invalid roc_label rejects (§3.4 #4)", "[manifest]") {
    std::string yaml = R"YAML(
schema_version: 1
name: yolo26n-coco
purpose: detection
input:
  layout: NCHW
  channel_order: RGB
  expected_shape: [1, 3, 640, 640]
  dtype: float32
  normalization: {scale: 0.00392, mean: [0,0,0], std: [1,1,1]}
output:
  head: yolo26_detect
  format: cxcywh_logits
  expected_shape: [1, 84, 8400]
  num_classes: 6
classes:
  - {id: 0, name: person, roc_label: BogusLabel}
)YAML";
    std::string err;
    auto spec = ManifestLoader::load_from_string(yaml, &err);
    CHECK_FALSE(spec.has_value());
    CHECK(err.find("BogusLabel") != std::string::npos);
}

TEST_CASE("ManifestLoader: bad expected_shape length rejects", "[manifest]") {
    std::string yaml = R"YAML(
schema_version: 1
name: bad
purpose: detection
input:
  layout: NCHW
  channel_order: RGB
  expected_shape: [3, 640, 640]
  dtype: float32
  normalization: {scale: 0.00392, mean: [0,0,0], std: [1,1,1]}
output:
  head: yolo26_detect
  format: cxcywh_logits
  expected_shape: [1, 84, 8400]
  num_classes: 80
)YAML";
    std::string err;
    auto spec = ManifestLoader::load_from_string(yaml, &err);
    CHECK_FALSE(spec.has_value());
    CHECK(err.find("expected_shape") != std::string::npos);
}

TEST_CASE("ManifestLoader: invalid dtype rejects", "[manifest]") {
    std::string yaml = R"YAML(
schema_version: 1
name: x
purpose: detection
input:
  layout: NCHW
  channel_order: RGB
  expected_shape: [1, 3, 640, 640]
  dtype: bfloat16
  normalization: {scale: 0.00392, mean: [0,0,0], std: [1,1,1]}
output:
  head: yolo26_detect
  format: cxcywh_logits
  expected_shape: [1, 84, 8400]
  num_classes: 80
)YAML";
    std::string err;
    auto spec = ManifestLoader::load_from_string(yaml, &err);
    CHECK_FALSE(spec.has_value());
    CHECK(err.find("bfloat16") != std::string::npos);
}

TEST_CASE("ManifestLoader: yaml syntax error rejects gracefully", "[manifest]") {
    std::string err;
    auto spec = ManifestLoader::load_from_string("schema_version: : invalid: : :", &err);
    CHECK_FALSE(spec.has_value());
    CHECK_FALSE(err.empty());
}

TEST_CASE("ManifestLoader: score threshold out of [0,1] rejects", "[manifest]") {
    std::string yaml = R"YAML(
schema_version: 1
name: x
purpose: detection
input:
  layout: NCHW
  channel_order: RGB
  expected_shape: [1, 3, 640, 640]
  dtype: float32
  normalization: {scale: 0.00392, mean: [0,0,0], std: [1,1,1]}
output:
  head: yolo26_detect
  format: cxcywh_logits
  expected_shape: [1, 84, 8400]
  num_classes: 80
  thresholds: {score: 1.5}
)YAML";
    std::string err;
    auto spec = ManifestLoader::load_from_string(yaml, &err);
    CHECK_FALSE(spec.has_value());
    CHECK(err.find("score") != std::string::npos);
}

TEST_CASE("label_from_string maps the locked enum names", "[manifest]") {
    CHECK(label_from_string("Person")             == Label::Person);
    CHECK(label_from_string("Head")               == Label::Head);
    CHECK(label_from_string("UpperTorso")         == Label::UpperTorso);
    CHECK(label_from_string("AccessoryHat")       == Label::AccessoryHat);
    CHECK(label_from_string("AccessoryBackpack")  == Label::AccessoryBackpack);
    CHECK(label_from_string("Unknown")            == Label::Unknown);
    CHECK(label_from_string("typo")               == Label::Unknown);
}
