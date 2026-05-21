// =============================================================================
// roc::kinematics::pinhole_measurement — projection / Jacobian / init policy.
// =============================================================================

#include "roc/kinematics/pinhole_measurement.hpp"

#include <cmath>

namespace roc::kinematics {

std::optional<Pixel> PinholeMeasurement::project(const Position& p_world) const noexcept
{
    const Vec3 p_cam = pose_.R_cw * p_world + pose_.t_cw;
    const double Z = p_cam.z();
    if (!(Z >= kZMin)) return std::nullopt;
    const double inv_z = 1.0 / Z;
    Pixel uv;
    uv.u = pose_.K(0, 0) * (p_cam.x() * inv_z) + pose_.K(0, 2);
    uv.v = pose_.K(1, 1) * (p_cam.y() * inv_z) + pose_.K(1, 2);
    return uv;
}

std::optional<Meas> PinholeMeasurement::compute_h(const State& x,
                                                  const WorldExtents& we) const noexcept
{
    const Vec3 p_w  = x.head<3>();
    const Vec3 p_c  = pose_.R_cw * p_w + pose_.t_cw;
    const double Z  = p_c.z();
    if (!(Z >= kZMin)) return std::nullopt;
    const double inv_z = 1.0 / Z;
    const double fx = pose_.K(0, 0);
    const double fy = pose_.K(1, 1);
    const double cx = pose_.K(0, 2);
    const double cy = pose_.K(1, 2);

    Meas h;
    h(0) = fx * (p_c.x() * inv_z) + cx;
    h(1) = fy * (p_c.y() * inv_z) + cy;
    h(2) = fx * (we.w_world * inv_z);
    h(3) = fy * (we.h_world * inv_z);
    return h;
}

std::optional<Jacobian> PinholeMeasurement::compute_H(const State& x,
                                                      const WorldExtents& we) const noexcept
{
    const Vec3 p_w  = x.head<3>();
    const Vec3 p_c  = pose_.R_cw * p_w + pose_.t_cw;
    const double Z  = p_c.z();
    if (!(Z >= kZMin)) return std::nullopt;
    const double inv_z  = 1.0 / Z;
    const double inv_z2 = inv_z * inv_z;
    const double fx = pose_.K(0, 0);
    const double fy = pose_.K(1, 1);

    // Rows of R_cw — see §3.2: r_i^T transforms p_W into p_C component i.
    const Eigen::RowVector3d r1 = pose_.R_cw.row(0);
    const Eigen::RowVector3d r2 = pose_.R_cw.row(1);
    const Eigen::RowVector3d r3 = pose_.R_cw.row(2);

    Jacobian H = Jacobian::Zero();

    // Row 1 (centroid u): (fx/Z)(r1ᵀ - (X/Z) r3ᵀ)
    H.block<1, 3>(0, 0) = fx * inv_z  * (r1 - (p_c.x() * inv_z) * r3);
    // Row 2 (centroid v): (fy/Z)(r2ᵀ - (Y/Z) r3ᵀ)
    H.block<1, 3>(1, 0) = fy * inv_z  * (r2 - (p_c.y() * inv_z) * r3);
    // Row 3 (box width): -(fx W_0 / Z²) r3ᵀ
    H.block<1, 3>(2, 0) = -(fx * we.w_world) * inv_z2 * r3;
    // Row 4 (box height): -(fy H_0 / Z²) r3ᵀ
    H.block<1, 3>(3, 0) = -(fy * we.h_world) * inv_z2 * r3;

    // Velocity / acceleration columns are zero — already initialised by
    // Jacobian::Zero().
    return H;
}

bool PinholeMeasurement::compute_h_and_H(const State& x,
                                         const WorldExtents& we,
                                         Meas& h_out,
                                         Jacobian& H_out) const noexcept
{
    const Vec3 p_w  = x.head<3>();
    const Vec3 p_c  = pose_.R_cw * p_w + pose_.t_cw;
    const double Z  = p_c.z();
    if (!(Z >= kZMin)) return false;
    const double inv_z  = 1.0 / Z;
    const double inv_z2 = inv_z * inv_z;
    const double fx = pose_.K(0, 0);
    const double fy = pose_.K(1, 1);
    const double cx = pose_.K(0, 2);
    const double cy = pose_.K(1, 2);

    h_out(0) = fx * (p_c.x() * inv_z) + cx;
    h_out(1) = fy * (p_c.y() * inv_z) + cy;
    h_out(2) = fx * (we.w_world * inv_z);
    h_out(3) = fy * (we.h_world * inv_z);

    H_out.setZero();
    const Eigen::RowVector3d r1 = pose_.R_cw.row(0);
    const Eigen::RowVector3d r2 = pose_.R_cw.row(1);
    const Eigen::RowVector3d r3 = pose_.R_cw.row(2);

    H_out.block<1, 3>(0, 0) = fx * inv_z  * (r1 - (p_c.x() * inv_z) * r3);
    H_out.block<1, 3>(1, 0) = fy * inv_z  * (r2 - (p_c.y() * inv_z) * r3);
    H_out.block<1, 3>(2, 0) = -(fx * we.w_world) * inv_z2 * r3;
    H_out.block<1, 3>(3, 0) = -(fy * we.h_world) * inv_z2 * r3;
    return true;
}

Jacobian PinholeMeasurement::compute_H_axis_aligned(const State& x,
                                                    const Vec3&  t_cw,
                                                    const Mat3&  K,
                                                    const WorldExtents& we) noexcept
{
    // §3.5 closed form for R_cw = I (after axis-permutation).  X = p_x + t_x,
    // Y = p_y + t_y, Z = p_z + t_z.  Used as a unit-test oracle.
    const double X  = x(0) + t_cw(0);
    const double Y  = x(1) + t_cw(1);
    const double Z  = x(2) + t_cw(2);
    const double inv_z  = 1.0 / Z;
    const double inv_z2 = inv_z * inv_z;
    const double fx = K(0, 0);
    const double fy = K(1, 1);

    Jacobian H = Jacobian::Zero();
    H(0, 0) = fx * inv_z;
    H(0, 2) = -fx * X * inv_z2;
    H(1, 1) = fy * inv_z;
    H(1, 2) = -fy * Y * inv_z2;
    H(2, 2) = -fx * we.w_world * inv_z2;
    H(3, 2) = -fy * we.h_world * inv_z2;
    return H;
}

// ---------------------------------------------------------------------------
// §3.10.3 selective-axis initialisation.
// ---------------------------------------------------------------------------
std::optional<InitResult> PinholeMeasurement::initialize_from_detection(
    const Detection& det,
    const WorldExtents& we,
    InitFailureReason* reason) const noexcept
{
    auto fail = [&](InitFailureReason r) -> std::optional<InitResult> {
        if (reason) *reason = r;
        return std::nullopt;
    };

    const double w_px = det.bbox_w_px;
    const double h_px = det.bbox_h_px;

    // Box-dim sanity (§3.9 / §3.10).
    if (!(w_px >= kBboxDimMin) || !(h_px >= kBboxDimMin)) {
        return fail(InitFailureReason::BoxDimensionTooSmall);
    }

    const double fx = pose_.K(0, 0);
    const double fy = pose_.K(1, 1);
    if (!(fx > 0.0) || !(fy > 0.0)) {
        return fail(InitFailureReason::DegenerateIntrinsics);
    }
    if (!(we.w_world > 0.0) || !(we.h_world > 0.0)) {
        return fail(InitFailureReason::DegenerateIntrinsics);
    }

    const double z_w = (fx * we.w_world) / w_px;
    const double z_h = (fy * we.h_world) / h_px;

    // §3.10.2 boundary contact predicates.
    const double img_w = static_cast<double>(pose_.img_w);
    const double img_h = static_cast<double>(pose_.img_h);
    const bool valid_image = (img_w > 0.0 && img_h > 0.0);

    bool contact_x = false;
    bool contact_y = false;
    if (valid_image) {
        const double left   = det.left();
        const double right  = det.right();
        const double top    = det.top();
        const double bottom = det.bottom();
        contact_x = (left  <= kEpsilonBoundaryPx) ||
                    (right >= img_w - kEpsilonBoundaryPx);
        contact_y = (top    <= kEpsilonBoundaryPx) ||
                    (bottom >= img_h - kEpsilonBoundaryPx);
    }

    // Disagreement gate κ.
    const double zmax = std::max(z_w, z_h);
    const double zmin = std::min(z_w, z_h);
    const double kappa = (zmin > 0.0) ? (zmax / zmin) : kKappaDisagree * 10.0;

    InitResult res;
    res.contact_x = contact_x;
    res.contact_y = contact_y;
    res.kappa = kappa;

    // §3.10.3 four-case selective policy.
    double Z0 = 0.0;
    if (contact_x && contact_y) {
        return fail(InitFailureReason::BothAxesClipped);
    } else if (!contact_x && !contact_y) {
        if (kappa > kKappaDisagree) {
            return fail(InitFailureReason::DepthDisagreement);
        }
        Z0 = 0.5 * (z_w + z_h);
        res.mode = InitMode::Full;
        res.boundary_probation = false;
    } else if (contact_x && !contact_y) {
        Z0 = z_h;                          // height-based depth survives
        res.mode = InitMode::HeightOnly;
        res.boundary_probation = true;
    } else {
        // contact_y && !contact_x
        Z0 = z_w;
        res.mode = InitMode::WidthOnly;
        res.boundary_probation = true;
    }

    if (!(Z0 > kZMin)) {
        return fail(InitFailureReason::DegenerateZ);
    }
    res.z_estimate = Z0;

    // Lateral state from K^{-1} (u, v, 1) Z0.
    const double cx = pose_.K(0, 2);
    const double cy = pose_.K(1, 2);
    const double X_c = (det.bbox_center.u - cx) * Z0 / fx;
    const double Y_c = (det.bbox_center.v - cy) * Z0 / fy;

    Vec3 p_cam{X_c, Y_c, Z0};
    Vec3 p_world = pose_.R_cw.transpose() * (p_cam - pose_.t_cw);

    State x0 = State::Zero();
    x0.head<3>() = p_world;
    res.x0 = x0;

    // §3.8 / §3.10.4 covariance bootstrap.
    StateCov P0 = StateCov::Identity();

    // Position uncertainty in the camera-frame triplet (X, Y, Z) before
    // rotating into F_W: lateral channels get sub-pixel precision; depth is
    // dominated by Z0² × σ(box_dim)/box_dim ratio.
    const double lateral_sigma_px = kSigmaCentroidPx;
    const double sigma_x_cam = (Z0 / fx) * lateral_sigma_px;
    const double sigma_y_cam = (Z0 / fy) * lateral_sigma_px;
    const double depth_sigma_px = kSigmaBoxDimPx;
    const double sigma_z_cam_w = (Z0 * depth_sigma_px) / std::max(1.0, w_px);
    const double sigma_z_cam_h = (Z0 * depth_sigma_px) / std::max(1.0, h_px);
    double sigma_z_cam = 0.5 * (sigma_z_cam_w + sigma_z_cam_h);

    if (res.boundary_probation) {
        // §3.10.4 ×4 inflation on depth.
        sigma_z_cam *= kPzInflateProbation;
    }

    // Rotate camera-frame diagonal covariance into world frame.
    Mat3 cov_cam = Mat3::Zero();
    cov_cam(0, 0) = sigma_x_cam * sigma_x_cam;
    cov_cam(1, 1) = sigma_y_cam * sigma_y_cam;
    cov_cam(2, 2) = sigma_z_cam * sigma_z_cam;
    Mat3 cov_world = pose_.R_cw.transpose() * cov_cam * pose_.R_cw;
    P0.block<3, 3>(0, 0) = cov_world;

    // Velocity / acceleration: §3.8 large diagonal so the filter rapidly
    // converges over the first few frames.
    const double v2 = kSigmaV0 * kSigmaV0;
    const double a2 = kSigmaA0 * kSigmaA0;
    P0.block<3, 3>(3, 3) = v2 * Mat3::Identity();
    P0.block<3, 3>(6, 6) = a2 * Mat3::Identity();

    res.P0 = P0;
    return res;
}

}  // namespace roc::kinematics
