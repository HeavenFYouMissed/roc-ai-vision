// =============================================================================
// roc::kinematics::wire_format — §7 binary serialiser / deserialiser.
//
// Endianness: little-endian (deployment target is x86-64).  We perform the
// byte-level packing via std::memcpy so we side-step UB from misaligned
// accesses or strict-aliasing violations.
// =============================================================================

#include "roc/kinematics/wire_format.hpp"
#include "roc/kinematics/regime_detector.hpp"

#include <cstring>
#include <limits>

namespace roc::kinematics {

namespace {

template <class T>
void write_le(std::uint8_t* dst, const T& v) noexcept
{
    static_assert(std::is_trivially_copyable_v<T>);
    std::memcpy(dst, &v, sizeof(T));
}

template <class T>
T read_le(const std::uint8_t* src) noexcept
{
    static_assert(std::is_trivially_copyable_v<T>);
    T v;
    std::memcpy(&v, src, sizeof(T));
    return v;
}

std::uint8_t status_flags_for(const Track& t) noexcept
{
    std::uint8_t f = 0;
    if (t.is_confirmed())                       f |= wire_status_bits::kConfirmed;
    if (t.is_boundary_probation)                f |= wire_status_bits::kBoundaryProbation;
    if (t.regime_detector.regime() == Regime::Stationary)
        f |= wire_status_bits::kStationaryRegime;
    if (t.hierarchy_violation_pending)          f |= wire_status_bits::kHierarchyViolation;
    return f;
}

}  // namespace

namespace {

// Shared per-track encoder used by both the contiguous and indexed paths.
// Writes exactly `kWireTrackSize` bytes starting at `base`.  All offsets are
// hand-derived from §7.2 and asserted by the static_assert below.
void encode_track_payload(std::uint8_t* base, const Track& t) noexcept
{
    write_le<std::uint64_t>(base + 0, t.id.value);
    const std::uint64_t parent_id_value =
        t.parent_id.has_value() ? t.parent_id->value : std::uint64_t{0};
    write_le<std::uint64_t>(base + 8, parent_id_value);
    write_le<std::uint8_t>(base + 16, static_cast<std::uint8_t>(t.label));
    write_le<std::uint8_t>(base + 17, status_flags_for(t));

    const float conf = static_cast<float>(t.last_confidence);
    write_le<float>(base + 18, conf);

    const Position     p_w = t.filter.position();
    const Velocity     v_w = t.filter.velocity();
    const Acceleration a_w = t.filter.acceleration();
    const float p_arr[3] = {static_cast<float>(p_w(0)), static_cast<float>(p_w(1)), static_cast<float>(p_w(2))};
    const float v_arr[3] = {static_cast<float>(v_w(0)), static_cast<float>(v_w(1)), static_cast<float>(v_w(2))};
    const float a_arr[3] = {static_cast<float>(a_w(0)), static_cast<float>(a_w(1)), static_cast<float>(a_w(2))};
    std::memcpy(base + 22, p_arr, 12);
    std::memcpy(base + 34, v_arr, 12);
    std::memcpy(base + 46, a_arr, 12);

    const StateCov& P = t.filter.cov();
    const float pp_diag[3] = {
        static_cast<float>(P(0, 0)),
        static_cast<float>(P(1, 1)),
        static_cast<float>(P(2, 2)),
    };
    std::memcpy(base + 58, pp_diag, 12);

    const float bbox[4] = {
        static_cast<float>(t.last_bbox_u),
        static_cast<float>(t.last_bbox_v),
        static_cast<float>(t.last_bbox_w),
        static_cast<float>(t.last_bbox_h),
    };
    std::memcpy(base + 70, bbox, 16);

    static_assert(kWireTrackSize == 86, "wire track size must match §7.2");
}

void encode_header(std::uint8_t* p, const FrameHeader& header) noexcept
{
    write_le<std::uint8_t>(p + 0, header.version);
    write_le<std::uint8_t>(p + 1, header.flags);
    write_le<std::uint32_t>(p + 2, header.frame_id);
    write_le<std::uint16_t>(p + 6, header.num_tracks);
    write_le<std::int32_t>(p + 8, header.t_capture_ms_offset);
}

}  // namespace

std::size_t serialize_frame(std::span<std::uint8_t> out_buf,
                            const FrameHeader& header,
                            std::span<const Track> tracks)
{
    const std::size_t need = static_cast<std::size_t>(kWireHeaderSize)
                           + tracks.size() * static_cast<std::size_t>(kWireTrackSize);
    if (out_buf.size() < need) return 0;

    std::uint8_t* p = out_buf.data();
    encode_header(p, header);
    p += kWireHeaderSize;
    for (const Track& t : tracks) {
        encode_track_payload(p, t);
        p += kWireTrackSize;
    }
    return need;
}

std::vector<std::uint8_t> serialize_frame(const FrameHeader& header,
                                          std::span<const Track> tracks)
{
    std::vector<std::uint8_t> buf(static_cast<std::size_t>(kWireHeaderSize)
                                  + tracks.size() * static_cast<std::size_t>(kWireTrackSize));
    serialize_frame(std::span<std::uint8_t>(buf.data(), buf.size()), header, tracks);
    return buf;
}

std::size_t serialize_frame_indexed(std::span<std::uint8_t> out_buf,
                                    const FrameHeader& header,
                                    std::span<const Track> track_pool,
                                    std::span<const int> track_indices)
{
    const std::size_t need = static_cast<std::size_t>(kWireHeaderSize)
                           + track_indices.size() * static_cast<std::size_t>(kWireTrackSize);
    if (out_buf.size() < need) return 0;

    std::uint8_t* p = out_buf.data();
    encode_header(p, header);
    p += kWireHeaderSize;
    for (int idx : track_indices) {
        // Caller guarantees idx is in range; defensive bound just in case.
        if (idx < 0 || static_cast<std::size_t>(idx) >= track_pool.size()) {
            // Zero-fill an unknown slot so the wire frame stays well-formed.
            std::memset(p, 0, static_cast<std::size_t>(kWireTrackSize));
        } else {
            encode_track_payload(p, track_pool[static_cast<std::size_t>(idx)]);
        }
        p += kWireTrackSize;
    }
    return need;
}

std::optional<DecodedFrame> deserialize_frame(std::span<const std::uint8_t> in)
{
    if (in.size() < static_cast<std::size_t>(kWireHeaderSize)) return std::nullopt;
    const std::uint8_t* p = in.data();

    DecodedFrame f;
    f.header.version             = read_le<std::uint8_t>(p + 0);
    f.header.flags               = read_le<std::uint8_t>(p + 1);
    f.header.frame_id            = read_le<std::uint32_t>(p + 2);
    f.header.num_tracks          = read_le<std::uint16_t>(p + 6);
    f.header.t_capture_ms_offset = read_le<std::int32_t>(p + 8);
    p += kWireHeaderSize;

    const std::size_t expected = static_cast<std::size_t>(kWireHeaderSize)
                               + static_cast<std::size_t>(f.header.num_tracks)
                                 * static_cast<std::size_t>(kWireTrackSize);
    if (in.size() < expected) return std::nullopt;

    f.tracks.reserve(f.header.num_tracks);
    for (std::uint16_t i = 0; i < f.header.num_tracks; ++i) {
        const std::uint8_t* base = p;
        DecodedTrackPayload r;
        r.track_id    = TrackId(read_le<std::uint64_t>(base + 0));
        r.parent_id   = TrackId(read_le<std::uint64_t>(base + 8));
        r.label       = static_cast<Label>(read_le<std::uint8_t>(base + 16));
        r.status_flags= read_le<std::uint8_t>(base + 17);
        r.confidence  = read_le<float>(base + 18);
        std::memcpy(r.p_world,    base + 22, 12);
        std::memcpy(r.v_world,    base + 34, 12);
        std::memcpy(r.a_world,    base + 46, 12);
        std::memcpy(r.P_pos_diag, base + 58, 12);
        std::memcpy(r.bbox_orig,  base + 70, 16);
        f.tracks.push_back(r);
        p += kWireTrackSize;
    }
    return f;
}

}  // namespace roc::kinematics
