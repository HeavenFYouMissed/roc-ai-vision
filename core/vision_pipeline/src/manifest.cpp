// =============================================================================
// roc::vision::manifest — string → Label resolver (§3.4 #4).
// =============================================================================

#include "roc/vision/manifest.hpp"

namespace roc {
namespace vision {

Label label_from_string(std::string_view s) noexcept {
    if (s == "Person")            return Label::Person;
    if (s == "Head")              return Label::Head;
    if (s == "UpperTorso")        return Label::UpperTorso;
    if (s == "AccessoryHat")      return Label::AccessoryHat;
    if (s == "AccessoryBackpack") return Label::AccessoryBackpack;
    if (s == "Unknown")           return Label::Unknown;
    return Label::Unknown;
}

}  // namespace vision
}  // namespace roc
