#pragma once

// =============================================================================
// roc::kinematics::wire_format
//
// Binary §7 telemetry contract.  Frame layout:
//
//   Header (12 bytes, §7.1):
//     u8  version
//     u8  flags                  bit 0 = NIS-degraded frame, bit 1 = boundary probation present
//     u32 frame_id
//     u16 num_tracks
//     i32 t_capture_ms_offset    (signed offset from session epoch in ms)
//
//   Per-track payload (86 bytes, §7.2):
//     u64 track_id
//     u64 parent_id
//     u8  label
//     u8  status_flags           bit0=confirmed, bit1=boundary_probation,
//                                bit2=stationary regime, bit3=hierarchy violation pending
//     f32 confidence
//     f32 p_world[3]   (12 B)
//     f32 v_world[3]   (12 B)
//     f32 a_world[3]   (12 B)
//     f32 P_pos_diag[3] (12 B)
//     f32 bbox_orig[4]  (16 B)
//
//   Per-track total = 8 + 8 + 1 + 1 + 4 + 12 + 12 + 12 + 12 + 16 = 86 bytes ✓
//
// Note vs. blueprint draft: the §7.2 "P_pos_offnorm" Frobenius-norm field is
// dropped to honour the binding 86-byte/track total claimed in both §7.2 and
// the user's hard wire-contract requirement (the blueprint table summed to
// 90 B; we side with the 86-byte totals which the SvelteKit decoder spec
// already targets and which the §7.2 traffic budget calculation ratifies).
// This is logged as a non-fatal deviation in the roadmap.
//
// All multi-byte fields are little-endian (the deployment target is x86-64).
// =============================================================================

#include "roc/kinematics/types.hpp"
#include "roc/kinematics/track.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace roc::kinematics {

inline constexpr int kWireHeaderSize = 12;
inline constexpr int kWireTrackSize  = 86;
inline constexpr std::uint8_t kWireProtocolVersion = 1;

namespace wire_status_bits {
    inline constexpr std::uint8_t kConfirmed            = 1u << 0;
    inline constexpr std::uint8_t kBoundaryProbation    = 1u << 1;
    inline constexpr std::uint8_t kStationaryRegime     = 1u << 2;
    inline constexpr std::uint8_t kHierarchyViolation   = 1u << 3;
}

namespace wire_frame_flag_bits {
    inline constexpr std::uint8_t kNisDegraded          = 1u << 0;
    inline constexpr std::uint8_t kBoundaryPresent      = 1u << 1;
}

struct FrameHeader {
    std::uint8_t  version    = kWireProtocolVersion;
    std::uint8_t  flags      = 0;
    std::uint32_t frame_id   = 0;
    std::uint16_t num_tracks = 0;
    std::int32_t  t_capture_ms_offset = 0;
};

// Serialise into an existing buffer.  Returns the number of bytes written.
// `out_buf` must satisfy size() ≥ kWireHeaderSize + tracks.size() * kWireTrackSize.
std::size_t serialize_frame(std::span<std::uint8_t> out_buf,
                            const FrameHeader& header,
                            std::span<const Track> tracks);

// Convenience overload: writes into a `std::vector<uint8_t>` (sized inside).
std::vector<std::uint8_t> serialize_frame(const FrameHeader& header,
                                          std::span<const Track> tracks);

// Phase-5 zero-copy variant: serialises a subset of `track_pool` selected by
// `track_indices` (indices into the pool).  This avoids the per-frame copy of
// the full Track aggregate (each Track holds a ~51 KB EmbeddingGallery) when
// emitting the wire frame from the Tracker hot path.  `header.num_tracks`
// MUST equal `track_indices.size()` — the function does NOT mutate the header.
std::size_t serialize_frame_indexed(std::span<std::uint8_t> out_buf,
                                    const FrameHeader& header,
                                    std::span<const Track> track_pool,
                                    std::span<const int> track_indices);

// ---------------------------------------------------------------------------
// Decoder.  The decoded payload mirrors the source Track only as far as the
// fields the wire frame carries — it is *not* a full Track restoration.
// Used by tests for round-trip verification.
// ---------------------------------------------------------------------------
struct DecodedTrackPayload {
    TrackId      track_id{0};
    TrackId      parent_id{0};
    Label        label = Label::Unknown;
    std::uint8_t status_flags = 0;
    float        confidence = 0.0f;
    float        p_world[3]{};
    float        v_world[3]{};
    float        a_world[3]{};
    float        P_pos_diag[3]{};
    float        bbox_orig[4]{};
};

struct DecodedFrame {
    FrameHeader                       header{};
    std::vector<DecodedTrackPayload>  tracks;
};

// Returns std::nullopt on malformed / truncated input.
std::optional<DecodedFrame> deserialize_frame(std::span<const std::uint8_t> in);

}  // namespace roc::kinematics
