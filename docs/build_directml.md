# Enabling DirectML GPU inference on Windows

The C++ vision pipeline ships with the ORT CPU EP enabled by default. This
gets you working bits with **zero install friction** but it caps a YOLO26m
@ 640×640 model at roughly 80–120 ms / frame on a typical desktop CPU.

With the DirectML EP enabled, the same model runs at roughly **8–15 ms /
frame** on any DX12-capable GPU (every NVIDIA card from the GTX 900 series,
every AMD card from the RX 400 series, all Intel Arc / iGPUs ≥ Skylake). On
the user's mobile RTX 4050 that is approximately a **10× speedup**.

This page documents the one-time install required to unlock that path.

---

## Prerequisites

* Windows 10 ≥ 1909 (DirectML 1.5+) — *required*
* A DirectX 12 capable GPU + driver — *required*
  * Verify with `dxdiag` → System Information → DirectX Version: 12
* The vision_pipeline build environment (MSYS2 + mingw64 g++ + CMake +
  Ninja) — same as a stock build.

---

## Install the DirectML SDK

### Option A — Pre-staged runtime DLLs already on this machine

This repo ships with the operator's existing `DirectML.dll` +
`onnxruntime.dll` already staged under `third_party/runtime_dlls/`. These
were copied from `C:\Users\heave\Desktop\Classic\` (the `GODSPLAN`
DirectML build) and are sufficient for **running** the orchestrator with
the EP enabled at runtime, *provided* the build was compiled with
DirectML headers + import lib available.

To get a full build going on this machine, you ALSO need the DirectML
header + import lib — those don't ship with the redistributable DLL.

### Option B — NuGet package (gets you the header + .lib)

The official Microsoft package is `Microsoft.AI.DirectML`. NuGet itself is
not the easiest tool from a non-Visual-Studio shell, so the easiest path is
to download the `.nupkg` directly from the [NuGet web UI](https://www.nuget.org/packages/Microsoft.AI.DirectML/)
and rename it to `.zip`:

```
# from a PowerShell session in the repo root
Invoke-WebRequest `
    -Uri https://www.nuget.org/api/v2/package/Microsoft.AI.DirectML/1.15.2 `
    -OutFile third_party\directml.nupkg
Expand-Archive third_party\directml.nupkg -DestinationPath third_party\directml
```

The expanded layout has:

```
third_party/directml/
├── include/DirectML.h
├── bin/x64-win/DirectML.dll
└── bin/x64-win/DirectML.lib
```

Set `DIRECTML_ROOT=D:/roc-ai-vision/third_party/directml` and CMake will
auto-enable the EP.

### Option C — Existing standalone DirectML SDK install

If you already have a directory containing `DirectML.h` + `DirectML.dll` +
`DirectML.lib`, point `DIRECTML_ROOT` at it.

---

## Configure the build with DirectML enabled

From an MSYS2 mingw64 shell, in `core/vision_pipeline/`:

```bash
cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DDIRECTML_ROOT=D:/roc-ai-vision/third_party/directml \
    -DROC_VISION_HAVE_DML_EP=ON
cmake --build build --config Release -j
```

If `DIRECTML_ROOT` resolves to a directory containing `include/DirectML.h`,
the CMake will:

1. Add the DirectML headers to the include path
2. Link against `DirectML.lib`
3. Copy `DirectML.dll` next to `onnxruntime.dll` in `build/` so the test
   binary and the orchestrator can find it at run time
4. Define the preprocessor macro `ROC_VISION_HAVE_DML_EP=1` so the
   `EpProbe::probe()` function actually attempts the DirectML EP first
   (CPU is always the last-resort fallback)

You should now see this line at orchestrator startup:

```
[roc-orchestrator] InferenceEngine ready — manifest="..." head="yolo26_detect" ep=DirectML (NVIDIA GeForce RTX 4050 Laptop GPU)
```

instead of the CPU fallback message.

---

## Verify the EP is actually live

Run the smoke test and check the `[INFO]` lines:

```bash
.\build\roc_vision_tests.exe "[ep_probe]"
```

In a CPU-only build you'll see:

```
[ep_probe] DirectML EP unavailable at compile time (ROC_VISION_HAVE_DML_EP not defined)
```

With DirectML enabled you'll see:

```
[ep_probe] DirectML EP available; device_id=0 description=NVIDIA GeForce RTX 4050 Laptop GPU
```

---

## Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| `cannot find -lDirectML` at link time | `DirectML.lib` not found in the search path | Set `DIRECTML_ROOT` correctly or pass `-DCMAKE_LIBRARY_PATH=...` |
| Orchestrator exits with `DML EP failed: D3D12CreateDevice returned E_FAIL` | No DX12 GPU available (or driver too old) | Update GPU drivers; check `dxdiag` |
| Orchestrator falls back to CPU silently | DirectML found but the model has an op the EP can't accelerate | Check model with `python -m onnxruntime.tools.symbolic_shape_infer` and verify all ops are in [the DML op coverage table](https://github.com/microsoft/onnxruntime/blob/main/docs/execution-providers/DirectML-ExecutionProvider.md#supported-ops) |
| First inference is slow (~500 ms) but subsequent ones are fast | Normal — DirectML JIT-compiles graph on first run; ORT will cache when `runtime_hints.cache_compiled: true` in the manifest | Re-run; the second frame onwards should be at the ~10× speedup |

---

## What about OpenVINO and CUDA?

`EpProbe`'s priority order is `DirectML → OpenVINO → CPU` by default. CUDA
EP is rejected as the primary path per `roadmap.md` Phase 1 lock note —
vendor lock-in and Windows toolchain fragility outweigh its single-vendor
speedup. If you really want CUDA, pass `-DROC_VISION_BUILD_CUDA_EP=ON`
with the CUDA-flavoured ORT prebuilt unpacked at `ORT_ROOT`. OpenVINO is
similar (`-DROC_VISION_HAVE_OPENVINO_EP=ON`) but is only useful on Intel
hardware (CPU, iGPU, NPU); for Intel Arc discrete GPUs, DirectML still
wins because it gets dedicated VRAM treatment.
