#pragma once

// =============================================================================
// roc::kinematics::track
//
// Track aggregate: filter + regime detector + appearance gallery + lifecycle
// counters.  Owned by the Tracker (§5).
//
// The appearance gallery is a hand-rolled fixed-capacity ring buffer of
// L_max = 100 entries (§4.1) — no boost::circular_buffer dependency, no heap
// allocation after construction.
// =============================================================================

#include "roc/kinematics/types.hpp"
#include "roc/kinematics/ca_ekf.hpp"
#include "roc/kinematics/regime_detector.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace roc::kinematics {

// ---------------------------------------------------------------------------
// Fixed-capacity FIFO ring of L2-normalised embeddings (§4.1).
// ---------------------------------------------------------------------------
class EmbeddingGallery {
public:
    static constexpr int kMaxSize = 100;     // §4.1 L_max

    void clear() noexcept { size_ = 0; head_ = 0; }

    void push(const Embedding& f) noexcept {
        if (size_ < kMaxSize) {
            buf_[static_cast<std::size_t>((head_ + size_) % kMaxSize)] = f;
            ++size_;
        } else {
            buf_[static_cast<std::size_t>(head_)] = f;
            head_ = (head_ + 1) % kMaxSize;
        }
    }

    int size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }

    // Read entry by FIFO order (0 = oldest, size-1 = newest).
    const Embedding& at(int i) const noexcept {
        return buf_[static_cast<std::size_t>((head_ + i) % kMaxSize)];
    }
    Embedding& at(int i) noexcept {
        return buf_[static_cast<std::size_t>((head_ + i) % kMaxSize)];
    }

    // §4.7 — copy the gallery into a contiguous (rows × 128) Eigen block.
    // Caller supplies the destination row-block; we write `size_` rows.
    template <class Derived>
    void copy_to_rows(Eigen::MatrixBase<Derived>& out) const noexcept {
        for (int i = 0; i < size_; ++i) {
            out.row(i) = at(i).transpose();
        }
    }

private:
    // EIGEN_ALIGN_MAX so 128-D float vectors enjoy 32-byte alignment when
    // packed into the outer std::array.
    alignas(32) std::array<Embedding, static_cast<std::size_t>(kMaxSize)> buf_{};
    int size_ = 0;
    int head_ = 0;
};

// ---------------------------------------------------------------------------
// Track lifecycle states (§4.6).
// ---------------------------------------------------------------------------
enum class TrackStatus : std::uint8_t {
    Tentative = 0,    // hits < kConfirmHits
    Confirmed = 1,    // hits >= kConfirmHits
    Deleted   = 2,    // ready for harvest
};

struct LifecycleConfig {
    int confirm_hits = 3;        // §4.6 — hits ≥ 3 to confirm
    int max_age      = 30;       // §4.4 A_max
};

// ---------------------------------------------------------------------------
// Track — the unit on which prediction, association, update, and the
// hierarchical constraint operate.
// ---------------------------------------------------------------------------
struct Track {
    // Identity / topology
    TrackId                id{0};
    std::optional<TrackId> parent_id{};      // §3.11 / §7.2
    Label                  label = Label::Unknown;

    // Filter + adaptive Q
    CaEkf                  filter{};
    NisRegimeDetector      regime_detector{};

    // Appearance
    EmbeddingGallery       gallery{};

    // Last-seen detection bookkeeping (the wire frame quotes `bbox_orig`
    // straight from the most recent matched detection, §7.2).
    double                 last_bbox_u  = 0.0;
    double                 last_bbox_v  = 0.0;
    double                 last_bbox_w  = 0.0;
    double                 last_bbox_h  = 0.0;
    double                 last_confidence = 0.0;

    // Lifecycle / cascade counters
    int                    age = 0;
    int                    hits = 0;
    int                    time_since_update = 0;
    int                    frames_since_spawn = 0;
    TrackStatus            status = TrackStatus::Tentative;

    // §3.10.4 boundary probation
    bool                   is_boundary_probation = false;
    int                    consecutive_in_frame_detections = 0;

    // §3.11.4 `hierarchy_violation` event flag.  Set when the constraint
    // gate rejects a parent↔sub-feature pair this frame; cleared at the
    // next valid update.  Surfaced through the wire format (§7.2 status_flags
    // bit 3) so the operator can see drift.
    bool                   hierarchy_violation_pending = false;

    // Diagnostic counters (§3.10.5)
    std::uint32_t          boundary_init_count = 0;
    std::uint32_t          disagree_init_count = 0;
    std::uint32_t          probation_frames_total = 0;

    // ---------- Methods ----------------------------------------------------
    // §2.5 prediction (caller has clamped Δt).
    void predict(double dt_clamped) noexcept {
        filter.predict(dt_clamped, regime_detector.qc_scale());
        ++age;
        ++time_since_update;
        ++frames_since_spawn;
        if (is_boundary_probation) {
            ++probation_frames_total;
        }
    }

    // §4.6 lifecycle book-keeping after a successful detection→track match.
    void on_match() noexcept {
        ++hits;
        time_since_update = 0;
    }

    // §4.6 lifecycle when a confirmed track failed to match this frame.
    void on_unmatched() noexcept {
        // age / time_since_update already advanced in predict().  Nothing else
        // to do here — the lifecycle gate decides deletion (§5).
    }

    bool is_confirmed() const noexcept {
        return status == TrackStatus::Confirmed;
    }

    void promote_if_eligible(const LifecycleConfig& cfg) noexcept {
        if (status == TrackStatus::Tentative && hits >= cfg.confirm_hits) {
            status = TrackStatus::Confirmed;
        }
    }
};

}  // namespace roc::kinematics
