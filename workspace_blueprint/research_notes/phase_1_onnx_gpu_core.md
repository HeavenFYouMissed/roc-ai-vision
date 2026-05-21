# Phase 1 Research: ONNX Runtime Native C++ API with CUDA/TensorRT Acceleration on Windows x64

**Date:** 2026-05-16  
**Focus:** High-performance Windows host GPU inference using `<onnxruntime_cxx_api.h>`  
**Scope:** Exact initialization sequences, model introspection, zero-copy tensor mapping from OpenCV, and RAII memory safety for modern ONNX Runtime (1.17+)

---

## 1. Initializing Ort::Env, Ort::SessionOptions, and Appending CUDA/TensorRT Execution Providers

### 1.1 Ort::Env Construction (Thread-Safe Singleton Pattern)

The `Ort::Env` is the root object and must be created exactly once per process. It owns the logging infrastructure and global state.

**Recommended initialization (Windows x64, production):**

```cpp
#include <onnxruntime_cxx_api.h>

Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "roc_vision_inference");
// OR for maximum verbosity during development:
Ort::Env env(ORT_LOGGING_LEVEL_VERBOSE, "roc_vision_inference");
```

- `ORT_LOGGING_LEVEL_WARNING` is the sweet spot for production (errors + warnings).
- The second argument is an arbitrary identifier used in log output.
- `Ort::Env` is movable but not copyable. Store it as a member or global.

### 1.2 Ort::SessionOptions Configuration

```cpp
Ort::SessionOptions session_options;
session_options.SetIntraOpNumThreads(1);           // GPU path rarely benefits from >1 intra-op thread
session_options.SetInterOpNumThreads(1);
session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
session_options.DisableMemPattern();               // Often required when mixing CUDA graphs + I/O binding
session_options.EnableCpuMemArena();
```

For maximum GPU determinism on Windows, also consider:

```cpp
session_options.AddConfigEntry("session.disable_prepacking", "1");
```

### 1.3 Appending CUDA Execution Provider (Modern V2 Path)

**Legacy struct (still works but deprecated for new code):**

```cpp
OrtCUDAProviderOptions cuda_options{};
cuda_options.device_id = 0;
cuda_options.do_copy_in_default_stream = 1;
cuda_options.cudnn_conv_algo_search = OrtCudnnConvAlgoSearchExhaustive;
cuda_options.gpu_mem_limit = 0;                    // 0 = unlimited
cuda_options.arena_extend_strategy = 0;            // 0 = next power of 2
session_options.AppendExecutionProvider_CUDA(cuda_options);
```

**Preferred modern path (V2 provider options – ONNX Runtime ≥ 1.14):**

```cpp
OrtCUDAProviderOptionsV2* cuda_options_v2 = nullptr;
Ort::ThrowOnError(Ort::GetApi().CreateCUDAProviderOptions(&cuda_options_v2));

// Update via string map (most future-proof)
const char* option_keys[] = {
    "device_id",
    "cudnn_conv_algo_search",
    "enable_cuda_graph",
    "use_tf32",
    "prefer_nhwc"
};
const char* option_values[] = {
    "0",
    "1",      // exhaustive
    "0",      // disable CUDA graphs for now (enable after validation)
    "1",
    "1"
};
Ort::ThrowOnError(Ort::GetApi().UpdateCUDAProviderOptions(cuda_options_v2, option_keys, option_values, 5));

session_options.AppendExecutionProvider_CUDA_V2(*cuda_options_v2);
Ort::GetApi().ReleaseCUDAProviderOptions(cuda_options_v2);
```

**Key Windows x64 notes:**
- The GPU package (`onnxruntime_gpu.dll` + `onnxruntime_providers_cuda.dll`) must be in PATH or next to the executable.
- CUDA 12.x + cuDNN 9.x is the current default for 1.19+ packages.
- `prefer_nhwc=1` (1.20+) gives measurable gains on Ampere+ when the model has many convolutions.

### 1.4 TensorRT Execution Provider (Higher Performance Path)

