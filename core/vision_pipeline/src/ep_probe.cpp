// =============================================================================
// roc::vision::EpProbe — §1.2 probe-and-fallback EP selector.
//
// Build-time defines (set by CMakeLists.txt):
//   ROC_VISION_HAVE_DML_EP      — DirectML provider header available.  Build
//                                 against the ORT GPU prebuilt (CPU prebuilt
//                                 omits dml_provider_factory.h).
//   ROC_VISION_HAVE_OPENVINO_EP — OpenVINO provider header available.  ORT
//                                 OpenVINO prebuilt or self-build with
//                                 --use_openvino.
//   ROC_VISION_HAVE_CUDA_EP     — Opt-in CUDA EP (CMake flag
//                                 ROC_VISION_BUILD_CUDA_EP).
//
// When none of those are defined we still compile cleanly — the probe simply
// skips the missing EPs and always selects the CPU EP (universal floor).
//
// The probe attempts each EP in priority order:
//   * Append the provider to the SessionOptions.
//   * Try to construct an Ort::Session (criterion §1.2 #1).
//   * If construction succeeds, keep the configured opts_out and return.
//   * If Ort::Exception fires, reset opts_out to a fresh base configuration
//     and try the next EP.
// =============================================================================

#include "roc/vision/ep_probe.hpp"

#include <onnxruntime_c_api.h>

