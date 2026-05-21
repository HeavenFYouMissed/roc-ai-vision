// =============================================================================
// test_byte_compat.cpp — verify roc::vision::{Detection,Embedding,Label,Pixel}
// are aliases for the kinematics_engine types, NOT independent re-definitions.
//
// The worker spec mandates byte-compatibility: the kinematics_engine consumer
// must be able to consume a `std::vector<roc::vision::Detection>` produced by
// the vision pipeline without any copy / cast / re-pack.
// =============================================================================

#include "roc/vision/types.hpp"
#include "roc/kinematics/types.hpp"

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

using kDet  = ::roc::kinematics::Detection;
using vDet  = ::roc::vision::Detection;
using kEmb  = ::roc::kinematics::Embedding;
using vEmb  = ::roc::vision::Embedding;
using kLbl  = ::roc::kinematics::Label;
using vLbl  = ::roc::vision::Label;
using kPix  = ::roc::kinematics::Pixel;
using vPix  = ::roc::vision::Pixel;

static_assert(std::is_same<kDet, vDet>::value,
              "vision::Detection must be the same type as kinematics::Detection");
static_assert(std::is_same<kEmb, vEmb>::value,
              "vision::Embedding must be the same type as kinematics::Embedding");
static_assert(std::is_same<kLbl, vLbl>::value,
              "vision::Label must be the same type as kinematics::Label");
static_assert(std::is_same<kPix, vPix>::value,
              "vision::Pixel must be the same type as kinematics::Pixel");
static_assert(sizeof(kDet) == sizeof(vDet),
              "sizeof Detection must match (alias invariant)");
static_assert(sizeof(kEmb) == sizeof(vEmb),
              "sizeof Embedding must match (alias invariant)");

TEST_CASE("Detection types are aliases, not separate structs (runtime check)", "[bytecompat]") {
    // Compile-time static asserts above provide the real guarantee; the
    // runtime block here exists so the Catch2 reporter counts this assertion
    // toward the test totals.
    SUCCEED("static_asserts above verify std::is_same and sizeof equivalence");
}

TEST_CASE("Embedding is 128-D float (locked typedef)", "[bytecompat]") {
    CHECK(::roc::vision::kEmbeddingDim == 128);
    CHECK(::roc::kinematics::kEmbeddingDim == 128);
    static_assert(std::is_same<vEmb::Scalar, float>::value,
                  "Embedding must be float-scalar");
    CHECK(vEmb::RowsAtCompileTime == 128);
    CHECK(vEmb::ColsAtCompileTime == 1);
}

TEST_CASE("Label enum values are wire-stable", "[bytecompat]") {
    CHECK(static_cast<int>(vLbl::Unknown)          == 0);
    CHECK(static_cast<int>(vLbl::Person)           == 1);
    CHECK(static_cast<int>(vLbl::Head)             == 2);
    CHECK(static_cast<int>(vLbl::UpperTorso)       == 3);
    CHECK(static_cast<int>(vLbl::AccessoryHat)     == 4);
    CHECK(static_cast<int>(vLbl::AccessoryBackpack) == 5);
}