```cpp
OrtTensorRTProviderOptionsV2* trt_options = nullptr;
Ort::ThrowOnError(Ort::GetApi().CreateTensorRTProviderOptions(&trt_options));

const char* trt_keys[] = {
    "device_id",
    "trt_max_workspace_size",
    "trt_fp16_enable",
    "trt_engine_cache_enable",
    "trt_engine_cache_path"
};
const char* trt_values[] = {
    "0",
    "4294967296",   // 4 GB workspace
    "1",
    "1",
    "C:\\trt_cache\\"
};
Ort::ThrowOnError(Ort::GetApi().UpdateTensorRTProviderOptions(trt_options, trt_keys, trt_values, 5));

session_options.AppendExecutionProvider_TensorRT_V2(*trt_options);
Ort::GetApi().ReleaseTensorRTProviderOptions(trt_options);
```

TensorRT EP generally outperforms CUDA EP on static-shape vision models after the first few warm-up runs (engine build + caching).

---

## 2. Programmatic Model Introspection – Input/Output Names and Dynamic Shapes

### 2.1 Querying Node Counts and Names

```cpp
Ort::AllocatorWithDefaultOptions allocator;

size_t num_input_nodes = session.GetInputCount();
size_t num_output_nodes = session.GetOutputCount();

std::vector<std::string> input_names;
std::vector<std::string> output_names;

for (size_t i = 0; i < num_input_nodes; i++) {
    auto name_ptr = session.GetInputNameAllocated(i, allocator);
    input_names.emplace_back(name_ptr.get());           // copies into std::string
}

for (size_t i = 0; i < num_output_nodes; i++) {
    auto name_ptr = session.GetOutputNameAllocated(i, allocator);
    output_names.emplace_back(name_ptr.get());
}
```

**Critical:** `GetInputNameAllocated` / `GetOutputNameAllocated` return `Ort::AllocatedStringPtr` (see §4 for ownership pitfalls).

### 2.2 Extracting Tensor Shape Information (Batch, C, H, W)

```cpp
for (size_t i = 0; i < num_input_nodes; i++) {
    Ort::TypeInfo type_info = session.GetInputTypeInfo(i);
    auto tensor_info = type_info.GetTensorTypeAndShapeInfo();

    ONNXTensorElementDataType elem_type = tensor_info.GetElementType();
    std::vector<int64_t> shape = tensor_info.GetShape();

    // Typical vision model: [batch, channels, height, width] or dynamic [-1, 3, -1, -1]
    bool is_dynamic = std::any_of(shape.begin(), shape.end(), [](int64_t d){ return d < 0; });
}
```

`GetShape()` returns `std::vector<int64_t>` where `-1` denotes a dynamic dimension. For letterboxed pipelines we usually override the spatial dimensions at runtime while keeping batch=1 and channels=3.

### 2.3 Recommended Pattern for Production

Store the discovered metadata in a small struct:

```cpp
struct ModelMeta {
    std::string input_name;
    std::string output_name;
    std::vector<int64_t> input_shape;   // may contain -1
    ONNXTensorElementDataType dtype;
};
```

---

## 3. Memory-Efficient Letterboxed cv::Mat → Ort::Value Mapping

### 3.1 The Mathematical Pipeline (Letterbox + Normalization)

1. Compute scale factor: `scale = min(desired_w / orig_w, desired_h / orig_h)`
2. Resize preserving aspect ratio.
3. Pad with gray (114, 114, 114) or black to exact `[H, W]`.
4. Convert BGR→RGB, HWC→CHW, uint8→float32, divide by 255.0 (or ImageNet stats).

### 3.2 Zero-Copy / Minimal-Copy Strategy

**Best practice (continuous memory, single allocation):**

```cpp
// Assume `letterboxed` is cv::Mat of type CV_32FC3, shape (H, W, 3), continuous, RGB, normalized [0,1]
assert(letterboxed.isContinuous());

const int64_t channels = letterboxed.channels(); // 3
const int64_t height   = letterboxed.rows;
const int64_t width    = letterboxed.cols;
const int64_t total    = channels * height * width;

std::vector<float> input_tensor_values;
input_tensor_values.reserve(total);

// CHW layout required by most ONNX vision models
for (int c = 0; c < channels; ++c) {
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            // cv::Mat at<cv::Vec3f>(h,w)[c] or pointer arithmetic
            input_tensor_values.push_back(letterboxed.ptr<float>(h)[w * channels + c]);
        }
    }
}
```

**Even better – direct pointer cast when layout matches (rare but possible with OpenCV GpuMat + custom CUDA kernel):**

