#pragma once

// =============================================================================
// roc::kinematics::target_selector
//
// Multi-object selection layer (architecture.md §2 / Phase 2 roadmap).
//
// The selector ranks confirmed tracks by a weighted score combining:
//   - Proximity to a reference image-plane point (cx, cy) — usually frame
//     centre but settable by the SvelteKit operator.
//   - Track confidence (last detection × NIS-derived health).
//   - Stability (low η̄ in the regime detector → high stability).
//
// Output: `SelectionResult { TrackId chosen, double dx, double dy }` — the
// pixel-space delta the actuator node consumes.  The (dx, dy) signal is
// EMA-smoothed at user-configurable α (default 0.4) to suppress single-frame
// jitter.
//
// Coordinate space:  the reference and the produced (dx, dy) live in the
// same image-plane pixel system the wire frame's `bbox_orig` uses.  The
// selector therefore ingests track image-plane projections (it does NOT
// re-project via the camera model — that costs an unnecessary pass).
// Instead the Tracker primes each Track's `last_bbox_*` fields each frame
// (§7.2) and the selector reads them directly.
// =============================================================================

#include "roc/kinematics/types.hpp"
#include "roc/kinematics/track.hpp"

#include <optional>
#include <span>

namespace roc::kinematics {

struct SelectorWeights {
    double w_proximity   = 1.0;
    double w_confidence  = 0.7;
    double w_stability   = 0.5;
};

struct SelectionResult {
    TrackId chosen{0};
    double  dx = 0.0;     // smoothed (target_u - cx)
    double  dy = 0.0;     // smoothed (target_v - cy)
    double  raw_dx = 0.0; // un-smoothed
    double  raw_dy = 0.0;
    double  score = 0.0;
    bool    valid = false;
};

class TargetSelector {
public:
    static constexpr double kDefaultEmaAlpha = 0.4;

    TargetSelector() = default;
    TargetSelector(double cx, double cy, SelectorWeights w = {},
                   double ema_alpha = kDefaultEmaAlpha) noexcept
        : cx_(cx), cy_(cy), weights_(w),
          ema_alpha_(ema_alpha) {}

    void set_reference(double cx, double cy) noexcept { cx_ = cx; cy_ = cy; }
    void set_weights(SelectorWeights w) noexcept { weights_ = w; }
    void set_smoothing_alpha(double a) noexcept;
    void clear_state() noexcept;

    // Optional manual override: pin the selector to a specific track id.
    // Set TrackId{0} to release.
    void set_locked_target(TrackId id) noexcept { locked_id_ = id; }

    SelectionResult select(std::span<const Track> tracks) noexcept;

private:
    double          cx_ = 0.0;
    double          cy_ = 0.0;
    SelectorWeights weights_{};
    double          ema_alpha_ = kDefaultEmaAlpha;

    bool            have_smooth_ = false;
    double          smooth_dx_ = 0.0;
    double          smooth_dy_ = 0.0;
    TrackId         locked_id_{0};
    TrackId         last_chosen_id_{0};
};

}  // namespace roc::kinematics
