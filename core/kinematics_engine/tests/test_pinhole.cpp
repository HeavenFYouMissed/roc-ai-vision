// =============================================================================
// test_pinhole — verifies §3.4 general H derivation against finite-difference
// Jacobian, the §3.5 simplified case, and the §3.10.3 four-case selective-axis
// initialisation policy.
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "roc/kinematics/pinhole_measurement.hpp"
#include "roc/kinematics/object_priors.hpp"

#include <cmath>

using namespace roc::kinematics;

namespace {

CameraPose make_test_pose(bool axis_aligned = true)
{
    CameraPose pose;
    pose.K(0, 0) = 800.0; pose.K(1, 1) = 800.0;
    pose.K(0, 2) = 320.0; pose.K(1, 2) = 240.0;
    pose.img_w = 640; pose.img_h = 480;
    if (axis_aligned) {
        pose.R_cw = Eigen::Matrix3d::Identity();
        pose.t_cw = Eigen::Vector3d::Zero();
    } else {
        // Small rotation about Y: tilt + slight pan.
        const double a = 0.13;       // ~7.5 deg
        const double b = 0.07;
        Eigen::Matrix3d Ry;
        Ry << std::cos(a), 0.0, std::sin(a),
              0.0,         1.0, 0.0,
             -std::sin(a), 0.0, std::cos(a);
        Eigen::Matrix3d Rx;
        Rx << 1.0, 0.0,         0.0,
              0.0, std::cos(b),-std::sin(b),
              0.0, std::sin(b), std::cos(b);
        pose.R_cw = Rx * Ry;
        pose.t_cw << 0.05, -0.03, 0.0;
    }
    return pose;
}

}  // namespace

TEST_CASE("§3.4 H matches numerical Jacobian within 1e-6", "[pinhole][jacobian]") {
    CameraPose pose = make_test_pose(false);
    PinholeMeasurement pin(pose);
    auto we = world_extents_for(Label::Person);

    State x = State::Zero();
    x.head<3>() << 0.7, -0.4, 6.0;

    Meas h0;
    Jacobian H_analytic;
    REQUIRE(pin.compute_h_and_H(x, we, h0, H_analytic));

    Jacobian H_num = Jacobian::Zero();
    const double eps = 1e-6;
    for (int i = 0; i < 3; ++i) {
        State xp = x;
        State xm = x;
        xp(i) += eps;
        xm(i) -= eps;
        auto hp = pin.compute_h(xp, we);
        auto hm = pin.compute_h(xm, we);
        REQUIRE(hp.has_value());
        REQUIRE(hm.has_value());
        H_num.col(i) = (*hp - *hm) / (2.0 * eps);
    }
    // velocity / acceleration columns should be exactly zero (no contribution).
    for (int i = 3; i < 9; ++i) H_num.col(i).setZero();

    const double err = (H_analytic - H_num).cwiseAbs().maxCoeff();
    REQUIRE(err < 1e-6);
}

TEST_CASE("§3.5 axis-aligned closed form == §3.4 general derivation",
          "[pinhole][jacobian]") {
    CameraPose pose = make_test_pose(true);
    PinholeMeasurement pin(pose);
    auto we = world_extents_for(Label::Head);

    State x = State::Zero();
    x.head<3>() << 0.5, -0.2, 4.0;

    Jacobian H_general = *pin.compute_H(x, we);
    Jacobian H_simple  = PinholeMeasurement::compute_H_axis_aligned(x, pose.t_cw, pose.K, we);
    REQUIRE((H_general - H_simple).cwiseAbs().maxCoeff() < 1e-12);
}

TEST_CASE("initialize_from_detection — full case (§3.10.3 case 1)",
          "[pinhole][init]") {
    CameraPose pose = make_test_pose(true);
    PinholeMeasurement pin(pose);

    Detection d;
    d.label = Label::Person;
    d.bbox_center.u = 320.0;   // image centre
    d.bbox_center.v = 240.0;
    // World extents 0.55 × 1.75 at Z = 5 m → w_px = 800·0.55/5 = 88, h_px = 800·1.75/5 = 280.
    d.bbox_w_px = 88.0;
    d.bbox_h_px = 280.0;
    d.confidence = 0.9;

    auto init = pin.initialize_from_detection(d, kPriorPerson);
    REQUIRE(init.has_value());
    REQUIRE(init->mode == InitMode::Full);
    REQUIRE_FALSE(init->boundary_probation);
    REQUIRE(std::abs(init->z_estimate - 5.0) < 1e-9);
}

