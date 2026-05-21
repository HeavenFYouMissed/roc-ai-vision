# ROC AI Vision Dashboard — Architecture Plan (v2)

**Status.** This document is the actionable Phase 5/6 implementation plan derived from the binding synthesis at `workspace_blueprint/research_notes/dashboard_menu_architecture_2026.md` (v2 §0–§10 + v3 §11 + v4 §12 + v5 §13). When the two documents disagree, the synthesis wins — open a PR to reconcile.

**Last update.** 2026-05-17 — translated v3/v4/v5 amendments into the Phase 5/6 plan. Locks the three-pool data layer, the version-byte wire dispatch, and the panel scaffolding plan for the camera-glue (Phase 6A) → Phase 5c handoff.

---

## 1. Shell architecture (v2 §1)

Three-zone shell, cardinality-of-4 destinations, dockable panels via JSON-persisted Blueprints. Old destination names (`Spatial Projection Canvas`, `Observer Matrix`, `Kinematic Trajectory Controller`, `Hardware Actuation Interface`) remain wired into the code today; rename to the forward names (`Operate`, `Inspect`, `Calibrate`, `Engage`) lands in Phase 5c alongside the panel-registry refactor.

| Zone | Component | Height/width | Purpose |
|---|---|---|---|
| TopBar | `TopBar.svelte` (Phase 5c) | 24 px | Scenario picker + Profile picker + connection / NIS / boundary status + Engage safety LED |
| DestinationsRail | `DestinationsRail.svelte` (Phase 5c) | 56 px | Vertical icon rail with 4 destination buttons + Command-Palette opener (`Ctrl+K`) |
| WorkArea | `<svelte:component>` per active destination | fills the rest | Hosts the destination's active Blueprint (dockable panel layout) |

Today: `NavigationShell.svelte` is the placeholder for all three zones. Phase 5c splits it into the three-zone composition above without breaking the existing 4-domain selection flow.

## 2. Destinations and their featured panels

| Forward name | Current code name | Featured default panel | Other panels (registry) |
|---|---|---|---|
| **Operate** | Spatial Projection Canvas | `TelemetryCanvas.svelte` (the composite camera + EKF + actuator-trace surface; future rename `SpatialProjectionPanel.svelte`) | `LayerVisibilityToggle`, `TargetLockInspector`, `OverlayHotkeyHelp` |
| **Inspect** | Observer Matrix | `MouseDynamicsInspector.svelte` (v4 §12.1) | `SpectatorTelemetryMatrix.svelte` (v5 §13.5), `VelocityJerkSpectrumPanel`, `FittsResidualPanel`, `ObserverMatrixPanel.svelte` (existing — kept as a system-health panel) |
| **Calibrate** | Kinematic Trajectory Controller | `KinematicTrajectoryControllerPanel.svelte` (existing) — expanded with v3 §11.2 sliders (FOV / linear smoothing α / deadzone radius / velocity / acceleration curve exponents) | `ModelManifestEditor`, `TargetingAnchorPicker` |
| **Engage** | Hardware Actuation Interface | `HardwareActuationInterfacePanel.svelte` (existing) — expanded with v3 §11.4 OutputSink selector + REPLAY-mode safety lock | `SinkDiagnosticPanel`, `EngagementHistoryViewer` |

## 3. Data layer (locked)

### 3.1 Hot-path stores (typed-array SoA — no reactivity touches the columns)

| Store | Source frame version | Rate | Layout | Backing memory |
|---|---|---|---|---|
| `trackPool` | v1 (12 B header + 86 B/track) | 30 Hz | per-track, 64 slots × 16-sample trajectory ring | ~14 KB |
| `actuatorPool` | v2 (36 B/frame) | 500 Hz Phase 5c / 60 Hz Phase 5b | GLOBAL ring of 4096 samples | ~144 KB |
| `bgOdometryPool` | v3 (32 B/frame) | 30 Hz | GLOBAL ring of 1024 samples | ~32 KB |

All three pools live in `dashboard/src/lib/stores/`. None of them allocate in steady state.

