#pragma once

// =============================================================================
// roc::vision::inference_engine
//
// End-to-end orchestrator per §6 data flow:
//
//   cv::Mat frame  ──► LetterboxPreprocessor ──► Ort::Session.Run ──►
//                                                            ▲
//                                                            │
//                                                            ▼
//                                          Postprocessor (yolo26 / osnet) ──►
//                                                          ──► Detection[] / Embedding[]
//
// One `InferenceEngine` owns one `Ort::Session`, one `ManifestSpec`, one
// `LetterboxPreprocessor`, and a non-owning pointer into the
// `PostprocessorRegistry` selected via the manifest's `output.head` field.
//
// Multi-session pipelines (e.g. detection + ReID) instantiate two engines
// that share the application-level `Ort::Env` singleton (§1.3 — Ort::Env is
// constructed exactly once per process).
//
// Both `run` and `run_batched` are noexcept on the hot path: any transient
// Ort::Exception is caught internally and yields an empty result (§"No
// exceptions in the hot path" in the worker spec).  Fatal initialisation
// errors propagate as exceptions out of the constructor.
// =============================================================================

#include "roc/vision/letterbox.hpp"
#include "roc/vision/manifest.hpp"
#include "roc/vision/postprocessor.hpp"
#include "roc/vision/postprocessor_registry.hpp"
#include "roc/vision/types.hpp"

#include <onnxruntime_cxx_api.h>
#include <opencv2/core.hpp>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace roc {
namespace vision {

struct InferenceEngineConfig {
    std::vector<EpKind> ep_priority{};  // empty → EpProbe::default_priority()
    bool                profile_first_run{false};
};

// Result of `try_swap_model` (Phase 4 hot-swap path).
// `loaded` == true means the new session is now armed and the next `run()`
// will use it.  `loaded` == false means the swap failed and the previous
// session remains active; `error` carries the human-readable reason.
struct SwapResult {
    bool        loaded{false};
    std::string error{};
    std::string active_model{};   // name of the manifest now in effect
    EpKind      active_ep{EpKind::Cpu};
};

class InferenceEngine {
public:
    // Loads manifest_path, runs §3.4 validation, creates Ort::Session via
    // §1.2 probe-and-fallback, cross-checks introspection shape against
    // manifest input.expected_shape (§3.4 #2).  Throws Ort::Exception or
    // std::runtime_error on any fatal init failure.
    InferenceEngine(Ort::Env&                         env,
                    const ModelPath&                  model_path,
                    const std::filesystem::path&      manifest_path,
                    PostprocessorRegistry&            registry,
                    const InferenceEngineConfig&      cfg = {});

    // Detection-style run.  Returns empty vector and logs on transient
    // failure (e.g. degenerate frame).  Never throws on the hot path.
    std::vector<Detection> run(const cv::Mat& frame_bgr) noexcept;

    // ReID-style batched run.  Same noexcept contract.
    std::vector<Embedding> run_batched(Span<const cv::Mat> crops_bgr) noexcept;

    // --- Hot model swap (Phase 4) -----------------------------------------
    // Build a brand-new Ort::Session from `manifest_path` + `model_path`,
    // then atomically swap it in to replace the active session. Safe to call
    // from any thread other than the inference thread executing `run()` —
    // a short mutex guards the swap, while the inference thread takes a
    // shared lock for the duration of one Ort::Run call. The old session is
    // released when the last `run()` referencing it returns.
    //
    // On failure, the previous session remains active and the returned
    // SwapResult.loaded is false with an explanatory `error`. NEVER leaves
    // the engine in a broken state — partial loads are rolled back.
    SwapResult try_swap_model(const ModelPath&             model_path,
                              const std::filesystem::path& manifest_path,
                              PostprocessorRegistry&       registry) noexcept;

    const ManifestSpec&    manifest()   const noexcept { return manifest_; }
    const EpProbeResult&   ep_result()  const noexcept { return ep_result_; }
    const std::string&     input_name() const noexcept { return input_name_; }
    const std::vector<std::string>& output_names() const noexcept { return output_names_; }

private:
    void build_session(Ort::Env& env, const ModelPath& model_path);
    void introspect();

    Ort::Env*                                env_{nullptr};
    Ort::SessionOptions                      opts_{};
    std::unique_ptr<Ort::Session>            session_{};
    ManifestSpec                             manifest_{};
    LetterboxPreprocessor                    preproc_;
    Postprocessor*                           postproc_{nullptr};
    InferenceEngineConfig                    cfg_{};
    EpProbeResult                            ep_result_{};

    std::string                              input_name_;
    std::vector<std::string>                 output_names_;
    std::vector<const char*>                 input_name_cstrs_;
    std::vector<const char*>                 output_name_cstrs_;

    // Reusable scratch buffers (zero per-frame heap allocation in steady state).
    std::vector<float>                       input_buf_;
    std::vector<LetterboxParams>             batch_letterbox_;

    // Hot-swap synchronisation. `swap_mtx_` serialises swappers and the
    // inference thread's read of the session pointer family. The inference
    // thread holds it for at most one Ort::Run call. Swap latency is therefore
    // bounded by the duration of one inference frame.
    mutable std::mutex                       swap_mtx_;
};

}  // namespace vision
}  // namespace roc
