#pragma once

// =============================================================================
// roc::vision::postprocess::OsnetReidPostprocessor
//
// Decodes `[B, D]` (or `[B, D, 1, 1]`) OSNet outputs into 128-D L2-normalised
// `Embedding` vectors per §5.2:
//
//   1. If D ≠ 128, project D → 128 with a stored linear matrix.  When no
//      projection matrix has been supplied, the decoder selects the first 128
//      output channels (acceptable when the exported graph already includes
//      OSNet's gallery-projection head; warned otherwise).
//   2. L2-normalise; epsilon-guard against zero vectors so the result is
//      always a finite unit vector.
//   3. Cast EmbScalar(float) → roc::kinematics::Embedding (Eigen 128×1).
//
// The class is detection-side empty: `decode_detections` returns an empty
// vector with a logged warning.
// =============================================================================

#include "roc/vision/postprocessor.hpp"

#include <Eigen/Dense>

namespace roc {
namespace vision {

class OsnetReidPostprocessor : public Postprocessor {
public:
    OsnetReidPostprocessor() = default;

    // Optional projection matrix [128 × D].  When unset, the decoder takes
    // the first 128 channels of each row (graph already includes projection).
    void set_projection(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> W);

    std::vector<Detection>
    decode_detections(const std::vector<Ort::Value>& /*outputs*/,
                      const PostprocessContext&      /*ctx*/) const override {
        return {};
    }

    std::vector<Embedding>
    decode_embeddings(const std::vector<Ort::Value>& outputs,
                      const PostprocessContext&      ctx) const override;

    const char* head_name() const noexcept override { return "osnet_reid"; }

    bool has_projection() const noexcept { return projection_.size() > 0; }
    int  projection_cols() const noexcept {
        return static_cast<int>(projection_.cols());
    }

private:
    // 128 × D projection matrix (empty when graph already outputs 128-D).
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> projection_{};
};

}  // namespace vision
}  // namespace roc