Reactive surface per pool: a small set of `$state` runes that bump once per decoded frame (e.g., `presenceVersion`, `framesSinceReset`, `lastTsHi/Lo`). These let non-canvas consumers (Observer Matrix system-health panel, MouseDynamicsInspector control strip) re-render without polling the typed-array columns.

### 3.2 Human-speed stores (full Svelte reactivity)

`aiming_config`, `hardware_config`, `profile`, `scenario`, `blueprint`. All mutated via UI; persisted to `localStorage` JSON. Slider changes flow:

```
slider drag → config store mutate → OperatorCommand encoded → WS send to C++ orchestrator
                                                              ↓
                                       next inbound v2 frame reflects new value
```

Slider changes never directly mutate hot-path stores. The slider→tracer feedback loop in v4 §12.3 is round-trip via the C++ orchestrator; round-trip latency at 500 Hz is ~2 ms.

## 4. Wire-format version dispatch (locked)

Single inbound WebSocket at `ws://127.0.0.1:8765`. Every binary frame is dispatched on byte 0 by `wire_decoder.decode_frame()`:

| Byte 0 | Decoder | Pool |
|---|---|---|
| `0x01` | `decode_v1_track_frame()` | `trackPool` |
| `0x02` | `decode_v2_actuator_telemetry()` | `actuatorPool` |
| `0x03` | `decode_v3_bg_odometry()` | `bgOdometryPool` |
| anything else | dropped with one dev-mode warning per session | — |

All decoders are zero-allocation `DataView` byte walkers with explicit `littleEndian=true`.

## 5. OperatorCommand reverse channel

Single 22-byte LE frame per command. Encoder lives in `telemetry_socket.svelte.ts`. Pre-allocated once; reused per command. Command-kind enum is locked at the wire level — never reorder. New kinds added in Phase 5c (see Cursor rule `.cursor/rules/200-dashboard-telemetry.mdc` for the full table).

## 6. JSONL export (Phase 5c — v4 §12.2 + v5 §13.6)

Pure-additive new module `dashboard/src/lib/exporter/jsonl_writer.svelte.ts`. Three event types per session file:

- `session` — header, exactly one, file start
- `sample` — per actuator-telemetry frame (high-rate)
- `feature` — per derived-feature window (low-rate)

File extension `.jsonl`, optional `.jsonl.gz` via native `CompressionStream('gzip')`. Auto-rotates every 30 min to cap individual files at ~350 MB gzipped.

The exporter consumes the three hot-path pools by name (`actuatorPool`, `bgOdometryPool`, optionally `trackPool` for target-relative fields). Pre-record labelling modal is mandatory per v4 §12 Risk B1.

## 7. Phase 5b vs Phase 5c work split

### Phase 5b (this week — alongside Phase 6A camera-glue worker)

| Item | File | Status |
|---|---|---|
| Cursor rule lock with v3/v4/v5 constraints | `.cursor/rules/200-dashboard-telemetry.mdc` | ✅ landed this commit |
| Architecture plan v2 | `dashboard/architecture_plan.md` | ✅ landed this commit (this file) |
| `actuatorPool` SoA store | `dashboard/src/lib/stores/actuator_pool.svelte.ts` | ✅ landed this commit |
| `bgOdometryPool` SoA store | `dashboard/src/lib/stores/bg_odometry_pool.svelte.ts` | ✅ landed this commit |
| Wire-decoder version-byte dispatch + v2/v3 decoders + self-tests | `dashboard/src/lib/telemetry/wire_decoder.ts` | ✅ landed this commit |
| Mock frame source v2/v3 emitters | `dashboard/src/lib/telemetry/mock_frame_source.ts` | ✅ landed this commit |
| `MouseDynamicsInspector.svelte` placeholder + panel-registry entry | Phase 5c | ⏸ blocked on camera-glue worker |
| `SpectatorTelemetryMatrix.svelte` placeholder + panel-registry entry | Phase 5c | ⏸ blocked on camera-glue worker |
| JSONL exporter session-header writer | Phase 5c | ⏸ blocked on camera-glue worker |
| C++ `actuator-telemetry-emit` thread bootstrap (60 Hz placeholder) | Phase 6A worker | ⏸ in flight |

### Phase 5c (next 1-2 worker days post-Phase-6A landing)

