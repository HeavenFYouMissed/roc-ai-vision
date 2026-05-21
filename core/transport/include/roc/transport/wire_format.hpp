#pragma once

// =============================================================================
// roc::transport::wire_format — v2/v3 dashboard wire helpers (§12.4, §13.4).
// v1 track frames remain in roc::kinematics::wire_format (production-locked).
// =============================================================================

#include <array>
#include <cstdint>
#include <cstring>

namespace roc {
namespace transport {

inline constexpr std::size_t kBgOdometryWireBytes = 32;
inline constexpr std::uint8_t kBgOdometryWireVersion = 3;

struct BgOdometryWirePayload {
    std::uint8_t  flags           = 0;
    std::uint16_t feature_count   = 0;
    std::uint16_t inlier_count    = 0;
    std::uint32_t frame_id        = 0;
    std::uint64_t ts_ns           = 0;
    float         bg_dx_px        = 0.0f;
    float         bg_dy_px        = 0.0f;
    float         bg_theta_rad    = 0.0f;
    std::uint16_t confidence_q15  = 0;
};

inline void write_u8_le(std::uint8_t* p, std::uint8_t v) noexcept { *p = v; }
inline void write_u16_le(std::uint8_t* p, std::uint16_t v) noexcept {
    p[0] = static_cast<std::uint8_t>(v & 0xFFu);
    p[1] = static_cast<std::uint8_t>((v >> 8) & 0xFFu);
}
inline void write_u32_le(std::uint8_t* p, std::uint32_t v) noexcept {
    for (int i = 0; i < 4; ++i) {
        p[i] = static_cast<std::uint8_t>((v >> (8 * i)) & 0xFFu);
    }
}
inline void write_u64_le(std::uint8_t* p, std::uint64_t v) noexcept {
    for (int i = 0; i < 8; ++i) {
        p[i] = static_cast<std::uint8_t>((v >> (8 * i)) & 0xFFu);
    }
}
inline void write_f32_le(std::uint8_t* p, float v) noexcept {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &v, 4);
    write_u32_le(p, bits);
}

inline std::uint16_t float_to_q15(float c) noexcept {
    const float clamped = c < 0.0f ? 0.0f : (c > 1.0f ? 1.0f : c);
    return static_cast<std::uint16_t>(clamped * 32767.0f);
}

inline std::array<std::uint8_t, kBgOdometryWireBytes>
pack_bg_odometry(const BgOdometryWirePayload& p) noexcept {
    std::array<std::uint8_t, kBgOdometryWireBytes> out{};
    write_u8_le(out.data() + 0, kBgOdometryWireVersion);
    write_u8_le(out.data() + 1, p.flags);
    write_u16_le(out.data() + 2, p.feature_count);
    write_u16_le(out.data() + 4, p.inlier_count);
    write_u32_le(out.data() + 6, p.frame_id);
    write_u64_le(out.data() + 10, p.ts_ns);
    write_f32_le(out.data() + 18, p.bg_dx_px);
    write_f32_le(out.data() + 22, p.bg_dy_px);
    write_f32_le(out.data() + 26, p.bg_theta_rad);
    write_u16_le(out.data() + 30, p.confidence_q15);
    return out;
}

inline bool unpack_bg_odometry(
        const std::uint8_t* data,
        std::size_t size,
        BgOdometryWirePayload& out) noexcept {
    if (size < kBgOdometryWireBytes || data[0] != kBgOdometryWireVersion) {
        return false;
    }
    out.flags = data[1];
    out.feature_count =
        static_cast<std::uint16_t>(data[2]) | (static_cast<std::uint16_t>(data[3]) << 8);
    out.inlier_count =
        static_cast<std::uint16_t>(data[4]) | (static_cast<std::uint16_t>(data[5]) << 8);
    std::uint32_t fid = 0;
    std::memcpy(&fid, data + 6, 4);
    out.frame_id = fid;
    std::uint64_t ts = 0;
    std::memcpy(&ts, data + 10, 8);
    out.ts_ns = ts;
    std::memcpy(&out.bg_dx_px, data + 18, 4);
    std::memcpy(&out.bg_dy_px, data + 22, 4);
    std::memcpy(&out.bg_theta_rad, data + 26, 4);
    out.confidence_q15 =
        static_cast<std::uint16_t>(data[30]) | (static_cast<std::uint16_t>(data[31]) << 8);
    return true;
}

}  // namespace transport
}  // namespace roc