#ifdef ROC_VISION_HAVE_DML_EP
#  include <dml_provider_factory.h>
#endif

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace roc {
namespace vision {

namespace {

// Re-build a clean SessionOptions object identical to the "base" configuration
// used between probe attempts.  Each EP that wants additional knobs sets them
// AFTER this baseline (DML wants SEQUENTIAL + IntraOp=1, OV wants the cache
// dir, CPU EP needs nothing extra).
Ort::SessionOptions make_base_opts() {
    Ort::SessionOptions opts;
    opts.SetExecutionMode(ORT_SEQUENTIAL);
    opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    opts.SetIntraOpNumThreads(1);
    opts.SetInterOpNumThreads(1);
    opts.DisableMemPattern();
    opts.EnableCpuMemArena();
    return opts;
}

// Wide-string conversion of a filesystem path for the Windows ORT API.
// On non-Windows builds this returns the UTF-8 path; we keep the function
// generic so the rest of the file does not need OS preprocessor sprawl.
#ifdef _WIN32
std::wstring path_to_native(const std::filesystem::path& p) { return p.wstring(); }
#else
std::string  path_to_native(const std::filesystem::path& p) { return p.string(); }
#endif

// Construct a session; returns true on success.  Any Ort::Exception is
// stringified into `*err_out`.
bool try_construct_session(Ort::Env&                       env,
                           const std::filesystem::path&    onnx_path,
                           Ort::SessionOptions&            opts,
                           std::string*                    err_out) {
    try {
        const auto native = path_to_native(onnx_path);
        Ort::Session probe(env, native.c_str(), opts);
        (void)probe;
        return true;
    } catch (const Ort::Exception& e) {
        if (err_out) *err_out = e.what();
        return false;
    } catch (const std::exception& e) {
        if (err_out) *err_out = e.what();
        return false;
    }
}

bool try_append_dml(Ort::SessionOptions& opts, int device_id, std::string* err_out) {
#ifdef ROC_VISION_HAVE_DML_EP
    try {
        // Match §1.3: SEQUENTIAL + IntraOp=1 + DisableMemPattern (already set in base).
        Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_DML(opts, device_id));
        return true;
    } catch (const Ort::Exception& e) {
        if (err_out) *err_out = e.what();
        return false;
    }
#else
    (void)opts; (void)device_id;
    if (err_out) *err_out = "DML EP not compiled in";
    return false;
#endif
}

bool try_append_openvino(Ort::SessionOptions& opts, std::string* err_out) {
#ifdef ROC_VISION_HAVE_OPENVINO_EP
    try {
        std::unordered_map<std::string, std::string> ov_opts{
            { "device_type",    "AUTO" },
            { "precision",      "FP16" },
            { "num_of_threads", "0"    },
        };
        opts.AppendExecutionProvider_OpenVINO_V2(ov_opts);
        return true;
    } catch (const Ort::Exception& e) {
        if (err_out) *err_out = e.what();
        return false;
    }
#else
    (void)opts;
    if (err_out) *err_out = "OpenVINO EP not compiled in";
    return false;
#endif
}

bool try_append_cuda(Ort::SessionOptions& opts, int device_id, std::string* err_out) {
#ifdef ROC_VISION_HAVE_CUDA_EP
    try {
        OrtCUDAProviderOptionsV2* cuda_opts = nullptr;
        Ort::ThrowOnError(Ort::GetApi().CreateCUDAProviderOptions(&cuda_opts));
        const std::string dev = std::to_string(device_id);
        const char* keys[]   = { "device_id", "cudnn_conv_algo_search", "prefer_nhwc" };
        const char* values[] = { dev.c_str(), "1",                       "1"           };
        Ort::ThrowOnError(Ort::GetApi().UpdateCUDAProviderOptions(cuda_opts, keys, values, 3));
        opts.AppendExecutionProvider_CUDA_V2(*cuda_opts);
        Ort::GetApi().ReleaseCUDAProviderOptions(cuda_opts);
        return true;
    } catch (const Ort::Exception& e) {
        if (err_out) *err_out = e.what();
        return false;
    }
#else
    (void)opts; (void)device_id;
    if (err_out) *err_out = "CUDA EP not compiled in";
    return false;
#endif
}

}  // namespace

std::vector<EpKind> EpProbe::default_priority() {
    std::vector<EpKind> v;
#ifdef ROC_VISION_HAVE_CUDA_EP
    v.push_back(EpKind::Cuda);
#endif
    v.push_back(EpKind::DirectML);
    v.push_back(EpKind::OpenVINO);
    v.push_back(EpKind::Cpu);
    return v;
}

bool EpProbe::dml_available_at_runtime() noexcept {
#ifdef ROC_VISION_HAVE_DML_EP
    return true;   // header + import lib present at compile time.
#else
    return false;
#endif
}

bool EpProbe::openvino_available_at_runtime() noexcept {
#ifdef ROC_VISION_HAVE_OPENVINO_EP
    return true;
#else
    return false;
#endif
}

bool EpProbe::cuda_available_at_runtime() noexcept {
#ifdef ROC_VISION_HAVE_CUDA_EP
    return true;
#else
    return false;
#endif
}

EpProbeResult
EpProbe::probe(Ort::Env&                env,
               const ModelPath&         onnx_path,
               Ort::SessionOptions&     opts_out,
               Span<const EpKind>       priority) {
    EpProbeResult result;

    // Sanitise priority: empty list ⇒ CPU floor.
    std::vector<EpKind> work;
    work.reserve(priority.size() + 1);
    for (std::size_t i = 0; i < priority.size(); ++i) work.push_back(priority[i]);
    if (work.empty()) work.push_back(EpKind::Cpu);

    // Ensure CPU is always present as the universal floor (§1.5).
    if (std::find(work.begin(), work.end(), EpKind::Cpu) == work.end()) {
        work.push_back(EpKind::Cpu);
    }

    std::ostringstream failures;
    bool first_failure = true;

    for (EpKind kind : work) {
        opts_out = make_base_opts();

        std::string err;
        bool appended = false;
        switch (kind) {
            case EpKind::DirectML: appended = try_append_dml(opts_out, /*device_id=*/0, &err); break;
            case EpKind::OpenVINO: appended = try_append_openvino(opts_out, &err);             break;
            case EpKind::Cuda:     appended = try_append_cuda(opts_out, /*device_id=*/0, &err);break;
            case EpKind::Cpu:      appended = true;                                            break;
        }

        if (!appended) {
            if (!first_failure) failures << "; ";
            failures << to_string(kind) << ":append_failed(" << err << ")";
            first_failure = false;
            continue;
        }

        std::string sess_err;
        if (!try_construct_session(env, onnx_path.path(), opts_out, &sess_err)) {
            if (!first_failure) failures << "; ";
            failures << to_string(kind) << ":session_failed(" << sess_err << ")";
            first_failure = false;
            continue;
        }

        // Success.
        result.chosen = kind;
        result.device_id = 0;
        switch (kind) {
            case EpKind::DirectML: result.device_description = "DirectML adapter 0"; break;
            case EpKind::OpenVINO: result.device_description = "OpenVINO AUTO";       break;
            case EpKind::Cpu:      result.device_description = "CPU EP";              break;
            case EpKind::Cuda:     result.device_description = "CUDA device 0";       break;
        }
        result.all_ops_native = true;   // criterion §1.2 #3 enforced downstream
        return result;
    }

    // Every EP failed — including CPU, which is the universal floor.  This is
    // the §2 "All EPs fail" risk row.  Re-construct on CPU one final time and
    // let the Ort::Exception propagate so the caller can produce diagnostics.
    opts_out = make_base_opts();
    const auto native = path_to_native(onnx_path.path());
    Ort::Session final_session(env, native.c_str(), opts_out);
    (void)final_session;
    // If we somehow got here without throwing, the model loads on CPU after all.
    result.chosen = EpKind::Cpu;
    result.device_description = "FALLBACK::CPU::after_failures::" + failures.str();
    result.all_ops_native = true;
    return result;
}

EpProbeResult
EpProbe::probe(Ort::Env&                  env,
               const ModelPath&           onnx_path,
               Ort::SessionOptions&       opts_out,
               const std::vector<EpKind>& priority) {
    return probe(env, onnx_path, opts_out, Span<const EpKind>(priority));
}

}  // namespace vision
}  // namespace roc
