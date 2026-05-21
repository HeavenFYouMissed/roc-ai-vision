#pragma once

// =============================================================================
// roc::vision::types
//
// Vocabulary types for the vision pipeline.  Cross-references in this header
// (and throughout `core/vision_pipeline/`) use the §-numbering from
// `workspace_blueprint/research_notes/phase_1_universal_ep_and_manifest.md`.
//
// Binding contract:
//   * Detection / Embedding / Label / Pixel are taken **verbatim** from
//     `roc::kinematics::types` — we do not redefine them.  See §3.4 (manifest
//     loader `roc_label` validation) and §4.1 (postprocessor output type).
//   * ImageSize / LetterboxParams / EpKind / EpProbeResult are vocabulary
//     types unique to the vision side of the SPSC boundary.
//
// All numeric tensor reads in `core/vision_pipeline/` assume little-endian
// IEEE-754 layout (x86_64 / ARM64 LE — every platform ONNX Runtime ships for).
// =============================================================================

#include "roc/kinematics/types.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>

namespace roc {
namespace vision {

// ---------------------------------------------------------------------------
// Re-export the kinematics vocabulary so vision callers don't need a second
// include.  These are aliases, **not** new types: byte-compat guaranteed.
// ---------------------------------------------------------------------------
using Detection = ::roc::kinematics::Detection;
using Embedding = ::roc::kinematics::Embedding;
using Label     = ::roc::kinematics::Label;
using Pixel     = ::roc::kinematics::Pixel;
using EmbScalar = ::roc::kinematics::EmbScalar;
inline constexpr int kEmbeddingDim = ::roc::kinematics::kEmbeddingDim;

// ---------------------------------------------------------------------------
// ImageSize — integer width/height pair (capture frames or model-input dims).
// ---------------------------------------------------------------------------
struct ImageSize {
    int w{0};
    int h{0};
};

inline constexpr bool operator==(ImageSize a, ImageSize b) noexcept {
    return a.w == b.w && a.h == b.h;
}
inline constexpr bool operator!=(ImageSize a, ImageSize b) noexcept { return !(a == b); }

// ---------------------------------------------------------------------------
// LetterboxParams — cached transform required by the YOLO26 inverse mapping
// in §4.2.  `scale` is the linear factor applied to source pixels before
// padding (pad_x, pad_y).  `letterboxed_w/h` are the rescaled image extents
// **before** padding (i.e. floor(src_w · scale), floor(src_h · scale)).
// ---------------------------------------------------------------------------
struct LetterboxParams {
    double scale{1.0};
    double pad_x{0.0};
    double pad_y{0.0};
    int    letterboxed_w{0};   // floor(src_w · scale)
    int    letterboxed_h{0};   // floor(src_h · scale)
    int    model_w{0};         // model input width  (after padding)
    int    model_h{0};         // model input height (after padding)
    int    src_w{0};           // original capture-frame width
    int    src_h{0};           // original capture-frame height
};

// ---------------------------------------------------------------------------
// EpKind — locked priority order per §1.1.  `Cuda` is opt-in only and is
// only emitted by the probe when the build flag `ROC_VISION_BUILD_CUDA_EP`
// was set at configure time.
// ---------------------------------------------------------------------------
enum class EpKind : std::uint8_t {
    DirectML = 0,
    OpenVINO = 1,
    Cpu      = 2,
    Cuda     = 3,
};

inline const char* to_string(EpKind k) noexcept {
    switch (k) {
        case EpKind::DirectML: return "DirectML";
        case EpKind::OpenVINO: return "OpenVINO";
        case EpKind::Cpu:      return "Cpu";
        case EpKind::Cuda:     return "Cuda";
    }
    return "Unknown";
}

// ---------------------------------------------------------------------------
// EpProbeResult — outcome of §1.2 probe-and-fallback.  `all_ops_native` is
// false if the ORT profiler indicated CPU silent-fallback on some ops (see
// §1.2 acceptance criterion #3 + §2 risk row "Silent CPU fallback").
// ---------------------------------------------------------------------------
struct EpProbeResult {
    EpKind      chosen{EpKind::Cpu};
    std::string device_description;
    int         device_id{0};
    bool        all_ops_native{true};
};

// ---------------------------------------------------------------------------
// ModelPath — strong typedef over std::filesystem::path that prevents
// accidental swap with the manifest YAML path at the InferenceEngine
// constructor's call site.
// ---------------------------------------------------------------------------
class ModelPath {
public:
    ModelPath() = default;
    explicit ModelPath(std::filesystem::path p) noexcept : path_(std::move(p)) {}
    const std::filesystem::path& path() const noexcept { return path_; }
    bool empty() const noexcept { return path_.empty(); }

private:
    std::filesystem::path path_{};
};

// ---------------------------------------------------------------------------
// Span<T> — minimal C++17 shim used in place of std::span.  Non-owning,
// read-only view over a contiguous range.  We intentionally keep this tiny
// (no constexpr, no iterator concepts) — see CMakeLists for the C++17
// strictness rationale.
// ---------------------------------------------------------------------------
template <class T>
class Span {
public:
    Span() noexcept = default;
    Span(const T* data, std::size_t size) noexcept : data_(data), size_(size) {}

    template <std::size_t N>
    explicit Span(const T (&arr)[N]) noexcept : data_(arr), size_(N) {}

    template <class Container,
              class = decltype(std::declval<const Container&>().data()),
              class = decltype(std::declval<const Container&>().size())>
    Span(const Container& c) noexcept : data_(c.data()), size_(c.size()) {}

    const T* data() const noexcept { return data_; }
    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }

    const T* begin() const noexcept { return data_; }
    const T* end()   const noexcept { return data_ + size_; }

    const T& operator[](std::size_t i) const noexcept { return data_[i]; }

private:
    const T*    data_{nullptr};
    std::size_t size_{0};
};

}  // namespace vision
}  // namespace roc
