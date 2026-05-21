---
name: roc-onnx-inference
description: >-
  ONNX Runtime C++ API patterns for implementing the ROC vision pipeline.
  Covers CUDA/TensorRT/DirectML EP selection, zero-copy tensor mapping from
  OpenCV, RAII memory management, model introspection, and quantisation
  strategy. Use when working on core/vision_pipeline/, implementing
  InferenceEngine, or integrating ONNX models.
disable-model-invocation: true
---

# ROC ONNX Inference Patterns

Load this skill before implementing anything under `core/vision_pipeline/`.

**Authoritative research (in priority order — newer overrides older):**
1. `workspace_blueprint/research_notes/phase_1_universal_ep_and_manifest.md` — **BINDING** for EP selection (DirectML/OpenVINO/CPU priority), model manifest sidecar format, YOLO26m + OSNet-AIN postprocessors. Read this first.
2. `workspace_blueprint/research_notes/phase_1_onnx_gpu_core.md` — Detailed CUDA/TensorRT EP code; **opt-in only** when `ROC_VISION_BUILD_CUDA_EP=ON`. The DML/OV path is the production default.
3. `workspace_blueprint/research_notes/phase_1_inference_and_targeting.md` — Foundational letterbox math + introspection sketch.

## Architecture Overview

`InferenceEngine` wraps `Ort::Session` and must:
1. Programmatically query ONNX model metadata at boot (input/output names, shapes).
2. Preprocess via dynamic letterbox scaling (OpenCV).
3. Execute inference with zero unnecessary copies.
4. Return raw output tensors for downstream NMS + detection extraction.

## Ort::Env (Singleton)

```cpp
Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "roc_vision_inference");
```

- Created **exactly once** per process. Movable, not copyable.
- Store as a member or global; do not recreate.

## Execution Provider Selection

### Priority Order (Windows x64) — UPDATED 2026-05-17

The production EP priority is **cross-vendor first**, not CUDA-first. Rationale: the revised hardware profile demands the same binary run on AMD/Intel/NVIDIA without a CUDA Toolkit install.

1. **DirectML EP** — DEFAULT. Universal DX12 path (NVIDIA, AMD, Intel, Qualcomm). FP16 native; no INT8 acceleration. Single-stream.
2. **OpenVINO EP** — Intel CPU/iGPU/NPU. Only EP with native INT8 acceleration. Required for INT8-quantised models.
3. **CPU EP** — Universal fallback (always present). Development / degraded mode only — production target is GPU-class latency.
4. **CUDA EP / TensorRT EP** — **OPT-IN ONLY** via `option(ROC_VISION_BUILD_CUDA_EP "" OFF)`. For power users on dedicated NVIDIA hardware. Code paths live in `phase_1_onnx_gpu_core.md` but are conditionally compiled out by default.

Probe-and-fallback at session construction: try each EP in priority order; the first that creates a session AND runs a warm-up inference within 250 ms wins. See `phase_1_universal_ep_and_manifest.md` §1.2 for the full algorithm.

### DirectML EP (production default)

```cpp
#include <dml_provider_factory.h>

Ort::SessionOptions opts;
opts.SetExecutionMode(ORT_SEQUENTIAL);   // DML is single-stream
opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
opts.DisableMemPattern();                // required when using DML + I/O binding
opts.SetIntraOpNumThreads(1);

const int dml_device_id = 0;             // 0 = OS-preferred adapter
Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_DML(opts, dml_device_id));
```

Use `IDXGIFactory4::EnumAdapters1` to enumerate available DX12 devices and pass an explicit index when targeting a specific GPU. Tensors that the DML EP consumes must be allocated with `Ort::MemoryInfo("DML", OrtAllocatorType::OrtArenaAllocator, dml_device_id, OrtMemTypeDefault)`.

### OpenVINO EP (Intel CPU/iGPU/NPU)

```cpp
std::unordered_map<std::string, std::string> ov_opts{
    {"device_type", "GPU"},
    {"precision",   "FP16"},
    {"cache_dir",   "C:\\roc\\ov_cache"}    // mandatory for predictable startup
};
opts.AppendExecutionProvider_OpenVINO_V2(ov_opts);
```

`cache_dir` must exist and be writable. First-run compilation is 5–60 s; cached runs start in milliseconds. INT8-quantised models REQUIRE this EP (DML cannot accelerate INT8).

### CUDA EP (V2 — opt-in only, NVIDIA dedicated hardware)

```cpp
OrtCUDAProviderOptionsV2* cuda_opts = nullptr;
Ort::ThrowOnError(Ort::GetApi().CreateCUDAProviderOptions(&cuda_opts));
const char* keys[]   = {"device_id", "cudnn_conv_algo_search", "use_tf32", "prefer_nhwc"};
const char* values[] = {"0",         "1",                       "1",        "1"};
Ort::ThrowOnError(Ort::GetApi().UpdateCUDAProviderOptions(cuda_opts, keys, values, 4));
session_options.AppendExecutionProvider_CUDA_V2(*cuda_opts);
Ort::GetApi().ReleaseCUDAProviderOptions(cuda_opts);
```

- `prefer_nhwc=1` (ORT 1.20+) measurably improves Ampere+ GPUs.
- `enable_cuda_graph=0` initially — enable after validation.

### TensorRT EP

