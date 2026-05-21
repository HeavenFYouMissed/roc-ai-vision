# ROC AI Vision — Copilot Working Context

> Auto-loaded by VS Code Copilot. Mirrors the Cursor rules in `.cursor/rules/` so chat sessions in either editor pick up the same architectural context.

## What this project is

A real-time computer-vision + behavioural-biometrics research instrument:

- **C++17/20 orchestrator** (`core/`) runs a high-throughput pipeline:
  camera → ONNX Runtime inference → CA-EKF + DeepSORT tracker →
  SPSC queues → IXWebSocket egress.
- **SvelteKit 5 dashboard** (`dashboard/`) is a zero-GC operator console
  rendering three independent telemetry streams over a localhost
  WebSocket and an MJPEG-over-HTTP camera feed.
- **Models** (`models/`) — ONNX files + YAML sidecar manifests (one
  `.yaml` per `.onnx`) describing input layout, output head schema,
  NMS-in-graph flag, class-id → `roc::kinematics::Label` mapping, and
  runtime EP hints.
- **Workspace blueprint** (`workspace_blueprint/`) — living docs:
  `project_tree.md`, `roadmap.md`, `architecture.md`, phase handoffs,
  and the deep research notes in `research_notes/`.

Author: `@HeavenFYouMissed`. End deliverable: a multi-modal dataset
generation tool producing ML-ready JSONL exports for training behavioural
classifiers, and a visual-odometry / pointing front-end for robotics.

## The three telemetry channels (the architectural differentiator)

| Channel              | Source thread                  | Rate    | Wire ver | Payload                              |
|----------------------|--------------------------------|---------|----------|--------------------------------------|
| Detection tracks     | tracker (EKF + DeepSORT)       | 30 Hz   | v=1      | 12 B header + 86 B/track             |
| Actuator commands    | actuator-telemetry-emit thread | 500 Hz  | v=2      | 36 B/frame (raw + smoothed)          |
| Background odometry  | optical-flow-sampler thread    | 30 Hz   | v=3      | 32 B/frame (sparse LK + 3-DOF RANSAC) |

All three are multiplexed onto a single WebSocket at `ws://127.0.0.1:8765`
with version-byte dispatch. They are joined at analysis time via shared
`frame_id` and `ts_ns` keys.

## Tech stack

**C++** — C++17 strict for `vision_pipeline` + `transport`; C++20 for
`kinematics_engine` (`std::span`). Build via CMake + Ninja on MSYS2
MinGW64. Dependencies:

- `FetchContent`: ONNX Runtime 1.19.2 (CPU + optional DML/OpenVINO),
  Eigen 3.4, Catch2 v3.5.4, rigtorp/SPSCQueue v1.1, IXWebSocket v11.4.5.
- `find_package`: OpenCV 4.13, yaml-cpp 0.8.0 (from MSYS2).
- Hot paths: `noexcept`, zero allocation after warmup, lock-free
  `rigtorp::SPSCQueue` between threads, cache-line alignment on shared payloads.

**Dashboard** — SvelteKit 2 + Svelte 5 runes (`$state`, `$derived`,
`$effect`). SSR disabled (local SPA). Zero-GC data layer: SoA pools
backed by typed arrays (`Float32Array`, `Uint16Array`), one ring buffer
per pool, fixed allocation at module load. Binary wire decoder uses a
single reused `DataView`, version-byte dispatch into the matching pool —
zero per-frame allocations. RAF-driven render loop sized for 144 Hz
(~6.94 ms budget/frame).

## Production-locked artifacts — DO NOT MODIFY

Treat these as frozen. Extensions only via the documented patterns.

- `core/kinematics_engine/include+src/` — Phase 2 production-locked. 503 assertions / 37 cases green.
- `core/vision_pipeline/include+src/` — Phase 4 + Phase 6A. 208 / 46 green.
- `core/transport/include+src/` — Phase 4. 4378 / 27 green.
- `core/kinematics_engine/include/roc/kinematics/wire_format.hpp` — v=1 layout pinned forever (12 B header + 86 B/track).
- `core/main.cpp` thread topology (5–6 threads) — extend per phase-handoff patterns; do not rearrange.

Extension patterns:

