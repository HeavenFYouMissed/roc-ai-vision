// =============================================================================
// roc::kinematics::association — gating + cost matrix + matching cascade.
// Implements §4 of phase_2_ekf_kinematics.md.
// =============================================================================

#include "roc/kinematics/association.hpp"
#include "roc/kinematics/object_priors.hpp"

#include <Eigen/Cholesky>

#include <algorithm>
#include <cmath>
#include <limits>

namespace roc::kinematics {

double iou_xywh(double u1, double v1, double w1, double h1,
                double u2, double v2, double w2, double h2) noexcept
{
    const double l1 = u1 - 0.5 * w1, r1 = u1 + 0.5 * w1;
    const double t1 = v1 - 0.5 * h1, b1 = v1 + 0.5 * h1;
    const double l2 = u2 - 0.5 * w2, r2 = u2 + 0.5 * w2;
    const double t2 = v2 - 0.5 * h2, b2 = v2 + 0.5 * h2;
    const double xi = std::max(0.0, std::min(r1, r2) - std::max(l1, l2));
    const double yi = std::max(0.0, std::min(b1, b2) - std::max(t1, t2));
    const double inter = xi * yi;
    const double area1 = std::max(0.0, w1) * std::max(0.0, h1);
    const double area2 = std::max(0.0, w2) * std::max(0.0, h2);
    const double uni = area1 + area2 - inter;
    return (uni > 0.0) ? (inter / uni) : 0.0;
}

// ---------------------------------------------------------------------------
// §4.2.1 cosine appearance distance — concatenated-gallery GEMM.
// ---------------------------------------------------------------------------
Eigen::MatrixXf Associator::compute_appearance_distance(
    const std::vector<int>&     track_indices,
    const std::vector<int>&     det_indices,
    std::span<const Track>      tracks,
    std::span<const Detection>  detections) const
{
    const int n_t = static_cast<int>(track_indices.size());
    const int n_d = static_cast<int>(det_indices.size());
    Eigen::MatrixXf D(n_t, n_d);
    if (n_t == 0 || n_d == 0) return D;

    // Phase-5: re-use the heap-resident scratch matrices on the Associator
    // instance instead of materialising a 100×128 stack buffer per call.
    // `conservativeResize` is a no-op once the matrix has grown to the right
    // shape, so steady-state is heap-touch-free.
    auto& F = det_F_scratch_;          // (n_d × 128)
    if (F.rows() != n_d || F.cols() != kEmbeddingDim) {
        F.resize(n_d, kEmbeddingDim);
    }
    for (int j = 0; j < n_d; ++j) {
        F.row(j) = detections[static_cast<std::size_t>(det_indices[static_cast<std::size_t>(j)])].appearance.transpose();
    }

    auto& R_buf = gallery_scratch_;    // (kMaxSize × 128) — only topRows(L) used
    if (R_buf.rows() != EmbeddingGallery::kMaxSize || R_buf.cols() != kEmbeddingDim) {
        R_buf.resize(EmbeddingGallery::kMaxSize, kEmbeddingDim);
    }
    auto& sims = sims_scratch_;        // (L × n_d) — grown per track as needed

    // For each track row, GEMM gallery × Fᵀ then take per-detection max.
    // §4.7 calls for "a single concatenated-gallery GEMM"; we use the
    // mathematically-equivalent per-track form because each gallery has a
    // dynamic size L_i ≤ L_max, and the concatenated form would require
    // either an offset-aware extractor or a wasteful pad-to-L_max copy.
    for (int i = 0; i < n_t; ++i) {
        const Track& tr = tracks[static_cast<std::size_t>(track_indices[static_cast<std::size_t>(i)])];
        const int L = tr.gallery.size();
        if (L == 0) {
            // No appearance yet — distance defaults to maximum (cos = -1 → d = 2).
            D.row(i).setConstant(2.0f);
            continue;
        }
        for (int k = 0; k < L; ++k) {
            R_buf.row(k) = tr.gallery.at(k).transpose();
        }
        if (sims.rows() != L || sims.cols() != n_d) {
            sims.resize(L, n_d);
        }
        sims.noalias() = R_buf.topRows(L) * F.transpose();
        D.row(i) = Eigen::Matrix<float, 1, Eigen::Dynamic>::Constant(n_d, 1.0f)
                   - sims.colwise().maxCoeff();
    }
    return D;
}

// ---------------------------------------------------------------------------
// §4.2.2 Mahalanobis-squared via Cholesky.  Populates innov cache (h, H).
// ---------------------------------------------------------------------------
double Associator::compute_motion_distance_sq(
    const Track&        t,
    const Detection&    d,
    const PinholeMeasurement& cam,
    InnovationCache&    cache) const
{
    cache.valid = false;
    Meas h_pred;
    Jacobian H;
    const auto we = world_extents_for(t.label);
    if (!cam.compute_h_and_H(t.filter.state(), we, h_pred, H)) {
        return kForbiddenCost;
    }
    Meas z;
    z << d.bbox_center.u, d.bbox_center.v, d.bbox_w_px, d.bbox_h_px;
    Meas nu = z - h_pred;

    MeasCov R = default_meas_cov(d.confidence);
    MeasCov S = H * t.filter.cov() * H.transpose() + R;
    S = 0.5 * (S + S.transpose()).eval();

    Eigen::LLT<MeasCov> S_llt(S);
    if (S_llt.info() != Eigen::Success) {
        return kForbiddenCost;
    }
    const Meas Sinv_nu = S_llt.solve(nu);
    const double d_mot_sq = nu.dot(Sinv_nu);

    cache.h = h_pred;
    cache.H = H;
    cache.valid = true;
    return d_mot_sq;
}

// ---------------------------------------------------------------------------
// §4.3 build the gated combined cost matrix.
// ---------------------------------------------------------------------------
Eigen::MatrixXd Associator::build_combined_cost(
    const std::vector<int>&     track_indices,
    const std::vector<int>&     det_indices,
    std::span<const Track>      tracks,
    std::span<const Detection>  detections,
    const PinholeMeasurement&   camera,
    const Eigen::MatrixXf&      app_cost,
    std::vector<InnovationCache>& innov_pair_cache) const
{
    const int n_t = static_cast<int>(track_indices.size());
    const int n_d = static_cast<int>(det_indices.size());
    Eigen::MatrixXd C(n_t, n_d);
    C.setConstant(kForbiddenCost);
    innov_pair_cache.assign(static_cast<std::size_t>(n_t * n_d), InnovationCache{});

    if (n_t == 0 || n_d == 0) return C;

    const double tau_mot = std::sqrt(cfg_.tau_mot_sq);

    for (int i = 0; i < n_t; ++i) {
        const Track& tr = tracks[static_cast<std::size_t>(track_indices[static_cast<std::size_t>(i)])];
        for (int j = 0; j < n_d; ++j) {
            const Detection& dt =
                detections[static_cast<std::size_t>(det_indices[static_cast<std::size_t>(j)])];

            // Label gate: only same-label associations (head ↔ head, etc.).
            // The Tracker calls run_cascade per-label so this is usually
            // already satisfied, but a defensive check keeps the API safe.
            if (tr.label != dt.label) continue;

            const double d_app = static_cast<double>(app_cost(i, j));
            if (d_app > cfg_.tau_app) continue;

            InnovationCache cache;
            const double d_mot_sq = compute_motion_distance_sq(tr, dt, camera, cache);
            if (!(d_mot_sq <= cfg_.tau_mot_sq)) continue;

            const double d_mot = std::sqrt(std::max(0.0, d_mot_sq));
            const double cost  = cfg_.lambda * (d_mot / tau_mot)
                               + (1.0 - cfg_.lambda) * d_app;
            C(i, j) = cost;
            innov_pair_cache[static_cast<std::size_t>(i * n_d + j)] = cache;
        }
    }
    return C;
}

// ---------------------------------------------------------------------------
// §4.4 matching cascade — outer loop over `time_since_update` bands.
// ---------------------------------------------------------------------------
AssociationResult Associator::run_cascade(
    const std::vector<int>&            track_indices,
    const std::vector<int>&            det_indices,
    std::span<const Track>             tracks,
    std::span<const Detection>         detections,
    const PinholeMeasurement&          camera,
    std::vector<InnovationCache>&      innov_out) const
{
    AssociationResult out;
    out.matches.reserve(std::min(track_indices.size(), det_indices.size()));
    innov_out.assign(tracks.size(), InnovationCache{});

    std::vector<int> remaining_dets = det_indices;

    // §4.4 — only confirmed tracks participate in the cascade; tentative
    // (hits < confirm_hits) tracks are passed to the IoU fallback by the
    // caller.  We still accept them here as a single age-band so the cascade
    // keeps association latency low for tentative tracks.
    for (int n_age = 1; n_age <= cfg_.max_age; ++n_age) {
        if (remaining_dets.empty()) break;

        // Tracks at exactly this age (and confirmed).
        std::vector<int> band;
        band.reserve(track_indices.size());
        for (int idx : track_indices) {
            const Track& t = tracks[static_cast<std::size_t>(idx)];
            if (t.is_confirmed() && t.time_since_update == n_age) {
                band.push_back(idx);
            }
        }
        if (band.empty()) continue;

        // Build appearance + combined cost.
        Eigen::MatrixXf app =
            compute_appearance_distance(band, remaining_dets, tracks, detections);
        std::vector<InnovationCache> innov_pair;
        Eigen::MatrixXd C = build_combined_cost(band, remaining_dets, tracks,
                                                detections, camera, app, innov_pair);

        // Solve LAP-JV.  Phase-5: re-use the heap-resident flat-cost scratch
        // instead of allocating per cascade band.
        const int n_t = static_cast<int>(band.size());
        const int n_d = static_cast<int>(remaining_dets.size());
        lap_flat_scratch_.assign(static_cast<std::size_t>(n_t) * n_d, 0.0);
        for (int i = 0; i < n_t; ++i) {
            for (int j = 0; j < n_d; ++j) {
                lap_flat_scratch_[static_cast<std::size_t>(i) * n_d + j] = C(i, j);
            }
        }
        const LapAssignment lap = solve_lap_jv(lap_flat_scratch_.data(), n_t, n_d);

        std::vector<bool> det_matched(static_cast<std::size_t>(n_d), false);
        for (int i = 0; i < n_t; ++i) {
            const int j = lap.col_for_row[static_cast<std::size_t>(i)];
            if (j < 0) continue;
            // Lap excludes kForbiddenCost cells already, but defend anyway.
            if (C(i, j) >= kForbiddenCost) continue;
            Match m;
            m.track_index = band[static_cast<std::size_t>(i)];
            m.det_index   = remaining_dets[static_cast<std::size_t>(j)];
            out.matches.push_back(m);
            innov_out[static_cast<std::size_t>(m.track_index)] =
                innov_pair[static_cast<std::size_t>(i * n_d + j)];
            det_matched[static_cast<std::size_t>(j)] = true;
        }
        // Filter `remaining_dets` to the unmatched ones.
        std::vector<int> still_unmatched;
        still_unmatched.reserve(remaining_dets.size());
        for (int j = 0; j < n_d; ++j) {
            if (!det_matched[static_cast<std::size_t>(j)]) {
                still_unmatched.push_back(remaining_dets[static_cast<std::size_t>(j)]);
            }
        }
        remaining_dets = std::move(still_unmatched);
    }

    // Confirmed tracks still unmatched after the cascade.
    std::vector<int> matched_track_set;
    matched_track_set.reserve(out.matches.size());
    for (auto& m : out.matches) matched_track_set.push_back(m.track_index);
    std::sort(matched_track_set.begin(), matched_track_set.end());

    out.unmatched_tracks.reserve(track_indices.size());
    for (int idx : track_indices) {
        const Track& t = tracks[static_cast<std::size_t>(idx)];
        if (!t.is_confirmed()) continue;
        if (!std::binary_search(matched_track_set.begin(), matched_track_set.end(), idx)) {
            out.unmatched_tracks.push_back(idx);
        }
    }
    out.unmatched_dets = std::move(remaining_dets);
    return out;
}

// ---------------------------------------------------------------------------
// §4.4 IoU fallback — pairs leftover detections with tentative tracks plus
// cascade-unmatched confirmed tracks using IoU distance only.
// ---------------------------------------------------------------------------
AssociationResult Associator::run_iou_fallback(
    const std::vector<int>&     track_indices,
    const std::vector<int>&     det_indices,
    std::span<const Track>      tracks,
    std::span<const Detection>  detections) const
{
    AssociationResult out;
    if (track_indices.empty() || det_indices.empty()) {
        out.unmatched_tracks = track_indices;
        out.unmatched_dets   = det_indices;
        return out;
    }

    const int n_t = static_cast<int>(track_indices.size());
    const int n_d = static_cast<int>(det_indices.size());
    lap_flat_scratch_.assign(static_cast<std::size_t>(n_t) * n_d, kForbiddenCost);

    for (int i = 0; i < n_t; ++i) {
        const Track& t = tracks[static_cast<std::size_t>(track_indices[static_cast<std::size_t>(i)])];
        for (int j = 0; j < n_d; ++j) {
            const Detection& d =
                detections[static_cast<std::size_t>(det_indices[static_cast<std::size_t>(j)])];
            if (t.label != d.label) continue;
            const double iou = iou_xywh(
                t.last_bbox_u, t.last_bbox_v, t.last_bbox_w, t.last_bbox_h,
                d.bbox_center.u, d.bbox_center.v, d.bbox_w_px, d.bbox_h_px);
            const double cost = 1.0 - iou;
            if (cost > cfg_.tau_iou) continue;
            lap_flat_scratch_[static_cast<std::size_t>(i) * n_d + j] = cost;
        }
    }

    const LapAssignment lap = solve_lap_jv(lap_flat_scratch_.data(), n_t, n_d);

    std::vector<bool> det_matched(static_cast<std::size_t>(n_d), false);
    for (int i = 0; i < n_t; ++i) {
        const int j = lap.col_for_row[static_cast<std::size_t>(i)];
        if (j < 0) continue;
        const double c = lap_flat_scratch_[static_cast<std::size_t>(i) * n_d + j];
        if (c >= kForbiddenCost) continue;
        Match m;
        m.track_index = track_indices[static_cast<std::size_t>(i)];
        m.det_index   = det_indices[static_cast<std::size_t>(j)];
        out.matches.push_back(m);
        det_matched[static_cast<std::size_t>(j)] = true;
    }

    std::vector<int> matched_track_set;
    matched_track_set.reserve(out.matches.size());
    for (auto& m : out.matches) matched_track_set.push_back(m.track_index);
    std::sort(matched_track_set.begin(), matched_track_set.end());

    out.unmatched_tracks.reserve(track_indices.size());
    for (int idx : track_indices) {
        if (!std::binary_search(matched_track_set.begin(), matched_track_set.end(), idx)) {
            out.unmatched_tracks.push_back(idx);
        }
    }
    for (int j = 0; j < n_d; ++j) {
        if (!det_matched[static_cast<std::size_t>(j)]) {
            out.unmatched_dets.push_back(det_indices[static_cast<std::size_t>(j)]);
        }
    }
    return out;
}

}  // namespace roc::kinematics
