#pragma once

// =============================================================================
// roc::vision::postprocessor
//
// Strategy interface for converting raw `Ort::Value` output tensors into the
// pipeline's wire type (`Detection[]` for detection heads, `Embedding[]` for
// ReID heads).  See §4.3 of the binding spec for the contract.
//
// Design rationale for the dual decode signature:
//   * Detection heads decode into `std::vector<Detection>` (§4.1 algorithm)
//     with the `Embedding appearance` field zeroed; the ReID stage populates
//     it later via `decode_embeddings`.
//   * ReID heads decode into `std::vector<Embedding>` (§5.2 — L2-normalised
//     128-D vectors).  Returning these as Detections-with-only-appearance
//     would be a typed-but-confusing API — we explicitly support both.
//
// The InferenceEngine knows which mode it is in from the manifest's
// `output.format` field (CxCyWhLogits → decode_detections, ReidEmbedding →
// decode_embeddings).  Postprocessors that don't support a given mode return
// an empty vector with a logged warning.
// =============================================================================

#include "roc/vision/manifest.hpp"
#include "roc/vision/types.hpp"

#include <onnxruntime_cxx_api.h>

#include <array>
#include <vector>

namespace roc {
namespace vision {

// ---------------------------------------------------------------------------
// PostprocessContext — supplied by the InferenceEngine on every call.
// ---------------------------------------------------------------------------
struct PostprocessContext {
    LetterboxParams        letterbox{};      // single-frame letterbox params
    std::array<int, 2>     model_input_hw{ {0, 0} };
    std::array<int, 2>     capture_input_hw{ {0, 0} };
    const ManifestSpec*    manifest{nullptr};

    // For batched ReID decode: one LetterboxParams per input crop.
    Span<const LetterboxParams> batch_letterbox{};
};

// ---------------------------------------------------------------------------
// Postprocessor strategy base class — see §4.3.
// ---------------------------------------------------------------------------
class Postprocessor {
public:
    virtual ~Postprocessor() = default;

    // Decode raw outputs into Detections (detection heads).  ReID-only
    // postprocessors return an empty vector and may log.
    virtual std::vector<Detection>
    decode_detections(const std::vector<Ort::Value>& outputs,
                      const PostprocessContext&      ctx) const = 0;

    // Decode raw outputs into Embeddings (ReID heads).  Detection-only
    // postprocessors return an empty vector and may log.
    virtual std::vector<Embedding>
    decode_embeddings(const std::vector<Ort::Value>& outputs,
                      const PostprocessContext&      ctx) const = 0;

    // Free-form head string ("yolo26_detect", "osnet_reid", ...).
    virtual const char* head_name() const noexcept = 0;
};

}  // namespace vision
}  // namespace roc
