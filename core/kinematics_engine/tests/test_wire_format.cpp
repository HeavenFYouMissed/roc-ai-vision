// =============================================================================
// test_wire_format — verifies the §7 binary contract: 12 B header + 86 B/track,
// round-trip identity, byte-for-byte expectations on the layout.
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "roc/kinematics/wire_format.hpp"
#include "roc/kinematics/track.hpp"
#include "roc/kinematics/object_priors.hpp"

using namespace roc::kinematics;

namespace {

Track build_track_for_wire(TrackId id, Label label, double conf,
                           const Eigen::Vector3d& p_world,
                           bool boundary_probation = false,
                           std::optional<TrackId> parent = std::nullopt)
{
    Track t;
    t.id = id;
    t.label = label;
    t.parent_id = parent;
    t.status = TrackStatus::Confirmed;
    t.last_confidence = conf;
    t.last_bbox_u = 320.0;
    t.last_bbox_v = 240.0;
    t.last_bbox_w = 50.0;
    t.last_bbox_h = 100.0;
    State x = State::Zero();
    x.head<3>() = p_world;
    x.segment<3>(3) << 0.1, -0.1, 0.05;
    x.tail<3>()      << 0.01, -0.02, 0.005;
    StateCov P = StateCov::Identity() * 0.5;
    t.filter = CaEkf(x, P, qc_diagonal_for(label));
    t.regime_detector.reset();
    t.is_boundary_probation = boundary_probation;
    return t;
}

}  // namespace

TEST_CASE("Header is exactly 12 bytes, per-track is exactly 86 bytes (§7.1, §7.2)",
          "[wire][layout]") {
    REQUIRE(kWireHeaderSize == 12);
    REQUIRE(kWireTrackSize  == 86);
}

TEST_CASE("Round-trip serialise → deserialise matches", "[wire][round_trip]") {
    std::vector<Track> tracks;
    tracks.push_back(build_track_for_wire(TrackId(101), Label::Person, 0.92, {1.0, 0.5, 4.0}));
    tracks.push_back(build_track_for_wire(TrackId(102), Label::Head,   0.81, {1.0, 0.5, 4.4},
                                          /*probation=*/true, TrackId(101)));

    FrameHeader h;
    h.version    = kWireProtocolVersion;
    h.flags      = wire_frame_flag_bits::kBoundaryPresent;
    h.frame_id   = 12345;
    h.num_tracks = 2;
    h.t_capture_ms_offset = -987;

    auto buf = serialize_frame(h, std::span<const Track>(tracks.data(), tracks.size()));
    REQUIRE(buf.size() == static_cast<std::size_t>(kWireHeaderSize + 2 * kWireTrackSize));

    auto decoded = deserialize_frame(std::span<const std::uint8_t>(buf.data(), buf.size()));
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->header.version    == kWireProtocolVersion);
    REQUIRE(decoded->header.flags      == wire_frame_flag_bits::kBoundaryPresent);
    REQUIRE(decoded->header.frame_id   == 12345);
    REQUIRE(decoded->header.num_tracks == 2);
    REQUIRE(decoded->header.t_capture_ms_offset == -987);
    REQUIRE(decoded->tracks.size()     == 2);

    REQUIRE(decoded->tracks[0].track_id  == TrackId(101));
    REQUIRE(decoded->tracks[0].parent_id == TrackId(0));
    REQUIRE(decoded->tracks[0].label     == Label::Person);
    REQUIRE((decoded->tracks[0].status_flags & wire_status_bits::kConfirmed));
    REQUIRE_FALSE(decoded->tracks[0].status_flags & wire_status_bits::kBoundaryProbation);
    REQUIRE(std::abs(decoded->tracks[0].confidence - 0.92f) < 1e-6);
    REQUIRE(std::abs(decoded->tracks[0].p_world[0] - 1.0f) < 1e-6);
    REQUIRE(std::abs(decoded->tracks[0].p_world[1] - 0.5f) < 1e-6);
    REQUIRE(std::abs(decoded->tracks[0].p_world[2] - 4.0f) < 1e-6);
    REQUIRE(std::abs(decoded->tracks[0].bbox_orig[0] - 320.0f) < 1e-6);
    REQUIRE(std::abs(decoded->tracks[0].bbox_orig[3] - 100.0f) < 1e-6);

    REQUIRE(decoded->tracks[1].track_id  == TrackId(102));
    REQUIRE(decoded->tracks[1].parent_id == TrackId(101));
    REQUIRE(decoded->tracks[1].label     == Label::Head);
    REQUIRE(decoded->tracks[1].status_flags & wire_status_bits::kBoundaryProbation);
}

