#pragma once

// =============================================================================
// roc::vision::manifest
//
// In-memory shape of the YAML manifest sidecar defined in
// `phase_1_universal_ep_and_manifest.md` §3.3.  Fields map one-to-one to the
// YAML keys; see §3.3 for the canonical YAML example and §3.4 for the
// validation rules enforced by `ManifestLoader`.
//
// `OutputHead` is extensible: registering a new postprocessor (§4.3) requires
// adding both a string key (matched against the manifest `output.head` field)
// and a concrete `Postprocessor` subclass.  The two heads shipped at Phase 4
// launch are `yolo26_detect` (§4.1) and `osnet_reid` (§5).
// =============================================================================

#include "roc/vision/types.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace roc {
namespace vision {

// ---------------------------------------------------------------------------
// §3.3 input contract — layout / channel order / dtype / normalisation.
// ---------------------------------------------------------------------------
enum class TensorLayout : std::uint8_t { NCHW, NHWC };
enum class ChannelOrder : std::uint8_t { RGB, BGR };
enum class TensorDType  : std::uint8_t { Float32, Float16, Uint8 };
enum class PadTo        : std::uint8_t { ModelInput, Square, None };

struct NormalizationSpec {
    double                 scale{1.0 / 255.0};
    std::array<double, 3>  mean{ {0.0, 0.0, 0.0} };
    std::array<double, 3>  std{  {1.0, 1.0, 1.0} };
};

struct PreprocessingSpec {
    bool                   letterbox{true};
    std::array<int, 3>     pad_value{ {114, 114, 114} };
    PadTo                  pad_to{PadTo::ModelInput};
};

struct InputSpec {
    TensorLayout           layout{TensorLayout::NCHW};
    ChannelOrder           channel_order{ChannelOrder::RGB};
    std::array<int64_t, 4> expected_shape{ {1, 3, 640, 640} };  // B, C, H, W
    TensorDType            dtype{TensorDType::Float32};
    NormalizationSpec      normalization{};
    PreprocessingSpec      preprocessing{};
};

// ---------------------------------------------------------------------------
// §3.3 output contract — head discriminator + thresholds.
// `Unknown` is the sentinel for "loaded a manifest but the head string did
// not match a registered postprocessor"; the loader rejects this at boot per
// §3.4 #6 — `Unknown` never escapes a successful `ManifestLoader::load`.
// ---------------------------------------------------------------------------
enum class OutputHead : std::uint8_t {
    Yolo26Detect = 0,
    OsnetReid    = 1,
    Yolov8Detect = 2,
    Unknown      = 0xFF,
};

inline const char* to_string(OutputHead h) noexcept {
    switch (h) {
        case OutputHead::Yolo26Detect: return "yolo26_detect";
        case OutputHead::OsnetReid:    return "osnet_reid";
        case OutputHead::Yolov8Detect: return "yolov8_detect";
        case OutputHead::Unknown:      return "unknown";
    }
    return "unknown";
}

enum class OutputFormat : std::uint8_t {
    CxCyWhLogits   = 0,   // YOLO26 NMS-free: (cx, cy, w, h) + raw class logits
    XyxySigmoid    = 1,   // pre-sigmoid xyxy + scores (legacy YOLOv8 family)
    XyxyPostNms    = 2,   // already NMS'd xyxy + score + class
    ReidEmbedding  = 3,   // OSNet-style appearance vector
    CxCyWhSigmoid  = 4,   // YOLOv8 cxcywh + already-sigmoid class probabilities
};

struct ThresholdSpec {
    double            score{0.25};
    std::optional<double> iou{};   // only when nms_in_graph == false AND XyxySigmoid
};

struct OutputSpec {
    OutputHead             head{OutputHead::Unknown};
    OutputFormat           format{OutputFormat::CxCyWhLogits};
    std::array<int64_t, 3> expected_shape{ {1, -1, -1} };
    int                    num_classes{0};   // 0 for ReID heads
    bool                   nms_in_graph{false};
    ThresholdSpec          thresholds{};
};

// ---------------------------------------------------------------------------
// §3.3 runtime hints — EP preference, precision, OV cache flag.
// ---------------------------------------------------------------------------
enum class Precision : std::uint8_t { Fp32, Fp16, Int8 };

struct RuntimeHints {
    EpKind     preferred_ep{EpKind::DirectML};
    Precision  precision{Precision::Fp16};
    bool       cache_compiled{true};
    bool       batchable{false};        // ReID extractors typically `true`
};

// ---------------------------------------------------------------------------
// §3.3 class mapping — id → application label (validated against
// roc::kinematics::Label per §3.4 #4).
// ---------------------------------------------------------------------------
struct ClassMapping {
    int          id{-1};
    std::string  name;
    Label        roc_label{Label::Unknown};
};

// ---------------------------------------------------------------------------
// `label_from_string` — case-sensitive mapping of the manifest's `roc_label`
// string to the locked `roc::kinematics::Label` enum.  Unknown strings
// resolve to `Label::Unknown` which the loader rejects per §3.4 #4.
// ---------------------------------------------------------------------------
Label label_from_string(std::string_view s) noexcept;

// ---------------------------------------------------------------------------
// ManifestSpec — top-level POD assembled by `ManifestLoader::load`.
// ---------------------------------------------------------------------------
struct ManifestSpec {
    int                       schema_version{1};
    std::string               name;
    std::string               purpose;      // "detection" | "reid" | ...
    std::string               trained_on;
    std::string               notes;

    InputSpec                 input{};
    OutputSpec                output{};
    RuntimeHints              runtime_hints{};
    std::vector<ClassMapping> classes{};

    std::string               head_string;  // exact `output.head` string from the YAML

    // Convenience accessors --------------------------------------------------
    int input_n() const noexcept { return static_cast<int>(input.expected_shape[0]); }
    int input_c() const noexcept { return static_cast<int>(input.expected_shape[1]); }
    int input_h() const noexcept { return static_cast<int>(input.expected_shape[2]); }
    int input_w() const noexcept { return static_cast<int>(input.expected_shape[3]); }

    ImageSize model_input_size() const noexcept {
        return ImageSize{ input_w(), input_h() };
    }
};

}  // namespace vision
}  // namespace roc
