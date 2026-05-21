#pragma once

// =============================================================================
// roc::vision::postprocessor_registry
//
// String-keyed lookup table mapping manifest `output.head` values to
// `Postprocessor` strategy instances.  Pre-registers `yolo26_detect` and
// `osnet_reid` at construction; additional heads may be registered via
// `register_postprocessor`.
//
// Duplicate registration is a §2 risk-row item ("Two manifests claiming the
// same head discriminator") — duplicate keys are rejected at registration
// time with a returned-false signal (no throw; the caller decides whether
// duplicates are fatal).
// =============================================================================

#include "roc/vision/postprocessor.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace roc {
namespace vision {

class PostprocessorRegistry {
public:
    PostprocessorRegistry();   // pre-registers built-in heads

    // Register a new postprocessor.  Returns false if `head_name` was already
    // present (registry unchanged).
    bool register_postprocessor(std::string head_name,
                                std::unique_ptr<Postprocessor> p);

    // Lookup.  Returns nullptr when the head is not registered.
    Postprocessor* find(const std::string& head_name) const noexcept;

    bool contains(const std::string& head_name) const noexcept {
        return entries_.find(head_name) != entries_.end();
    }

    std::size_t size() const noexcept { return entries_.size(); }

private:
    std::unordered_map<std::string, std::unique_ptr<Postprocessor>> entries_;
};

}  // namespace vision
}  // namespace roc
