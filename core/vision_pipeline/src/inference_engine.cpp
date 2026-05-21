// =============================================================================
// roc::vision::InferenceEngine — orchestrator binding §6.
//
// Lifecycle:
//   * Constructor: load + validate manifest, EP-probe, construct session,
//     introspect input/output names + shapes, cross-check vs manifest §3.4 #2.
//   * run(frame):  preprocess via LetterboxPreprocessor → Session::Run →
//                  Postprocessor::decode_detections → return.
//   * run_batched(crops): same but produces Embedding[] via decode_embeddings.
//
// Hot-path noexcept contract: any Ort::Exception inside `run` / `run_batched`
// is swallowed and an empty vector is returned.  Initialisation-time
// exceptions propagate.
// =============================================================================

#include "roc/vision/inference_engine.hpp"
#include "roc/vision/ep_probe.hpp"
#include "roc/vision/manifest_loader.hpp"

#include <onnxruntime_cxx_api.h>

#include <cstring>
#include <iostream>
#include <stdexcept>

namespace roc {
namespace vision {

namespace {

// Wide / UTF-8 path conversion for ORT (Session takes wchar_t* on Windows).
#ifdef _WIN32
std::wstring native_path(const std::filesystem::path& p) { return p.wstring(); }
#else
std::string  native_path(const std::filesystem::path& p) { return p.string(); }
#endif

// Compare introspection vs manifest input shape (§3.4 #2).  Dynamic axes on
// the model side (-1) are accepted — only positive introspection dims must
// match the manifest's positive dims.
bool shapes_compatible(const std::vector<int64_t>& live,
                       const std::array<int64_t, 4>& expected) {
    if (live.size() != expected.size()) return false;
    for (std::size_t i = 0; i < live.size(); ++i) {
        if (live[i] > 0 && live[i] != expected[i]) return false;
    }
    return true;
}

}  // namespace

InferenceEngine::InferenceEngine(Ort::Env&                    env,
                                 const ModelPath&             model_path,
                                 const std::filesystem::path& manifest_path,
                                 PostprocessorRegistry&       registry,
                                 const InferenceEngineConfig& cfg)
    : env_(&env)
    , preproc_(InputSpec{})    // re-initialised below once the manifest loads
    , cfg_(cfg) {
    std::string err;
    auto opt_spec = ManifestLoader::load(manifest_path, &err);
    if (!opt_spec) {
        throw std::runtime_error("manifest load failed: " + err);
    }
    manifest_ = *opt_spec;
    preproc_  = LetterboxPreprocessor(manifest_.input);

    postproc_ = registry.find(manifest_.head_string);
    if (!postproc_) {
        throw std::runtime_error("no postprocessor registered for head '" +
                                 manifest_.head_string + "'");
    }

    build_session(env, model_path);
    introspect();
}

void InferenceEngine::build_session(Ort::Env& env, const ModelPath& model_path) {
    std::vector<EpKind> priority = cfg_.ep_priority.empty()
        ? EpProbe::default_priority()
        : cfg_.ep_priority;

    // The probe constructs a temporary session internally — that wastes one
    // construction but the resulting `opts_` is fully prepared.  Then we
    // construct the real session here.
    ep_result_ = EpProbe::probe(env, model_path, opts_, priority);
    const auto native = native_path(model_path.path());
    session_ = std::unique_ptr<Ort::Session>(new Ort::Session(env, native.c_str(), opts_));
}

void InferenceEngine::introspect() {
    Ort::AllocatorWithDefaultOptions alloc;

    // ---- inputs ----
    const std::size_t n_in = session_->GetInputCount();
    if (n_in == 0) {
        throw std::runtime_error("ONNX model has zero inputs");
    }
    auto in_name_ptr = session_->GetInputNameAllocated(0, alloc);
    input_name_.assign(in_name_ptr.get());

    auto in_type = session_->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo();
    auto in_shape = in_type.GetShape();

    if (!shapes_compatible(in_shape, manifest_.input.expected_shape)) {
        std::string actual = "[";
        for (std::size_t i = 0; i < in_shape.size(); ++i) {
            if (i) actual += ", ";
            actual += std::to_string(in_shape[i]);
        }
        actual += "]";
        std::string expected = "[";
        for (std::size_t i = 0; i < manifest_.input.expected_shape.size(); ++i) {
            if (i) expected += ", ";
            expected += std::to_string(manifest_.input.expected_shape[i]);
        }
        expected += "]";
        throw std::runtime_error("manifest input shape " + expected +
                                 " incompatible with ONNX introspection " + actual);
    }

    // ---- outputs ----
    const std::size_t n_out = session_->GetOutputCount();
    if (n_out == 0) {
        throw std::runtime_error("ONNX model has zero outputs");
    }
    output_names_.clear();
    output_names_.reserve(n_out);
    for (std::size_t i = 0; i < n_out; ++i) {
        auto p = session_->GetOutputNameAllocated(i, alloc);
        output_names_.emplace_back(p.get());
    }

    // Cache the C-string pointer arrays Ort::Session::Run expects.
    input_name_cstrs_.clear();
    input_name_cstrs_.push_back(input_name_.c_str());
    output_name_cstrs_.clear();
    output_name_cstrs_.reserve(output_names_.size());
    for (const auto& s : output_names_) output_name_cstrs_.push_back(s.c_str());
}

std::vector<Detection>
InferenceEngine::run(const cv::Mat& frame_bgr) noexcept {
    std::vector<Detection> empty;
    if (frame_bgr.empty()) return empty;

    // Hold the swap mutex for the duration of one Ort::Run call. Swap
    // requests from other threads will queue until this returns. This is
    // bounded to one inference frame (~10-100 ms) which is acceptable for
    // an infrequent operator action (manual model switch from the dashboard).
    std::lock_guard<std::mutex> lock(swap_mtx_);

    if (!session_ || !postproc_) return empty;

    try {
        LetterboxParams lp{};
        preproc_.preprocess(frame_bgr, input_buf_, lp);

        const int C = manifest_.input_c();
        const int H = manifest_.input_h();
        const int W = manifest_.input_w();
        std::vector<int64_t> shape{1, C, H, W};
        if (manifest_.input.layout == TensorLayout::NHWC) {
            shape = {1, H, W, C};
        }

        Ort::MemoryInfo mem_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            mem_info,
            input_buf_.data(), input_buf_.size(),
            shape.data(), shape.size());

        auto outputs = session_->Run(Ort::RunOptions{nullptr},
                                     input_name_cstrs_.data(), &input_tensor, 1,
                                     output_name_cstrs_.data(), output_name_cstrs_.size());

        PostprocessContext ctx;
        ctx.letterbox        = lp;
        ctx.model_input_hw   = { H, W };
        ctx.capture_input_hw = { frame_bgr.rows, frame_bgr.cols };
        ctx.manifest         = &manifest_;
        ctx.batch_letterbox  = Span<const LetterboxParams>(&lp, 1);
        return postproc_->decode_detections(outputs, ctx);
    } catch (const Ort::Exception& e) {
        std::cerr << "[roc::vision] InferenceEngine::run Ort::Exception: "
                  << e.what() << std::endl;
        return empty;
    } catch (const std::exception& e) {
        std::cerr << "[roc::vision] InferenceEngine::run std::exception: "
                  << e.what() << std::endl;
        return empty;
    }
}

std::vector<Embedding>
InferenceEngine::run_batched(Span<const cv::Mat> crops_bgr) noexcept {
    std::vector<Embedding> empty;
    if (crops_bgr.empty()) return empty;

    std::lock_guard<std::mutex> lock(swap_mtx_);

    if (!session_ || !postproc_) return empty;

    try {
        preproc_.preprocess_batch(crops_bgr, input_buf_, batch_letterbox_);

        const int C = manifest_.input_c();
        const int H = manifest_.input_h();
        const int W = manifest_.input_w();
        const int64_t K = static_cast<int64_t>(crops_bgr.size());
        std::vector<int64_t> shape{K, C, H, W};
        if (manifest_.input.layout == TensorLayout::NHWC) {
            shape = {K, H, W, C};
        }

        Ort::MemoryInfo mem_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            mem_info,
            input_buf_.data(), input_buf_.size(),
            shape.data(), shape.size());

        auto outputs = session_->Run(Ort::RunOptions{nullptr},
                                     input_name_cstrs_.data(), &input_tensor, 1,
                                     output_name_cstrs_.data(), output_name_cstrs_.size());

        PostprocessContext ctx;
        ctx.letterbox        = batch_letterbox_.empty() ? LetterboxParams{} : batch_letterbox_.front();
        ctx.model_input_hw   = { H, W };
        ctx.capture_input_hw = { 0, 0 };
        ctx.manifest         = &manifest_;
        ctx.batch_letterbox  = Span<const LetterboxParams>(batch_letterbox_.data(),
                                                            batch_letterbox_.size());
        return postproc_->decode_embeddings(outputs, ctx);
    } catch (const Ort::Exception& e) {
        std::cerr << "[roc::vision] InferenceEngine::run_batched Ort::Exception: "
                  << e.what() << std::endl;
        return empty;
    } catch (const std::exception& e) {
        std::cerr << "[roc::vision] InferenceEngine::run_batched std::exception: "
                  << e.what() << std::endl;
        return empty;
    }
}

