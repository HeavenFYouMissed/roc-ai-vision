// =============================================================================
// test_regime — verifies §2.6.1 EMA-NIS regime detector and §2.6.2 piecewise-
// linear ρ(η̄).
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "roc/kinematics/regime_detector.hpp"

using namespace roc::kinematics;

TEST_CASE("EMA-NIS converges to a held constant (§2.6.1)", "[regime]") {
    NisRegimeDetector det;
    det.reset();
    REQUIRE(std::abs(det.eta_bar() - NisRegimeDetector::kChiSqMean) < 1e-12);

    const double target = 7.5;
    for (int i = 0; i < 1000; ++i) {
        det.update(target);
    }
    REQUIRE(std::abs(det.eta_bar() - target) < 1e-3);
}

TEST_CASE("ρ piecewise-linear interpolation matches §2.6.2 breakpoints", "[regime][rho]") {
    NisRegimeDetector det;
    det.reset();   // η̄ = 4 → ρ = 1
    REQUIRE(std::abs(det.qc_scale() - NisRegimeDetector::kRhoMid) < 1e-12);

    // At η̄ = η_low → ρ = ρ_low.
    while (det.eta_bar() > NisRegimeDetector::kEtaLow + 1e-9) {
        det.update(0.0);   // pull EMA toward 0
    }
    // Force exact value via repeated 1.0 NIS updates after near-zero.
    NisRegimeDetector det2;
    det2.reset();
    // Drive EMA to exactly 1.0 by feeding 1.0 a thousand times.
    for (int i = 0; i < 5000; ++i) det2.update(1.0);
    REQUIRE(std::abs(det2.eta_bar() - 1.0) < 1e-3);
    REQUIRE(std::abs(det2.qc_scale() - NisRegimeDetector::kRhoLow) < 1e-3);

    // At η̄ = η_high → ρ = ρ_high.
    NisRegimeDetector det3;
    det3.reset();
    for (int i = 0; i < 5000; ++i) det3.update(15.0);
    REQUIRE(std::abs(det3.eta_bar() - 15.0) < 1e-3);
    REQUIRE(std::abs(det3.qc_scale() - NisRegimeDetector::kRhoHigh) < 1e-3);

    // Exactly halfway between low and mid.
    NisRegimeDetector det4;
    det4.reset();
    const double mid_low = 0.5 * (NisRegimeDetector::kEtaLow + NisRegimeDetector::kChiSqMean);
    for (int i = 0; i < 5000; ++i) det4.update(mid_low);
    const double rho_expected_mid_low =
        0.5 * (NisRegimeDetector::kRhoLow + NisRegimeDetector::kRhoMid);
    REQUIRE(std::abs(det4.qc_scale() - rho_expected_mid_low) < 1e-3);

    // Exactly halfway between mid and high.
    NisRegimeDetector det5;
    det5.reset();
    const double mid_high = 0.5 * (NisRegimeDetector::kChiSqMean + NisRegimeDetector::kEtaHigh);
    for (int i = 0; i < 5000; ++i) det5.update(mid_high);
    const double rho_expected_mid_high =
        0.5 * (NisRegimeDetector::kRhoMid + NisRegimeDetector::kRhoHigh);
    REQUIRE(std::abs(det5.qc_scale() - rho_expected_mid_high) < 1e-3);
}

TEST_CASE("Regime transitions fire at thresholds (§2.6.1)", "[regime][transition]") {
    // Stationary: η̄ < η_low.
    NisRegimeDetector det;
    det.reset();
    for (int i = 0; i < 5000; ++i) det.update(0.5);
    REQUIRE(det.regime() == Regime::Stationary);

    // Steady: η_low ≤ η̄ ≤ η_high.
    NisRegimeDetector det2;
    det2.reset();   // already η̄ = 4 → STEADY
    REQUIRE(det2.regime() == Regime::Steady);
    for (int i = 0; i < 5000; ++i) det2.update(8.0);
    REQUIRE(det2.regime() == Regime::Steady);

    // Maneuver: η̄ > η_high.
    NisRegimeDetector det3;
    det3.reset();
    for (int i = 0; i < 5000; ++i) det3.update(20.0);
    REQUIRE(det3.regime() == Regime::Maneuver);
}

TEST_CASE("ρ is monotone non-decreasing in η̄", "[regime][rho]") {
    NisRegimeDetector probe;
    double prev_rho = -1.0;
    for (double eta = 0.0; eta <= 25.0; eta += 0.25) {
        // Hack: directly drive eta_bar by feeding many updates at this value.
        NisRegimeDetector d;
        d.reset();
        for (int i = 0; i < 5000; ++i) d.update(eta);
        const double rho = d.qc_scale();
        REQUIRE(rho >= prev_rho - 1e-9);
        prev_rho = rho;
    }
}
