# ROC AI Vision — Project Brief

> Self-contained description suitable for handing to an outside agent (Claude Code, ChatGPT, a fresh Cursor chat in a different workspace, a collaborator) so they understand what this project is, where it sits, and what the deliverable looks like — without having to read 1300 lines of binding spec.

---

## 1. The one-paragraph version

ROC AI Vision is a real-time computer-vision and behavioural-biometrics research instrument. It pairs a high-throughput C++ inference + tracking engine (ONNX Runtime → letterboxed YOLO-family detection → CA-EKF + DeepSORT tracking → SPSC queues → WebSocket egress) with a SvelteKit 5 zero-GC dashboard that visualises three independent telemetry channels: detection-conditioned tracks, raw vs smoothed pointer/actuator commands, and background optical-flow odometry. The end deliverable is a multi-modal dataset-generation tool that produces ML-ready JSONL exports (BEACON-compatible folder layout) suitable for training foundation-model classifiers on human-vs-automation behavioural signatures. The same engine, via a sink-agnostic adapter pattern, doubles as a visual-odometry front-end for robotics / sovereign-defence camera-pointing applications.

## 2. Identity and framing

- **Author:** `@HeavenFYouMissed` — security and machine-learning researcher; public projects include `Prometheus-guard` (anti-tampering, Rust) and DMA-firmware defensive-research toolchains.
- **Framing (non-negotiable):** strictly defensive analytical research. The architecture is designed the way an anti-cheat vendor (Vanguard, EAC, Faceit-AC) or an academic mouse-dynamics researcher would build a telemetry instrument to study automation signatures. There is no offensive tooling, no exploitation research, no evasion guidance anywhere in the codebase, the comments, the docs, or the exported data. A sanitised-vocabulary CI lint enforces neutral naming (`actuator_*`, `pointer_*`, `clean_trace`, `dirty_trace`, `bg_odometry_*`, `behavioural_features_*`).
- **Dual-use abstraction:** the same engine is sink-agnostic. The `OutputSink` adapter pattern routes the tracking output to (a) a `NoOpSink` for pure data collection, (b) HID-microcontroller mouse packets via COM3 for game-security behavioural-profiling research (Phase 6 part B), or (c) high-rate physical servo platforms like Dynamixel for robotics / sovereign-defence pointing applications. The dashboard's Spectator Telemetry Matrix doubles as a visual-SLAM odometry health panel via a Scenario label-map swap.

## 3. What the final product looks like

**A local operator console (no cloud).** User launches a single C++ orchestrator binary that opens a camera (laptop webcam, capture card, or industrial camera), runs ONNX inference, tracks detections through a Constant-Acceleration Extended Kalman Filter with DeepSORT-style appearance + motion association, and broadcasts binary telemetry over a localhost WebSocket. A separate SvelteKit dashboard at `http://127.0.0.1:5173` connects to that WebSocket and renders four destinations (Operate / Inspect / Calibrate / Engage), each with dockable panel Blueprints persisted to localStorage.

The dashboard's hero panels:

- **Spatial Projection Canvas** — composite live view: camera feed + EKF-predicted forward trajectories + detection boxes with uncertainty ellipses + parent→child hierarchy edges + a continuously-painted ring-buffer trace of the last N actuator commands (raw polyline showing sharp corners vs cubic-Bezier-smoothed showing fluid curves — the visual diagnostic for clean-vs-dirty input signatures).
- **MouseDynamicsInspector** — side-by-side painter showing the same actuator command stream rendered two ways: clean trace (left half, EKF + Ruckig smoothed, cubic Bezier) and dirty trace (right half, raw step-deltas, polyline). Below: live mini-plots of velocity profile / jerk FFT / Fitts'-Law residual / path entropy. Above: Record / Stop / Export controls with mandatory pre-record session-labelling modal.
- **Spectator Telemetry Matrix** — dual-line chart of actuator velocity vs background optical-flow velocity. Below: cross-correlation strip with human (ρ ≈ 0.85-0.98, lag ≈ +1 frame) and automation (ρ < 0.6, lag drift > 3 frames) bands shaded. The visual signature of any zero-inertia automation snap that doesn't match the rendering pipeline's mouse-smoothing curve becomes immediately apparent.
- **Recording → Export pipeline** — produces gzipped JSONL session files in a BEACON-compatible per-session folder layout: `<session_id>/mouse.jsonl.gz` (380 B/sample @ 500 Hz + 720 B/feature-event @ 5 Hz + session-header with model + Profile + Scenario + attestation), optional companion `screen.mp4` and `hardware_info.json`. Directly loadable via `datasets.load_dataset('json', data_files=..., lines=True)` for HuggingFace, `pd.read_json(..., lines=True)` for pandas, or any PyTorch DataLoader.

## 4. The three independent telemetry channels (the architectural differentiator)

What makes this project a research contribution rather than just another tracker is the **three-channel forensic model**:

| Channel | Source | Rate | Forensic meaning |
|---|---|---|---|
| Detection-conditioned tracks | C++ tracker thread (EKF + DeepSORT) | 30 Hz | "What the model says the target is doing." |
| Actuator command stream | C++ actuator-telemetry-emit thread | 500 Hz | "What the pointing command actually was, both raw and smoothed — captured independently of model rate so HID polling distributions are preserved." |
| Background optical-flow odometry | C++ optical-flow-sampler thread (sparse Lucas-Kanade + rigid 3-DOF affine RANSAC, center-mask-excluded) | 30 Hz | "What the camera scene actually did, independent of any model or actuator. Ground truth for camera motion." |