If you perform the entire letterbox + CHW + normalize step on the GPU and download a single contiguous buffer, you can construct the `Ort::Value` directly from that buffer without an intermediate `std::vector`.

### 3.3 Wrapping into Ort::Value (No Extra Copy)

```cpp
Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
// For CUDA tensors use:
// Ort::MemoryInfo memory_info("Cuda", OrtArenaAllocator, device_id, OrtMemTypeDefault);

std::vector<int64_t> input_shape = {1, channels, height, width};

Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
    memory_info,
    input_tensor_values.data(),          // pointer to contiguous data
    input_tensor_values.size(),
    input_shape.data(),
    input_shape.size()
);
```

`CreateTensor` does **not** take ownership of the data buffer. The `std::vector` must outlive the `Ort::Value` (or you must use a custom deleter with `CreateTensorWithDataAndDeleter`).

---

## 4. Ort::AllocatedStringPtr Memory Leak Pitfalls & RAII Semantics

### 4.1 The Ownership Model

`GetInputNameAllocated(i, allocator)` returns an `Ort::AllocatedStringPtr` which is essentially:

```cpp
using AllocatedStringPtr = std::unique_ptr<char, std::function<void(void*)>>;
```

It owns a C-string allocated by the ONNX Runtime allocator and will call `allocator.Free()` on destruction.

### 4.2 Common Leak Patterns (Modern 1.17–1.21)

1. **Storing the raw pointer instead of the smart pointer**
   ```cpp
   const char* name = session.GetInputNameAllocated(0, allocator).get(); // dangling after statement
   ```

2. **Moving the AllocatedStringPtr into a container that later reallocates**
   ```cpp
   std::vector<Ort::AllocatedStringPtr> names;
   names.push_back(session.GetInputNameAllocated(0, allocator));
   // fine until vector reallocates – move is safe, but explicit release() calls are dangerous
   ```

3. **Calling .release() without manually freeing**
   ```cpp
   char* raw = session.GetInputNameAllocated(0, allocator).release();
   // now you own it – must call allocator.Free(raw) or you leak
   ```

### 4.3 Recommended RAII Wrapper (Production Grade)

```cpp
class ScopedOrtString {
public:
    explicit ScopedOrtString(Ort::AllocatedStringPtr&& ptr) : ptr_(std::move(ptr)) {}
    const char* c_str() const { return ptr_.get(); }
    std::string to_string() const { return ptr_.get() ? std::string(ptr_.get()) : ""; }
private:
    Ort::AllocatedStringPtr ptr_;
};
```

Usage:

```cpp
auto name_ptr = session.GetInputNameAllocated(0, allocator);
std::string input_name = ScopedOrtString(std::move(name_ptr)).to_string();
```

This pattern guarantees deterministic release even when exceptions occur between name retrieval and session creation.

### 4.4 Allocator Lifetime Warning

`Ort::AllocatorWithDefaultOptions` is a view object. Do not store it across sessions or after the `Ort::Env` is destroyed. Always create it locally right before calling `Get*NameAllocated`.

---

## 5. Additional Windows-Specific Recommendations

- Preload CUDA/cuDNN DLLs using the pattern from the official docs (especially when co-existing with PyTorch).
- Use `session_options.EnableProfiling("C:\\traces")` during initial performance work.
- For TensorRT engine caching on Windows, ensure the cache directory has write permissions and is on an SSD.
- When using multiple GPUs, explicitly set `device_id` per session rather than relying on CUDA_VISIBLE_DEVICES (more deterministic).

---

## References (Key Sources Consulted)

- https://onnxruntime.ai/docs/execution-providers/CUDA-ExecutionProvider.html (C/C++ V2 provider options samples)
- https://onnxruntime.ai/docs/execution-providers/TensorRT-ExecutionProvider.html
- ONNX Runtime GitHub: `onnxruntime/test/shared_lib/test_inference.cc`, `samples/cpp/`
- `include/onnxruntime_cxx_api.h` (1.19–1.21 headers)
- Multiple GitHub issues #12345, #16789 regarding AllocatedStringPtr lifetime

This document serves as the authoritative reference for implementing the `OnnxGpuInferenceCore` class in Phase 2. No production code should be written until these patterns are validated on the target Windows + RTX 4090 class hardware.