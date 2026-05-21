# Phase 1 — Universal-EP ONNX Loader + Model Manifest Sidecar

**Date:** 2026-05-17
**Focus:** Cross-vendor ONNX Runtime execution-provider selection (DirectML → OpenVINO → CPU) and the model-manifest YAML sidecar that converts a heterogeneous collection of drop-in `.onnx` files into a uniform `Detection` / `Embedding` producer.
**Scope:** Binding spec for `core/vision_pipeline/` Phase 4 production code.
**Supersedes:** EP-selection sections of `phase_1_onnx_gpu_core.md` (kept for ORT C++ API reference but the priority order is reversed).
**Cross-refs:** `research_detection_models_2026.md` §8 (YOLO26m selection), `research_reid_embeddings_2026.md` §10 (OSNet-AIN x1.0 selection), `phase_1_inference_and_targeting.md` §§1–2 (letterbox math, foundational).

---

## 0. Why this document exists

Three earlier docs partially answer "how do we load and run an ONNX model in C++":

- `phase_1_inference_and_targeting.md` — letterbox math + introspection sketch
- `phase_1_onnx_gpu_core.md` — full CUDA / TensorRT EP initialisation sequences
- `roc-onnx-inference` SKILL — quick reference for any agent touching `vision_pipeline/`

But all three were written under the old assumption that the deployment host is "Windows x64 + RTX 4090 + CUDA Toolkit installed." The revised hardware profile (locked 2026-05-17) is **universal cross-vendor**: the same compiled binary must run on a budget AMD laptop, an Intel mini-PC, or a beefy NVIDIA workstation with no CUDA dependency. That changes the EP priority order from CUDA-first to **DirectML-first**, eliminates TensorRT entirely from the production path, and introduces a new layer the older docs don't address at all: a **model manifest** that lets the engine consume arbitrary third-party `.onnx` files without hand-coded postprocess branches.

This document is the binding spec for both.

---

## 1. EP Selection — Priority Order + Probe-and-Fallback

### 1.1 The locked priority list

| Priority | EP | Hardware Coverage | When chosen |
|---|---|---|---|
| 1 | **DirectML (DML)** | Any DX12 GPU on Windows: NVIDIA RTX, AMD Radeon, Intel Arc/Iris, Qualcomm Adreno | Default on Windows host with a usable DX12 device |
| 2 | **OpenVINO (OV)** | Intel CPU / iGPU / NPU (Core Ultra Lunar Lake / Meteor Lake) | Fallback when no DX12 device, OR when an Intel NPU is present and CPU-class quantised models benefit from it |
| 3 | **CPU EP** | Anything that runs `onnxruntime.dll` | Universal floor — always present, used when 1 and 2 fail |

**Explicitly rejected** as a primary path:

- **CUDA EP / TensorRT EP** — vendor lock-in (NVIDIA-only); CUDA Toolkit install dependency; TRT engine cache invalidated by driver updates; build machine must match runtime driver. Kept as an optional `ROC_VISION_BUILD_CUDA_EP` CMake flag for power users with explicit hardware, but **never the default path**.

### 1.2 Probe-and-fallback algorithm

The engine attempts each EP in priority order at session-construction time. The first EP that successfully creates a session with the model graph wins; any EP that throws `Ort::Exception` or returns a degraded session (e.g., all ops fell back to CPU EP under the hood) is logged and the next EP is tried.

```cpp
enum class EpKind { DirectML, OpenVINO, Cpu, Cuda /* opt-in only */ };

struct EpProbeResult {
    EpKind chosen;
    std::string device_description;   // e.g. "DML adapter 0: NVIDIA RTX 4090"
    int device_id;
    bool all_ops_native;              // false if ORT silently CPU-fell-back any ops
};

std::optional<EpProbeResult> probe_and_select_ep(
    Ort::Env& env,
    const std::filesystem::path& onnx_path,
    Ort::SessionOptions& opts_out,
    const std::vector<EpKind>& priority_list);
```

**Acceptance criterion for "EP works":**

