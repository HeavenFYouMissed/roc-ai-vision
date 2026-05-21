// =============================================================================
// test_osnet_decode.cpp — §5.2 OSNet decode: projection + L2-normalisation.
// =============================================================================

#include "roc/vision/postprocess/osnet_reid.hpp"

#include <onnxruntime_cxx_api.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <Eigen/Dense>

#include <vector>

using namespace roc::vision;
using Catch::Matchers::WithinAbs;

TEST_CASE("OSNet decode: 128-D output produces unit-norm vectors", "[osnet]") {
    // Two synthesised samples: arbitrary positive vectors.
    constexpr int B = 2;
    constexpr int D = 128;
    std::vector<float> raw(B * D, 0.0f);
    for (int b = 0; b < B; ++b) {
        for (int i = 0; i < D; ++i) {
            raw[static_cast<std::size_t>(b * D + i)] = static_cast<float>(b + 1) * (i + 1);
        }
    }

    Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<int64_t> shape{B, D};
    Ort::Value v = Ort::Value::CreateTensor<float>(mi, raw.data(), raw.size(),
                                                   shape.data(), shape.size());
    std::vector<Ort::Value> outs;
    outs.push_back(std::move(v));

    OsnetReidPostprocessor pp;
    PostprocessContext ctx;
    auto embs = pp.decode_embeddings(outs, ctx);
    REQUIRE(embs.size() == 2u);
    for (const auto& e : embs) {
        CHECK_THAT(static_cast<double>(e.norm()), WithinAbs(1.0, 1e-5));
    }
}

TEST_CASE("OSNet decode: 512-D output projects to 128-D + L2", "[osnet]") {
    constexpr int B = 3;
    constexpr int D = 512;
    std::vector<float> raw(B * D);
    for (int i = 0; i < B * D; ++i) raw[static_cast<std::size_t>(i)] = std::sin(static_cast<float>(i) * 0.01f);

    Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<int64_t> shape{B, D};
    Ort::Value v = Ort::Value::CreateTensor<float>(mi, raw.data(), raw.size(),
                                                   shape.data(), shape.size());
    std::vector<Ort::Value> outs;
    outs.push_back(std::move(v));

    OsnetReidPostprocessor pp;
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> W(kEmbeddingDim, D);
    W.setRandom();   // any non-degenerate projection
    pp.set_projection(W);
    CHECK(pp.has_projection());
    CHECK(pp.projection_cols() == D);

    PostprocessContext ctx;
    auto embs = pp.decode_embeddings(outs, ctx);
    REQUIRE(embs.size() == static_cast<std::size_t>(B));
    for (const auto& e : embs) {
        CHECK_THAT(static_cast<double>(e.norm()), WithinAbs(1.0, 1e-5));
    }
}

TEST_CASE("OSNet decode: zero-vector input becomes zero (epsilon guard)", "[osnet]") {
    constexpr int B = 1;
    constexpr int D = 128;
    std::vector<float> raw(B * D, 0.0f);   // exactly zero

    Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<int64_t> shape{B, D};
    Ort::Value v = Ort::Value::CreateTensor<float>(mi, raw.data(), raw.size(),
                                                   shape.data(), shape.size());
    std::vector<Ort::Value> outs;
    outs.push_back(std::move(v));

    OsnetReidPostprocessor pp;
    PostprocessContext ctx;
    auto embs = pp.decode_embeddings(outs, ctx);
    REQUIRE(embs.size() == 1u);
    CHECK(embs[0].norm() == 0.0f);
}

TEST_CASE("OSNet decode: rank-4 [B, D, 1, 1] tensor accepted", "[osnet]") {
    constexpr int B = 1;
    constexpr int D = 128;
    std::vector<float> raw(B * D);
    for (int i = 0; i < D; ++i) raw[static_cast<std::size_t>(i)] = static_cast<float>(i + 1);

    Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<int64_t> shape{B, D, 1, 1};
    Ort::Value v = Ort::Value::CreateTensor<float>(mi, raw.data(), raw.size(),
                                                   shape.data(), shape.size());
    std::vector<Ort::Value> outs;
    outs.push_back(std::move(v));

    OsnetReidPostprocessor pp;
    PostprocessContext ctx;
    auto embs = pp.decode_embeddings(outs, ctx);
    REQUIRE(embs.size() == 1u);
    CHECK_THAT(static_cast<double>(embs[0].norm()), WithinAbs(1.0, 1e-5));
}

TEST_CASE("OSNet decode: empty outputs vector yields empty", "[osnet]") {
    OsnetReidPostprocessor pp;
    PostprocessContext ctx;
    CHECK(pp.decode_embeddings({}, ctx).empty());
}

TEST_CASE("OSNet decode: malformed tensor rank returns empty (hot-path)", "[osnet]") {
    Ort::MemoryInfo mi = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::vector<float> raw{1.0f, 2.0f, 3.0f};
    std::vector<int64_t> shape{3};
    Ort::Value v = Ort::Value::CreateTensor<float>(mi, raw.data(), raw.size(),
                                                   shape.data(), shape.size());
    std::vector<Ort::Value> outs;
    outs.push_back(std::move(v));
    OsnetReidPostprocessor pp;
    PostprocessContext ctx;
    CHECK(pp.decode_embeddings(outs, ctx).empty());
}