SwapResult
InferenceEngine::try_swap_model(const ModelPath&             model_path,
                                const std::filesystem::path& manifest_path,
                                PostprocessorRegistry&       registry) noexcept {
    SwapResult result;
    try {
        // --- 1. Validate the new manifest BEFORE touching the live state.
        std::string err;
        auto opt_spec = ManifestLoader::load(manifest_path, &err);
        if (!opt_spec) {
            result.error = "manifest load failed: " + err;
            return result;
        }
        ManifestSpec new_manifest = *opt_spec;

        Postprocessor* new_postproc = registry.find(new_manifest.head_string);
        if (!new_postproc) {
            result.error = "no postprocessor registered for head '" +
                           new_manifest.head_string + "'";
            return result;
        }

        // --- 2. Build the new session on the calling thread (slow path).
        //     This may take 100-1000 ms (EP probe + ORT JIT compile). The
        //     inference thread can keep running on the old session in
        //     parallel because we don't hold the mutex yet.
        Ort::SessionOptions new_opts{};
        std::vector<EpKind> priority = cfg_.ep_priority.empty()
            ? EpProbe::default_priority()
            : cfg_.ep_priority;
        EpProbeResult new_ep = EpProbe::probe(*env_, model_path, new_opts, priority);

        const auto native = native_path(model_path.path());
        std::unique_ptr<Ort::Session> new_session(
            new Ort::Session(*env_, native.c_str(), new_opts));

        // --- 3. Cross-check the new ONNX I/O against the new manifest.
        Ort::AllocatorWithDefaultOptions alloc;
        if (new_session->GetInputCount() == 0) {
            result.error = "new ONNX has zero inputs";
            return result;
        }
        auto in_type = new_session->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo();
        if (!shapes_compatible(in_type.GetShape(), new_manifest.input.expected_shape)) {
            result.error = "new ONNX input shape incompatible with new manifest";
            return result;
        }

        // Capture introspection metadata.
        auto in_name_ptr = new_session->GetInputNameAllocated(0, alloc);
        std::string new_input_name(in_name_ptr.get());

        const std::size_t n_out = new_session->GetOutputCount();
        if (n_out == 0) {
            result.error = "new ONNX has zero outputs";
            return result;
        }
        std::vector<std::string> new_output_names;
        new_output_names.reserve(n_out);
        for (std::size_t i = 0; i < n_out; ++i) {
            auto p = new_session->GetOutputNameAllocated(i, alloc);
            new_output_names.emplace_back(p.get());
        }

        // --- 4. Acquire the swap mutex (waits for any in-flight Ort::Run).
        std::lock_guard<std::mutex> lock(swap_mtx_);

        // Commit the swap. Old session_ unique_ptr destructor runs here,
        // freeing the old Ort::Session AFTER we've already pointed the
        // engine at the new one — but the inference thread is blocked on
        // swap_mtx_ so it sees a consistent set of (session_, manifest_,
        // postproc_, names) once we release.
        opts_         = std::move(new_opts);
        session_      = std::move(new_session);
        manifest_     = std::move(new_manifest);
        preproc_      = LetterboxPreprocessor(manifest_.input);
        postproc_     = new_postproc;
        ep_result_    = new_ep;
        input_name_   = std::move(new_input_name);
        output_names_ = std::move(new_output_names);

        // Rebuild the C-string name caches.
        input_name_cstrs_.clear();
        input_name_cstrs_.push_back(input_name_.c_str());
        output_name_cstrs_.clear();
        output_name_cstrs_.reserve(output_names_.size());
        for (const auto& s : output_names_) output_name_cstrs_.push_back(s.c_str());

        // Steady-state scratch buffers will resize themselves on the next run.

        result.loaded       = true;
        result.active_model = manifest_.name;
        result.active_ep    = ep_result_.chosen;
        return result;
    } catch (const Ort::Exception& e) {
        result.error = std::string("Ort::Exception: ") + e.what();
        return result;
    } catch (const std::exception& e) {
        result.error = std::string("std::exception: ") + e.what();
        return result;
    } catch (...) {
        result.error = "unknown exception during swap";
        return result;
    }
}

}  // namespace vision
}  // namespace roc
