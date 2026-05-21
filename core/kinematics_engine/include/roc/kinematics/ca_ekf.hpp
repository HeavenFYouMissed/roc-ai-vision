#pragma once

// =============================================================================
// roc::kinematics::ca_ekf
//
// 9-D constant-acceleration EKF.  Implements:
//
//   §2.1  closed-form F(Δt)            via the four scalar blocks.
//   §2.2  closed-form Q(Δt) (CWNJ)     via the van-Loan integral.
//   §2.6  adaptive Q_c scaling at predict-time (caller passes ρ).
//   §3.7  Joseph-form covariance update (mandatory on every update).
//   §3.10 centroid-only update path (boundary probation row dropout).
//   §2.6.3 zero-velocity / zero-acceleration shrinkage pseudo-measurement.
//   §3.11 hierarchical-constraint pseudo-measurement (relative-position).
//
// Hard guarantees:
//   - All matrices are compile-time fixed-size (§1.2).
//   - No heap allocation inside any method (Eigen::internal::set_is_malloc_allowed
//     is asserted at the tracker boot when EIGEN_RUNTIME_NO_MALLOC is on).
//   - S is never inverted explicitly — Cholesky-solve only (§3.7).
//   - Joseph form on every update.  Symmetrise after every prediction and
//     after every Joseph update.
// =============================================================================

#include "roc/kinematics/types.hpp"

namespace roc::kinematics {

// ---------------------------------------------------------------------------
// Closed-form F(Δt) — §2.1
// ---------------------------------------------------------------------------
inline StateCov compute_F(double dt) noexcept {
    StateCov F = StateCov::Identity();
    const double half_dt2 = 0.5 * dt * dt;
    F.block<3, 3>(0, 3).diagonal().setConstant(dt);
    F.block<3, 3>(0, 6).diagonal().setConstant(half_dt2);
    F.block<3, 3>(3, 6).diagonal().setConstant(dt);
    return F;
}

// ---------------------------------------------------------------------------
// Closed-form Q(Δt) under continuous white-noise jerk — §2.2 boxed equation.
//
//   Q = [ Δt^5/20 Qc   Δt^4/8  Qc   Δt^3/6  Qc ]
//       [ Δt^4/8  Qc   Δt^3/3  Qc   Δt^2/2  Qc ]
//       [ Δt^3/6  Qc   Δt^2/2  Qc   Δt      Qc ]
//
// `qc_diag_active` is the pre-scaled diagonal (§2.6.2 ρ already applied).
// ---------------------------------------------------------------------------
inline StateCov compute_Q(double dt, const QcDiag& qc_diag_active) noexcept {
    const double dt2 = dt * dt;
    const double dt3 = dt2 * dt;
    const double dt4 = dt3 * dt;
    const double dt5 = dt4 * dt;

    const double c_pp = dt5 / 20.0;
    const double c_pv = dt4 / 8.0;
    const double c_pa = dt3 / 6.0;
    const double c_vv = dt3 / 3.0;
    const double c_va = dt2 / 2.0;
    const double c_aa = dt;

    StateCov Q = StateCov::Zero();
    // Each 3×3 block is diagonal because Qc is diagonal — exploit it.
    auto fill_diag = [&](int r, int c, double k) noexcept {
        Q(r + 0, c + 0) = k * qc_diag_active(0);
        Q(r + 1, c + 1) = k * qc_diag_active(1);
        Q(r + 2, c + 2) = k * qc_diag_active(2);
    };
    fill_diag(0, 0, c_pp); fill_diag(0, 3, c_pv); fill_diag(0, 6, c_pa);
    fill_diag(3, 0, c_pv); fill_diag(3, 3, c_vv); fill_diag(3, 6, c_va);
    fill_diag(6, 0, c_pa); fill_diag(6, 3, c_va); fill_diag(6, 6, c_aa);
    return Q;
}

// ---------------------------------------------------------------------------
// CaEkf — owning state + covariance for one tracked sub-feature.
// ---------------------------------------------------------------------------
class CaEkf {
public:
    CaEkf() = default;

    CaEkf(const State& x0, const StateCov& P0, const QcDiag& qc_nominal) noexcept
        : x_(x0), P_(P0), qc_nominal_(qc_nominal) {
        symmetrize(P_);
    }

    // §2.5 prediction step.  qc_scale = ρ(η̄) per §2.6.2, supplied by caller.
    // dt is assumed to be already clamped to [kDtMin, kDtMax] at the tracker.
    void predict(double dt, double qc_scale) noexcept {
        const StateCov F = compute_F(dt);
        const QcDiag   qc_active = qc_scale * qc_nominal_;
        const StateCov Q = compute_Q(dt, qc_active);

        x_ = F * x_;
        P_ = F * P_ * F.transpose() + Q;
        symmetrize(P_);
    }

    // §3.7 full 4-row Joseph-form update.  Returns the NIS scalar
    // ν^T S^{-1} ν (used to drive the regime detector).  `H` and the
    // pre-computed innovation `nu` come from the PinholeMeasurement helper.
    //
    // Precondition: R is positive-definite, Z >= kZMin already enforced
    // upstream (otherwise the caller skipped the update per §3.9).
    double update_full(const Meas& nu,
                       const Jacobian& H,
                       const MeasCov&  R) noexcept;

    // §3.10.4 boundary-probation centroid-only update — strips H and R
    // to their first two rows / 2×2 block before the standard Joseph
    // formula.  Returns the (2-D) NIS scalar.
    double update_centroid_only(const Meas2&     nu_uv,
                                const Jacobian2& H_uv,
                                const MeasCov2&  R_uv) noexcept;

    // §2.6.3 stationary-regime shrinkage pseudo-measurement.
    //   z_rest = 0_6,  H_rest = [0_{6×3} | I_6],  R_rest = diag(σ_v² I_3, σ_a² I_3)
    // Joseph form preserved.
    void update_zero_velocity_shrinkage(double sigma_v, double sigma_a) noexcept;

    // §3.11 relative-position pseudo-measurement applied to *this* filter,
    // treating the other endpoint's position as known.
    //   sign = +1  → this filter is the sub-feature S (h_rel = p_S - p_P - μ_S)
    //   sign = -1  → this filter is the parent      P (same residual sign convention)
    // The effective Jacobian is:  H_rel = [ sign * I_3 | 0_{3×6} ]
    void update_relative_position(const Position& p_other_world,
                                  const Vec3&     mu_offset,
                                  const Vec3&     sigma_offset,
                                  int             sign) noexcept;

    // Direct accessors (read-only).
    const State&     state() const noexcept { return x_; }
    const StateCov&  cov()   const noexcept { return P_; }
    Position         position() const noexcept { return x_.head<3>(); }
    Velocity         velocity() const noexcept { return x_.segment<3>(3); }
    Acceleration     acceleration() const noexcept { return x_.tail<3>(); }
    const QcDiag&    qc_nominal() const noexcept { return qc_nominal_; }

    // Direct mutators — used only by the bootstrap path (track spawn).
    void set_state(const State& x) noexcept { x_ = x; }
    void set_cov(const StateCov& P) noexcept { P_ = P; symmetrize(P_); }

private:
    State    x_  = State::Zero();
    StateCov P_  = StateCov::Identity();
    QcDiag   qc_nominal_ = QcDiag::Ones();
};

}  // namespace roc::kinematics
