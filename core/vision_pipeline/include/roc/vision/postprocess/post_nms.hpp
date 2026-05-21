#pragma once

// =============================================================================
// roc::vision::postprocess::PostNmsPostprocessor
//
// Decodes end-to-end ONNX models that output post-NMS format:
//   [1, 300, 6]  where the 6 channels are [x1, y1, x2, y2, confidence, class_id]
//
// Coordinates are in normalized [0,1] image space.  No NMS, no sigmoid, no
// letterbox inversion needed — the ONNX graph already handled all of that.
// =============================================================================

#include "roc/vision/postprocessor.hpp"

namespace roc {
namespace vision {

class PostNmsPostprocessor : public Postprocessor {
public:
    PostNmsPostprocessor() = default;

    std::vector<Detection>
    decode_detections(const std::vector<Ort::Value>& outputs,
                      const PostprocessContext&      ctx) const override;

    std::vector<Embedding>
    decode_embeddings(const std::vector<Ort::Value>& /*outputs*/,
                      const PostprocessContext&      /*ctx*/) const override {
        return {};
    }

    const char* head_name() const noexcept override { return "post_nms"; }
};

}  // namespace vision
}  // namespace roc
