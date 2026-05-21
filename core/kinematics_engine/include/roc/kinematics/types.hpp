#pragma once

// =============================================================================
// roc::kinematics::types
//
// Compile-time-fixed Eigen types that bind every hot-path data structure to
// the contract laid out in `phase_2_ekf_kinematics.md` §1.2.  Dynamic-size
// `MatrixXd / MatrixXf` is permitted ONLY at the association layer where the
// (#tracks × #detections) shape is data-dependent (§4.7).
//
// Cross-references throughout the code base use the same § numbering as the
// blueprint to make verification audit-friendly.
// =============================================================================

#include <Eigen/Dense>

#include <array>
#include <cstdint>
#include <limits>

namespace roc::kinematics {

// ---------------------------------------------------------------------------
// Scalar precision
// ---------------------------------------------------------------------------
// Filter math runs at double precision per §1.2; the wire payload (§7) and
// appearance embeddings (§4.1) downcast to float for transmission.
using Scalar = double;
using EmbScalar = float;

// ---------------------------------------------------------------------------
// State / covariance / measurement (binding §1.2 contract)
// ---------------------------------------------------------------------------
// State layout: [ p_x p_y p_z | v_x v_y v_z | a_x a_y a_z ] in F_W (metres,
// metres/s, metres/s²).  Position indices {0,1,2}, velocity {3,4,5},
// acceleration {6,7,8}.
using State    = Eigen::Matrix<Scalar, 9, 1>;
using StateCov = Eigen::Matrix<Scalar, 9, 9>;

// Measurement: [u, v, w_px, h_px]^T — capture-frame pixels.
using Meas     = Eigen::Matrix<Scalar, 4, 1>;
using MeasCov  = Eigen::Matrix<Scalar, 4, 4>;
using Jacobian = Eigen::Matrix<Scalar, 4, 9>;

// Sub-matrix slices we use repeatedly (centroid-only update path, §3.10.4).
using Meas2     = Eigen::Matrix<Scalar, 2, 1>;
using MeasCov2  = Eigen::Matrix<Scalar, 2, 2>;
using Jacobian2 = Eigen::Matrix<Scalar, 2, 9>;

// 3-vector aliases for the kinematic blocks.
using Vec3        = Eigen::Matrix<Scalar, 3, 1>;
using Mat3        = Eigen::Matrix<Scalar, 3, 3>;
using Position    = Vec3;
using Velocity    = Vec3;
using Acceleration = Vec3;

// Continuous-time process-noise PSD diagonal (§2.2 Qc).
using QcDiag = Eigen::Matrix<Scalar, 3, 1>;

// Appearance embedding — 128-D L2-normalised vector (§4.1).
inline constexpr int kEmbeddingDim = 128;
using Embedding = Eigen::Matrix<EmbScalar, kEmbeddingDim, 1>;

// ---------------------------------------------------------------------------
// Strong typedef for track identifiers — prevents accidental conversion
// from raw uint64_t.
// ---------------------------------------------------------------------------
struct TrackId {
    std::uint64_t value{0};

    constexpr TrackId() = default;
    constexpr explicit TrackId(std::uint64_t v) noexcept : value(v) {}

    constexpr bool operator==(TrackId o) const noexcept { return value == o.value; }
    constexpr bool operator!=(TrackId o) const noexcept { return value != o.value; }
    constexpr bool operator<(TrackId o)  const noexcept { return value <  o.value; }
};
inline constexpr TrackId kInvalidTrackId{0};

// ---------------------------------------------------------------------------
// Sub-feature label hierarchy (binding §7.2 wire enum)
// ---------------------------------------------------------------------------
enum class Label : std::uint8_t {
    Unknown          = 0,
    Person           = 1,
    Head             = 2,
    UpperTorso       = 3,
    AccessoryHat     = 4,
    AccessoryBackpack = 5,
};

inline constexpr bool is_parent_label(Label l) noexcept {
    return l == Label::Person;
}
inline constexpr bool is_subfeature_label(Label l) noexcept {
    return l == Label::Head
        || l == Label::UpperTorso
        || l == Label::AccessoryHat
        || l == Label::AccessoryBackpack;
}

// ---------------------------------------------------------------------------
// Pixel struct — light-weight container; not an Eigen vector because the wire
// format and detector callbacks treat (u, v) as separate fields.
// ---------------------------------------------------------------------------
struct Pixel {
    double u{0.0};
    double v{0.0};
};

// ---------------------------------------------------------------------------
// Camera pose (intrinsics + extrinsics) — used by PinholeMeasurement.  The
// extrinsics map a vector in F_W into F_C: p_C = R_cw * p_W + t_cw  (§3.2).
// ---------------------------------------------------------------------------
struct CameraPose {
    Mat3 K       = Mat3::Identity();    // intrinsics
    Mat3 R_cw    = Mat3::Identity();    // R_cw — world→camera rotation
    Vec3 t_cw    = Vec3::Zero();        // t_cw — translation
    int  img_w   = 0;                   // image width  (px) — for §3.10.2 contact gate
    int  img_h   = 0;                   // image height (px)
};

// ---------------------------------------------------------------------------
// Detection — what the upstream Phase 1 pipeline produces (§4.6 lifecycle).
// ---------------------------------------------------------------------------
struct Detection {
    Pixel        bbox_center;   // (u, v) — centroid in capture-frame px
    double       bbox_w_px = 0; // box width  (px), already inverse-letterbox
    double       bbox_h_px = 0; // box height (px)
    double       confidence = 1.0;
    Label        label = Label::Unknown;
    Embedding    appearance = Embedding::Zero();   // L2-normalised, §4.1

    // Convenience: reconstruct the four corners.
    constexpr double left()   const noexcept { return bbox_center.u - 0.5 * bbox_w_px; }
    constexpr double right()  const noexcept { return bbox_center.u + 0.5 * bbox_w_px; }
    constexpr double top()    const noexcept { return bbox_center.v - 0.5 * bbox_h_px; }
    constexpr double bottom() const noexcept { return bbox_center.v + 0.5 * bbox_h_px; }
};

// ---------------------------------------------------------------------------
// Numeric guards (§9 checklist).
// ---------------------------------------------------------------------------
inline constexpr double kDtMin = 0.005;   // §0 ground-rule (5  ms)
inline constexpr double kDtMax = 0.300;   // §0 ground-rule (300 ms)
inline constexpr double kZMin  = 0.30;    // §3.9 — pinhole removable singularity
inline constexpr double kBboxDimMin = 4.0;// §3.9 — selective measurement gate

inline double clamp_dt(double dt) noexcept {
    if (dt < kDtMin) return kDtMin;
    if (dt > kDtMax) return kDtMax;
    return dt;
}

// Project P → ½(P+Pᵀ) — invariant after every prediction and Joseph update.
inline void symmetrize(StateCov& P) noexcept {
    P = 0.5 * (P + P.transpose()).eval();
}

}  // namespace roc::kinematics
