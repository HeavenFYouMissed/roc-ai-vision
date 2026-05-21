#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "roc/transport/wire_format.hpp"

#include <cstring>

using Catch::Matchers::WithinAbs;

TEST_CASE("pack/unpack bg odometry round-trip", "[wire_format]") {
    roc::transport::BgOdometryWirePayload in{};
    in.flags          = 0x05;
    in.feature_count  = 180;
    in.inlier_count   = 142;
    in.frame_id       = 4242;
    in.ts_ns          = 0x0102030405060708ULL;
    in.bg_dx_px       = 1.25f;
    in.bg_dy_px       = -0.5f;
    in.bg_theta_rad   = 0.02f;
    in.confidence_q15 = roc::transport::float_to_q15(0.75f);

    const auto bytes = roc::transport::pack_bg_odometry(in);
    REQUIRE(bytes[0] == roc::transport::kBgOdometryWireVersion);
    REQUIRE(bytes.size() == roc::transport::kBgOdometryWireBytes);

    roc::transport::BgOdometryWirePayload out{};
    REQUIRE(roc::transport::unpack_bg_odometry(bytes.data(), bytes.size(), out));
    REQUIRE(out.flags == in.flags);
    REQUIRE(out.feature_count == in.feature_count);
    REQUIRE(out.inlier_count == in.inlier_count);
    REQUIRE(out.frame_id == in.frame_id);
    REQUIRE(out.ts_ns == in.ts_ns);
    REQUIRE_THAT(out.bg_dx_px, WithinAbs(in.bg_dx_px, 1e-5f));
    REQUIRE_THAT(out.bg_dy_px, WithinAbs(in.bg_dy_px, 1e-5f));
    REQUIRE_THAT(out.bg_theta_rad, WithinAbs(in.bg_theta_rad, 1e-5f));
    REQUIRE(out.confidence_q15 == in.confidence_q15);
}

TEST_CASE("unpack rejects wrong version", "[wire_format]") {
    auto bytes = roc::transport::pack_bg_odometry({});
    bytes[0] = 99;
    roc::transport::BgOdometryWirePayload out{};
    REQUIRE_FALSE(roc::transport::unpack_bg_odometry(
        bytes.data(), bytes.size(), out));
}
