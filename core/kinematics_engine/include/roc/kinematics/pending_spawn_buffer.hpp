#pragma once

// =============================================================================
// roc::kinematics::pending_spawn_buffer
//
// Fixed-capacity buffer for §3.10.3 DEFERRED detections — items live for at
// most T_hold = 5 frames before being dropped.  Each entry is keyed by the
// detection's appearance embedding so that a follow-up detection (which now
// satisfies one of the first three §3.10.3 cases) can be matched back to it
// via cosine similarity and promoted into a real track.
//
// The buffer is intentionally a hand-rolled fixed-size vector (no heap
// allocation past construction) — it sits on the tracker hot path.
// =============================================================================

#include "roc/kinematics/types.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace roc::kinematics {

class PendingSpawnBuffer {
public:
    static constexpr int kCapacity = 32;
    static constexpr int kHoldFrames = 5;          // §3.10.3 T_hold
    static constexpr float kCosineMatchThreshold = 0.4f;  // 1 - cosθ ≤ 0.4

    struct Entry {
        Detection    det{};
        std::int32_t age_frames = 0;
        bool         occupied = false;
    };

    void clear() noexcept;
    int  size() const noexcept;
    int  capacity() const noexcept { return kCapacity; }

    // Insert a deferred detection.  Returns true if accepted, false if the
    // buffer was full (oldest entry evicted to make room).
    bool insert(const Detection& det) noexcept;

    // Tick all live entries by one frame; drop those that age out.
    void tick() noexcept;

    // Find the closest match for `det.appearance` among live entries.
    // Returns the matching slot index (-1 for no match, ≤ kCosineMatchThreshold).
    int find_match(const Detection& det) const noexcept;

    void erase(int slot) noexcept;

    const std::array<Entry, kCapacity>& slots() const noexcept { return slots_; }

private:
    std::array<Entry, kCapacity> slots_{};
};

}  // namespace roc::kinematics
