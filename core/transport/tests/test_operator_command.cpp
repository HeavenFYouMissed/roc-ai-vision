// =============================================================================
// test_operator_command — round-trip serialise → deserialise, byte layout
// against hex-encoded test vectors, every kind enumerator exercised, error
// paths checked.
// =============================================================================

#include "roc/transport/operator_command.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <cstring>

using namespace roc::transport;

namespace {

float bits_to_float(std::uint32_t bits) {
    float f;
    std::memcpy(&f, &bits, sizeof(f));
    return f;
}

}  // namespace

TEST_CASE("Operator command: kOperatorCommandWireSize is 22 bytes", "[opcmd][size]") {
    STATIC_REQUIRE(kOperatorCommandWireSize == 22);
}

TEST_CASE("Operator command: LockTarget round-trip", "[opcmd][roundtrip]") {
    OperatorCommand original{};
    original.kind            = OperatorCommandKind::LockTarget;
    original.target_track_id = 0x0123456789ABCDEFull;

    const auto encoded = serialize_operator_command(original);
    REQUIRE(encoded.size() == 22);
    REQUIRE(encoded[0] == 1);  // LockTarget == 1
    REQUIRE(encoded[1] == 0);  // reserved

    auto decoded = deserialize_operator_command(encoded.data(), encoded.size());
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->kind == OperatorCommandKind::LockTarget);
    REQUIRE(decoded->target_track_id == 0x0123456789ABCDEFull);
    REQUIRE(decoded->weight_proximity  == 0.0f);
    REQUIRE(decoded->weight_confidence == 0.0f);
    REQUIRE(decoded->weight_stability  == 0.0f);
}

TEST_CASE("Operator command: ReleaseLock round-trip", "[opcmd][roundtrip]") {
    OperatorCommand original{};
    original.kind = OperatorCommandKind::ReleaseLock;
    const auto encoded = serialize_operator_command(original);
    auto decoded = deserialize_operator_command(encoded);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->kind == OperatorCommandKind::ReleaseLock);
    REQUIRE(decoded->target_track_id == 0);
}

TEST_CASE("Operator command: UpdateSelectorWeights round-trip", "[opcmd][roundtrip]") {
    OperatorCommand original{};
    original.kind              = OperatorCommandKind::UpdateSelectorWeights;
    original.weight_proximity  = 1.25f;
    original.weight_confidence = 0.5f;
    original.weight_stability  = 0.0625f;

    const auto encoded = serialize_operator_command(original);
    auto decoded = deserialize_operator_command(encoded.data(), encoded.size());
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->kind == OperatorCommandKind::UpdateSelectorWeights);
    REQUIRE(decoded->weight_proximity  == 1.25f);
    REQUIRE(decoded->weight_confidence == 0.5f);
    REQUIRE(decoded->weight_stability  == 0.0625f);
    REQUIRE(decoded->value == 0.0f);
}

TEST_CASE("Operator command: SetSmoothingAlpha round-trip", "[opcmd][roundtrip]") {
    OperatorCommand original{};
    original.kind  = OperatorCommandKind::SetSmoothingAlpha;
    original.value = 0.35f;

    const auto encoded = serialize_operator_command(original);
    REQUIRE(encoded[0] == 4);
    auto decoded = deserialize_operator_command(encoded.data(), encoded.size());
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->kind == OperatorCommandKind::SetSmoothingAlpha);
    REQUIRE(decoded->value == 0.35f);
    REQUIRE(decoded->weight_proximity == 0.0f);
    REQUIRE(decoded->weight_confidence == 0.0f);
    REQUIRE(decoded->weight_stability == 0.0f);
}

TEST_CASE("Operator command: SetDeadbandRadiusPx round-trip", "[opcmd][roundtrip]") {
    OperatorCommand original{};
    original.kind  = OperatorCommandKind::SetDeadbandRadiusPx;
    original.value = 2.5f;

    const auto encoded = serialize_operator_command(original);
    REQUIRE(encoded[0] == 5);
    auto decoded = deserialize_operator_command(encoded);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->kind == OperatorCommandKind::SetDeadbandRadiusPx);
    REQUIRE(decoded->value == 2.5f);
    REQUIRE(decoded->weight_proximity == 0.0f);
    REQUIRE(decoded->weight_confidence == 0.0f);
    REQUIRE(decoded->weight_stability == 0.0f);
}

