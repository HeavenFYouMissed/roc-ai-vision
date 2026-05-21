#pragma once

// =============================================================================
// roc::transport::operator_command
//
// Reverse-channel command from the SvelteKit dashboard → Tracker thread, per
// `research_ipc_middleware_2026.md` §8.2 (Command Path).  The browser sends
// a small fixed-layout binary frame over the same WebSocket the dashboard is
// already listening to; the WS server thread deserialises and pushes into the
// `OperatorCommandQueue` SPSC that the tracker drains each step.
//
// Wire layout (little-endian, total = 22 bytes, fits in 1 cache line):
//
//   offset  size   field
//   ------  -----  --------------------------------------------------------
//        0     1   u8   kind          (OperatorCommandKind enum)
//        1     1   u8   reserved      (always 0 — alignment padding)
//        2     8   u64  target_track_id  (0 when not applicable)
//       10     4   f32  weight_proximity   (`kind == UpdateSelectorWeights`, §12.3),
//                                      OR `value` for `SetSmoothingAlpha` /
//                                         `SetDeadbandRadiusPx` / `SetFovRadiusPx` /
//                                         `SetTargetingAnchor` (anchor as f32 cast)
//       14     4   f32  weight_confidence  (zeros when `kind >= 4`)
//       18     4   f32  weight_stability   (zeros when `kind >= 4`)
//       22     ─   (end)
//
// `weight_*` are consulted when `kind == UpdateSelectorWeights`.  `value` when
// `kind` is §12.3 live-parameter commands (4–7).  Other legacy kinds carry zero
// in those fields.  The decoder validates total length = 22 so a future schema
// bump can be detected and rejected.
//
// All multi-byte fields are little-endian (the deployment target is x86-64;
// the dashboard runs the same protocol assumption via DataView reads).
// =============================================================================

#include <array>
#include <cstdint>
#include <optional>

namespace roc {
namespace transport {

// ---------------------------------------------------------------------------
// Command enum.  Stable wire-id mapping — DO NOT reorder.
// ---------------------------------------------------------------------------
enum class OperatorCommandKind : std::uint8_t {
    None                  = 0,
    LockTarget            = 1,   // payload: target_track_id
    ReleaseLock           = 2,   // payload: none
    UpdateSelectorWeights = 3,   // payload: weight_{proximity,confidence,stability}
    SetSmoothingAlpha     = 4,   // §12.3 payload: value (linear smoothing alpha)
    SetDeadbandRadiusPx   = 5,   // §12.3 payload: value (deadband radius px)
    SetFovRadiusPx        = 6,   // §12.3 payload: value (FOV radius px)
    SetTargetingAnchor    = 7,   // §12.3 payload: value as target anchor enum ordinal
};

inline constexpr std::size_t kOperatorCommandWireSize = 22;

// ---------------------------------------------------------------------------
// OperatorCommand — decoded form pushed into the SPSC queue.
// `track_id` mirrors `roc::kinematics::TrackId::value` (the strong typedef
// can't appear in a transport-layer POD because it would create a circular
// dependency with the kinematics_engine).  The orchestrator wraps the raw
// u64 in `TrackId{cmd.target_track_id}` when applying.
// ---------------------------------------------------------------------------
struct OperatorCommand {
    OperatorCommandKind kind              = OperatorCommandKind::None;
    std::uint64_t       target_track_id   = 0;
    float               weight_proximity  = 0.0f;
    float               weight_confidence = 0.0f;
    float               weight_stability  = 0.0f;
    /** Single-f32 §12.3 commands (`SetSmoothingAlpha` … `SetTargetingAnchor`). */
    float               value             = 0.0f;
};

// ---------------------------------------------------------------------------
// Serialise / deserialise.  Both are noexcept and never allocate.  The output
// buffer is a fixed-size `std::array<uint8_t, 22>` to encode the wire size
// at the type level.  `deserialize` returns `std::nullopt` on:
//   * truncated input (size != 22)
//   * unknown `kind` byte (forward-compat: drop instead of crash)
//   * non-zero reserved byte (catches schema drift early)
// ---------------------------------------------------------------------------
std::array<std::uint8_t, kOperatorCommandWireSize>
serialize_operator_command(const OperatorCommand& cmd) noexcept;

std::optional<OperatorCommand>
deserialize_operator_command(const std::uint8_t* data, std::size_t size) noexcept;

// Convenience overload — accepts any contiguous byte range with .data()/.size().
template <class ByteRange>
inline std::optional<OperatorCommand> deserialize_operator_command(const ByteRange& bytes) noexcept {
    return deserialize_operator_command(
        reinterpret_cast<const std::uint8_t*>(bytes.data()),
        bytes.size());
}

}  // namespace transport
}  // namespace roc
