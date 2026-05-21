#pragma once

// =============================================================================
// roc::vision::manifest_loader
//
// YAML → ManifestSpec parser with the §3.4 validation chain wired in.
//
// Validation order (matches §3.4):
//   1. Schema validation     — every required key present, types correct.
//   2. Shape cross-check     — see InferenceEngine; this loader checks YAML
//                              well-formedness, not the live ORT session.
//   3. Class-id range        — every classes[*].id in [0, output.num_classes).
//   4. roc_label validity    — every roc_label resolves to a Label != Unknown.
//   5. EP feasibility        — runtime_hints.preferred_ep is a known EP.
//   6. Postprocessor existence — output.head is in {yolo26_detect, osnet_reid}.
//
// On failure: returns std::nullopt and writes a human-readable description
// to `*err_out` (if non-null).  No exceptions ever leave this class.
// =============================================================================

#include "roc/vision/manifest.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace roc {
namespace vision {

class ManifestLoader {
public:
    // Load + validate.  Returns nullopt on any §3.4 deficiency.
    static std::optional<ManifestSpec>
    load(const std::filesystem::path& yaml_path, std::string* err_out = nullptr);

    // Parse a YAML string in memory (test/debug convenience).  Same validation
    // rules as the path-based overload — file existence is the only difference.
    static std::optional<ManifestSpec>
    load_from_string(const std::string& yaml_text, std::string* err_out = nullptr);
};

}  // namespace vision
}  // namespace roc