1. `Ort::Session` constructor returns without exception.
2. A dummy 1-frame inference completes within a generous deadline (≤ 250 ms warm-up).
3. The session profiler (`SetExecutionMode(ORT_PARALLEL)` + `EnableProfiling`) confirms the chosen EP claimed the convolution layers — not the CPU EP silently catching everything.

### 1.3 DirectML EP initialisation

DML is the **default** on Windows. Single-line append:

```cpp
#include <onnxruntime_cxx_api.h>
#include <dml_provider_factory.h>

Ort::SessionOptions opts;
opts.SetExecutionMode(ORT_SEQUENTIAL);   // DML is single-stream; SEQUENTIAL avoids contention
opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
opts.DisableMemPattern();                // required when using DML + I/O binding
opts.SetIntraOpNumThreads(1);

// Pick a specific DX12 device by index. 0 = "default" (whichever the OS prefers).
const int dml_device_id = 0;
Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_DML(opts, dml_device_id));
```

**Device enumeration** (used to populate `device_description` for telemetry): query DXGI via `IDXGIFactory4::EnumAdapters1` before session construction, store adapter descriptions, pass index to `AppendExecutionProvider_DML`. Standard Windows boilerplate — wrap in a `dml_devices.hpp` helper.

**Notes:**
- DML supports **FP16** natively; **INT8 acceleration is NOT available** on the DML EP path. Quantised INT8 models will run but will be slower than FP16.
- DML is fundamentally a single execution stream per device — `SetIntraOpNumThreads(1)` is the right call.
- DML's `CreateTensor` requires explicit memory-info construction; **do NOT** use the default CPU allocator for tensors that the DML EP will consume — bind via `Ort::MemoryInfo("DML", OrtAllocatorType::OrtArenaAllocator, dml_device_id, OrtMemTypeDefault)`.

### 1.4 OpenVINO EP initialisation

OpenVINO is the Intel-CPU / iGPU / NPU path:

```cpp
#include <onnxruntime_cxx_api.h>
// OpenVINO provider header is exposed via the OV-enabled ORT build.

std::unordered_map<std::string, std::string> ov_options{
    {"device_type", "GPU"},          // or "CPU", "NPU", "GPU.0", "MULTI:GPU,CPU", etc.
    {"precision",   "FP16"},          // FP32 / FP16 / INT8 (INT8 requires a quantised graph)
    {"num_of_threads", "0"},          // 0 = OV picks; otherwise pin to a core count
    {"cache_dir",   "C:\\roc\\ov_cache"} // engine compilation cache — first run is slow without it
};
opts.AppendExecutionProvider_OpenVINO_V2(ov_options);
```

**Device selection logic:** on first probe, try `"AUTO"` (OV chooses best); record what was actually selected via the OV runtime API. Subsequent runs cache the choice in the model manifest's `runtime_hints` block (§4.5).

**Notes:**
- OpenVINO is the **only** EP in our default list that supports **INT8 acceleration**. If a manifest declares `precision: int8`, the engine MUST select OV (or fall through to CPU EP with a logged perf warning).
- OV's `cache_dir` must exist and be writable; compilation is 5–60 s on first session creation, milliseconds on subsequent runs.
- `device_type: "AUTO"` is convenient for development; production should pin explicit devices for determinism (e.g., `"GPU.0"`).

### 1.5 CPU EP

Always present. No special init beyond the base `Ort::SessionOptions`. Used as the universal floor:

```cpp
// No AppendExecutionProvider call needed — CPU EP is implicit.
// Just construct the session with the base opts.
Ort::Session session(env, onnx_path.wstring().c_str(), opts);
```

CPU EP performance on YOLO26m / OSNet-AIN x1.0:

| Model | Input | Est. CPU-EP latency (8-core consumer x86_64) |
|---|---|---|
| YOLO26n | 640×640 | ~38 ms (per research_detection_models_2026 §1.5) |
| YOLO26s | 640×640 | ~85 ms |
| YOLO26m | 640×640 | ~220 ms |
| OSNet-AIN x1.0 | 256×128 | ~6–10 ms per crop |

