# ROC AI Vision

> Real-time humanoid entity tracking + biometric capture instrument.
> Multi-threaded C++ orchestrator (ONNX Runtime + DirectML) feeding a
> SvelteKit cockpit dashboard over WebSocket binary + MJPEG-over-HTTP.

```
[ camera / desktop / window / region ]
              │
              ▼
   camera_capture_thread  ──┐
                            ├──► inference_thread (FOV crop ─► ORT::Run ─► postprocess)
   optical_flow_thread     ─┘                            │
                                                         ▼
                                             tracker_thread (EKF + DeepSORT + selector)
                                                         │
                                       ┌─────────────────┼─────────────────┐
                                       ▼                 ▼                 ▼
                          dashboard_egress_thread   actuator_emit    wire JSON / MJPEG
                                       │
                                       ▼
                            SvelteKit dashboard (Operate / Inspect / Calibrate / Engage / Biometrics)
```

---

## What's here

| Subsystem | Tech | Location |
|---|---|---|
| Vision pipeline (ONNX inference, manifest loader, EP probe, postprocessors) | C++17, ONNX Runtime 1.19, DirectML, OpenCV 4.13 | `core/vision_pipeline/` |
| Kinematics engine (CA-EKF, DeepSORT, hierarchical Gauss-Seidel) | C++20, Eigen 3.4 | `core/kinematics_engine/` |
| Transport (SPSC ring queues, binary WS server, MJPEG HTTP, desktop capture, REST API) | C++17, IXWebSocket, Win32 GDI | `core/transport/` |
| Orchestrator (six-thread main, signal-handling, runtime config) | C++17 | `core/main.cpp` |
| Dashboard (5 destinations, 4 drawers, runes-based reactive stores) | Svelte 5, TypeScript, Vite | `dashboard/` |
| Living blueprint (architecture, roadmap, UI spec, project tree) | Markdown | `workspace_blueprint/` |
| Manifest auto-generator (bulk ONNX → YAML stub) | Python 3.10+, `onnx`, `pyyaml` | `scripts/scan_onnx_to_manifest.py` |

---

## Quickstart on a fresh machine

### 1. Clone

```powershell
git clone https://github.com/HeavenFYouMissed/roc-ai-vision.git
cd roc-ai-vision
```

### 2. Toolchain prerequisites (Windows)

