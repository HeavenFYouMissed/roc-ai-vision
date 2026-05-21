// =============================================================================
// roc::kinematics::track — anchor TU; the Track aggregate's methods are inline
// in the header for performance, but we still want a stable .o file so the
// debugger can locate the type definition through a symbol.
// =============================================================================

#include "roc/kinematics/track.hpp"

namespace roc::kinematics {

// Force linker to keep at least one symbol from this TU even with -Wl,--gc-sections.
extern const char kTrackAnchor[] = "roc::kinematics::Track";

}  // namespace roc::kinematics