The CPU floor is real but **30 fps is barely achievable** with the nano variant; below that everything stutters. CPU EP is for development and degraded-fallback, not production.

### 1.6 Optional CUDA EP (build-flag-gated)

For power users on dedicated NVIDIA hardware who explicitly opt-in:

```cmake
option(ROC_VISION_BUILD_CUDA_EP "Enable CUDA Execution Provider in InferenceEngine" OFF)
```

When `ON`, the EP probe list gains a leading `EpKind::Cuda` entry. The full CUDA / TensorRT initialisation (Provider Options V2 form, `prefer_nhwc=1`, `cudnn_conv_algo_search=1`, optional TRT cache directory) lives in `phase_1_onnx_gpu_core.md` §§1.3–1.4 — that doc remains the reference for the CUDA-specific code paths. The point is that the engine **builds without CUDA Toolkit on the host** when the flag is `OFF`; the CUDA-EP code is conditionally compiled.

---

## 2. EP-related risk analysis

| Risk | Severity | Mitigation |
|---|---|---|
| DX12 driver missing on a Windows 7/8 / older Server box | High | Probe fails cleanly → falls to OpenVINO → falls to CPU EP. Engine reports the chosen path via telemetry. |
| INT8 manifest under DML (which doesn't accelerate INT8) | Medium | Manifest loader emits a warning and the EP probe demotes DML below OV for this specific model. |
| OpenVINO cache_dir not writable on first run | Medium | Engine creates the dir at boot, logs an error if creation fails, falls back to CPU EP. |
| Two manifests claiming the same `head` discriminator | Low | Postprocessor registry is keyed by string; duplicate registration throws at boot. |
| All EPs fail (corrupt model file, opset newer than installed ORT) | High | Engine throws a fatal at boot with the chosen `EpProbeResult`s for each tried EP for diagnostics. |
| Silent CPU fallback within DML/OV (some ops not supported) | Medium | Enable ORT profiling on first run, parse the trace, log any ops that landed on CPU. Acceptance criterion §1.2 #3. |

---

## 3. The Model Manifest Sidecar — Why and Format

### 3.1 Why a manifest is needed

ONNX introspection (`GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape()`) reveals **input shape** but says nothing about:

- Channel order (RGB vs BGR)
- Normalisation strategy (raw `[0, 255]`, `/255.0`, ImageNet `mean/std`)
- Output head schema (YOLOv5 / v8 / v10 / v11 / v12 / NAS / RT-DETR all differ)
- NMS in graph vs external
- Quantisation tags (INT8 scale/zero-point need explicit decoding)
- Class-id → application-domain label mapping
- Confidence threshold defaults

Without these, a single "drop in any YOLO `.onnx`" claim is **false** — the engine would silently produce wrong outputs (e.g., parsing YOLOv5 row-major tensors as YOLOv8 column-major, or sigmoid-ing already-sigmoided detections). The manifest sidecar makes the missing semantics explicit and machine-readable.

### 3.2 File convention

For every `models/<name>.onnx` there MUST exist `models/<name>.yaml` co-located in the same directory. The engine refuses to load any `.onnx` without a manifest (no implicit defaults — explicit configuration is the whole point).

### 3.3 YAML schema (v1)

```yaml
# models/yolo26m-roc-humanoid.yaml
schema_version: 1

# ── Identification ─────────────────────────────────────────────
name:        yolo26m-roc-humanoid
purpose:     detection            # one of: detection, reid, pose, segmentation
trained_on:  CrowdHuman+COCO-WB+PPE   # free-form provenance string
notes:       Custom-trained for ROC AI Vision 6-class humanoid hierarchy

# ── Input contract ─────────────────────────────────────────────
input:
  layout:           NCHW          # NCHW | NHWC
  channel_order:    RGB           # RGB | BGR
  expected_shape:   [1, 3, 640, 640]   # B, C, H, W — engine cross-checks against introspection
  dtype:            float32       # float32 | float16 | uint8 (quantised pre-scale)
  normalization:
    scale:          0.00392156862745   # 1 / 255
    mean:           [0.0, 0.0, 0.0]
    std:            [1.0, 1.0, 1.0]    # ImageNet would be [0.229, 0.224, 0.225]
  preprocessing:
    letterbox:      true
    pad_value:      [114, 114, 114]    # YOLO-family neutral grey
    pad_to:         model_input         # model_input | square (= max(H, W))

# ── Output contract ────────────────────────────────────────────
output:
  head:             yolo26_detect      # discriminator → Postprocessor registry key
  format:           cxcywh_logits      # cxcywh_logits | xyxy_sigmoid | xyxy_post_nms | reid_embedding
  expected_shape:   [1, 84, 8400]      # for verification; -1 allowed for dynamic axes
  num_classes:      6
  nms_in_graph:     false              # YOLO26 is NMS-free by design
  thresholds:
    score:          0.25
    # iou:          0.45               # only required when nms_in_graph: false AND head expects external NMS

# ── Runtime hints ──────────────────────────────────────────────
runtime_hints:
  preferred_ep:     DirectML            # DirectML | OpenVINO | Cpu | Cuda
  precision:        fp16                # fp32 | fp16 | int8
  cache_compiled:   true                # for OV's compile cache; ignored on DML/CPU

# ── Class table (for detection heads only) ─────────────────────
classes:
  - {id: 0, name: person,        roc_label: Person}
  - {id: 1, name: head,          roc_label: Head}
  - {id: 2, name: upper_torso,   roc_label: UpperTorso}
  - {id: 3, name: hat,           roc_label: AccessoryHat}
  - {id: 4, name: vest,          roc_label: AccessoryHat}    # vests also map to hat slot for now
  - {id: 5, name: backpack,      roc_label: AccessoryBackpack}
```

### 3.4 Manifest validation — what the loader checks

At session-construction time the `ManifestLoader` performs:

1. **Schema validation** — every required key present, types correct.
2. **Shape cross-check** — `input.expected_shape` matches `session.GetInputTypeInfo(0)`; mismatch = fatal.
3. **Class-id range** — every `classes[*].id` is in `[0, output.num_classes)`.
4. **`roc_label` validity** — every `roc_label` matches a `roc::kinematics::Label` enum value.
5. **EP feasibility** — `runtime_hints.preferred_ep` is in the probe list; `runtime_hints.precision == int8` warns if `preferred_ep != OpenVINO`.
6. **Postprocessor existence** — `output.head` is a registered key in the `Postprocessor` strategy map; unregistered = fatal at boot (catches typos before first inference).

### 3.5 Reference manifests shipped with the project

| File | Model | Purpose |
|---|---|---|
| `models/yolo26m-roc-humanoid.yaml` | YOLO26m (custom-trained) | 6-class detection (production target) |
| `models/yolo26n-coco.yaml` | YOLO26n COCO-pretrained | Bring-up / smoke test (no custom training needed) |
| `models/osnet-ain-x1-0.yaml` | OSNet-AIN x1.0 | Re-ID embedding extractor (128-D output) |

The first two share the same `head: yolo26_detect` schema; the third uses `head: osnet_reid` with `format: reid_embedding`. The Postprocessor registry has exactly two entries at Phase 4 launch.

---

## 4. The `yolo26_detect` postprocessor (NMS-free)

YOLO26's defining feature is **no NMS**. The output tensor is `[1, 4 + num_classes, num_anchors]` (column-major), where:

- Channels `[0:4]` are `(cx, cy, w, h)` in letterboxed-image pixel coordinates (model space, 640×640).
- Channels `[4:4+num_classes]` are **raw class logits** — apply sigmoid OR softmax depending on training (YOLO26 uses sigmoid per-class).

### 4.1 Decode algorithm (single-pass)

```
inputs:
  Y : tensor of shape [1, 4 + C, N], dtype float32   (C = num_classes, N = num_anchors)
  score_threshold : float (e.g., 0.25 from manifest)
  letterbox : LetterboxParams { scale s, pad_x, pad_y }
  letterbox_image_size : (model_w, model_h)
  capture_image_size   : (src_w, src_h)
  class_map : id → roc::kinematics::Label

outputs:
  detections : vector<Detection>

algorithm:
  detections.clear()
  for n in 0 .. N-1:
    # transpose-on-the-fly: read channel-major
    cx_m = Y[0, 0, n]
    cy_m = Y[0, 1, n]
    w_m  = Y[0, 2, n]
    h_m  = Y[0, 3, n]
    best_score = -inf
    best_class = -1
    for c in 0 .. C-1:
      logit = Y[0, 4 + c, n]
      score = sigmoid(logit)
      if score > best_score:
        best_score = score
        best_class = c
    if best_score < score_threshold:
      continue                                # gated out, no NMS needed (NMS-free guarantee)
    # inverse letterbox: model-space → capture-frame pixels
    cx_src = (cx_m - letterbox.pad_x) / letterbox.scale
    cy_src = (cy_m - letterbox.pad_y) / letterbox.scale
    w_src  = w_m / letterbox.scale
    h_src  = h_m / letterbox.scale
    # populate roc::kinematics::Detection
    det.bbox_center  = { cx_src, cy_src }
    det.bbox_w_px    = w_src
    det.bbox_h_px    = h_src
    det.confidence   = best_score
    det.label        = class_map[best_class]
    det.appearance   = Embedding::Zero()        # ReID extractor populates later
    detections.push_back(det)
  return detections
```

**Why this is NMS-free:** YOLO26's training-time "Consistent Dual Assignment" head (one-to-one assignment during inference, one-to-many during training) means at most one anchor predicts each ground-truth object. There are no redundant overlapping predictions to suppress. The `score_threshold` gate alone suffices.

**Per-frame cost:** `N = 8400` anchors × (5 reads + 6 sigmoids worst-case) → ~80 K float ops; under 200 µs on a single core. Negligible compared to the GPU inference itself.

### 4.2 Inverse-letterbox math (foundational, restated for completeness)

If the letterbox preprocessor stored `(s, pad_x, pad_y)` such that:
- `pad_x = (model_w - src_w · s) / 2`
- `pad_y = (model_h - src_h · s) / 2`

then a detection at letterboxed pixel `(cx_m, cy_m)` maps to original-frame pixel:

$$
cx_{\text{src}} = \frac{cx_m - pad_x}{s},\qquad cy_{\text{src}} = \frac{cy_m - pad_y}{s}
$$

Box width/height scale by `1 / s` (no padding term — width is invariant to translation).

### 4.3 Postprocessor interface contract

```cpp
struct PostprocessContext {
    LetterboxParams      letterbox;
    std::array<int, 2>   model_input_hw;   // (H, W) of model input tensor
    std::array<int, 2>   capture_input_hw; // (H, W) of source frame
    const ManifestSpec&  manifest;          // gives thresholds, class_map, etc.
};

class Postprocessor {
public:
    virtual ~Postprocessor() = default;
    virtual std::vector<roc::kinematics::Detection>
        decode(const std::vector<Ort::Value>& outputs,
               const PostprocessContext& ctx) const = 0;
};
```

The `yolo26_detect` implementation lives in `core/vision_pipeline/src/postprocess/yolo26_detect.cpp`. The registry is a `std::unordered_map<std::string, std::unique_ptr<Postprocessor>>` populated at `InferenceEngine` construction.

---

## 5. The `osnet_reid` postprocessor

OSNet-AIN x1.0 is the embedding extractor — runs once per detection crop and produces the 128-D L2-normalised vector that DeepSORT consumes.

### 5.1 Crop preprocessing (different from detection!)

Unlike YOLO26 which sees a letterboxed full frame, OSNet sees a **per-detection crop** stretched (not letterboxed) to a fixed `256×128` aspect ratio:

| Step | Detail |
|---|---|
| 1. Crop | Extract `bbox_orig` from the capture frame |
| 2. Resize | `cv::resize` to exactly **256×128** (H×W) — stretches, does NOT preserve aspect ratio (OSNet was trained this way) |
| 3. Channel order | BGR → RGB |
| 4. Normalisation | ImageNet: `mean = [0.485, 0.456, 0.406]`, `std = [0.229, 0.224, 0.225]`, applied per-channel after dividing by 255 |
| 5. Layout | HWC → CHW |
| 6. dtype | uint8 → float32 (or float16 if `runtime_hints.precision: fp16`) |

The manifest for OSNet declares all of this explicitly:

```yaml
input:
  layout:           NCHW
  channel_order:    RGB
  expected_shape:   [1, 3, 256, 128]      # H × W (note: NOT 640×640)
  dtype:            float32
  normalization:
    scale:          0.00392156862745
    mean:           [0.485, 0.456, 0.406]
    std:            [0.229, 0.224, 0.225]
  preprocessing:
    letterbox:      false                  # stretched, NOT letterboxed
    pad_value:      []
    pad_to:         none
```

The `LetterboxPreprocessor` must therefore be configurable per-manifest: detection models letterbox, ReID models stretch.

### 5.2 Output decode

OSNet outputs a `[1, 512]` tensor (or `[1, 128]` if the gallery-projection head is in the graph). The postprocessor:

1. If output dim ≠ 128, project via a stored `[128 × output_dim]` matrix (loaded from a co-located `.bin` file referenced in the manifest).
2. L2-normalise: `v = v / ||v||_2` (must yield unit length; epsilon-guard against zero vectors).
3. Cast to `Eigen::Matrix<float, 128, 1>` (= the locked `roc::kinematics::Embedding` typedef).

### 5.3 Per-frame batching

A frame with `K` detections requires `K` ReID forward passes. ONNX Runtime supports batched inference if the model's input has a dynamic batch axis. The manifest's `runtime_hints` can declare `batchable: true` — the engine then builds a `[K, 3, 256, 128]` input tensor in one pass. OSNet-AIN x1.0 supports this out of the box (Kaiyang Zhou's export script enables dynamic batch).

At the design ceiling of `K ≤ 32` detections per frame, a single batched forward pass on a mid-range GPU completes in ~3–5 ms (per research_reid_embeddings_2026 §2.4). 32 sequential single-image passes would take ~30 ms — 10× slower.

---

## 6. Engine-level data flow

```mermaid
flowchart LR
  Cam[Camera Frame<br/>cv::Mat BGR uint8] --> LB[LetterboxPreprocessor<br/>per-manifest config]
  LB -->|model-input tensor| Det[InferenceEngine.run<br/>detection session]
  Det -->|raw output tensors| YD[yolo26_detect Postprocessor]
  YD -->|Detection[]| CropPool[Per-detection crops]
  CropPool -->|batched tensor| Reid[InferenceEngine.run<br/>reid session]
  Reid -->|raw output tensor| OD[osnet_reid Postprocessor]
  OD -->|Detection[] with Embedding populated| Out[SPSC: DetectionBatch]
```

A single `InferenceEngine` instance can host multiple sessions (one per loaded manifest). The application code instantiates two: one for detection (`yolo26m-roc-humanoid`), one for ReID (`osnet-ain-x1-0`). They share the `Ort::Env` singleton.

---

## 7. Benchmark table (estimates from research docs, calibrated to design ceiling)

| Component | Model | EP | Precision | Latency target | Source |
|---|---|---|---|---|---|
| Detection | YOLO26m | DML | FP16 | ≤ 6 ms @ 640px on consumer dGPU | research_detection_models_2026 §1.5 + DML 20-40% overhead estimate |
| Detection | YOLO26m | OV (Intel iGPU) | FP16 | ≤ 12 ms @ 640px | OpenVINO Intel Arc + iGPU benchmarks |
| Detection | YOLO26n | DML | FP16 | ≤ 2 ms @ 640px | Same source, nano variant |
| ReID (32 crops batched) | OSNet-AIN x1.0 | DML | FP16 | ≤ 5 ms total | research_reid_embeddings_2026 §2.4 |
| Letterbox preprocess | OpenCV | CPU | uint8 → fp32 | ≤ 0.5 ms per 1080p frame | OpenCV resize + warpAffine bench |
| Yolo26_detect postprocess | hand-rolled C++ | CPU | float32 | ≤ 0.2 ms (8400 anchors × 6 classes) | First-principles op count |
| **Pipeline total (det + reid + pre + post)** | — | DML | FP16 | **≤ 12 ms @ 640px** | Sum of above |

At 30 fps the per-frame budget is 33 ms — the inference pipeline sits comfortably at ~36% of budget, leaving headroom for the EKF + actuator path.

---

## 8. Numbered actionable recommendations

1. **Implement `EpProbeResult probe_and_select_ep(...)`** per §1.2. EP priority list passed in by the caller so tests can stub it. First production caller uses `[DirectML, OpenVINO, Cpu]`. The optional `Cuda` entry is build-flag gated.
2. **Implement `ManifestSpec ManifestLoader::load(const std::filesystem::path&)`** per §3.3 schema. Validate at boot, throw on any deficiency. Use `yaml-cpp` (header-light, MIT) — add via FetchContent.
3. **Implement `LetterboxPreprocessor`** per `phase_1_inference_and_targeting.md` §2 + the `preprocessing.letterbox` toggle from the manifest. Cache `(s, pad_x, pad_y)` per inference for the inverse transform.
4. **Implement `class Postprocessor`** interface (§4.3) and the two concrete classes `Yolo26Detect` (§4.1 algorithm) and `OsnetReid` (§5.2).
5. **Implement `class InferenceEngine`** that owns the EP probe result, the `Ort::Session`, the loaded `ManifestSpec`, the `LetterboxPreprocessor`, and a reference to the registered `Postprocessor`. Public `run(const cv::Mat& frame) -> std::vector<Detection>` for detection sessions; `run_batched(span<const cv::Mat> crops) -> std::vector<Embedding>` for ReID sessions.
6. **Catch2 tests** with a fixture YOLO26n COCO model (downloaded by a Python script under `models/scripts/fetch_yolo26n.py`, gated behind `option(ROC_VISION_FETCH_TEST_MODELS "" ON)` so CI without network can skip). End-to-end: synthetic 640×640 image → `Detection[]` → assert at least one above the score threshold.
7. **Author a `yolo26m-roc-humanoid.yaml` placeholder manifest** in `models/` even before the trained model exists, so the manifest loader unit tests can validate against a real schema example.
8. **Skill update:** rewrite the `roc-onnx-inference` SKILL.md priority section to put DirectML / OpenVINO first; mark CUDA / TRT as opt-in only. (Tracked as a separate todo from this research doc.)

---

## 9. References

- Sapkota et al., "YOLO26: An Analysis of NMS-Free End to End Framework for Real-Time Object Detection," arXiv:2601.12882, Jan 2026.
- Zhou et al., "Learning Generalisable Omni-Scale Representations for Person Re-Identification (OSNet-AIN)," TPAMI 2021.
- Microsoft DirectML ONNX Runtime EP docs: https://onnxruntime.ai/docs/execution-providers/DirectML-ExecutionProvider.html (accessed 2026-05-17).
- Intel OpenVINO ONNX Runtime EP docs: https://onnxruntime.ai/docs/execution-providers/OpenVINO-ExecutionProvider.html (accessed 2026-05-17).
- `research_detection_models_2026.md` §1.5, §8 — YOLO26m architecture + selection rationale.
- `research_reid_embeddings_2026.md` §2.1–§2.4, §10 — OSNet-AIN x1.0 architecture + selection rationale.
- `phase_1_inference_and_targeting.md` §§1–2 — foundational letterbox math + introspection sketch (kept; this doc layers on top, doesn't replace).
- `phase_1_onnx_gpu_core.md` §§1.3–1.4 — CUDA / TensorRT EP code blocks (kept as opt-in reference for `ROC_VISION_BUILD_CUDA_EP=ON` builds).
