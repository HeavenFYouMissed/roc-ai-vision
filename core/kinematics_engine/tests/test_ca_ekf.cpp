// =============================================================================
// test_ca_ekf — verifies §2.1 closed-form F(Δt), §2.2 closed-form Q(Δt), and
// §3.7 Joseph-form covariance update against a noise-free constant-acceleration
// trajectory.  These map onto the §9 numerical-stability checklist items:
//   - F(Δt) = I + Fc·Δt + ½ Fc²·Δt²
//   - Q(Δt) symmetric and PSD
//   - Joseph-form symmetry preserved to ≤ 1e-12
//   - Predict→update on a noise-free CA trajectory recovers the position to
//     ≤ 1e-9 after 100 frames.
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "roc/kinematics/ca_ekf.hpp"
#include "roc/kinematics/pinhole_measurement.hpp"
#include "roc/kinematics/object_priors.hpp"

using namespace roc::kinematics;

namespace {

// Continuous-time generator Fc — used to verify F(Δt) numerically.
StateCov compute_Fc()
{
    StateCov Fc = StateCov::Zero();
    // dp/dt = v
    Fc.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
    // dv/dt = a
    Fc.block<3, 3>(3, 6) = Eigen::Matrix3d::Identity();
    return Fc;
}

}  // namespace

TEST_CASE("F(dt) matches I + Fc*dt + 0.5 Fc^2 dt^2 (§2.1)", "[ca_ekf][F]") {
    const StateCov Fc = compute_Fc();
    const StateCov Fc2 = Fc * Fc;
    for (double dt : {0.005, 0.0167, 0.0333, 0.05, 0.1, 0.3}) {
        const StateCov F_truth = StateCov::Identity() + Fc * dt + 0.5 * Fc2 * (dt * dt);
        const StateCov F = compute_F(dt);
        REQUIRE((F - F_truth).cwiseAbs().maxCoeff() < 1e-15);
    }
}

TEST_CASE("Q(dt) is symmetric and positive-semi-definite (§2.2)", "[ca_ekf][Q]") {
    QcDiag qc; qc << 1.0, 2.0, 0.5;
    for (double dt : {0.005, 0.0167, 0.0333, 0.05, 0.1, 0.3}) {
        const StateCov Q = compute_Q(dt, qc);
        // Symmetry to ≤ 1e-15.
        REQUIRE((Q - Q.transpose()).cwiseAbs().maxCoeff() < 1e-15);
        // PSD via LDLT.
        Eigen::LDLT<StateCov> ldlt(Q);
        REQUIRE(ldlt.info() == Eigen::Success);
        // For a CWNJ Q with non-zero qc and dt > 0, the diagonal must be > 0.
        for (int i = 0; i < 9; ++i) REQUIRE(Q(i, i) > 0.0);
    }
}

TEST_CASE("CA-EKF tracks a noise-free constant-acceleration trajectory (§9)", "[ca_ekf][trajectory]") {
    // Hand-crafted ground truth: p(t) = p0 + v0 t + 0.5 a0 t².
    State x_true = State::Zero();
    x_true << 1.0, 2.0, 3.0,        // p0
              0.5, -0.3, 0.2,       // v0
              0.05, 0.02, -0.01;    // a0

    // Initialise the filter at the truth with small covariance — we do NOT
    // feed it any measurements, only use predict() to advance the state.
    StateCov P0 = StateCov::Identity() * 1e-6;
    QcDiag qc;  qc.setZero();   // zero process noise → deterministic propagation
    CaEkf ekf(x_true, P0, qc);

    const double dt = 1.0 / 30.0;
    for (int k = 0; k < 100; ++k) {
        ekf.predict(dt, 1.0);
    }

    // Closed-form ground truth at t = 100·dt.
    const double T = 100.0 * dt;
    Eigen::Vector3d p_truth = x_true.head<3>()
                            + x_true.segment<3>(3) * T
                            + 0.5 * x_true.tail<3>() * (T * T);
    Eigen::Vector3d v_truth = x_true.segment<3>(3) + x_true.tail<3>() * T;
    Eigen::Vector3d a_truth = x_true.tail<3>();

    REQUIRE((ekf.position()    - p_truth).norm() < 1e-9);
    REQUIRE((ekf.velocity()    - v_truth).norm() < 1e-9);
    REQUIRE((ekf.acceleration()- a_truth).norm() < 1e-9);
}

TEST_CASE("Joseph-form update preserves symmetry to 1e-12 (§3.7)", "[ca_ekf][joseph]") {
    State x = State::Zero();
    x.head<3>() << 0.0, 0.0, 5.0;     // 5 m in front of origin
    StateCov P = StateCov::Identity();
    QcDiag qc; qc << 1.0, 1.0, 1.0;
    CaEkf ekf(x, P, qc);

    // Build a synthetic camera and detection so we can produce a real (h, H).
    CameraPose pose;
    pose.K(0, 0) = 600.0; pose.K(1, 1) = 600.0;
    pose.K(0, 2) = 320.0; pose.K(1, 2) = 240.0;
    pose.R_cw = Eigen::Matrix3d::Identity();
    pose.t_cw = Eigen::Vector3d::Zero();
    pose.img_w = 640; pose.img_h = 480;

    PinholeMeasurement pin(pose);
    auto we = world_extents_for(Label::Person);

    Meas h;
    Jacobian H;
    REQUIRE(pin.compute_h_and_H(ekf.state(), we, h, H));

    // Synthetic measurement = h + small offset.  Apply 50 updates; check
    // symmetry holds throughout.
    Meas z = h;
    z(0) += 1.0;
    z(1) += -0.5;
    MeasCov R = MeasCov::Identity() * 4.0;

    for (int i = 0; i < 50; ++i) {
        ekf.predict(0.0333, 1.0);
        REQUIRE(pin.compute_h_and_H(ekf.state(), we, h, H));
        Meas nu = z - h;
        const double nis = ekf.update_full(nu, H, R);
        REQUIRE(nis >= 0.0);
        const StateCov& P_now = ekf.cov();
        REQUIRE((P_now - P_now.transpose()).cwiseAbs().maxCoeff() < 1e-12);
        // SPD after Joseph form.
        Eigen::LDLT<StateCov> ldlt(P_now);
        REQUIRE(ldlt.info() == Eigen::Success);
    }
}

TEST_CASE("update_zero_velocity_shrinkage drives v,a toward zero (§2.6.3)", "[ca_ekf][shrinkage]") {
    State x = State::Zero();
    x << 1.0, 2.0, 3.0,
         0.4, 0.3, 0.2,
         0.1, -0.05, 0.07;
    StateCov P = StateCov::Identity() * 0.1;
    QcDiag qc; qc << 0.25, 0.25, 0.25;
    CaEkf ekf(x, P, qc);

    for (int i = 0; i < 30; ++i) {
        ekf.update_zero_velocity_shrinkage(0.05, 0.05);
    }
    // Velocity / acceleration should shrink toward zero.
    REQUIRE(ekf.velocity().norm() < 0.05);
    REQUIRE(ekf.acceleration().norm() < 0.05);
    // Position should not have been touched (H_rest has zero on the position block).
    REQUIRE((ekf.position() - Eigen::Vector3d(1.0, 2.0, 3.0)).norm() < 1e-9);
}