- **New output sinks** → subclass `OutputSink` (`core/transport/include/roc/transport/output_sink.hpp`).
- **New SPSC queues** → add a typed alias in `core/transport/include/roc/transport/spsc_links.hpp`.
- **New ONNX head decoders** → register in `core/vision_pipeline/include/roc/vision/postprocessor_registry.hpp`; one `.cpp` per head under `postprocess/`.
- **New operator commands** → add enum member + payload struct in `operator_command.hpp` with a round-trip Catch2 test.

## Living documents — keep these current

Always update these in the same change that touches code:

- [`workspace_blueprint/project_tree.md`](workspace_blueprint/project_tree.md) — curated architectural tree with one-line role per file. Update on every file create/rename/move.
- [`workspace_blueprint/roadmap.md`](workspace_blueprint/roadmap.md) — append a log entry on every phase completion: timestamp + model name + task IDs + acceptance verifications.
- [`workspace_blueprint/PHASE_5C_HANDOFF.md`](workspace_blueprint/PHASE_5C_HANDOFF.md) — current execution plan with atomic tasks T01–T24. Mark tasks done as they ship.

`FILE_TREE.md` at the repo root is a separate **auto-generated disk snapshot** (regenerate via `python _make_tree.py`) — do not hand-edit it.

## Key entry points

| Concern                       | File                                                                                            |
|------------------------------|--------------------------------------------------------------------------------------------------|
| Orchestrator main             | [`core/main.cpp`](core/main.cpp)                                                                 |
| Tracker pipeline              | [`core/kinematics_engine/src/tracker.cpp`](core/kinematics_engine/src/tracker.cpp)               |
| CA-EKF                        | [`core/kinematics_engine/include/roc/kinematics/ca_ekf.hpp`](core/kinematics_engine/include/roc/kinematics/ca_ekf.hpp) |
| Wire format (v=1 tracks)      | [`core/kinematics_engine/include/roc/kinematics/wire_format.hpp`](core/kinematics_engine/include/roc/kinematics/wire_format.hpp) |
| Wire format (v=2/v=3)         | [`core/transport/include/roc/transport/wire_format.hpp`](core/transport/include/roc/transport/wire_format.hpp) |
| ONNX engine                   | [`core/vision_pipeline/include/roc/vision/inference_engine.hpp`](core/vision_pipeline/include/roc/vision/inference_engine.hpp) |
| Manifest schema (YAML sidecar) | [`core/vision_pipeline/include/roc/vision/manifest.hpp`](core/vision_pipeline/include/roc/vision/manifest.hpp) |
| EP probe (DML / OV / CUDA / CPU) | [`core/vision_pipeline/include/roc/vision/ep_probe.hpp`](core/vision_pipeline/include/roc/vision/ep_probe.hpp) |
| Letterbox                     | [`core/vision_pipeline/include/roc/vision/letterbox.hpp`](core/vision_pipeline/include/roc/vision/letterbox.hpp) |
| YOLOv8 decoder                | [`core/vision_pipeline/include/roc/vision/postprocess/yolov8_detect.hpp`](core/vision_pipeline/include/roc/vision/postprocess/yolov8_detect.hpp) |
| SPSC queue typedefs           | [`core/transport/include/roc/transport/spsc_links.hpp`](core/transport/include/roc/transport/spsc_links.hpp) |
| WebSocket egress              | [`core/transport/include/roc/transport/ws_egress.hpp`](core/transport/include/roc/transport/ws_egress.hpp) |
| MJPEG HTTP server             | [`core/transport/include/roc/transport/mjpeg_server.hpp`](core/transport/include/roc/transport/mjpeg_server.hpp) |
| OutputSink base               | [`core/transport/include/roc/transport/output_sink.hpp`](core/transport/include/roc/transport/output_sink.hpp) |
| OperatorCommand               | [`core/transport/include/roc/transport/operator_command.hpp`](core/transport/include/roc/transport/operator_command.hpp) |
| Dashboard shell               | [`dashboard/src/lib/components/NavigationShell.svelte`](dashboard/src/lib/components/NavigationShell.svelte) |
| Track pool                    | [`dashboard/src/lib/stores/track_pool.svelte.ts`](dashboard/src/lib/stores/track_pool.svelte.ts) |
| Wire decoder                  | [`dashboard/src/lib/telemetry/wire_decoder.ts`](dashboard/src/lib/telemetry/wire_decoder.ts)     |
| Telemetry socket              | [`dashboard/src/lib/telemetry/telemetry_socket.ts`](dashboard/src/lib/telemetry/telemetry_socket.ts) |
| Mock frame source             | [`dashboard/src/lib/telemetry/mock_frame_source.ts`](dashboard/src/lib/telemetry/mock_frame_source.ts) |
| Dashboard architecture plan   | [`dashboard/architecture_plan.md`](dashboard/architecture_plan.md)                               |
| UI spec sheet                 | [`workspace_blueprint/UI_SPEC_SHEET.md`](workspace_blueprint/UI_SPEC_SHEET.md)                   |