```cpp
OrtTensorRTProviderOptionsV2* trt_opts = nullptr;
Ort::ThrowOnError(Ort::GetApi().CreateTensorRTProviderOptions(&trt_opts));
const char* trt_keys[]   = {"device_id", "trt_max_workspace_size", "trt_fp16_enable",
                             "trt_engine_cache_enable", "trt_engine_cache_path"};
const char* trt_values[] = {"0", "4294967296", "1", "1", "C:\\trt_cache\\"};
Ort::ThrowOnError(Ort::GetApi().UpdateTensorRTProviderOptions(trt_opts, trt_keys, trt_values, 5));
session_options.AppendExecutionProvider_TensorRT_V2(*trt_opts);
Ort::GetApi().ReleaseTensorRTProviderOptions(trt_opts);
```

- Cache dir must exist, have write permissions, and preferably be on SSD.
- First run builds the TRT engine (slow); subsequent runs load from cache.

## Model Introspection

```cpp
Ort::AllocatorWithDefaultOptions allocator;
auto name_ptr = session.GetInputNameAllocated(0, allocator);
std::string input_name(name_ptr.get()); // copy before ptr goes out of scope

auto type_info = session.GetInputTypeInfo(0);
auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
std::vector<int64_t> shape = tensor_info.GetShape(); // -1 = dynamic dim
```

Store discovered metadata in a `ModelMeta` struct. Handle dynamic shapes (`-1`) by overriding spatial dims at runtime.

### RAII Pitfall: AllocatedStringPtr

`GetInputNameAllocated` returns `Ort::AllocatedStringPtr` (a `unique_ptr` with custom deleter). **Copy the string immediately** — do not store the raw `char*`.

```cpp
// WRONG — dangling pointer after statement
const char* name = session.GetInputNameAllocated(0, allocator).get();

// CORRECT — copy into std::string
auto ptr = session.GetInputNameAllocated(0, allocator);
std::string name(ptr.get());
```

## Zero-Copy Tensor Mapping from OpenCV

### Letterbox Pipeline

1. Scale: `s = min(dst_w/src_w, dst_h/src_h)`
2. Resize preserving aspect ratio.
3. Pad with gray `(114, 114, 114)` to exact model input dims.
4. BGR→RGB, HWC→CHW, uint8→float32, divide by 255.0.

### Creating Ort::Value (no extra copy)

```cpp
Ort::MemoryInfo mem_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
std::vector<int64_t> shape = {1, channels, height, width};
Ort::Value tensor = Ort::Value::CreateTensor<float>(
    mem_info, data_ptr, data_size, shape.data(), shape.size());
```

`CreateTensor` does **not** take ownership. The data buffer must outlive the `Ort::Value`.

## Quantisation Strategy

| Model | Precision | Rationale |
|-------|-----------|-----------|
| Detection (YOLO family) | FP16 | Best throughput/accuracy trade-off on Ampere+ |
| GRU / temporal embedding | INT8 QDQ | Recurrent weights tolerate quantisation well; saves memory bandwidth |
| Appearance embedding (ReID) | FP16 | L2-normalised output; INT8 would degrade cosine similarity |

Use ONNX Runtime's built-in quantisation tools or TensorRT's implicit FP16 mode (`trt_fp16_enable=1`).

## SessionOptions Checklist

```cpp
Ort::SessionOptions opts;
opts.SetIntraOpNumThreads(1);    // GPU path rarely benefits from >1
opts.SetInterOpNumThreads(1);
opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
opts.DisableMemPattern();        // required when mixing CUDA graphs + I/O binding
opts.EnableCpuMemArena();
```

## Project Constraints

- **Target:** Windows x64, single-machine, **any DX12 GPU** (AMD Radeon, Intel Arc/Iris, NVIDIA RTX) — the binary must NOT require CUDA Toolkit on the host.
- **Latency budget:** < 33 ms per frame (30 fps pipeline); current YOLO26m + OSNet-AIN x1.0 pipeline sits at ~12 ms total on DML FP16.
- **RAII mandatory:** zero raw pointers for resource management (`unique_ptr` / `shared_ptr`).
- **No hardcoded dimensions:** engine must handle any ONNX input shape dynamically; cross-check against the manifest's `input.expected_shape`.
- **No naked ONNX loads:** every model MUST have a co-located `.yaml` manifest per `phase_1_universal_ep_and_manifest.md` §3.3 — the engine refuses unmanifested files.
- **Build system:** CMake + Ninja, linking against `onnxruntime.dll` + `DirectML.dll` (and optionally `onnxruntime_providers_cuda.dll` when `ROC_VISION_BUILD_CUDA_EP=ON`).

## Key Files

| File | Role |
|------|------|
| `core/vision_pipeline/CMakeLists.txt` | Build configuration (stub → real in Phase 4) |
| `core/vision_pipeline/include/roc/vision/` | Public headers (pending Phase 4) |
| `core/vision_pipeline/src/` | Implementation (pending Phase 4) |
| `workspace_blueprint/research_notes/phase_1_universal_ep_and_manifest.md` | **BINDING** spec — EP probe, manifest schema, decoders |
| `workspace_blueprint/research_notes/phase_1_onnx_gpu_core.md` | CUDA/TRT EP reference (opt-in only) |
| `workspace_blueprint/research_notes/phase_1_inference_and_targeting.md` | Letterbox math + shape querying (foundational) |
| `models/<name>.onnx` + `models/<name>.yaml` | Production model + mandatory manifest sidecar |

## Windows-Specific Notes

- `DirectML.dll` ships with Windows 10 1903+ and Windows 11 — no separate install needed.
- OpenVINO runtime requires Intel Distribution install OR the ORT-OpenVINO prebuilt package.
- Use `session_options.EnableProfiling("C:\\traces")` during perf work.
- Ensure `cache_dir` for OpenVINO and (opt-in) TRT cache directories exist with write permissions.
- DML adapter index 0 = OS-preferred; use `IDXGIFactory4::EnumAdapters1` for explicit selection.