TEST_CASE("Truncated input is rejected", "[wire][round_trip]") {
    std::vector<std::uint8_t> short_buf(5, 0);
    auto r = deserialize_frame(std::span<const std::uint8_t>(short_buf.data(), short_buf.size()));
    REQUIRE_FALSE(r.has_value());
}

TEST_CASE("Empty track list still encodes a valid header", "[wire][round_trip]") {
    FrameHeader h;
    h.version = kWireProtocolVersion;
    h.frame_id = 1;
    h.num_tracks = 0;
    auto buf = serialize_frame(h, {});
    REQUIRE(buf.size() == static_cast<std::size_t>(kWireHeaderSize));
    auto decoded = deserialize_frame(std::span<const std::uint8_t>(buf.data(), buf.size()));
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->header.frame_id == 1);
    REQUIRE(decoded->tracks.empty());
}

// Phase-5 — verifies the zero-copy indexed serializer used by the Tracker
// egress path produces byte-for-byte identical output to the contiguous form.
TEST_CASE("serialize_frame_indexed is byte-identical to contiguous form on a subset",
          "[wire][indexed]") {
    std::vector<Track> pool;
    pool.push_back(build_track_for_wire(TrackId(1), Label::Person,    0.9, {2.0, 1.0, 5.0}));
    pool.push_back(build_track_for_wire(TrackId(2), Label::Head,      0.8, {2.0, 1.0, 5.4},
                                         /*probation=*/false, TrackId(1)));
    pool.push_back(build_track_for_wire(TrackId(3), Label::UpperTorso, 0.7, {2.0, 1.0, 5.05},
                                         /*probation=*/true,  TrackId(1)));

    // Pick indices {0, 2} — emulates the Tracker filtering "confirmed only"
    // tracks for wire emission without copying.
    const std::vector<int> picked{0, 2};

    FrameHeader h;
    h.version    = kWireProtocolVersion;
    h.flags      = wire_frame_flag_bits::kBoundaryPresent;
    h.frame_id   = 777;
    h.num_tracks = static_cast<std::uint16_t>(picked.size());
    h.t_capture_ms_offset = 4242;

    // Reference: build a contiguous subset by copying just the picked tracks
    // (the legacy egress path).  Compare against the indexed output.
    std::vector<Track> subset;
    subset.reserve(picked.size());
    for (int idx : picked) subset.push_back(pool[static_cast<std::size_t>(idx)]);
    const auto ref_buf = serialize_frame(h, std::span<const Track>(subset.data(), subset.size()));

    std::vector<std::uint8_t> idx_buf(ref_buf.size(), 0);
    const std::size_t written = serialize_frame_indexed(
        std::span<std::uint8_t>(idx_buf.data(), idx_buf.size()), h,
        std::span<const Track>(pool.data(), pool.size()),
        std::span<const int>(picked.data(), picked.size()));
    REQUIRE(written == ref_buf.size());
    REQUIRE(idx_buf == ref_buf);

    // And it round-trips back to (1, 3) — verifies the indices did pick the
    // right tracks, not an off-by-one.
    auto decoded = deserialize_frame(std::span<const std::uint8_t>(idx_buf.data(), idx_buf.size()));
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->tracks.size() == 2);
    REQUIRE(decoded->tracks[0].track_id == TrackId(1));
    REQUIRE(decoded->tracks[1].track_id == TrackId(3));
    REQUIRE(decoded->tracks[1].parent_id == TrackId(1));
    REQUIRE((decoded->tracks[1].status_flags & wire_status_bits::kBoundaryProbation));
}

TEST_CASE("serialize_frame_indexed rejects undersized output buffer",
          "[wire][indexed]") {
    std::vector<Track> pool;
    pool.push_back(build_track_for_wire(TrackId(7), Label::Person, 0.5, {0, 0, 3}));
    const std::vector<int> picked{0};
    FrameHeader h;
    h.num_tracks = 1;
    std::vector<std::uint8_t> too_small(kWireHeaderSize + kWireTrackSize - 1, 0);
    const std::size_t written = serialize_frame_indexed(
        std::span<std::uint8_t>(too_small.data(), too_small.size()), h,
        std::span<const Track>(pool.data(), pool.size()),
        std::span<const int>(picked.data(), picked.size()));
    REQUIRE(written == 0);
}
