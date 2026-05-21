#pragma once

// =============================================================================
// roc::kinematics::association
//
// DeepSORT-style association layer (§4):
//   §4.2.1  cosine appearance distance via concatenated-gallery GEMM (§4.7).
//   §4.2.2  Mahalanobis motion gating (Cholesky, never explicit S^{-1}).
//   §4.3    gated combined cost matrix.
//   §4.4    matching cascade (age-stratified) + IoU fallback.
//   §4.5    LAP-JV linear assignment.
//
// All N×M data structures are dynamic Eigen MatrixX since they depend on the
// per-frame counts (§1.2 explicitly allows this — strictly bounded to ≤256).
// =============================================================================

#include "roc/kinematics/types.hpp"
#include "roc/kinematics/track.hpp"
#include "roc/kinematics/pinhole_measurement.hpp"
#include "roc/kinematics/lap_jv.hpp"

#include <span>
#include <vector>

namespace roc::kinematics {

// ---------------------------------------------------------------------------
// AssociationConfig — defaults from §4.3 / §4.4.
// ---------------------------------------------------------------------------
struct AssociationConfig {
    double lambda             = 0.0;       // §4.3: 0.0 (camera moving) / 0.2 (static)
    double tau_app            = 0.2;       // §4.3 — cosine gate
    double tau_mot_sq         = 9.4877;    // §4.2.2 — χ²_{4, 0.95}
    double tau_iou            = 0.7;       // §4.4 — IoU fallback gate
    int    max_age            = 30;        // §4.4 A_max
};

// One association record produced by the cascade.
struct Match {
    int track_index = -1;
    int det_index   = -1;
};

struct AssociationResult {
    std::vector<Match> matches;
    std::vector<int>   unmatched_tracks;
    std::vector<int>   unmatched_dets;
};

// Innovation cache produced during gating.  Re-used by the EKF update so we
// do not pay for the full (h, H, S) computation twice (§4.2.2 / §3.7).
struct InnovationCache {
    Meas      h{Meas::Zero()};      // h(x̂_{k|k-1})
    Jacobian  H{Jacobian::Zero()};
    bool      valid = false;
};

// ---------------------------------------------------------------------------
// Associator — owns the dynamic scratch buffers; lives across frames.
// ---------------------------------------------------------------------------
class Associator {
public:
    explicit Associator(AssociationConfig cfg = {}) noexcept : cfg_(cfg) {}

    void set_config(const AssociationConfig& cfg) noexcept { cfg_ = cfg; }
    const AssociationConfig& config() const noexcept { return cfg_; }

    // §4.4 matching cascade + §4.4 IoU fallback.  Operates on tracks and
    // detections of one specific Label only (caller groups by label).
    //
    // `track_indices` / `det_indices` are indices into the caller's master
    // arrays (so cascading the per-label slice does not lose mapping back to
    // the global track / detection vectors).
    //
    // The pinhole model is needed to compute h() for motion gating; the
    // resulting innovation cache is filled into `innov_out` for the matched
    // pairs only — unmatched cells are left in their default state.
    AssociationResult run_cascade(
        const std::vector<int>&            track_indices,
        const std::vector<int>&            det_indices,
        std::span<const Track>             tracks,
        std::span<const Detection>         detections,
        const PinholeMeasurement&          camera,
        std::vector<InnovationCache>&      innov_out) const;

    // §4.4 IoU fallback — applied to any tentative tracks plus any confirmed
    // tracks left unmatched after the cascade.  Pure 2-D IoU; no motion model.
    AssociationResult run_iou_fallback(
        const std::vector<int>&     track_indices,
        const std::vector<int>&     det_indices,
        std::span<const Track>      tracks,
        std::span<const Detection>  detections) const;

    // ---- Exposed for unit tests ------------------------------------------
    // §4.2.1 — concatenated-gallery GEMM cosine distance for one slice.
    // Returns `D_app` of shape (n_tracks × n_dets).
    Eigen::MatrixXf compute_appearance_distance(
        const std::vector<int>&     track_indices,
        const std::vector<int>&     det_indices,
        std::span<const Track>      tracks,
        std::span<const Detection>  detections) const;

    // §4.2.2 — Mahalanobis-squared cost for a single (track, det) pair plus
    // populated innovation cache (h, H).  Returns >= +∞ sentinel when the
    // linearisation degenerated (Z ≤ kZMin) — caller treats as gated-out.
    double compute_motion_distance_sq(
        const Track&        t,
        const Detection&    d,
        const PinholeMeasurement& cam,
        InnovationCache&    cache) const;

    // §4.3 — combine appearance + motion + boolean gates into the final cost
    // matrix that LAP-JV consumes.  Forbidden cells are set to kForbiddenCost.
    Eigen::MatrixXd build_combined_cost(
        const std::vector<int>&     track_indices,
        const std::vector<int>&     det_indices,
        std::span<const Track>      tracks,
        std::span<const Detection>  detections,
        const PinholeMeasurement&   camera,
        const Eigen::MatrixXf&      app_cost,
        std::vector<InnovationCache>& innov_cache_pair) const;

private:
    AssociationConfig cfg_;

    // Phase-5 scratch buffers (heap-allocated once, reused across frames).
    // Avoids placing the 100×128 float gallery matrix (~50 KB) on the stack
    // every cascade band — important when run_cascade is called inside the
    // per-label loop of Tracker::step.  `mutable` because the helpers
    // (compute_appearance_distance / build_combined_cost / run_cascade)
    // are conceptually const but mutate scratch state.
    mutable Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> gallery_scratch_;
    mutable Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> sims_scratch_;
    mutable Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> det_F_scratch_;
    mutable std::vector<double>                                  lap_flat_scratch_;
};

// 2-D IoU between two axis-aligned bboxes.
double iou_xywh(double u1, double v1, double w1, double h1,
                double u2, double v2, double w2, double h2) noexcept;

}  // namespace roc::kinematics
