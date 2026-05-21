#pragma once

// =============================================================================
// roc::kinematics::hierarchical_constraint
//
// Two-pass Gauss–Seidel coupled-EKF update implementing the soft hierarchical
// constraint of §3.11.  Each (parent, sub-feature) pair is updated:
//   - Pass 1: refresh sub-feature treating parent as known.
//   - Pass 2: refresh parent treating refreshed sub-feature as known.
//
// Iteration count: §3.11.3 specifies N_iter = 2 (two outer sweeps); we expose
// it as a constant so unit-tests can compare to a single-iteration result.
//
// §3.11.4 application gate enforced before each pair:
//   1. Both filters must be confirmed (`hits >= confirm_hits`).
//   2. Both filters must have `time_since_update == 0` post-update.
//   3. ‖h_rel‖ < 3 ‖σ_S‖, otherwise emit a `hierarchy_violation` event.
// =============================================================================

#include "roc/kinematics/types.hpp"
#include "roc/kinematics/track.hpp"
#include "roc/kinematics/object_priors.hpp"

#include <functional>
#include <span>

namespace roc::kinematics {

class HierarchicalConstraint {
public:
    static constexpr int kNumIter = 2;       // §3.11.3
    static constexpr double kPlausibilityNSigma = 3.0;  // §3.11.4

    // `event_callback` is invoked once per (parent, sub) pair that hits the
    // §3.11.4 plausibility cut.  The Tracker forwards this onto the wire
    // frame status_flags and the operator-facing telemetry stream.
    using EventCallback = std::function<void(TrackId parent_id,
                                             TrackId sub_id)>;

    HierarchicalConstraint() = default;
    explicit HierarchicalConstraint(EventCallback cb) noexcept
        : on_violation_(std::move(cb)) {}

    void set_event_callback(EventCallback cb) noexcept { on_violation_ = std::move(cb); }

    // Apply the 2-pass coupled update across all confirmed parent ↔ subfeature
    // pairs in `tracks` (in-place).  Pairs are determined by Track::parent_id.
    void apply(std::span<Track> tracks) const;

    // Single-pair test hook.  Returns true if the pair was updated, false if
    // the §3.11.4 gate suppressed it.
    bool apply_pair(Track& parent, Track& sub) const;

private:
    EventCallback on_violation_;
};

}  // namespace roc::kinematics
