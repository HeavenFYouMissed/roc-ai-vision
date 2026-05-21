#pragma once

// =============================================================================
// roc::kinematics::pinhole_measurement
//
// Pinhole projection model (§3.2 / §3.4 / §3.5) and selective-axis
// initialisation policy (§3.10).  All operations are fixed-size Eigen.
//
// The general §3.4 derivation is the runtime path; the §3.5 closed form
// (axis-permuted level camera) is exposed as a static helper used as the
// unit-test oracle.
// =============================================================================

#include "roc/kinematics/types.hpp"
#include "roc/kinematics/object_priors.hpp"

#include <optional>

namespace roc::kinematics {

// ---------------------------------------------------------------------------
// Init failure / selective-axis classification (§3.10.3).
// ---------------------------------------------------------------------------
enum class InitMode : std::uint8_t {
    Full         = 0,   // both axes valid, κ ≤ κ_disagree
    HeightOnly   = 1,   // contact_x ∧ ¬contact_y → use z_h
    WidthOnly    = 2,   // contact_y ∧ ¬contact_x → use z_w
};

enum class InitFailureReason : std::uint8_t {
    BothAxesClipped     = 0,
    DepthDisagreement   = 1,
    BoxDimensionTooSmall = 2,
    DegenerateZ         = 3,
    DegenerateIntrinsics = 4,
};

struct InitResult {
    State    x0;
    StateCov P0;
    InitMode mode = InitMode::Full;
    bool     boundary_probation = false;   // true for HeightOnly / WidthOnly
    double   z_estimate = 0.0;
    double   kappa = 1.0;
    bool     contact_x = false;
    bool     contact_y = false;
};

// ---------------------------------------------------------------------------
// PinholeMeasurement — wraps a single camera pose, exposes h(x) and H(x).
// ---------------------------------------------------------------------------
class PinholeMeasurement {
public:
    // §3.10.2 boundary-margin (px).
    static constexpr double kEpsilonBoundaryPx = 4.0;
    // §3.10.1 disagreement gate.
    static constexpr double kKappaDisagree = 1.6;
    // §3.10.4 / §3.8 covariance bootstrap parameters.
    static constexpr double kSigmaV0 = 2.0;       // m/s
    static constexpr double kSigmaA0 = 2.0;       // m/s²
    static constexpr double kPzInflateProbation = 4.0;

    PinholeMeasurement() = default;
    explicit PinholeMeasurement(CameraPose pose) noexcept : pose_(pose) {}

    // ---- accessors --------------------------------------------------------
    const CameraPose& pose() const noexcept { return pose_; }
    void set_pose(const CameraPose& pose) noexcept { pose_ = pose; }

    // §3.2 — project a world point to a pixel.  Returns (u, v) in F_I.
    // Z_C ≤ kZMin returns std::nullopt (caller must skip the update §3.9).
    std::optional<Pixel> project(const Position& p_world) const noexcept;

    // §3.4 full 4-row h(x) and H(x).  Returns std::nullopt when the
    // linearisation has degenerated (Z_C ≤ kZMin).
    std::optional<Meas>     compute_h(const State&         x,
                                      const WorldExtents&  we) const noexcept;
    std::optional<Jacobian> compute_H(const State&         x,
                                      const WorldExtents&  we) const noexcept;

    // §3.4 combined helper — computes both h and H in a single pass and
    // returns false if Z_C is degenerate.  Avoids re-projecting the world
    // point twice.
    bool compute_h_and_H(const State& x,
                         const WorldExtents& we,
                         Meas& h_out,
                         Jacobian& H_out) const noexcept;

    // §3.10.3 four-case selective-axis initialisation.  Returns nullopt for
    // DEFERRED with `*reason` populated.
    std::optional<InitResult> initialize_from_detection(
        const Detection& det,
        const WorldExtents& we,
        InitFailureReason* reason = nullptr) const noexcept;

    // §3.5 simplified-camera closed-form Jacobian.  Used as the §9 unit-test
    // oracle against the §3.4 general derivation.
    static Jacobian compute_H_axis_aligned(const State& x,
                                           const Vec3&  t_cw,
                                           const Mat3&  K,
                                           const WorldExtents& we) noexcept;

private:
    CameraPose pose_;
};

}  // namespace roc::kinematics
