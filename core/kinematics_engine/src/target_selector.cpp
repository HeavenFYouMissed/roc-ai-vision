// =============================================================================
// roc::kinematics::target_selector — multi-target ranking + EMA-smoothed
// (dx, dy) output.  Implements the "TargetSelector" entry under Phase 2/3 of
// the roadmap and architecture.md §2.
// =============================================================================

#include "roc/kinematics/target_selector.hpp"

#include <cmath>
#include <limits>

namespace roc::kinematics {

void TargetSelector::set_smoothing_alpha(double a) noexcept
{
    if (a < 0.0) a = 0.0;
    if (a > 1.0) a = 1.0;
    ema_alpha_ = a;
}

void TargetSelector::clear_state() noexcept
{
    have_smooth_ = false;
    smooth_dx_ = 0.0;
    smooth_dy_ = 0.0;
    last_chosen_id_ = TrackId{0};
}

namespace {

double proximity_score(double u, double v, double cx, double cy) noexcept
{
    const double du = u - cx;
    const double dv = v - cy;
    const double d2 = du * du + dv * dv;
    return 1.0 / (1.0 + std::sqrt(d2));
}

double stability_score(const Track& t) noexcept
{
    // Lower η̄ → more stable.  Map to (0, 1] via 1 / (1 + η̄/4) so that
    // a freshly-initialised track (η̄ = 4) maps to 0.5 and η̄ → 0 maps to 1.
    const double eta = std::max(0.0, t.regime_detector.eta_bar());
    return 1.0 / (1.0 + 0.25 * eta);
}

}  // namespace

SelectionResult TargetSelector::select(std::span<const Track> tracks) noexcept
{
    SelectionResult res;
    int    chosen_idx = -1;
    double best_score = -std::numeric_limits<double>::infinity();

    if (locked_id_.value != 0) {
        for (std::size_t i = 0; i < tracks.size(); ++i) {
            if (tracks[i].id == locked_id_) {
                chosen_idx = static_cast<int>(i);
                best_score = std::numeric_limits<double>::max();
                break;
            }
        }
        if (chosen_idx < 0) {
            // Locked target lost; fall back to organic ranking.
            locked_id_ = TrackId{0};
        }
    }

    if (chosen_idx < 0) {
        for (std::size_t i = 0; i < tracks.size(); ++i) {
            const Track& t = tracks[i];
            if (!t.is_confirmed()) continue;
            // Skip subfeatures by default — we want the parent person bbox
            // unless the operator explicitly locks onto a subfeature via
            // set_locked_target().
            if (!is_parent_label(t.label)) continue;

            const double s_prox = proximity_score(t.last_bbox_u, t.last_bbox_v,
                                                  cx_, cy_);
            const double s_conf = std::max(0.0, t.last_confidence);
            const double s_stab = stability_score(t);
            const double score  = weights_.w_proximity   * s_prox
                                + weights_.w_confidence  * s_conf
                                + weights_.w_stability   * s_stab;
            if (score > best_score) {
                best_score = score;
                chosen_idx = static_cast<int>(i);
            }
        }
    }

    if (chosen_idx < 0) {
        // No valid target.  Reset smoothing so next frame starts clean.
        have_smooth_ = false;
        return res;
    }

    const Track& t = tracks[static_cast<std::size_t>(chosen_idx)];
    res.chosen = t.id;
    res.raw_dx = t.last_bbox_u - cx_;
    res.raw_dy = t.last_bbox_v - cy_;
    res.score  = best_score;

    // EMA smoothing — reset when target id flips.
    if (!have_smooth_ || last_chosen_id_ != t.id) {
        smooth_dx_ = res.raw_dx;
        smooth_dy_ = res.raw_dy;
        have_smooth_ = true;
    } else {
        smooth_dx_ = (1.0 - ema_alpha_) * smooth_dx_ + ema_alpha_ * res.raw_dx;
        smooth_dy_ = (1.0 - ema_alpha_) * smooth_dy_ + ema_alpha_ * res.raw_dy;
    }
    last_chosen_id_ = t.id;
    res.dx = smooth_dx_;
    res.dy = smooth_dy_;
    res.valid = true;
    return res;
}

}  // namespace roc::kinematics
