#pragma once

// =============================================================================
// roc::vision::ep_probe
//
// Probe-and-fallback execution-provider selector per §1.2 of the binding spec.
// Production priority list is `{DirectML, OpenVINO, Cpu}` (§1.1); the optional
// `Cuda` entry is only honoured when the translation unit was compiled with
// `ROC_VISION_HAVE_CUDA_EP` defined (§1.6 — see CMakeLists option
// `ROC_VISION_BUILD_CUDA_EP`).
//
// The implementation honours the §1.2 acceptance criteria:
//   1. `Ort::Session` construction succeeds without exception.
//   2. (Deferred to caller — InferenceEngine does the warm-up run.)
//   3. (Deferred to caller — profiling-based all-ops-native check.)
//
// `EpProbe::probe()` itself returns as soon as criterion #1 is satisfied for
// the first EP in the priority list; criteria #2 and #3 are enforced by
// `InferenceEngine` once it owns the session.  Splitting it this way avoids
// constructing the session twice.
// =============================================================================

#include "roc/vision/types.hpp"

#include <onnxruntime_cxx_api.h>

#include <filesystem>
#include <string>
#include <vector>

namespace roc {
namespace vision {

class EpProbe {
public:
    // Default production priority list (DML → OV → CPU, plus Cuda iff built in).
    static std::vector<EpKind> default_priority();

    // §1.2 algorithm.  Configures `opts_out` for the chosen EP (any prior
    // configuration of `opts_out` is preserved EXCEPT for fields the EP probe
    // explicitly sets — execution_mode, intra_op threads, graph opt level,
    // mem-pattern toggle).  `priority` is required to be non-empty; an empty
    // span yields `Cpu` (universal floor).
    //
    // On success returns the chosen EP description; `opts_out` is ready to be
    // passed to `Ort::Session(env, model_path, opts_out)` by the caller.
    //
    // On failure (every EP raised an Ort::Exception): returns a result with
    // `chosen == EpKind::Cpu` and `device_description` prefixed
    // "FALLBACK::CPU::after_failures::" listing the EPs that failed.  The CPU
    // EP is itself attempted last as the universal floor — if even CPU EP
    // construction fails, this function throws Ort::Exception (the caller
    // treats this as a fatal model-load error per §2 risk row "All EPs fail").
    static EpProbeResult
    probe(Ort::Env&                          env,
          const ModelPath&                   onnx_path,
          Ort::SessionOptions&               opts_out,
          Span<const EpKind>                 priority);

    // Test-only: probe with an explicit priority list expressed as a plain
    // vector.  Wraps the Span overload for convenience.
    static EpProbeResult
    probe(Ort::Env&                          env,
          const ModelPath&                   onnx_path,
          Ort::SessionOptions&               opts_out,
          const std::vector<EpKind>&         priority);

    // Returns true at runtime iff the ROC_VISION_HAVE_DML_EP compile-time
    // define was set AND the DML provider library is loadable.  Used by tests
    // that want to gracefully skip DML-specific assertions on CI machines
    // without a DX12 device.
    static bool dml_available_at_runtime() noexcept;

    // Same for OpenVINO.
    static bool openvino_available_at_runtime() noexcept;

    // Same for CUDA (only true when `ROC_VISION_HAVE_CUDA_EP` was built in).
    static bool cuda_available_at_runtime() noexcept;
};

}  // namespace vision
}  // namespace roc