* [MSYS2](https://www.msys2.org/) with the `mingw-w64-x86_64-{cmake,ninja,gcc,opencv,yaml-cpp}` packages
* Python 3.10+ (used by `scan_onnx_to_manifest.py`)
* Node 20+ + npm (used by the dashboard)
* A DX12-capable GPU + recent drivers (any NVIDIA ≥ GTX 900, any AMD ≥ RX 400, Intel Arc / iGPU ≥ Skylake)

### 3. Download the third-party redistributables

The repo does **not** ship the DirectML SDK (~200 MB) or the DirectML-flavoured
ONNX Runtime NuGet (~15 MB) — those re-download deterministically. See
`docs/build_directml.md` for the full recipe; the short version is:

```powershell
# from the repo root, PowerShell
$tp = "third_party"
New-Item -ItemType Directory -Force -Path $tp\directml_nupkg,$tp\ort_dml_nupkg | Out-Null

Invoke-WebRequest -UseBasicParsing `
    -Uri "https://www.nuget.org/api/v2/package/Microsoft.AI.DirectML/1.15.2" `
    -OutFile "$tp\directml_nupkg\directml.zip"
Expand-Archive -Force "$tp\directml_nupkg\directml.zip" -DestinationPath "$tp\directml"

Invoke-WebRequest -UseBasicParsing `
    -Uri "https://www.nuget.org/api/v2/package/Microsoft.ML.OnnxRuntime.DirectML/1.19.2" `
    -OutFile "$tp\ort_dml_nupkg\ort-dml.zip"
Expand-Archive -Force "$tp\ort_dml_nupkg\ort-dml.zip" -DestinationPath "$tp\onnxruntime_dml"

# repackage into the layout the build expects
$src = "$tp\onnxruntime_dml"
$dst = "$tp\ort_dml_root"
New-Item -ItemType Directory -Force -Path "$dst\include","$dst\lib" | Out-Null
Copy-Item -Force "$src\build\native\include\*.h" "$dst\include\"
Copy-Item -Force "$src\runtimes\win-x64\native\onnxruntime.dll" "$dst\lib\"
Copy-Item -Force "$src\runtimes\win-x64\native\onnxruntime.lib" "$dst\lib\"
Copy-Item -Force "$tp\directml\bin\x64-win\DirectML.dll"        "$dst\lib\"
```

### 4. Build the C++ side

```powershell
$env:PATH = "C:\msys64\mingw64\bin;C:\msys64\usr\bin;$env:PATH"
cmake -S core -B core/build -G Ninja `
    -DCMAKE_BUILD_TYPE=Release `
    -DORT_ROOT="$(Resolve-Path third_party/ort_dml_root)" `
    -DDIRECTML_ROOT="$(Resolve-Path third_party/directml)" `
    -DROC_VISION_HAVE_DML_EP=ON
cmake --build core/build --config Release -j
```

This produces `core/build/roc_vision.exe` (~1.5 MB) plus a sibling pile of
runtime DLLs (DirectML, ORT, OpenCV, MinGW, FFmpeg).

### 5. Get some ONNX weights

Drop any `.onnx` files into `models/`, then auto-generate `.yaml` manifests:

```powershell
pip install onnx pyyaml
python scripts/scan_onnx_to_manifest.py models --verbose
```

The script ships pre-populated class tables for COCO-80 / ROC-humanoid-6 /
single-class and emits `# REVIEW:` comments for anything it can't classify
confidently (e.g. NMS-in-graph end-to-end exports).

### 6. Boot the dashboard

```powershell
cd dashboard
npm install
npm run dev
# → http://localhost:5173/
```

### 7. Boot the orchestrator

```powershell
# Webcam, default model
.\core\build\roc_vision.exe `
    --models-dir "$(Resolve-Path models)" `
    --manifest "models\yolo26n-coco.yaml" `
    --model    "models\yolo26n-coco.onnx"

# Capture the YouTube tab in Chrome
.\core\build\roc_vision.exe `
    --models-dir "$(Resolve-Path models)" `
    --manifest "models\Universal-xl.yaml" `
    --model    "models\Universal-xl.onnx" `
    --desktop-capture window --capture-window "YouTube"

# Primary monitor, downscaled to 1080p, 60 fps target
.\core\build\roc_vision.exe `
    --models-dir "$(Resolve-Path models)" `
    --manifest "models\Apex-Heavy.yaml" `
    --model    "models\Apex-Heavy.onnx" `
    --desktop-capture primary --capture-downscale 1920 --capture-fps 60
```

Startup banner should show:

```
[roc-orchestrator] InferenceEngine ready — manifest="..." head="..."
    ep=DirectML (DirectML adapter 0)
```

If you see `ep=Cpu` you're falling back — verify `DirectML.dll` is next to
`roc_vision.exe` and your GPU supports DX12 (`dxdiag` → System Information).

---

## Five dashboard destinations

| # | Destination | Purpose |
|---|---|---|
| 1 | **Operate** | Live video + overlays (FOV ring, detection boxes, EKF tracks, actuator paths). |
| 2 | **Inspect** | Pipeline graph, model inspector, topic graph, event log, hierarchy explorer. |
| 3 | **Calibrate** | Five-step camera intrinsics wizard. |
| 4 | **Engage** | Hardware actuator interlock + arm/disarm gate (4-light safety system). |
| 5 | **Biometrics** | Mouse Dynamics Capture: Pointer Lock recorder + 4-panel anti-cheat signature analysis (rolling sign-flip rate, etc.). Self-contained data plane — no dependency on the vision pipeline. |

Four left-rail drawers: **Sources** (camera/capture/file/desktop), **Models**
(live manifest browser + hot-swap), **FOV** (operator-driven inference
crop with follow-target), **Settings**.

---

## REST API surface (orchestrator-side)

| Method | Path | Purpose |
|---|---|---|
| GET | `/health` | Returns "ok" |
| GET | `/sources` | Enumerated camera devices |
| GET | `/mjpeg/{idx}` | Multipart Motion-JPEG stream for camera `idx` |
| GET | `/api/models` | Enumerate every `.yaml` manifest in `--models-dir` |
| GET | `/api/model/active` | Currently loaded manifest |
| POST | `/api/model/active` | `{"name": "..."}` — hot-swap models |
| GET | `/api/fov` | Current FOV crop settings + diagnostics |
| POST | `/api/fov` | Update FOV settings (cx/cy/radius/enabled/followTarget) |
| GET | `/api/capture-sources` | Live monitor + visible-window enumeration |

Default ports: WebSocket binary egress on `:8765`, HTTP API + MJPEG on `:8766`. Loopback-only by default; override with `--ws-bind 0.0.0.0` if you must.

---

## Documentation map

* [`workspace_blueprint/roadmap.md`](workspace_blueprint/roadmap.md) — phase-by-phase execution log
* [`workspace_blueprint/UI_SPEC_SHEET.md`](workspace_blueprint/UI_SPEC_SHEET.md) — design tokens, component tree, hotkey map, hard constraints
* [`workspace_blueprint/project_tree.md`](workspace_blueprint/project_tree.md) — living file-tree with amendment history
* [`docs/build_directml.md`](docs/build_directml.md) — DirectML EP enablement walkthrough
* [`PROJECT_BRIEF.md`](PROJECT_BRIEF.md) — high-level architectural narrative

---

## License

License TBD. All third-party ML weights remain under their respective
upstream licenses (Ultralytics AGPL-3.0 for YOLO-family models, etc.).

---

## Status

| Area | Phase |
|---|---|
| Vision pipeline (Phase 1) | Complete — 48/48 tests green with DirectML EP active |
| Kinematics engine (Phase 2 + 3) | Complete — 37/37 tests, ID-switch=0 on 200-frame occlusion scenario |
| Six-thread orchestrator (Phase 4-6) | Complete |
| Dashboard cockpit | Complete — 5 destinations + 4 drawers |
| Biometrics destination | Complete (Amendment v6) |
| Model hot-swap + FOV crop pipeline | Complete (Amendment v7) |
| Desktop / window / region capture | Complete (Amendment v8) |
| Multi-model ensemble | Design complete — implementation pinned to Phase 1.5 |