TEST_CASE("initialize_from_detection — left-clipped → HeightOnly (§3.10.3 case 2)",
          "[pinhole][init]") {
    CameraPose pose = make_test_pose(true);
    PinholeMeasurement pin(pose);

    // Box left edge at u=2 px → contact_x triggered (margin 4 px).
    Detection d;
    d.label = Label::Person;
    d.bbox_w_px = 50.0;            // truncated width
    d.bbox_h_px = 280.0;           // height intact → z_h = 5 m
    d.bbox_center.u = 2.0 + 0.5 * d.bbox_w_px;   // left edge ≈ 2 px
    d.bbox_center.v = 240.0;

    auto init = pin.initialize_from_detection(d, kPriorPerson);
    REQUIRE(init.has_value());
    REQUIRE(init->mode == InitMode::HeightOnly);
    REQUIRE(init->boundary_probation);
    // z_h = fy * H_0 / h_px = 800 * 1.75 / 280 = 5.0
    REQUIRE(std::abs(init->z_estimate - 5.0) < 1e-9);
    REQUIRE(init->contact_x);
    REQUIRE_FALSE(init->contact_y);
}

TEST_CASE("initialize_from_detection — top-clipped → WidthOnly (§3.10.3 case 3)",
          "[pinhole][init]") {
    CameraPose pose = make_test_pose(true);
    PinholeMeasurement pin(pose);

    Detection d;
    d.label = Label::Person;
    d.bbox_w_px = 88.0;     // width intact → z_w = 5 m
    d.bbox_h_px = 100.0;    // truncated
    d.bbox_center.u = 320.0;
    // Top edge ≈ 1 px → contact_y triggered.
    d.bbox_center.v = 1.0 + 0.5 * d.bbox_h_px;

    auto init = pin.initialize_from_detection(d, kPriorPerson);
    REQUIRE(init.has_value());
    REQUIRE(init->mode == InitMode::WidthOnly);
    REQUIRE(init->boundary_probation);
    REQUIRE(std::abs(init->z_estimate - 5.0) < 1e-9);
    REQUIRE(init->contact_y);
    REQUIRE_FALSE(init->contact_x);
}

TEST_CASE("initialize_from_detection — both clipped → DEFERRED (§3.10.3 case 4)",
          "[pinhole][init]") {
    CameraPose pose = make_test_pose(true);
    PinholeMeasurement pin(pose);

    Detection d;
    d.label = Label::Person;
    d.bbox_w_px = 60.0;
    d.bbox_h_px = 100.0;
    d.bbox_center.u = 1.0 + 0.5 * d.bbox_w_px;   // left clipped
    d.bbox_center.v = 1.0 + 0.5 * d.bbox_h_px;   // top  clipped

    InitFailureReason reason{};
    auto init = pin.initialize_from_detection(d, kPriorPerson, &reason);
    REQUIRE_FALSE(init.has_value());
    REQUIRE(reason == InitFailureReason::BothAxesClipped);
}

TEST_CASE("initialize_from_detection — depth disagreement κ > 1.6 deferred",
          "[pinhole][init]") {
    CameraPose pose = make_test_pose(true);
    PinholeMeasurement pin(pose);

    Detection d;
    d.label = Label::Person;
    // Force κ > 1.6: w_px corresponds to z_w=10 m, h_px to z_h=5 m.
    d.bbox_w_px = 800.0 * 0.55 / 10.0;    // ≈ 44
    d.bbox_h_px = 800.0 * 1.75 / 5.0;     // 280
    d.bbox_center.u = 320.0;
    d.bbox_center.v = 240.0;

    InitFailureReason reason{};
    auto init = pin.initialize_from_detection(d, kPriorPerson, &reason);
    REQUIRE_FALSE(init.has_value());
    REQUIRE(reason == InitFailureReason::DepthDisagreement);
}

TEST_CASE("project rejects Z < kZMin (§3.9)", "[pinhole][edge]") {
    CameraPose pose = make_test_pose(true);
    PinholeMeasurement pin(pose);

    Eigen::Vector3d p_close{0.0, 0.0, 0.1};   // 10 cm in front
    REQUIRE_FALSE(pin.project(p_close).has_value());

    Eigen::Vector3d p_ok{0.0, 0.0, 5.0};
    auto px = pin.project(p_ok);
    REQUIRE(px.has_value());
    REQUIRE(std::abs(px->u - 320.0) < 1e-9);
    REQUIRE(std::abs(px->v - 240.0) < 1e-9);
}
