#pragma once

// =============================================================================
// roc::kinematics::tracker
//
// Top-level orchestrator implementing the §5 per-frame pipeline in the exact
// order:
//   Prediction → Association → Update → Stationary-regime shrinkage →
//   Hierarchical constraint → Boundary-probation maintenance → Lifecycle →
//   Egress (binary §7 wire frame).
//
// The order is binding (re-ordering breaks §3.11.4 which depends on
// `time_since_update == 0` post-update).
// =============================================================================

#include "roc/kinematics/types.hpp"
#include "roc/kinematics/track.hpp"
#include "roc/kinematics/association.hpp"
#include "roc/kinematics/pinhole_measurement.hpp"
#include "roc/kinematics/hierarchical_constraint.hpp"
#include "roc/kinematics/target_selector.hpp"
#include "roc/kinematics/pending_spawn_buffer.hpp"
#include "roc/kinematics/wire_format.hpp"
#include "roc/kinematics/regime_detector.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace roc::kinematics {

struct TrackerConfig {
    AssociationConfig assoc{};
    LifecycleConfig   lifecycle{};
    SelectorWeights   selector_weights{};
    double            stationary_shrinkage_period_frames = 5.0;  // §2.6.3 K
    double            sigma_v_stationary = 0.05;  // §2.6.3
    double            sigma_a_stationary = 0.05;
    double            deadband_uv_px = 0.5;       // §2.6.4 δ_dead
    bool              enable_target_selector = true;
    double            selector_cx = 0.0;
    double            selector_cy = 0.0;
};

// Per-frame statistics — surfaced for logging and tests.
struct FrameStats {
    int    num_tracks_predicted = 0;
    int    num_tracks_confirmed = 0;
    int    num_matches = 0;
    int    num_iou_matches = 0;
    int    num_unmatched_tracks = 0;
    int    num_unmatched_dets = 0;
    int    num_spawned = 0;
    int    num_promoted = 0;
    int    num_deleted = 0;
    int    num_constraint_pairs = 0;
    int    num_hierarchy_violations = 0;
    int    num_deferred_to_pending = 0;
    int    num_promoted_from_pending = 0;
    int    num_pending_dropped_age = 0;
    double dt_used = 0.0;
};

class Tracker {
public:
    Tracker() = default;

    Tracker(CameraPose pose, TrackerConfig cfg = {}) noexcept;

    void set_camera_pose(const CameraPose& pose) noexcept;
    void set_config(const TrackerConfig& cfg) noexcept;

    const TrackerConfig& config() const noexcept { return cfg_; }
    const FrameStats& last_frame_stats() const noexcept { return stats_; }

    // §5 per-frame pipeline.  Mutates internal state and produces a
    // serialised wire frame in `out_wire_frame`.  The output buffer is
    // resized to match the actual encoded length and returned as the byte
    // count.  Returns the number of bytes written.
    //
    // `t_now_seconds` is from a monotonic clock (§0 ground rule).  The
    // tracker computes Δt internally and clamps to [kDtMin, kDtMax].
    std::size_t step(double t_now_seconds,
                     std::span<const Detection> detections,
                     std::vector<std::uint8_t>& out_wire_frame);

    // Read-only views of the current track table (in-flight + tentative).
    std::span<const Track> tracks() const noexcept { return tracks_; }

    // Selector access (manual override hooks for the SvelteKit dashboard).
    TargetSelector& selector() noexcept { return selector_; }
    const TargetSelector& selector() const noexcept { return selector_; }

    // Allow tests to seed the tracker with an arbitrary frame counter / time.
    void reset();

private:
    void predict_all(double dt);
    void update_matched(const std::vector<Match>& matches,
                        std::span<const Detection> detections,
                        std::vector<InnovationCache>& innov_cache);
    void apply_stationary_shrinkage();
    void maintain_boundary_probation(std::span<const Detection> detections,
                                     const std::vector<Match>& matches);
    void run_lifecycle(const std::vector<int>& unmatched_dets,
                       std::span<const Detection> detections);
    void prune_dead();
    void update_target_selector();
    void emit_wire_frame(std::vector<std::uint8_t>& out, std::uint32_t frame_id);

    // Helpers
    int  find_track_index_by_id(TrackId id) const noexcept;
    Track* maybe_find_parent_for(const Detection& det,
                                 const InitResult& init_res);
    void spawn_track(const Detection& det, const InitResult& init_res);

private:
    CameraPose             camera_pose_{};
    PinholeMeasurement     pinhole_{};
    TrackerConfig          cfg_{};
    Associator             associator_{};
    HierarchicalConstraint hierarchy_;
    PendingSpawnBuffer     pending_spawn_{};
    TargetSelector         selector_{};

    std::vector<Track>     tracks_{};
    std::uint64_t          next_track_id_ = 1;
    std::uint32_t          frame_counter_ = 0;
    double                 last_t_seconds_ = 0.0;
    bool                   have_last_t_ = false;
    double                 session_epoch_seconds_ = 0.0;

    FrameStats             stats_{};

    // Phase-5 per-frame scratch buffers (kept across calls, grown on demand
    // so steady-state allocation is zero).  These eliminate the per-step
    // std::vector constructor overhead inside the §5 pipeline.
    std::vector<std::vector<int>>    bucket_tracks_scratch_;
    std::vector<std::vector<int>>    bucket_dets_scratch_;
    std::vector<Match>               matches_scratch_;
    std::vector<int>                 unmatched_tracks_scratch_;
    std::vector<int>                 unmatched_dets_scratch_;
    std::vector<InnovationCache>     innov_cache_scratch_;
    std::vector<InnovationCache>     innov_label_scratch_;
    std::vector<int>                 iou_tracks_scratch_;
    std::vector<int>                 wire_track_indices_scratch_;
};

}  // namespace roc::kinematics
