// =============================================================================
// roc::kinematics::ca_ekf — Joseph-form measurement update implementations.
// All methods are noexcept and avoid any heap allocation; Eigen's
// `noalias()` and intermediate `eval()` calls are sized so the temporaries
// fit inside fixed Matrix expressions.
// =============================================================================

#include "roc/kinematics/ca_ekf.hpp"

#include <Eigen/Cholesky>

namespace roc::kinematics {

namespace {

// §3.7 Joseph-form covariance update for an arbitrary measurement size M.
// Templated by Jacobian rows so we can share the same body between the full
// 4-row update, the centroid-only 2-row update, the §2.6.3 6-row shrinkage,
// and the §3.11 3-row constraint pseudo-measurement.
//
// Inputs:
//   nu_M : innovation vector (M × 1)
//   H_M  : Jacobian (M × 9)
//   R_M  : measurement noise (M × M, SPD)
//   x    : state (9 × 1, in/out)
//   P    : covariance (9 × 9, in/out — Joseph + symmetrise)
// Returns:
//   NIS scalar  ν^T S^{-1} ν   (≥ 0)
template <int M>
double joseph_update(const Eigen::Matrix<Scalar, M, 1>& nu,
                     const Eigen::Matrix<Scalar, M, 9>& H,
                     const Eigen::Matrix<Scalar, M, M>& R,
                     State& x,
                     StateCov& P) noexcept
{
    using MatMM = Eigen::Matrix<Scalar, M, M>;
    using Mat9M = Eigen::Matrix<Scalar, 9, M>;
    using Mat99 = StateCov;

    // S = H P Hᵀ + R
    const Mat9M PHt = P * H.transpose();
    MatMM S = H * PHt + R;
    S = 0.5 * (S + S.transpose()).eval();   // symmetrise S before factoring

    // Cholesky-solve K instead of forming S^{-1} explicitly (§3.7).
    Eigen::LLT<MatMM> S_llt(S);
    if (S_llt.info() != Eigen::Success) {
        // §3.9 — degenerate linearisation; bail without touching state.
        return -1.0;
    }
    // K = P Hᵀ S^{-1}  ⇔  K^T = S^{-1} (H P)
    // Note: PHt is 9×M, so K = (S^{-1} PHtᵀ)^T  — but PHt is already P Hᵀ.
    // We want X with shape 9×M satisfying X S = PHt.
    // S is symmetric SPD; using the LLT solver with S y = b gives y = S^{-1} b.
    // We need PHt * S^{-1} → so transpose, solve, transpose back.
    const Mat9M K = S_llt.solve(PHt.transpose()).transpose();

    // NIS: ν^T S^{-1} ν via one extra solve on the M-vector.
    const Eigen::Matrix<Scalar, M, 1> S_inv_nu = S_llt.solve(nu);
    const double nis = nu.dot(S_inv_nu);

    // State update.
    x.noalias() += K * nu;

    // Joseph form covariance update.
    Mat99 IKH = Mat99::Identity();
    IKH.noalias() -= K * H;
    Mat99 P_new = IKH * P * IKH.transpose();
    P_new.noalias() += K * R * K.transpose();
    P_new = 0.5 * (P_new + P_new.transpose()).eval();
    P = P_new;
    return nis;
}

}  // namespace

double CaEkf::update_full(const Meas& nu,
                          const Jacobian& H,
                          const MeasCov&  R) noexcept
{
    return joseph_update<4>(nu, H, R, x_, P_);
}

double CaEkf::update_centroid_only(const Meas2&     nu_uv,
                                   const Jacobian2& H_uv,
                                   const MeasCov2&  R_uv) noexcept
{
    return joseph_update<2>(nu_uv, H_uv, R_uv, x_, P_);
}

void CaEkf::update_zero_velocity_shrinkage(double sigma_v, double sigma_a) noexcept
{
    // §2.6.3:  z_rest = 0_6,  H_rest = [0_{6×3} | I_6],  R_rest = diag(σ_v² I_3, σ_a² I_3)
    using Vec6   = Eigen::Matrix<Scalar, 6, 1>;
    using Mat69  = Eigen::Matrix<Scalar, 6, 9>;
    using Mat66  = Eigen::Matrix<Scalar, 6, 6>;

    Mat69 H = Mat69::Zero();
    H.block<6, 6>(0, 3).setIdentity();

    Mat66 R = Mat66::Zero();
    const double v2 = sigma_v * sigma_v;
    const double a2 = sigma_a * sigma_a;
    R(0, 0) = v2; R(1, 1) = v2; R(2, 2) = v2;
    R(3, 3) = a2; R(4, 4) = a2; R(5, 5) = a2;

    // Innovation: ν = z_rest - H x = -H x = -[v; a] (i.e., shrink toward zero)
    Vec6 nu;
    nu.head<3>() = -x_.segment<3>(3);
    nu.tail<3>() = -x_.tail<3>();

    (void)joseph_update<6>(nu, H, R, x_, P_);
}

void CaEkf::update_relative_position(const Position& p_other_world,
                                     const Vec3&     mu_offset,
                                     const Vec3&     sigma_offset,
                                     int             sign) noexcept
{
    // §3.11.2:
    //   For sub-feature S (sign = +1):  h_rel(x_S, x_P) = (p_S - p_P) - μ_S
    //   z_rel = 0;  H_rel^S = [+I_3 | 0_{3×6}];  R_rel = diag(σ_S²)
    //
    //   For parent P  (sign = -1):  same residual, H_rel^P = [-I_3 | 0_{3×6}]
    //
    // Innovation:  ν_rel = z_rel - h_rel = -h_rel
    //   sub-feature side  (sign=+1):  ν_rel = -((p_S - p_P) - μ_S) = (p_P + μ_S) - p_S
    //   parent side       (sign=-1):  ν_rel = -((p_S - p_P) - μ_S) = (p_P + μ_S) - p_S
    //   (residual is identical; the difference shows up via the *Jacobian's*
    //    sign because we are linearising w.r.t. *this* filter only).
    using Mat39  = Eigen::Matrix<Scalar, 3, 9>;
    using Mat33  = Eigen::Matrix<Scalar, 3, 3>;

    Mat39 H = Mat39::Zero();
    if (sign >= 0) {
        H.block<3, 3>(0, 0) =  Mat33::Identity();
    } else {
        H.block<3, 3>(0, 0) = -Mat33::Identity();
    }

    Mat33 R = Mat33::Zero();
    R(0, 0) = sigma_offset(0) * sigma_offset(0);
    R(1, 1) = sigma_offset(1) * sigma_offset(1);
    R(2, 2) = sigma_offset(2) * sigma_offset(2);

    // Residual: ν_rel = z_rel - h_rel = -h_rel = (p_P + μ_S) - p_S.  When
    // updating the sub-feature we have x_self = p_S and p_other = p_P, so
    //   ν = p_other + μ - x_self.
    // When updating the parent we have x_self = p_P and p_other = p_S, so
    //   ν = x_self + μ - p_other = -(p_other - μ - x_self).
    // The Jacobian sign (±I) is absorbed into K via H_rel^{S/P}.
    Vec3 nu_rel;
    if (sign >= 0) {
        nu_rel = (p_other_world + mu_offset) - x_.head<3>();
    } else {
        nu_rel = x_.head<3>() + mu_offset - p_other_world;
    }

    (void)joseph_update<3>(nu_rel, H, R, x_, P_);
}

}  // namespace roc::kinematics