These three streams are independently captured, independently exported, and joined at analysis time via shared `frame_id` and `ts_ns` keys. Cross-correlating the actuator velocity stream against the background optical-flow velocity stream produces a physical-plausibility check that's difficult to forge — the dashboard renders this correlation live.

## 5. Technical stack

**C++ orchestrator** (`core/`):
- C++17 strict for `vision_pipeline` + `transport`; C++20 for `kinematics_engine` (std::span).
- Build: CMake + Ninja, MSYS2 MinGW64 toolchain on Windows. Dependencies fetched via CMake FetchContent (ONNX Runtime 1.19.2 CPU + DirectML/OpenVINO when probe succeeds, Eigen 3.4, Catch2 v3.5.4) and `find_package` (OpenCV 4.13, yaml-cpp from MSYS2).
- Three production-locked libraries: `kinematics_engine` (CA-EKF + DeepSORT + wire format), `vision_pipeline` (ORT EP probe-and-fallback + universal letterbox + manifest-driven postprocessor registry: `yolo26_detect` / `yolov8_detect` / `osnet_reid`), `transport` (SPSC links, IXWebSocket egress server, OperatorCommand reverse-channel decoder, OutputSink adapter base class).
- Orchestrator (`core/main.cpp`): five-to-six-thread architecture (camera-capture → inference → tracker → dashboard-egress + actuator-telemetry-emit + optical-flow-sampler in Phase 5C). All inter-thread links are `rigtorp::SPSCQueue` (lock-free, cache-aligned). All hot paths zero-allocation after warmup.
- Wire format: 12-byte header + 86-byte tracks for v=1 frames; 36-byte v=2 actuator-telemetry frames; 32-byte v=3 bg-odometry frames; all multiplexed on a single localhost WebSocket at `ws://127.0.0.1:8765` with version-byte dispatch.

**SvelteKit 5 dashboard** (`dashboard/`):
- Svelte 5 runes (`$state`, `$derived`, `$effect`) for fine-grained reactivity. SSR explicitly disabled — local client-rendered SPA.
- Zero-GC data layer: three Structure-of-Arrays pools (`track_pool.svelte.ts`, `actuator_pool.svelte.ts`, `bg_odometry_pool.svelte.ts`) backed by typed arrays (`Float32Array`, `Uint16Array`). One ring buffer per pool, fixed allocation at module load.
- Binary wire decoder (`wire_decoder.ts`): single reused DataView, version-byte dispatch into the matching pool. Zero per-frame allocations.
- Live WebSocket client (`telemetry_socket.svelte.ts`) with exponential-backoff reconnect; mock frame source (`mock_frame_source.ts`) synthesises all three stream versions for offline UI development.
- Composite Operate canvas: layered SVG over a `<video>` element, RAF-driven render loop sized for 144 Hz (~6.94 ms budget per frame).

**Models** (`models/`):
- Reference YAML manifests describing input layout, output head schema, NMS-in-graph flag, class-id → `roc::kinematics::Label` mapping, and runtime EP hints. Sidecar pattern: one `.yaml` per `.onnx` file.
- Currently authored: `coco-yolov8-baseline.yaml` (80-class COCO YOLOv8 from the user's collection) and `person-only-yolov8.yaml` (single-class YOLOv8 person detector).

**Research deliverables** (`workspace_blueprint/research_notes/`):
- Mathematical specs (CA-EKF derivation, DeepSORT association math, OSNet-AIN re-ID architecture, OpenCV letterbox inverse-transform).
- Dashboard menu architecture binding spec (v2 + v3 + v4 + v5 amendments, ~1300 lines).
- Anti-cheat data-exposure derived-feature catalogue (25 features F01-F25, 13 numbered recommendations, BEACON-compatible export schema).
- Per-phase ensemble architecture, IPC middleware, telemetry UI plans.

## 6. Project structure (top level)

```
roc-ai-vision/
├── core/                          # C++ orchestrator + three libraries
│   ├── kinematics_engine/         # CA-EKF + DeepSORT + wire_format.hpp (production-locked)
│   ├── vision_pipeline/           # ORT EP probe + letterbox + postprocessor registry
│   ├── transport/                 # SPSC links + WS egress + OutputSink
│   ├── cmake/                     # RocBundleMingwRuntime + RocBundleRuntimeDeps
│   └── main.cpp                   # Five-to-six-thread orchestrator
├── dashboard/                     # SvelteKit 5 zero-GC operator console
│   ├── src/lib/stores/            # SoA typed-array pools
│   ├── src/lib/telemetry/         # wire_decoder, telemetry_socket, mock_frame_source
│   ├── src/lib/components/        # Composite canvas + panel components
│   └── architecture_plan.md
├── models/                        # ONNX files + YAML manifest sidecars
├── workspace_blueprint/           # Living docs (project_tree, roadmap, research_notes)
│   ├── research_notes/
│   │   ├── dashboard_menu_architecture_2026.md     # BINDING SPEC (v2+v3+v4+v5)
│   │   ├── anticheat_data_exposure_2026.md         # 25 derived features F01-F25
│   │   ├── phase_2_ekf_kinematics.md               # EKF math contract
│   │   ├── phase_1_universal_ep_and_manifest.md    # ORT EP + manifest schema
│   │   └── ...                                     # ~15 other research docs
│   ├── PHASE_5C_HANDOFF.md        # Atomic task list T01-T24
│