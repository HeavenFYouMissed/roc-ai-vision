// =============================================================================
// roc::transport::operator_command  —  serialise / deserialise
//
// Fixed 22-byte little-endian wire format documented in operator_command.hpp.
// No allocations, no exceptions, no dependencies beyond the standard library.
// =============================================================================

#include "roc/transport/operator_command.hpp"

#include <cstring>
#include <type_traits>

namespace roc {
namespace transport {

namespace {

// Little-endian writer / reader helpers.  std::memcpy avoids any strict-
// aliasing / alignment concern; on x86-64 g++ optimises these into a single
// MOV instruction at -O3.
template <class T>
inline void write_le(std::uint8_t* dst, const T& v) noexcept {
    static_assert(std::is_trivially_copyable_v<T>, "write_le requires a POD type");
    std::memcpy(dst, &v, sizeof(T));
}

template <class T>
inline T read_le(const std::uint8_t* src) noexcept {
    static_assert(std::is_trivially_copyable_v<T>, "read_le requires a POD type");
    T v{};
    std::memcpy(&v, src, sizeof(T));
    return v;
}

}  // namespace

std::array<std::uint8_t, kOperatorCommandWireSize>
serialize_operator_command(const OperatorCommand& cmd) noexcept {
    std::array<std::uint8_t, kOperatorCommandWireSize> out{};
    out[0] = static_cast<std::uint8_t>(cmd.kind);
    out[1] = 0u;  // reserved
    write_le<std::uint64_t>(out.data() + 2, cmd.target_track_id);
    if (cmd.kind >= OperatorCommandKind::SetSmoothingAlpha) {
        write_le<float>(out.data() + 10, cmd.value);
        write_le<float>(out.data() + 14, 0.0f);
        write_le<float>(out.data() + 18, 0.0f);
    } else {
        write_le<float>(out.data() + 10, cmd.weight_proximity);
        write_le<float>(out.data() + 14, cmd.weight_confidence);
        write_le<float>(out.data() + 18, cmd.weight_stability);
    }
    return out;
}

std::optional<OperatorCommand>
deserialize_operator_command(const std::uint8_t* data, std::size_t size) noexcept {
    if (data == nullptr || size != kOperatorCommandWireSize) {
        return std::nullopt;
    }

    const std::uint8_t kind_byte = data[0];
    const std::uint8_t reserved  = data[1];
    if (reserved != 0u) {
        return std::nullopt;
    }

    OperatorCommand cmd{};
    switch (kind_byte) {
        case static_cast<std::uint8_t>(OperatorCommandKind::None):
        case static_cast<std::uint8_t>(OperatorCommandKind::LockTarget):
        case static_cast<std::uint8_t>(OperatorCommandKind::ReleaseLock):
        case static_cast<std::uint8_t>(OperatorCommandKind::UpdateSelectorWeights):
        case static_cast<std::uint8_t>(OperatorCommandKind::SetSmoothingAlpha):
        case static_cast<std::uint8_t>(OperatorCommandKind::SetDeadbandRadiusPx):
        case static_cast<std::uint8_t>(OperatorCommandKind::SetFovRadiusPx):
        case static_cast<std::uint8_t>(OperatorCommandKind::SetTargetingAnchor):
            cmd.kind = static_cast<OperatorCommandKind>(kind_byte);
            break;
        default:
            return std::nullopt;  // unknown kind — schema mismatch, reject.
    }

    cmd.target_track_id = read_le<std::uint64_t>(data + 2);
    if (cmd.kind == OperatorCommandKind::UpdateSelectorWeights) {
        cmd.weight_proximity  = read_le<float>(data + 10);
        cmd.weight_confidence = read_le<float>(data + 14);
        cmd.weight_stability  = read_le<float>(data + 18);
        cmd.value             = 0.0f;
    } else if (cmd.kind >= OperatorCommandKind::SetSmoothingAlpha) {
        cmd.value            = read_le<float>(data + 10);
        cmd.weight_proximity = 0.0f;
        cmd.weight_confidence = 0.0f;
        cmd.weight_stability  = 0.0f;
    } else {
        cmd.weight_proximity  = read_le<float>(data + 10);
        cmd.weight_confidence = read_le<float>(data + 14);
        cmd.weight_stability  = read_le<float>(data + 18);
        cmd.value             = 0.0f;
    }

    return cmd;
}

}  // namespace transport
}  // namespace roc