TEST_CASE("Operator command: SetFovRadiusPx round-trip", "[opcmd][roundtrip]") {
    OperatorCommand original{};
    original.kind  = OperatorCommandKind::SetFovRadiusPx;
    original.value = 120.0f;

    const auto encoded = serialize_operator_command(original);
    REQUIRE(encoded[0] == 6);
    auto decoded = deserialize_operator_command(encoded);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->kind == OperatorCommandKind::SetFovRadiusPx);
    REQUIRE(decoded->value == 120.0f);
    REQUIRE(decoded->weight_proximity == 0.0f);
    REQUIRE(decoded->weight_confidence == 0.0f);
    REQUIRE(decoded->weight_stability == 0.0f);
}

TEST_CASE("Operator command: SetTargetingAnchor round-trip", "[opcmd][roundtrip]") {
    OperatorCommand original{};
    original.kind  = OperatorCommandKind::SetTargetingAnchor;
    original.value = 3.0f;

    const auto encoded = serialize_operator_command(original);
    REQUIRE(encoded[0] == 7);
    auto decoded = deserialize_operator_command(encoded);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->kind == OperatorCommandKind::SetTargetingAnchor);
    REQUIRE(decoded->value == 3.0f);
    REQUIRE(decoded->weight_proximity == 0.0f);
    REQUIRE(decoded->weight_confidence == 0.0f);
    REQUIRE(decoded->weight_stability == 0.0f);
}

TEST_CASE("Operator command: explicit byte layout (LockTarget hex vector)",
          "[opcmd][hex]") {
    // Vector:
    //   kind = LockTarget (1)
    //   reserved = 0
    //   target_track_id = 0x0807060504030201 (little-endian: 01 02 03 04 05 06 07 08)
    //   weight_proximity  = 1.0f   (0x3F800000 → 00 00 80 3F)
    //   weight_confidence = 0.5f   (0x3F000000 → 00 00 00 3F)
    //   weight_stability  = 0.25f  (0x3E800000 → 00 00 80 3E)
    OperatorCommand cmd{};
    cmd.kind              = OperatorCommandKind::LockTarget;
    cmd.target_track_id   = 0x0807060504030201ull;
    cmd.weight_proximity  = bits_to_float(0x3F800000);
    cmd.weight_confidence = bits_to_float(0x3F000000);
    cmd.weight_stability  = bits_to_float(0x3E800000);

    const auto enc = serialize_operator_command(cmd);
    const std::uint8_t expected[22] = {
        0x01, 0x00,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x00, 0x00, 0x80, 0x3F,
        0x00, 0x00, 0x00, 0x3F,
        0x00, 0x00, 0x80, 0x3E,
    };
    for (std::size_t i = 0; i < 22; ++i) {
        INFO("byte " << i);
        REQUIRE(enc[i] == expected[i]);
    }
}

TEST_CASE("Operator command: deserialize rejects truncated input", "[opcmd][err]") {
    std::array<std::uint8_t, 22> good{};
    good[0] = static_cast<std::uint8_t>(OperatorCommandKind::ReleaseLock);

    REQUIRE_FALSE(deserialize_operator_command(good.data(), 21).has_value());
    REQUIRE_FALSE(deserialize_operator_command(good.data(), 23).has_value());
    REQUIRE_FALSE(deserialize_operator_command(good.data(), 0).has_value());
    REQUIRE_FALSE(deserialize_operator_command(nullptr,    22).has_value());

    auto ok = deserialize_operator_command(good.data(), 22);
    REQUIRE(ok.has_value());
    REQUIRE(ok->kind == OperatorCommandKind::ReleaseLock);
}

TEST_CASE("Operator command: deserialize rejects unknown kind byte", "[opcmd][err]") {
    std::array<std::uint8_t, 22> buf{};
    buf[0] = 99;  // not in enum
    REQUIRE_FALSE(deserialize_operator_command(buf.data(), 22).has_value());

    buf[0] = static_cast<std::uint8_t>(OperatorCommandKind::None);
    REQUIRE(deserialize_operator_command(buf.data(), 22).has_value());
}

TEST_CASE("Operator command: deserialize rejects non-zero reserved byte", "[opcmd][err]") {
    OperatorCommand cmd{OperatorCommandKind::LockTarget, 1, 0, 0, 0};
    auto enc = serialize_operator_command(cmd);
    REQUIRE(deserialize_operator_command(enc.data(), enc.size()).has_value());

    enc[1] = 0xFF;  // corrupt reserved byte
    REQUIRE_FALSE(deserialize_operator_command(enc.data(), enc.size()).has_value());
}
