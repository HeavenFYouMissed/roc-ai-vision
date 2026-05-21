// =============================================================================
// roc::vision::OsnetReidPostprocessor — §5.2 decode.
//
// Accepts output rank 2 ([B, D]) or rank 4 ([B, D, 1, 1]).  D may be 128 (graph
// already includes gallery projection) or larger (512 typical for OSNet
// trunks); when D != 128 a projection matrix [128 × D] must be set ahead of
// time via `set_projection`, otherwise the decoder defensively takes the
// first 128 channels and logs (in practice unusable for cosine similarity —
// the engine config should always provide the projection for non-128 graphs).
//
// L2 normalisation includes an epsilon guard to avoid NaN on zero vectors
// (which arise from degenerate crops or quantised-to-zero outputs).
// =============================================================================

#include "roc/vision/postprocess/osnet_reid.hpp"

#include <cstring>
#include <cmath>

namespace roc {
namespace vision {

void OsnetReidPostprocessor::set_projection(
        Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> W) {
    projection_ = std::move(W);
}

std::vector<Embedding>
OsnetReidPostprocessor::decode_embeddings(const std::vector<Ort::Value>& outputs,
                                          const PostprocessContext&      /*ctx*/) const {
    std::vector<Embedding> out;
    if (outputs.empty()) return out;

    const Ort::Value& y = outputs[0];
    if (!y.IsTensor()) return out;

    auto info = y.GetTensorTypeAndShapeInfo();
    if (info.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) return out;

    auto shape = info.GetShape();
    int64_t B = 0;
    int64_t D = 0;
    if (shape.size() == 2) {
        B = shape[0];
        D = shape[1];
    } else if (shape.size() == 4 && shape[2] == 1 && shape[3] == 1) {
        B = shape[0];
        D = shape[1];
    } else {
        return out;
    }
    if (B <= 0 || D <= 0) return out;

    const float* data = y.GetTensorData<float>();
    const int Dn = static_cast<int>(D);
    out.reserve(static_cast<std::size_t>(B));

    Eigen::Matrix<float, kEmbeddingDim, 1> v;

    for (int64_t b = 0; b < B; ++b) {
        const float* row = data + static_cast<std::size_t>(b) * static_cast<std::size_t>(D);

        if (Dn == kEmbeddingDim) {
            for (int i = 0; i < kEmbeddingDim; ++i) v(i) = row[i];
        } else if (projection_.rows() == kEmbeddingDim && projection_.cols() == Dn) {
            // y = W * x (W: 128 × D)
            Eigen::Map<const Eigen::Matrix<float, Eigen::Dynamic, 1>> x(row, Dn);
            v.noalias() = projection_ * x;
        } else {
            // Defensive: select first 128 channels.  Not cosine-meaningful but
            // keeps the engine running rather than crashing.
            const int n = std::min(Dn, kEmbeddingDim);
            for (int i = 0; i < n; ++i) v(i) = row[i];
            for (int i = n; i < kEmbeddingDim; ++i) v(i) = 0.0f;
        }

        const float norm = v.norm();
        if (norm > 1e-12f) {
            v /= norm;
        } else {
            v.setZero();
        }
        out.push_back(v);
    }
    return out;
}

}  // namespace vision
}  // namespace roc