## Models

- 113 unique `.onnx` files are catalogued at
  `C:\Users\heave\Desktop\onnx_collection\` with `MANIFEST.csv` +
  `MANIFEST.md`. **89 of them** share the dominant contract:
  - Input  `images : float32 [1,3,640,640]`, normalized 0–1, RGB, NCHW.
  - Output `output0 : float32 [1,5,8400]` (YOLOv8 single-class head:
    `[cx, cy, w, h, conf]` per anchor, NMS external).
- A few are `1×84×8400` (80-class COCO YOLOv8) — handled by the same
  `yolov8_detect` postprocessor.
- Bring-up reference manifests in `models/`:
  `yolo26n-coco.yaml`, `coco-yolov8-baseline.yaml`,
  `person-only-yolov8.yaml`, `osnet-ain-x1-0.yaml`.
- The ONNX engine **interrogates the model's input shape at boot** and
  auto-configures the letterbox pipeline. Don't hardcode dimensions.

## Build & test commands

```powershell
# Configure + build everything (Release, MinGW64)
cmake -S core -B core/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build core/build

# Run the orchestrator
core/build/roc_vision.exe --camera 0 --model models/yolo26n-coco.onnx --actuator-emit-hz 500

# Test suites
core/build/kinematics_engine/roc_kinematics_tests.exe
core/build/vision_pipeline/roc_vision_tests.exe
core/build/transport/roc_transport_tests.exe

# Dashboard
cd dashboard
npm install
npm run dev      # http://127.0.0.1:5173
```

## Coding conventions (terse)

- **C++:** snake_case for functions/locals, PascalCase for types, ALL_CAPS for compile-time constants. Headers under `include/roc/<lib>/`. One translation unit per public class. Use `Eigen::Matrix<...>` with fixed sizes wherever the dimension is known. Tests live in `tests/test_*.cpp`, Catch2 v3.
- **TypeScript / Svelte:** `.svelte.ts` for runed modules, `.ts` for pure. snake_case files, PascalCase Svelte components. Hot-path pools (`track_pool`, `actuator_pool`, `bg_odometry_pool`) must stay zero-alloc — never `.push()` into them, write into the typed-array slots directly.
- **YAML manifests:** one sidecar per `.onnx`; schema in `manifest.hpp`.

## Test discipline

Every new C++ feature ships with a Catch2 case in the matching
`tests/test_*.cpp` and counts as "done" only when the corresponding test
binary still reports green totals (no regressions, +N assertions).

## What's out of scope

- Hardware mouse output (USB HID / COM3) — Phase 6+. Today only `NoOpSink` is wired.
- Ed25519 dataset attestation signing — Phase 6+.
- Multi-model ensemble (WBF + IoS) — Phase 1.5, pinned until single-model end-to-end demo is green.
- ROS 2 packages under `ros2_ws/` are deprecated; the in-process `transport/` layer owns IPC now. Don't add code there.

## When in doubt

Read [`PROJECT_BRIEF.md`](PROJECT_BRIEF.md) (the elevator pitch) and
[`workspace_blueprint/research_notes/dashboard_menu_architecture_2026.md`](workspace_blueprint/research_notes/dashboard_menu_architecture_2026.md)
(the ~1300-line binding spec — v2 + v3 §11 + v4 §12 + v5 §13) for the
canonical answer. Cite specs by section number (`§13.4`) instead of
re-quoting.
