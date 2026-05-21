#pragma once

// =============================================================================
// roc::vision::postprocess::Yolo26DetectPostprocessor
//
// Implements the NMS-free decode of §4.1 for tensors of shape
// `[1, 4 + num_classes, num_anchors]`, dtype float32.  Per the §4.1
// pseudocode:
//   * channels [0:4]            → (cx, cy, w, h) in letterboxed pixels.
//   * channels [4 .. 4+C-1]     → raw per-class logits; we apply sigmoid.
//   * best class chosen per anchor; gated by `manifest.output.thresholds.score`.
//   * inverse letterbox (§4.2) projects boxes back to capture-frame pixels.
//
// Single-pass, branch-light, vectorisable by the auto-vectoriser at -O3.  N is
// typically 8400 (YOLO26 at 640×640); per-frame cost ~80 K float ops per the
// §4.1 closing note — under 200 µs on a single x86_64 core.
// =============================================================================

#include "roc/vision/postprocessor.hpp"

namespace roc {
namespace vision {

class Yolo26DetectPostprocessor : public Postprocessor {
public:
    Yolo26DetectPostprocessor() = default;

    std::vector<Detection>
    decode_detections(const std::vector<Ort::Value>& outputs,
                      const PostprocessContext&      ctx) const override;

    // ReID heads return empty + logged warning.
    std::vector<Embedding>
    decode_embeddings(const std::vector<Ort::Value>& /*outputs*/,
                      const PostprocessContext&      /*ctx*/) const override {
        return {};
    }

    const char* head_name() const noexcept override { return "yolo26_detect"; }
};

}  // namespace vision
}  // namespace roc
