// =============================================================================
// roc::kinematics::hierarchical_constraint — §3.11 two-pass Gauss–Seidel.
// =============================================================================

#include "roc/kinematics/hierarchical_constraint.hpp"
#include "roc/kinematics/object_priors.hpp"

#include <cmath>

namespace roc::kinematics {

bool HierarchicalConstraint::apply_pair(Track& parent, Track& sub) const
{
    // §3.11.4 application gate.
    //   1. Both tracks must be confirmed.
    //   2. Both tracks had a real detection this frame (time_since_update == 0).
    //   3. Plausibility: ‖h_rel‖ < 3 ‖σ_S‖.
    if (!parent.is_confirmed() || !sub.is_confirmed()) {
        return false;
    }
    if (parent.time_since_update != 0 || sub.time_since_update != 0) {
        return false;
    }

    const AnatomicalOffset off = anatomical_offset_for(sub.label);
    const Vec3 mu     = off.mu;
    const Vec3 sigma  = off.sigma;
    const Vec3 sigma_sq{sigma(0) * sigma(0), sigma(1) * sigma(1), sigma(2) * sigma(2)};
    const double sigma_norm = sigma.norm();
    const double plausibility_threshold_sq =
        kPlausibilityNSigma * kPlausibilityNSigma * sigma_norm * sigma_norm;

    Vec3 h_rel = sub.filter.position() - parent.filter.position() - mu;
    if (h_rel.squaredNorm() > plausibility_threshold_sq) {
        sub.hierarchy_violation_pending = true;
        parent.hierarchy_violation_pending = true;
        if (on_violation_) {
            on_violation_(parent.id, sub.id);
        }
        return false;
    }
    sub.hierarchy_violation_pending = false;
    parent.hierarchy_violation_pending = false;

    // §3.11.3 N_iter outer sweeps; per sweep run pass 1 (sub), pass 2 (parent).
    for (int iter = 0; iter < kNumIter; ++iter) {
        // Pass 1: refresh sub-feature treating parent as known.
        sub.filter.update_relative_position(parent.filter.position(), mu, sigma, +1);

        // Pass 2: refresh parent treating refreshed sub-feature as known.
        parent.filter.update_relative_position(sub.filter.position(), mu, sigma, -1);
    }
    return true;
}

void HierarchicalConstraint::apply(std::span<Track> tracks) const
{
    // Build (parent_idx, sub_idx) pairs from the parent_id field.
    // We do a single linear scan: for each sub-feature track, locate its
    // parent index by id.  N is small (≤ 50), so no need for hashing.
    const int n = static_cast<int>(tracks.size());
    for (int s = 0; s < n; ++s) {
        Track& sub = tracks[static_cast<std::size_t>(s)];
        if (!sub.parent_id.has_value()) continue;
        if (!is_subfeature_label(sub.label)) continue;
        const TrackId pid = sub.parent_id.value();
        int p_idx = -1;
        for (int p = 0; p < n; ++p) {
            if (tracks[static_cast<std::size_t>(p)].id == pid) {
                p_idx = p;
                break;
            }
        }
        if (p_idx < 0) continue;
        Track& parent = tracks[static_cast<std::size_t>(p_idx)];
        if (!is_parent_label(parent.label)) continue;
        apply_pair(parent, sub);
    }
}

}  // namespace roc::kinematics