| Item | File | Notes |
|---|---|---|
| TopBar + DestinationsRail extraction | `dashboard/src/lib/components/shell/` | Splits `NavigationShell.svelte` into three zones |
| Composite Operate canvas rename + Layer 1.5 implementation | `dashboard/src/lib/components/operate/SpatialProjectionPanel.svelte` | Reads `actuatorPool` for the actuator-trace layer |
| MouseDynamicsInspector full implementation | `dashboard/src/lib/components/inspect/MouseDynamicsInspector.svelte` | Side-by-side clean/dirty + derived-feature mini-plots + record/export strip |
| SpectatorTelemetryMatrix full implementation | `dashboard/src/lib/components/inspect/SpectatorTelemetryMatrix.svelte` | Dual-line chart + cross-correlation strip + KPI strip |
| JSONL exporter sample+feature event writers + supervised-labelling modal | `dashboard/src/lib/exporter/jsonl_writer.svelte.ts` | Per v4 §12.2 + v5 §13.6 |
| Blueprint persistence to localStorage | `dashboard/src/lib/blueprints/` | JSON layout schemas + 5 starter Blueprints + AC Research Scenarios |
| Profile schema with v3 §11.2 sliders + v3 §11.3 anchors + v3 §11.4 sinks | `dashboard/src/lib/profiles/schema.ts` | Mutates `aiming_config`/`hardware_config` under the hood |
| Command palette `Ctrl+K` | `dashboard/src/lib/components/shell/CommandPalette.svelte` | Fuzzy-search actions + panels + tracks |
| Overlay sheet `Alt+Z` | `dashboard/src/lib/components/shell/OverlaySheet.svelte` | Compact mode |

### Phase 6+ (post-demo)

| Item | Notes |
|---|---|
| C++ `OpticalFlowSampler` + tests | v5 §13.3 — sparse Lucas-Kanade + rigid 3-DOF estimation + center-mask |
| C++ camera-SPSC fanout (`camera_to_optflow`) | v5 §13.3 |
| C++ 500 Hz actuator-telemetry-emit thread (replace 60 Hz placeholder) | v4 §12.4 |
| C++ extended `OperatorCommand` kinds 4–7 (alpha / deadband / FOV / anchor) | v4 §12.3 + v3 §11.3 |
| Defensive-research attestation signing (Ed25519) | v4 Risk B5 |

## 8. Component boundary rules (locked)

- **`TelemetryCanvas.svelte`** stays hot-path-only. Reads `trackPool` today; Phase 5c will add reads from `actuatorPool` (Layer 1.5) and `bgOdometryPool` (Layer 1.6 — visual-odometry overlay for the robotics dual-use Scenarios per v5 §13.7).
- **Domain config panels** never read hot-path stores and never run inside RAF.
- **Wire decoders** never import Destination panels. The dependency arrow points from `components/*` → `stores/*` + `telemetry/*`, never the reverse.

## 9. SSR posture

`+layout.ts` exports `ssr = false`. The dashboard is a local-only operator console; SSR would attempt to evaluate browser APIs (`WebSocket`, `requestAnimationFrame`, `localStorage`) in Node and crash. This is intentional and locked.

## 10. Dependencies on in-flight work

- **Phase 6A worker (background)** — replaces synthetic detection generator in `core/main.cpp` with real `cv::VideoCapture` + ONNX inference. Produces:
  - Working webcam demo emitting v1 track frames
  - First C++ `actuator-telemetry-emit` thread (60 Hz placeholder per v4 §12.7)
  - Two reference YAML manifests for the user's 113-ONNX collection
  - Possibly `yolov8_detect` postprocessor + Catch2 test
- **AC research subagent C (background)** — produces `workspace_blueprint/research_notes/anticheat_data_exposure_2026.md`. Will extend v4 §12.2 + v5 §13.6 derived-feature lists with academic ground truth.

Phase 5c starts the moment Phase 6A lands. The Phase 5c worker subagent's brief will be: implement the full Phase 5c row above against the now-real camera+inference pipeline, using `mockFrameSource` synthetic v2/v3 data as a development fallback when the C++ side hasn't emitted v2/v3 yet.
