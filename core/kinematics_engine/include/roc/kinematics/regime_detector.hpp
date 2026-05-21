#pragma once

// =============================================================================
// roc::kinematics::regime_detector
//
// EMA-NIS regime detector (§2.6.1) and adaptive jerk-PSD scale ρ(η̄) (§2.6.2).
// One instance per Track; updated with the post-update NIS scalar
// ν^T S^{-1} ν every frame.
//
// Initialisation: η̄_0 = E[χ²_4] = 4 (§9 checklist) — starting at zero would
// register every fresh track as STATIONARY for the first ~10 frames and
// strangle its acceleration channel before convergence.
// =============================================================================

#include <algorithm>
#include <cstdint>

namespace roc::kinematics {

enum class Regime : std::uint8_t {
    Stationary = 0,
    Steady     = 1,
    Maneuver   = 2,
};

class NisRegimeDetector {
public:
    // §2.6.1 / §2.6.2 constants — verbatim from the boxed equations.
    static constexpr double kEmaAlpha   = 0.1;     // §2.6.1 α
    static constexpr double kEtaLow     = 1.0;     // §2.6.1 η_low
    static constexpr double kChiSqMean  = 4.0;     // E[χ²_4]
    static constexpr double kEtaHigh    = 15.0;    // §2.6.1 η_high
    static constexpr double kRhoLow     = 0.05;    // §2.6.2 ρ at η_low
    static constexpr double kRhoMid     = 1.0;     // §2.6.2 ρ at χ² mean
    static constexpr double kRhoHigh    = 4.0;     // §2.6.2 ρ at η_high

    NisRegimeDetector() noexcept = default;

    // §9 — boot at the χ² mean so a brand-new track is in STEADY.
    void reset() noexcept { eta_bar_ = kChiSqMean; }

    void update(double nis) noexcept {
        // Guard: NIS is non-negative by construction; clamp to avoid NaN
        // poisoning the EMA if the upstream ever feeds a malformed value.
        if (!(nis >= 0.0)) {
            return;
        }
        eta_bar_ = (1.0 - kEmaAlpha) * eta_bar_ + kEmaAlpha * nis;
    }

    double eta_bar() const noexcept { return eta_bar_; }

    Regime regime() const noexcept {
        if (eta_bar_ < kEtaLow)  return Regime::Stationary;
        if (eta_bar_ > kEtaHigh) return Regime::Maneuver;
        return Regime::Steady;
    }

    // §2.6.2 piecewise-linear ρ(η̄) clamped to [kRhoLow, kRhoHigh].
    // Three segments: [η_low, χ²_mean] linearly from ρ_low → ρ_mid,
    // and (χ²_mean, η_high] linearly from ρ_mid → ρ_high.  Outside
    // those bounds the function is clamped to the endpoint values.
    double qc_scale() const noexcept {
        const double e = eta_bar_;
        if (e <= kEtaLow)    return kRhoLow;
        if (e >= kEtaHigh)   return kRhoHigh;
        if (e <= kChiSqMean) {
            const double t = (e - kEtaLow) / (kChiSqMean - kEtaLow);
            return kRhoLow + t * (kRhoMid - kRhoLow);
        }
        const double t = (e - kChiSqMean) / (kEtaHigh - kChiSqMean);
        return kRhoMid + t * (kRhoHigh - kRhoMid);
    }

private:
    double eta_bar_ = kChiSqMean;
};

}  // namespace roc::kinematics
