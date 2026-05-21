#pragma once

// =============================================================================
// roc::kinematics::object_priors
//
// Anthropometric metric priors used by the pinhole measurement model (§3.2)
// and by the hierarchical-constraint pseudo-measurement (§3.11.1).
//
// All priors are constexpr — they are part of the locked kinematic contract,
// not runtime-tunable parameters.  The tables come directly from the boxed
// equations in `phase_2_ekf_kinematics.md`.
// =============================================================================

#include "roc/kinematics/types.hpp"

#include <algorithm>

namespace roc::kinematics {

// ---------------------------------------------------------------------------
// World-extent priors per sub-feature (§3.2 anthropometric table).
// ---------------------------------------------------------------------------
struct WorldExtents {
    double w_world = 0.0;   // metric width  (m), W_0 in §3.2
    double h_world = 0.0;   // metric height (m), H_0
};

// §3.2 table values.
inline constexpr WorldExtents kPriorPerson           {0.55, 1.75};
inline constexpr WorldExtents kPriorHead             {0.18, 0.23};
inline constexpr WorldExtents kPriorUpperTorso       {0.45, 0.55};
inline constexpr WorldExtents kPriorAccessoryHat     {0.30, 0.18};
inline constexpr WorldExtents kPriorAccessoryBackpack{0.35, 0.50};
inline constexpr WorldExtents kPriorUnknown          {0.50, 1.00}; // safe default

inline constexpr WorldExtents world_extents_for(Label label) noexcept {
    switch (label) {
        case Label::Person:            return kPriorPerson;
        case Label::Head:              return kPriorHead;
        case Label::UpperTorso:        return kPriorUpperTorso;
        case Label::AccessoryHat:      return kPriorAccessoryHat;
        case Label::AccessoryBackpack: return kPriorAccessoryBackpack;
        case Label::Unknown:
        default:                       return kPriorUnknown;
    }
}

// ---------------------------------------------------------------------------
// Per-class jerk PSD σ_j  (m·s^(-5/2)), §2.4 tuning ranges.
// Squared per-axis to feed Q_c diagonals directly.
// ---------------------------------------------------------------------------
inline constexpr double kJerkSigmaPerson     = 1.0;
inline constexpr double kJerkSigmaHead       = 4.0;
inline constexpr double kJerkSigmaUpperTorso = 0.5;
inline constexpr double kJerkSigmaAccessory  = 2.0;

inline constexpr double jerk_sigma_for(Label label) noexcept {
    switch (label) {
        case Label::Person:            return kJerkSigmaPerson;
        case Label::Head:              return kJerkSigmaHead;
        case Label::UpperTorso:        return kJerkSigmaUpperTorso;
        case Label::AccessoryHat:      return kJerkSigmaAccessory;
        case Label::AccessoryBackpack: return kJerkSigmaAccessory;
        case Label::Unknown:
        default:                       return kJerkSigmaPerson;
    }
}

inline QcDiag qc_diagonal_for(Label label) noexcept {
    const double s = jerk_sigma_for(label);
    const double s2 = s * s;
    QcDiag q;
    q << s2, s2, s2;
    return q;
}

// ---------------------------------------------------------------------------
// Anatomical offsets and tolerances (§3.11.1 hierarchical constraint).
//
//   μ_S  : sub-feature offset relative to torso anchor (gravity-aligned F_W)
//   σ_S  : 1-σ tolerance per axis used as diag(σ²) in R_rel
//
// Note: §3.11.1 states upper_torso is the anchor (μ = 0) — Person plays the
// role of the parent in this implementation, so for Person↔UpperTorso the
// offset is (0,0,0).  All other sub-features sit relative to the torso /
// person centroid (we treat them as identical in F_W to first order).
// ---------------------------------------------------------------------------
struct AnatomicalOffset {
    Vec3 mu;       // metres, F_W
    Vec3 sigma;    // metres, 1-σ
};

inline AnatomicalOffset anatomical_offset_for(Label sub_label) noexcept {
    AnatomicalOffset out;
    switch (sub_label) {
        case Label::UpperTorso:
            out.mu    << 0.0,  0.0,  0.0;
            out.sigma << 0.02, 0.02, 0.05;
            break;
        case Label::Head:
            out.mu    << 0.0,  0.0,  0.40;
            out.sigma << 0.15, 0.15, 0.25;
            break;
        case Label::AccessoryHat:
            out.mu    << 0.0,  0.0,  0.55;
            out.sigma << 0.20, 0.20, 0.10;
            break;
        case Label::AccessoryBackpack:
            out.mu    << 0.0, -0.20, 0.10;
            out.sigma << 0.20, 0.15, 0.30;
            break;
        case Label::Person:
        case Label::Unknown:
        default:
            // Person itself is the parent — offset undefined; return wide tolerances
            // so any accidental call short-circuits the §3.11.4 plausibility gate.
            out.mu    << 0.0, 0.0, 0.0;
            out.sigma << 1.0, 1.0, 1.0;
            break;
    }
    return out;
}

// ---------------------------------------------------------------------------
// Default measurement-noise R diagonal (§3.6).
// ---------------------------------------------------------------------------
inline constexpr double kSigmaCentroidPx = 2.0;   // σ_u, σ_v
inline constexpr double kSigmaBoxDimPx   = 6.0;   // σ_w, σ_h

inline MeasCov default_meas_cov(double confidence = 1.0) noexcept {
    const double conf = std::max(0.05, confidence);   // clamp to avoid blow-up
    const double scale = 1.0 / (conf * conf);
    MeasCov R = MeasCov::Zero();
    R(0, 0) = kSigmaCentroidPx * kSigmaCentroidPx * scale;
    R(1, 1) = kSigmaCentroidPx * kSigmaCentroidPx * scale;
    R(2, 2) = kSigmaBoxDimPx   * kSigmaBoxDimPx   * scale;
    R(3, 3) = kSigmaBoxDimPx   * kSigmaBoxDimPx   * scale;
    return R;
}

}  // namespace roc::kinematics
