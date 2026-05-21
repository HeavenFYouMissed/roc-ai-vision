// =============================================================================
// roc::kinematics::regime_detector — header carries the implementation; this
// translation unit exists so the library has a stable .o file for the symbol
// `roc::kinematics::NisRegimeDetector` even if the optimiser inlines all the
// member functions away from the header.  Anchored ODR symbol provided here.
// =============================================================================

#include "roc/kinematics/regime_detector.hpp"

namespace roc::kinematics {

const double NisRegimeDetectorAnchor =
    NisRegimeDetector::kEmaAlpha
  + NisRegimeDetector::kEtaLow
  + NisRegimeDetector::kChiSqMean
  + NisRegimeDetector::kEtaHigh
  + NisRegimeDetector::kRhoLow
  + NisRegimeDetector::kRhoMid
  + NisRegimeDetector::kRhoHigh;

}  // namespace roc::kinematics
