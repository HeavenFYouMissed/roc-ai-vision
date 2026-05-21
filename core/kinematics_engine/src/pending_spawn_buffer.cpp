// =============================================================================
// roc::kinematics::pending_spawn_buffer — fixed-capacity DEFERRED detection
// holding pen, §3.10.3.
// =============================================================================

#include "roc/kinematics/pending_spawn_buffer.hpp"

#include <algorithm>

namespace roc::kinematics {

void PendingSpawnBuffer::clear() noexcept
{
    for (auto& s : slots_) s.occupied = false;
}

int PendingSpawnBuffer::size() const noexcept
{
    int n = 0;
    for (const auto& s : slots_) if (s.occupied) ++n;
    return n;
}

bool PendingSpawnBuffer::insert(const Detection& det) noexcept
{
    // Find a free slot; if none, evict the oldest.
    int free_slot = -1;
    int oldest_slot = 0;
    int oldest_age  = -1;
    for (int i = 0; i < kCapacity; ++i) {
        if (!slots_[static_cast<std::size_t>(i)].occupied) {
            free_slot = i;
            break;
        }
        if (slots_[static_cast<std::size_t>(i)].age_frames > oldest_age) {
            oldest_age  = slots_[static_cast<std::size_t>(i)].age_frames;
            oldest_slot = i;
        }
    }
    int target = free_slot >= 0 ? free_slot : oldest_slot;
    slots_[static_cast<std::size_t>(target)].det = det;
    slots_[static_cast<std::size_t>(target)].age_frames = 0;
    slots_[static_cast<std::size_t>(target)].occupied = true;
    return free_slot >= 0;
}

void PendingSpawnBuffer::tick() noexcept
{
    for (auto& s : slots_) {
        if (!s.occupied) continue;
        ++s.age_frames;
        if (s.age_frames >= kHoldFrames) {
            s.occupied = false;
        }
    }
}

int PendingSpawnBuffer::find_match(const Detection& det) const noexcept
{
    int best_idx = -1;
    float best_dist = kCosineMatchThreshold + 1e-6f;   // strict-less than
    for (int i = 0; i < kCapacity; ++i) {
        if (!slots_[static_cast<std::size_t>(i)].occupied) continue;
        const auto& other = slots_[static_cast<std::size_t>(i)].det;
        // Cosine distance via dot product on L2-normalised embeddings.
        const float dot = other.appearance.dot(det.appearance);
        const float dist = 1.0f - dot;
        if (dist < best_dist) {
            best_dist = dist;
            best_idx = i;
        }
    }
    return best_idx;
}

void PendingSpawnBuffer::erase(int slot) noexcept
{
    if (slot < 0 || slot >= kCapacity) return;
    slots_[static_cast<std::size_t>(slot)].occupied = false;
}

}  // namespace roc::kinematics
