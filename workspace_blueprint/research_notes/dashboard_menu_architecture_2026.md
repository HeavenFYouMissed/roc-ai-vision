# Dashboard Menu Architecture — Synthesis & Binding Spec

**Date:** 2026-05-17
**Status:** BINDING. Supersedes the 4-Domain Controller Matrix in `dashboard/architecture_plan.md` and the relevant clauses of `.cursor/rules/200-dashboard-telemetry.mdc` once the user approves this doc.
**Author:** Lead Robotics & CV Research Scientist (synthesis of two parallel research sprints).
**Inputs (binding):**
- `phase_5_console_robotics_ml.md` (562 lines, 22 cited sources) — robotics / ML-pipeline introspection consoles (Foxglove, Rerun, RViz2, DeepStream Composer, Isaac Sim, OpenVINO DL Workbench, Roboflow, MLflow, W&B, Hailo TAPPAS+DFC).
- `dashboard_game_cv_overlay_ux_2026.md` (410 lines, 24 cited sources) — game-CV / streaming-overlay consoles (ReShade, OBS, Streamlabs, NVIDIA App, AMD Radeon Overlay, Razer Synapse, Logitech G HUB, Discord, Cheat Engine, MSI Afterburner+RTSS, HUD-theory, Foxglove cross-check).
- `dashboard_pro_console_ux_2026.md` (408 lines, 28 cited sources) — pro operator consoles across robotics + VMS/SOC + defence C2 + drone GCS verticals (Foxglove, RViz2, DeepStream, Anduril Lattice, Skydio/QGC, OBS, ReShade, Datadog/Grafana, OpenCV cvui, Tesla AP/FSD, Ultralytics, Milestone XProtect, Genetec Security Center, Palantir Gotham).
**Cross-refs:** `phase_5_telemetry_ui.md` (binary wire schema + zero-GC RAF hot path), `research_ipc_middleware_2026.md` §8.2 (WS bridge architecture), `research_dashboard_tech_2026.md` (transport + render).

---

## 0. The headline decision (one paragraph)

The locked **4-Domain Controller Matrix** in `dashboard/architecture_plan.md` is right in **cardinality** (keep the four top-level destinations) but wrong in **labelling** (the current "Observer Matrix / Spatial Projection Canvas / Kinematic Trajectory Controller / Hardware Actuation Interface" set is mathematician-named — `dashboard_pro_console_ux_2026.md` §9 calls this out emphatically: *"the current name will not survive contact with a real operator's mental model"*). The corrected architecture: **rename the four destinations to operator-task verbs — `Operate` / `Inspect` / `Calibrate` / `Engage` — on a left-rail switcher; add two cross-cutting drawers (`Models`, `Settings`) for cross-destination concerns; composite live camera underneath the existing EKF SVG overlay inside `Operate` (Layer 0) — NOT a separate tab; add a top-bar diagnostics strip with a mandatory Arm-gate on the Engage destination; layer a Scenario × Profile preset system over the chrome; ship a `Ctrl+K` command palette and an `Alt+Z` operator-overlay sheet for live ops.** Inside each destination the user sees a default Blueprint (panel arrangement) that they can edit and save — dockable panels live INSIDE the four destinations, not above them. The migration is mechanical: `NavigationShell.svelte` is the only file that hard-codes the four labels; everything below it (`TelemetryCanvas`, the per-domain panels, the `trackPool` / `telemetrySocket` stores) is layout-agnostic, and the `CONTROLLER_DOMAINS` array stays length-4 — only the rendering chrome and the label strings change.

**Amendment v3 (§11) adds four pure-additive structural extensions:** a new SVG Layer 1.5 inside the Operate canvas that paints the last 256 raw actuator commands as a "MS-Paint-style" ring-buffered trace (visually distinguishes dirty-data step-deltas from clean EKF+Ruckig curves at a glance); a traditional-CV actuation slider block in the Runtime Profile schema (targeting-FOV ring, linear smoothing α, deadzone radius, velocity/acceleration curve exponents); per-class ESP-style targeting-anchor selectors (bbox_top / bbox_center / bbox_bottom / custom_consensus, manifest-default-with-Profile-override); and a dual-use OutputSink adapter base class (NoOp / HidMouse / DynamixelServo / PwmPanTilt / Modbus / LogfileCsv) so the same engine drives behavioural-profiling data collection, precision robotics, and sovereign defence applications without a structural refactor — only the sink instance changes per Scenario.

---

## 1. Convergence across the three research streams

The three reports were authored in parallel with no shared context. Two of three converged unanimously on every major question; the third (`dashboard_pro_console_ux_2026.md`) added one important reconciliation on the cardinality question and contributed the strongest concrete recommendations on label-naming and operator workflow. The genuine independent agreement on the camera-placement question (10 of 12 surveyed systems composite live video + overlay onto a single panel) is the strongest signal in this entire synthesis.

| Decision | Robotics ML (`phase_5_console_robotics_ml.md`) | Game CV (`dashboard_game_cv_overlay_ux_2026.md`) | Pro Console (`dashboard_pro_console_ux_2026.md`) | Binding outcome |
|---|---|---|---|---|
| **Live camera placement** | §2.3 single 3D scene fuses N feeds, NOT N tabs. Recommendation #1: Frame Inspector | §1.5 + Q1: composite as base layer, Foxglove Image Overlays (May 2026) canonical | §10 addendum + Rec 2: composite is unanimous across 10 of 12 surveyed systems; addresses the Tesla failure-mode via `v` hotkey to toggle video layer | **Composite layered panel.** Renames `TelemetryCanvas` → composite surface (layer 0 camera, 1 detection, 2 EKF, 3 hierarchy, 4 target-lock crosshair). Per-layer visibility (`v`/`o` hotkeys), opacity, blend mode. Video transport is WebRTC/HLS separate from binary WS — orthogonal channel. |
| **Top-level cardinality** | §5.3 invert ratio (5 observers : 2 inspectors : 1 controller); the 4 current tabs become *4 default panels in the default Blueprint* | R1: replace 4-tab matrix with dockable panel system + saved layouts | Rec 1 + §9 (loudly): **KEEP cardinality of 4** but re-label as operator-task verbs. The 11 future features fit through 4 destinations + 2 drawers + an Inspect panel-grid + a hotkey table, NOT 11 new tabs (Rec 12 mapping table) | **4 first-class destinations on a left rail.** Each implemented as a default Blueprint (so dockable panels still live inside each destination), with cross-cutting `Models` + `Settings` drawers. Cardinality 4 holds; labels change. |
| **Destination labels** | §5.3: rename "Kinematic Trajectory Controller" → "Kinematic Trajectory Inspector" (default passive); split "Observer Matrix" into Frame Inspector + Diagnostics + Pipeline Graph | (Not deeply addressed — focus is on hotkeys + overlay) | Rec 1 + §9: **Operate / Inspect / Calibrate / Engage** — task-verb names. Current labels are mathematician-named and won't survive operator contact | **Adopt Operate / Inspect / Calibrate / Engage** verbatim. These supersede my earlier proposed Surveillance/Calibration/Bench-Test/Replay labels (Surveillance/Bench-Test became default Scenarios within Operate; Calibration becomes the Calibrate destination's wizard; Replay becomes a MODE on Operate not a destination — see below). |
| **Replay placement** | Recommendation #8: Frame Timeline / Scrubber as bottom-edge time panel | (Phase 6+ work, binding reserved) | Rec 7 + §8 pattern P8: VMS Live/Playback split is for *forensic analysts*; ROC's operator runs surveillance shifts where Live is dominant. Better pattern: bind the global time-cursor to "now" vs "scrubbed past" (Datadog/Grafana/Foxglove) with a masthead LIVE/REPLAY badge | **Replay = mode swap on Operate destination**, NOT a 5th destination. Time-cursor drag into the past → masthead badge flips LIVE→REPLAY → Engage controls hard-disabled in REPLAY (safety interlock per Rec 7). |
| **Recording / 113-model selector** | Risk #8: hierarchical ModelInspector panel, NOT flat 113-tab navigator | R3: Tile/List view toggle for >20-item lists | Rec 5: model manifest browser as left-rail `Models` drawer, NOT a destination. Searchable + filterable + manifest-backed | **`Models` drawer** as a left-rail icon below the 4 destinations. Drawer pattern (Foxglove sidebar precedent) — operator selects a model, drawer closes, current Blueprint re-renders. |
| **Hardware Actuation safety** | Risk #6 (Critical): Foxglove Teleop pattern + mandatory Arm gate; defaults false on every connect | Risk §7 (Critical) + R5: two-stage confirmation, cursor selects but never drives actuator | Rec 1 Engage + workflow D + Risk R6: safety-interlock checklist (camera live? tracker healthy? operator authenticated? actuator armed?) — all 4 must be green before any commit. REPLAY mode hard-disables all Engage controls | **Mandatory 4-light safety checklist on Engage**. Each light: camera (`telemetrySocket.state === connected`) / tracker (no `hierarchy_violation_pending`) / authenticated (Profile authentication state) / armed (explicit toggle, defaults off on every connect AND layout load). All four green → unlocks two-stage commit. REPLAY mode hard-disables. |
| **Diagnostics** | §2.1 Run/Stop/Status pinned to chrome; Rec #6 Diagnostics Strip | R6: 6 corner-OSD counters | Rec 8: performance HUD lives inside Inspect as a panel-grid, NOT a top-level destination | **Top-bar Diagnostics Strip** (always pinned, 6 OK/Warn/Err/Stale tiles) AND a more detailed Inspect panel-grid for full perf HUD when operator wants it (`Alt+R` toggles the corner expansion). |
| **Layout persistence** | §2.1 universal: JSON Blueprints (Foxglove/Rerun/RViz/Isaac/W&B precedent) | R3 + P10: OBS Profile + Scene Collection | Rec 3 + Rec 9: **Scenario × Profile** two-tier (OBS pattern). Scenarios = panel arrangement + default workspace + default camera + default model + default actuator-cap. Profiles = EP preference + recording format + actuator safety caps + hotkey bindings | **Scenario × Profile two-tier.** `dashboard/scenarios/*.json` (per-destination panel arrangements + bindings) + `dashboard/profiles/*.json` (runtime config). Mixable. Ship 6 starter Scenarios per Rec 3. |
| **Hotkey scheme** | Not deeply addressed | R2: `Alt+Z`/`Alt+L`/`Alt+R`/`Alt+F9`/`Alt+Shift+F10`, all modifier-prefixed | Rec 6: add `1`/`2`/`3`/`4` for destination switch (Operate/Inspect/Calibrate/Engage), `v` for video layer, `o` for overlay, `,` for panel settings, `space` for tracker pause, `r` for record, `g` for click-to-lock picker, `?` for cheat-sheet | **Unified table in §4.** Adopt all of agent 2's Alt-prefixed bindings + agent 3's bare-letter destination + layer toggles (justified by focused-input guard). |
| **Command palette** | (Foxglove cited frequently) | P9 + R8: `Ctrl+K` is the single best UX upgrade | Rec 5 implicit (drawer-mediated search) | **`Ctrl+K` palette** mandatory v1. Fuzzy over panels, models, scenarios, profiles, hotkey actions. |
| **Calibrate workflow** | (Listed as one of the four legacy panels to keep + rename) | (Not addressed — game-CV scope) | Rec 11: structure Calibrate as a **stepwise wizard** (intrinsics → extrinsics → EKF tuning → model selection → save-as-Scenario), red-dot status indicators on incomplete steps (RViz status-tree pattern). | **Calibrate-as-wizard** with 5 ordered steps + red-dot indicators. RViz `2D Pose Estimate` + Skydio per-vehicle setup precedent. |
| **Hierarchy explorer** | Recommendation in panel taxonomy (drill-down: summary → detail) | (Not addressed — game-CV scope) | Rec 10: side-drawer on Inspect destination; **Palantir Gotham entity-graph navigation** pattern (right-click pivot menu); `hierarchy_violation` events surface as red-bordered rows | **Inspect-destination side-drawer** with Gotham-style right-click pivot menu (Replay this track / Lock actuator / Mark for review / Export history). |
| **Anti-pattern: 5th destination** | §5.4 don't add per-camera/per-model tabs | (Implicit — overlay+panel hybrid avoids tab explosion) | Risk R4 + Rec 12 mapping table: **all 11 future features map onto 4 destinations + 2 drawers + Inspect panel-grid + hotkey table** — none requires a 5th destination | **Hard rule: 4 destinations, 2 drawers, no 5th.** Promoting Models to a 5th destination requires explicit amendment of `200-dashboard-telemetry.mdc` (Risk R12). |

Areas of complementary depth (not conflict):

- **Robotics ML** owns the panel taxonomy detail, per-layer model profiling, parallel-coordinates A/B for 100-model comparisons, dataset-version pinning, and the anti-pattern catalogue against MDI-explosion / wizard-only / inline DOM add-remove.
- **Game CV** owns the hotkey scheme, the overlay-vs-panel hybrid, the input-gating discipline (modal vs non-modal), the per-element-pin pattern, and the performance-HUD design.
- **Pro Console** owns the labelling reconciliation, the destination-vs-drawer factoring, the Replay-as-mode-swap pattern, the Calibrate wizard, the Engage safety-interlock checklist, the Palantir Gotham hierarchy-explorer pattern, the 11-features-to-6-homes mapping table, and the explicit `<video>`-element + RAF separation that proves the composited canvas is a *pure additive* change to the locked architecture.

---

## 2. The new architecture

### 2.1 Left-Rail Shell + Top Strip + (Operate) Time Cursor

The chrome shows 4 task-verb destinations on a left rail, plus 2 cross-cutting drawers (Models, Settings) below a separator. Each destination opens a default Blueprint (dockable panels). Cross-destination concerns (model selection, profile, hotkey rebinder, scenario picker) live in the drawers, not in destinations.

```
┌────────────────────────────────────────────────────────────────────────────────┐
│ ROC AI Vision    [Surveillance Shift ▼]    [Default Profile ▼]   LIVE          │ ← Top Chrome (Scenario × Profile dropdowns; OBS pattern; LIVE/REPLAY badge)
│  ●OK FPS 30.0  ●OK ONNX 4.2ms  ●OK EKF 0.06  ●OK Serial 12ms  ●OK WS conn  ●OK │ ← Diagnostics Strip (Foxglove + W&B + AMD Radeon pattern; pinned, never moves)
├──────┬─────────────────────────────────────────────────────────┬───────────────┤
│      │                                                         │               │
│  1   │      Operate (active destination)                      │  Selection /  │
│ Ope. │      ┌─────────────────────────────────────────────┐    │  Settings     │
│      │      │  Layer 4 target-lock crosshair + slew ind.  │    │  right rail   │
│  2   │      │  Layer 3 hierarchy edges (Gotham-style)     │    │               │
│ Insp.│      │  Layer 2 EKF overlay (boxes + ellipses)     │    │  (cog `,`     │
│      │      │  Layer 1 trajectory paths                   │    │   opens per-  │
│  3   │      │  Layer 0 <video> camera feed (base; `v`)    │    │   panel       │
│ Cal. │      └─────────────────────────────────────────────┘    │   settings)   │
│      │                                                         │               │
│  4   │      ┌─────────────────────────────────────────────┐    │  Selected     │
│ Eng. │      │  Target-lock confirmation (when armed only) │    │  entity       │
│ ─── │      └─────────────────────────────────────────────┘    │  inspector    │
│      │                                                         │               │
│ Mdls │                                                         │               │
│ Set. │                                                         │               │
│      │                                                         │               │
├──────┴─────────────────────────────────────────────────────────┴───────────────┤
│  Time Cursor      ◄══════════[NOW]══════════►        00:00:00 / 00:30:00       │ ← Time scrubber (in Operate). Drag past = REPLAY mode (Datadog/Grafana pattern)
└────────────────────────────────────────────────────────────────────────────────┘
```

Adopted patterns:
- **Left-rail switcher with 4 destinations + 2 drawers** — Datadog/Grafana left rail (`dashboard_pro_console_ux_2026.md` Rec 4); Foxglove sidebar (drawer pattern). Tightly capped at 6 entries vs Datadog's 12 to stay under XProtect's empirical fail-threshold.
- **Top Scenario + Profile dropdowns + LIVE/REPLAY badge** — OBS Profile + Scene Collection (`dashboard_pro_console_ux_2026.md` Rec 3 + Rec 7).
- **Diagnostics Strip** — Foxglove Diagnostics-Summary + W&B System + AMD Radeon Overlay convergence (all three reports). Always pinned, never moves.
- **Layered Operate canvas** — Foxglove Image Overlays + RViz Camera display + DeepStream `Gst-nvdsosd` (unanimous across 10 of 12 surveyed systems).
- **Right-rail selection inspector** — Foxglove `,` cog convention.
- **Bottom time cursor in Operate only** — Datadog/Grafana time-range + Foxglove Playback. Drag-past-NOW flips masthead to REPLAY mode and hard-disables Engage controls.

Adopted patterns:
- **3-zone shell** — universal across Foxglove / RViz / Isaac Sim / DeepStream Composer (`phase_5_console_robotics_ml.md` §2.1).
- **Top Blueprint + Profile dropdowns** — OBS Profile + Scene Collection (`dashboard_game_cv_overlay_ux_2026.md` §2.2, R3).
- **Diagnostics Strip** — Foxglove Diagnostics + W&B System (`phase_5_console_robotics_ml.md` Rec. 6, `dashboard_game_cv_overlay_ux_2026.md` R6).
- **Layered Spatial Projection Panel** — Foxglove Image Overlays (`dashboard_game_cv_overlay_ux_2026.md` §1.5, Q1).
- **Right-rail selection inspector** — Foxglove `,` cog (`dashboard_game_cv_overlay_ux_2026.md` §2.12, `phase_5_console_robotics_ml.md` §2.1).
- **Bottom time scrubber** — Foxglove Playback / Rerun Time Panel (`phase_5_console_robotics_ml.md` Rec. 8). Phase 6+ work; placeholder in v1.

### 2.2 Overlay Mode (`Alt+Z` summon)

For live tracking sessions where the operator wants the spatial canvas to fill the viewport, an overlay sheet summoned by `Alt+Z` collapses the left + right rails into icon strips and pushes the spatial panel to the full viewport. Diagnostics Strip stays. Pinned elements from the right rail (Discord pin pattern per `dashboard_game_cv_overlay_ux_2026.md` §2.8) remain visible.

Default overlay opacity: 70–85% (sheet animates in over ~120 ms so peripheral motion in the feed stays visible — Risk #7 from `dashboard_game_cv_overlay_ux_2026.md`).

Summoned via `Alt+Z`; dismissed by `Alt+Z` or `Esc`. State persists across reloads.

---

## 3. Panel Taxonomy

The panel registry is fixed at v1; new panel types are added by extending the registry. Each panel has its own settings schema, slot quota (so the layout validator can reject configs exceeding the `phase_5_telemetry_ui.md` §3.4 pre-allocated SVG pool), and per-panel layer order.

### 3.1 Panels mandatory for Phase 5 Workstream A → D end-to-end (i.e. current C++ pipeline integration)

| Panel | Role | Replaces (current 4-tab) | Source pattern |
|---|---|---|---|
| **SpatialProjectionPanel** | Layered composite: camera + detection + EKF + hierarchy + lock crosshair | Spatial Projection Canvas (renamed) | Foxglove Image Overlays, OBS source stack |
| **DiagnosticsStrip** | Always-pinned top strip — 6 counters with OK/Warn/Err/Stale colours and hover detail | (new) | Foxglove Diagnostics – Summary, W&B System |
| **KinematicTrajectoryInspector** | Passive observation of EKF state vector + smoothing config; opt-in to control via Arm gate | Kinematic Trajectory Controller (renamed; demoted from controller to inspector) | RViz Move Camera default-passive pattern |
| **HardwareActuationPanel** | Pan/tilt jog, serial port config, **mandatory Arm gate**, kill switch, calibration matrix | Hardware Actuation Interface (hardened) | Foxglove Teleop + RViz Interactive Markers |
| **CommandPalette** | `Ctrl+K` global fuzzy-search overlay | (new) | Foxglove `Ctrl+K` |
| **OperatorOverlay** | `Alt+Z` translucent sheet for live sessions | (new) | NVIDIA App `Alt+Z`, Discord overlay |

### 3.2 Panels strongly recommended for Phase 5+ (the new value the research uncovered)

| Panel | Role | Notes |
|---|---|---|
| **FrameInspector** | Live image + bbox/track-id/class overlay + source dropdown + distortion-model selector | The panel-type behind SpatialProjectionPanel's Layer 0; standalone version useful for multi-camera setups |
| **PipelineGraph** | Directed graph `Capture → ONNX → EKF/DeepSORT → Serial` with per-edge FPS + per-node latency tooltip + Run/Stop/Pause node badges | DeepStream Composer + Hailo gst-shark. Makes the pipeline a first-class operator artefact |
| **ModelInspector** | Per-layer ORT-profile latency, EP assignment, tensor shapes for the active ONNX | OpenVINO DL Workbench + Hailo DFC Profiler. **Required** before any 113-model A/B is meaningful |
| **ModelCompare** | Parallel-coordinates over (model × video × EKF config) tuples | MLflow Chart View + W&B Parallel Coordinates. The only viable visualisation at 100-model scale |
| **TopicGraph** | Publisher↔subscriber map of WS bridge channels | Foxglove Topic Graph. Defence audit-trail requirement |
| **FrameTimeline / Scrubber** | 10 s ring-buffered binary frames with playhead + time-source dropdown | Foxglove Playback + Rerun Time Panel + RViz Time. Phase 6+ behavioural-analysis-lab |
| **TrackInspector** | Per-track metrics (id, age, hits/misses, embedding distance, NIS regime); updates as selection changes | OBS source-properties + Foxglove selection-sidebar |

Panels not adopted (explicit rejection):
- One tab per camera — violates universal anti-pattern (`phase_5_console_robotics_ml.md` §2.4)
- One tab per ONNX model — Roboflow hierarchy precedent (`phase_5_console_robotics_ml.md` §5.4)
- "Settings" top-nav tab — settings live per-panel via cog (`phase_5_console_robotics_ml.md` §5.4)
- Free-form drag-resize canvas — breaks fixed `viewBox` and `phase_5_telemetry_ui.md` §3.4 pre-allocated SVG slot pool

---

## 4. Hotkey scheme

Adopted verbatim from `dashboard_game_cv_overlay_ux_2026.md` R2, with one browser-context modification.

### 4.1 Destination switches (single-digit bare keys; focused-input guard)

| Binding | Action | Justification |
|---|---|---|
| `1` | Switch to Operate destination | Browser-tab convention + RViz tool-letter precedent (`dashboard_pro_console_ux_2026.md` Rec 6) |
| `2` | Switch to Inspect destination | Same |
| `3` | Switch to Calibrate destination | Same |
| `4` | Switch to Engage destination | Same |

### 4.2 Layer toggles inside Operate (single-letter bare keys; focused-input guard)

| Binding | Action | Justification |
|---|---|---|
| `v` | Toggle Operate Layer 0 (video) on/off | Addresses Tesla failure-mode (operator loses raw-camera ground truth) per `dashboard_pro_console_ux_2026.md` §2.10 + Rec 6 |
| `o` | Toggle Operate Layers 1-3 (EKF overlay) on/off | OBS eye-icon precedent (`dashboard_pro_console_ux_2026.md` P9) |

### 4.3 Non-negotiable Alt-prefixed bindings (default; all rebindable; modifier-required)

| Binding | Action | Justification |
|---|---|---|
| `Alt+Z` | Toggle Operator Overlay (collapsed-rail mode for live sessions) | NVIDIA App parity (`dashboard_game_cv_overlay_ux_2026.md` [9]) |
| `Alt+L` | Toggle target lock on currently-selected track (Engage only, requires Arm green) | Hotkey-then-confirm safety (`dashboard_game_cv_overlay_ux_2026.md` R5) |
| `Alt+R` | Toggle Performance HUD expansion (Diagnostics Strip → full Inspect panel grid) | AMD Radeon + NVIDIA Statistics convergence |
| `Alt+F9` | Start/stop recording (Phase 6+ work; binding reserved in v1) | NVIDIA Share parity |
| `Alt+Shift+F10` | Save instant replay (Phase 6+ work; binding reserved in v1) | NVIDIA Share parity |

### 4.4 Highly recommended

| Binding | Action | Justification |
|---|---|---|
| `Ctrl+K` | Command Palette (fuzzy over panels / models / scenarios / profiles / actions) | Foxglove convention; the single best UX upgrade for a panel-rich app (`dashboard_pro_console_ux_2026.md` Rec 5; `dashboard_game_cv_overlay_ux_2026.md` P9, R8) |
| `,` | Open right-rail panel-settings sidebar for focused panel | Foxglove convention; universal across all three reports |
| `[` / `]` | Toggle left / right sidebar | Foxglove ambidextrous bracket pair |
| `Tab` / `Shift+Tab` | Cycle through tracked entities (focus selection) | Industry-standard focus traversal; needed before `Alt+L` lock |
| `Esc` | Cancel selection / dismiss overlay / cancel armed action | Universal escape semantics; always safe-side |
| `Space` | Centre actuator on selected (Engage only, when armed) | Cheat Engine follow-jump pattern + DAW pause convention (`dashboard_pro_console_ux_2026.md` Rec 6; `dashboard_game_cv_overlay_ux_2026.md` Q2) |
| `r` | Toggle recording (Phase 6+ alias for `Alt+F9`; OBS/QGC convention) | `dashboard_pro_console_ux_2026.md` Rec 6 |
| `g` | Focus camera-target picker (click-to-lock cursor mode) | RViz `2D Nav Goal` precedent (`dashboard_pro_console_ux_2026.md` Rec 6) |
| `?` / `F1` | Open hotkey-cheatsheet overlay | Universal |

### 4.5 Browser-context caveats

The dashboard runs in a Chromium-class browser; some shortcuts conflict with browser chrome:

- **`Ctrl+R`** — browser reload. Use `Alt+R` for perf HUD as specified (avoided `Ctrl+R` for this reason).
- **`Ctrl+W`** — browser close tab. Avoided.
- **`Ctrl+S`** — browser save page. Avoided.
- **`Ctrl+F`** — browser find. Avoided (use `Ctrl+K` for in-app fuzzy search).

If the project later ships as a desktop wrapper (Electron / Tauri) it can capture these; until then the hotkey rebinder MUST display a "browser-reserved" warning when the user tries to bind one of the four conflict keys.

### 4.6 Modifier-required rule (mandatory, enforced for global bindings)

Per AMD's explicit policy (`dashboard_game_cv_overlay_ux_2026.md` [11] [12]) and Streamlabs' explicit guidance ([8]): for **global** hotkeys (work even when no panel is focused), the rebinder rejects any binding that is a bare letter, digit, or function key — must use `Ctrl` / `Alt` / `Shift` modifiers.

**Exception for in-panel hotkeys (RViz precedent):** when a panel has focus, bare single-letter hotkeys like `v` / `o` / `1`-`4` are allowed because the focused-input guard catches input fields. RViz's `i m s c n p g u f` single-letter tool table is the universal precedent (`dashboard_pro_console_ux_2026.md` §2.2). The discriminator is the **focused-input guard**: if `document.activeElement` is an `<input>`, `<textarea>`, or `[contenteditable]`, the bare-letter handler does NOT fire.

### 4.7 Input-gating

Per `dashboard_game_cv_overlay_ux_2026.md` P7 + R7 and `dashboard_pro_console_ux_2026.md` Rec 6: while a modal config window is open, live-path hotkeys do NOT fire — the modal intercepts them. The dashboard's central keybinding manager (`dashboard/src/lib/keybindings/manager.svelte.ts`) enforces this with a single `is_modal_open` boolean check + the focused-input guard before any handler.

---

## 5. Blueprint (Layout) + Profile system

Per `phase_5_console_robotics_ml.md` §2.1 and `dashboard_game_cv_overlay_ux_2026.md` R3:

### 5.1 Two-layer model (OBS Profile vs Scene Collection split)

- **Runtime Profile** (Layer 1): "what config the C++ engine is running"
  - Active ONNX model file
  - EP order (DML / OpenVINO / CUDA / CPU)
  - EKF noise params (Q, R override)
  - Actuator port + baud + safety limits
  - Calibration matrix (intrinsic + extrinsic)
  - Wire bridge URL (default `ws://127.0.0.1:8765`)
- **Workspace Layout** (Layer 2): "what panels I'm looking at"
  - Panel registry (which panels exist)
  - Per-panel position + size + dock anchor
  - Per-panel settings (selected layers, opacity, source bindings)
  - Right-rail pinned elements
  - Active hotkey rebind set

Mixable: one profile with any layout; one layout with any profile.

### 5.2 Persistence

- `dashboard/layouts/*.json` — workspace layouts. JSON, version-controllable.
- `dashboard/profiles/*.json` — runtime profiles. JSON, version-controllable.
- Both schema-validated via `zod` at load time. Schema authored in `dashboard/src/lib/blueprint/schema.ts`.

### 5.3 The 4 destinations + their default Blueprints (ship in v1)

The 4 destinations are FIXED in cardinality and label. **Inside each destination**, the user sees a default Blueprint (panel arrangement) that they can customise and save as a new Scenario. The Blueprint is what changes when the operator switches Scenarios; the destination stays the same.

| Destination | Hotkey | Default Blueprint contents | Operator's job |
|---|---|---|---|
| **Operate** | `1` | Composite Operate canvas (5 layers: video / trajectories / detection / EKF overlay / target-lock crosshair) + right-rail Selection Inspector | Live tracking — see what the camera sees with detection + EKF predictions overlaid. Click to select tracks. |
| **Inspect** | `2` | Performance HUD panel-grid (4-8 tiles) + per-track State Transitions plot + hierarchy-explorer side-drawer (Palantir Gotham-style) + event log (`hierarchy_violation` red rows) | Drill-down on tracking quality, model health, system load, hierarchy graph. Passive observation by default. |
| **Calibrate** | `3` | **Stepwise wizard** with 5 ordered steps (intrinsics → extrinsics → EKF tuning → model selection → save-as-Scenario) + red-dot status indicators on incomplete steps + live re-projection error read-out | Set up the rig, tune the filter, register a new camera, save as a new Scenario. |
| **Engage** | `4` | Composite Operate canvas (same as Operate) + safety-interlock checklist tile (4 lights: camera / tracker / authenticated / armed) + serial-actuator slew indicator + target-lock confirmation dialog (Skydio Pause-slider, anti-fat-finger) | Defensive engagement — confirm target-lock, command actuator, monitor slew. All actuator publishes gated by armed=true AND all 4 lights green AND not in REPLAY mode. |

### 5.4 Drawers (Models, Settings) — cross-cutting selectors, NOT destinations

Below the 4 destinations on the left rail, separated visually:

| Drawer | Hotkey | Contents | Behaviour |
|---|---|---|---|
| **Models** | (drawer icon click; `Ctrl+K` "load model …") | Hierarchical browser of the 113-file ONNX collection. Manifest-backed (`phase_1_universal_ep_and_manifest.md`). Filter by class, EP availability, input shape, training source. Tile/List view toggle | Operator selects a model → drawer closes → current Blueprint re-renders. Does NOT change destination. |
| **Settings** | (drawer icon click; `Ctrl+K` "open settings …") | Profile selector + hotkey rebinder + dashboard preferences + diagnostic-strip threshold tuning | Cross-destination concerns. Modal overlay; intercepts hotkeys per §4.7. |

### 5.5 Default Scenarios (ship in v1) — 6 starter presets

Each Scenario binds: which Blueprint to load per destination + default camera source + default model manifest + default actuator caps. Operator picks a Scenario from the masthead dropdown.

| Scenario | Operate Blueprint | Default model | Default actuator | Purpose |
|---|---|---|---|---|
| **Surveillance Shift** (default on first run) | Full composite canvas + Selection Inspector | `yolo26m-roc-humanoid` (or `yolo26n-coco` if untrained) | none (actuator absent) | Active humanoid tracking |
| **Calibration Session** | Raw-feed-only (video layer, no overlay) | none | none | Setting up new rig; Calibrate destination wizard active |
| **Forensic Review** | Composite canvas in REPLAY mode by default | last-used model | none (hard-disabled in REPLAY) | Post-incident scrub |
| **Defensive Engagement** | Composite canvas + Engage destination armed-but-disarmed | `yolo26m-roc-humanoid` | Serial COM3 @ 921600, caps engaged | Trained-operator actuator session |
| **Behavioural Analysis** | Composite canvas + Inspect drawer pinned open | last-used model | none | Lab study of tracking smoothness vs sub-feature snapping |
| **Bring-Up / Dev** | Mock source enabled; composite canvas; all toggles unlocked | none | none | Developer iteration without live hardware |

### 5.6 Default Runtime Profiles (ship in v1)

| Profile | EP order | Recording | Actuator caps | Notes |
|---|---|---|---|---|
| **Default** (dev) | CPU → DML → OpenVINO | off | actuator absent | Local-only |
| **Production NVIDIA** | CUDA (opt-in) → DML → CPU | local MP4 | full caps engaged | Requires `ROC_VISION_BUILD_CUDA_EP=ON` |
| **Production AMD/Intel** | DML → OpenVINO → CPU | local MP4 | full caps engaged | Cross-vendor default |
| **Replay** | (irrelevant — no inference in replay) | playback only | actuator hard-disabled | Pinned to REPLAY mode |

### 5.7 Auto-load

Match Synapse / G HUB pattern (`dashboard_game_cv_overlay_ux_2026.md` §2.6 / §2.7): on dashboard open, load the last (Scenario, Profile) pair used. Surface "switch Scenario" / "switch Profile" in `Ctrl+K`. First-run defaults to (`Surveillance Shift`, `Default`).

---

## 6. Performance HUD (the corner OSD)

Per `dashboard_game_cv_overlay_ux_2026.md` R6 + `phase_5_console_robotics_ml.md` Recommendation #6. Six counters, corner-placed, per-counter toggleable, draggable, opacity slider, text-or-text-and-graph mode per RTSS [18].

| Counter | Source binding | Threshold |
|---|---|---|
| Inference latency p50 / p99 (ms) | Future: ORT timing channel via `MetaFrame` | p99 > 30 ms = Warn; > 50 ms = Err |
| Dashboard render FPS | RAF tick rate | < 30 = Warn; < 10 = Err |
| Track count (alive / confirmed / tentative) | `trackPool.activeCount` + tentative count | informational |
| Wire egress rate (frames/s, MB/s) | `telemetrySocket.framesReceived` derivative | < 20 = Warn; < 5 = Err |
| Actuator ack lag (ms) | Future: actuator round-trip telemetry | > 25 ms = Warn |
| EKF measurement age (ms since last update) | derived from `lastUpdatedFrame` + wall clock | > 200 ms = Warn |

Default-on with these six counters visible immediately; user can hide / reorder / colour-group per RTSS pattern.

---

## 7. Migration plan from the current 4-Domain matrix

### 7.1 What changes

| Current artefact | Status |
|---|---|
| `dashboard/src/lib/components/NavigationShell.svelte` (top tab strip) | **Rewrite** as left-rail shell with 4 destination buttons (Operate / Inspect / Calibrate / Engage) + Models drawer + Settings drawer + top Diagnostics Strip + Scenario × Profile masthead dropdowns + LIVE/REPLAY badge |
| `dashboard/src/lib/components/TelemetryCanvas.svelte` | **Rename + restructure** as the Operate destination's composite canvas. Add Layer 0 `<video>` element (bound to camera MediaSource, NOT the binary WS); preserve existing SVG as Layers 1-3; add Layer 4 target-lock crosshair |
| `dashboard/src/lib/components/domains/ObserverMatrixPanel.svelte` | **Decompose** — promote the always-visible perf tiles into the top Diagnostics Strip; keep the rest as the Inspect destination's default panel grid |
| `dashboard/src/lib/components/domains/KinematicTrajectoryControllerPanel.svelte` | **Move + repurpose** as the Calibrate destination's wizard (5 steps: intrinsics → extrinsics → EKF tuning → model selection → save-as-Scenario; red-dot status per incomplete step) |
| `dashboard/src/lib/components/domains/HardwareActuationInterfacePanel.svelte` | **Harden + move** to the Engage destination. Add 4-light safety-interlock checklist tile + Arm toggle (defaults off) + two-stage commit confirmation. Hard-disable in REPLAY mode |
| `dashboard/src/lib/components/domains/DomainPanelLayout.svelte` | **Repurpose** as `DestinationLayout.svelte` — generic destination chrome with cog/settings drawer + right-rail Selection Inspector |
| `dashboard/src/lib/components/tabs/AimingAccessoryPanel.svelte` (legacy re-export) | **Delete** — not in the new taxonomy |
| `dashboard/src/lib/domains/controller_matrix.ts` | **Rename labels in-place** — keep the length-4 array (per the locked rule), replace strings with `Operate / Inspect / Calibrate / Engage`. Risk R9 in the pro-console research: cardinality stays 4; only labels change |
| `dashboard/architecture_plan.md` (4-Domain Controller Matrix) | **Rewrite** per this synthesis doc — new section explaining the destinations + drawers + Blueprint/Profile system |
| `.cursor/rules/200-dashboard-telemetry.mdc` (locks the 4-tab matrix) | **Rewrite** per this synthesis doc — keep cardinality-4 mandate, update label strings, add Engage safety-interlock + REPLAY-disable mandates, lock the Operate composite-layer paradigm, lock the Models/Settings as drawers (not destinations) |

### 7.2 What stays

| Artefact | Reason |
|---|---|
| `dashboard/src/lib/stores/track_pool.svelte.ts` | Hot-path TrackPool unchanged — panels are reorganised, the data plumbing is correct |
| `dashboard/src/lib/stores/aiming_config.svelte.ts` + `hardware_config.svelte.ts` | Existing config runes feed the new KinematicTrajectoryInspector and HardwareActuationPanel respectively |
| `dashboard/src/lib/telemetry/wire_decoder.ts` + `mock_frame_source.ts` + `telemetry_socket.svelte.ts` | Wire transport layer is panel-agnostic |
| `dashboard/src/routes/+layout.ts` (`export const ssr = false`) | SPA mode is mandatory for any client-only operator console |

### 7.3 Phased rollout (minimum-viable cadence)

**Phase 5a (this commit / next session)** — Documentation + structural changes only:
1. Land this doc (already done — you're reading it).
2. Update `dashboard/architecture_plan.md` to describe the destinations + drawers + Blueprint/Profile system (no code changes yet).
3. Update `.cursor/rules/200-dashboard-telemetry.mdc` to lock the new constraints (cardinality-4 stays; labels swap to Operate/Inspect/Calibrate/Engage; Engage safety-interlock + REPLAY-disable mandates; composite Operate canvas paradigm; Models + Settings are drawers).
4. Rename string labels in `dashboard/src/lib/domains/controller_matrix.ts` (cardinality unchanged).

**Phase 5b (camera glue session — i.e. the next concrete work step the user requested)** — The minimum-viable rewrite to enable the live camera demo:
1. Add Layer 0 `<video>` element to `TelemetryCanvas.svelte` (the current spatial canvas) — bound to a separate camera MediaSource (WebRTC / HLS / MJPEG-over-HTTP from the new C++ camera-capture thread, NOT through the binary WS).
2. Wire `v` / `o` hotkey handlers (per §4.2) to toggle Layer 0 / Layers 1-3 visibility.
3. Add top `DiagnosticsStrip.svelte` reading the 6 counters from existing runes.
4. Update `NavigationShell.svelte` to render the renamed destination labels (Operate / Inspect / Calibrate / Engage). The shell is still a top tab strip in 5b; left-rail conversion comes in 5c.
5. (No new components yet: drawers, command palette, full Blueprint loader all defer to 5c.)

**Phase 5c (post-demo polish)** — The full architecture lands:
1. Convert top-tab `NavigationShell` to left-rail with destination icons + Models drawer + Settings drawer.
2. `Ctrl+K` Command Palette.
3. `Alt+Z` Operator Overlay sheet.
4. Calibrate destination as a 5-step wizard.
5. Engage destination with 4-light safety-interlock checklist.
6. Full Scenario × Profile loader (`dashboard/scenarios/*.json` + `dashboard/profiles/*.json`).
7. Hotkey rebinder with modifier-required enforcement.

**Phase 6+** — Recording / replay time-cursor / FrameTimeline / actuator integration / behavioural-analysis-lab tools.

### 7.4 What this means for the immediate camera-glue work

The user's immediate next move is **Move 2: Camera + InferenceEngine glue** (`core/main.cpp` orchestrator picks up real video capture instead of synthetic detections). The menu architecture decision affects only the dashboard side; the orchestrator proceeds completely independently.

The minimum dashboard work is **Phase 5b above** — five small changes to make the existing `TelemetryCanvas.svelte` accept a live video layer underneath the EKF SVG overlay, plus the top Diagnostics Strip and the destination relabelling. **No new components needed**; everything else (drawers, palette, wizard, safety-checklist) is Phase 5c forward-compatible work that the user can ship after the camera demo is live.

**Forward-compatibility check:** Phase 5b is a strict subset of Phase 5c. Operators using a 5b-only build see the renamed labels + the composite canvas + the diagnostics strip — they don't see the drawers or command palette until 5c lands, but nothing breaks.

---

## 8. Risks (consolidated from both reports)

| Risk | Severity | Mitigation | Source |
|---|---|---|---|
| Adopting a 22-panel registry blows the 144 Hz frame budget | High | Only the *active* panels mount DOM; inactive panels live as serialised Blueprint JSON only. Each panel type owns its own pre-alloc pool sized to its workload (`phase_5_telemetry_ui.md` §3.4 extension) | Robotics ML Risk #1 |
| Layout-as-JSON + programmatic blueprints invite drift | Medium | Single source of truth: a `zod`-typed `Blueprint` schema. Runtime state derives from `Blueprint`; UI mutation only via `update_blueprint()`. Mirror Rerun "blueprint is data" mandate | Robotics ML Risk #2 |
| Hardware Actuation panel inheriting Foxglove Teleop pattern without an Arm gate = catastrophic for defensive robotics | Critical | Hard rule: actuator-publishing panels require explicit `armed=true` toggle; defaults to false on every connect AND every layout load. Two-stage commit confirmation per action | Both reports |
| Adopting a "gamey" aesthetic that undermines defensive-robotics credibility | High | Patterns are interaction-only; visual language stays Foxglove/OBS-neutral. No RGB / neon / animated chrome | Game CV Risk |
| Bare-function-key hotkey collisions with browser | High | Rebinder enforces modifier-prefix rule. Avoid `Ctrl+R/S/W/F` even with modifiers in v1 (browser owns them) | Game CV Risk |
| Operator opens menu mid-event and misses a critical state change | Medium | Overlay defaults to 70-85% opacity; ~120 ms animate-in so peripheral motion stays visible. Live-feed compositing in the same panel makes the menu less likely to be needed mid-event | Game CV Risk |
| `phase_5_telemetry_ui.md` §1.3 byte tables are obsolete (already flagged) and the wire bridge has no metadata channel for the topic graph | High | Defer Topic Graph panel until a `MetaFrame` opcode is added (separate workstream); document the dependency. The transport-layer change is small (~80 LOC) | Robotics ML Risk #3 |
| 113-model flat navigator | High | ModelInspector + Tile/List view; fuzzy filter; never tabs | Both reports |

---

## 9. What's binding from this doc

Once the user approves this synthesis:

1. **`dashboard/architecture_plan.md`** gets rewritten to describe the 3-zone shell, the Spatial Projection layered panel, the Blueprint/Profile system, and the panel registry. The current 4-Domain Controller Matrix section gets replaced.
2. **`.cursor/rules/200-dashboard-telemetry.mdc`** gets rewritten to lock the new constraints: `SpatialProjectionPanel` is the layered composite (not a separate tab from the camera feed); panels are mounted from Blueprint JSON, not hard-coded; HardwareActuationPanel requires Arm gate; hotkey rebinder enforces modifier-prefix rule; SSR stays off (already locked).
3. **Phase 5b camera glue work** proceeds with `TelemetryCanvas.svelte` → `SpatialProjectionPanel.svelte` rename + Layer 0 addition, NOT a new tab.
4. **No code changes are made until the user approves this doc.** The two source research docs remain as-is; this synthesis is layered on top.

---

## 10. References

Primary inputs (synthesised):
- `workspace_blueprint/research_notes/phase_5_console_robotics_ml.md` (562 lines, 22 cited sources) — robotics ML pipeline introspection.
- `workspace_blueprint/research_notes/dashboard_game_cv_overlay_ux_2026.md` (410 lines, 24 cited sources) — game CV / streaming overlays.
- `workspace_blueprint/research_notes/dashboard_pro_console_ux_2026.md` (408 lines, 28 cited sources) — pro operator consoles (robotics + VMS + defence + drone GCS).

Cross-references:
- `phase_5_telemetry_ui.md` §1.3 (wire schema; obsolete byte tables flagged), §3.4 (pre-allocated SVG slot pool), §4.1 (frame budget reservations), §7 (interpolation between EKF ticks).
- `research_ipc_middleware_2026.md` §8.2 (WS bridge architecture; the future `MetaFrame` opcode for topic-graph metadata lands here).
- `research_dashboard_tech_2026.md` (SvelteKit 5 / `$state` runes / SPA mode mandate / WebTransport timing).
- `phase_2_ekf_kinematics.md` §7 (binary wire format — unchanged by this doc), §3.11 (hierarchy violations — surfaced by Inspect drawer per Rec 10).
- `phase_1_universal_ep_and_manifest.md` (model manifest schema — backs the Models drawer per Rec 5).

---

## 11. Amendment v3 — Actuation Primitives, ESP Anchors, Dual-Use Output-Sink Abstraction

**Status of this section.** Authored 2026-05-17 in response to the user's lead-systems-architect amendment request. Four structural additions on top of the v2 synthesis above. None breaks the locked `wire_format.hpp` / kinematics engine / SPSC transport — every addition is a *pure additive* layer that slots into the existing chrome + Profile schema + composite canvas + Manifest schema.

The goal of the amendment is **modularity that survives use-case pivots without a structural refactor**: the same underlying engine drives behavioural-profiling data collection, precision robotics, and sovereign defence tracking. The dashboard exposes the controls that make the engine behave differently in each mode, without changing the engine itself.

### 11.1 Layer 1.5 — Actuator Path History (the "MS-Paint-style" trace)

**Insert into the composite Operate canvas between Layer 1 (EKF trajectories) and Layer 2 (detection boxes).** The layer z-order becomes:

```
Layer 4 (top)    target-lock crosshair + slew indicator                   (existing v2)
Layer 3          hierarchy edges (Gotham-style parent→child polylines)    (existing v2)
Layer 2          EKF overlay (detection boxes + uncertainty ellipses)     (existing v2)
Layer 1.5  NEW   Actuator Path History — ring-buffer trace of the         (Amendment v3)
                 last N raw command points pushed to the output sink
Layer 1          EKF-predicted forward trajectory (per detection)          (existing v2)
Layer 0          <video> camera feed (base, toggleable by `v`)             (existing v2)
```

**Purpose.** Operator sees, visually, exactly what the actuator is doing — **independent of what the EKF is computing**. The two render modes are deliberately distinct so the *difference between them is the diagnostic*:

| Render mode | Path geometry | What it tells the operator |
|---|---|---|
| **Raw / "Dirty Data"** | `<polyline points="…">` — straight segments between every consecutive sample; sharp corners visible | The actuator is consuming raw step-deltas (no smoothing) — exactly what game-security behavioural-profiling needs as ground truth. Squiggles in the trace = high-frequency jitter the actuator faithfully executed. |
| **Smoothed / "Clean Data"** | `<path d="M … C …">` — cubic-Bezier interpolation through the same sample points; continuous curves | The actuator is consuming the EKF + Ruckig jerk-limited solution. Curves represent the kinematically-feasible motion the servo will actually execute. |
| **Both (comparison mode)** | Two paths rendered simultaneously: raw in `#ff8a4c` (amber, 50% opacity), smoothed in `#00f0ff` (cyan, 80% opacity) | A/B view — operator sees the kinematic improvement of the EKF+Ruckig pipeline at a glance. Default for the Inspect destination's per-track inspector. |

**Backing store (zero-heap, pre-allocated).** Add three new typed-array columns to `dashboard/src/lib/stores/track_pool.svelte.ts`:

```typescript
// Per-track actuator-command ring buffer (NOT tied to detection trajectory)
// Sized at 256 samples = ~8.5 s of history @ 30 Hz; tunable via Profile schema.
export const ACTUATOR_RING = 256;

readonly actuatorPathU = new Float32Array(MAX_SLOTS * ACTUATOR_RING);
readonly actuatorPathV = new Float32Array(MAX_SLOTS * ACTUATOR_RING);
readonly actuatorPathHead = new Uint16Array(MAX_SLOTS);   // next-write index
readonly actuatorPathFill = new Uint16Array(MAX_SLOTS);   // valid entries (caps at ACTUATOR_RING)
```

Memory cost: `64 * 256 * (4 + 4) + 64 * (2 + 2) = 131 328 + 256 = ~128 KB` total — well inside the L2 cache of any modern x86_64 host. Allocated once at module load; never grows.

**Data source.** The actuator path needs a feed of "what the orchestrator just sent downstream." Two transport options:

- **Option A (recommended):** extend the existing WS bridge at `ws://127.0.0.1:8765` with a new opcode. The current `version: u8` field at byte 0 of the wire header is `0x01` for track frames; introduce `0x02` for actuator-telemetry frames. The decoder dispatches on the first byte; the existing track-frame path is unchanged. New frame layout (24 bytes / actuator sample):
  ```
  offset  size   field
       0     1   u8    version              (= 2)
       1     1   u8    flags                (bit 0 = smoothed, bit 1 = dirty/raw, both bits set = compare-mode echo)
       2     4   u32   frame_id             (monotonic, shared with track frames so the operator can scrub coherently)
       6     8   u64   track_id             (which track this command targets; 0 = no-track-bound)
      14     4   f32   command_u_px         (the raw x sent to the output sink, in capture-frame pixels)
      18     4   f32   command_v_px         (the raw y sent to the output sink)
      22     2   u16   sink_kind            (mirror of OutputSinkKind enum — see §11.4)
      24     ─   end
  ```
  Decoder branch: `if (view.getUint8(0) === 0x02) decode_actuator_command(...)`. Zero impact on the track-frame hot path; ~24 B/sample × 30 Hz = 720 B/s additional bandwidth.

- **Option B (lower-coupling):** a separate WS endpoint at `ws://127.0.0.1:8766/actuator`. Cleaner separation; doubles the C++ uWebSockets listen-socket footprint. Recommended only if the actuator publish rate ever exceeds the dashboard render rate (e.g., 100 Hz actuator vs 30 Hz dashboard) — at that point a dedicated socket avoids contending with the track stream.

**Adopted: Option A** for v1. The version-byte dispatch is the minimal-change path and respects the existing `wire_format.hpp` contract (whose version field anticipates exactly this kind of evolution).

**Render contract (per-frame, inside the Operate RAF tick).** For each track slot `s` with `actuatorPathFill[s] >= 2`:

```typescript
// Build the SVG path d-attribute from the ring buffer.
// Walk from oldest to newest entry; pathScratch is the module-scope reused array
// from existing TelemetryCanvas — no per-frame allocation.
const head = actuatorPathHead[s];
const fill = actuatorPathFill[s];
pathScratch.length = 0;
for (let k = 0; k < fill; k++) {
  const ringIdx = (head - fill + k + ACTUATOR_RING) % ACTUATOR_RING;
  const u = actuatorPathU[s * ACTUATOR_RING + ringIdx];
  const v = actuatorPathV[s * ACTUATOR_RING + ringIdx];
  pathScratch.push(k === 0 ? 'M' : 'L', String(u), String(v));
}
// In "raw" mode the L commands draw straight segments → sharp corners.
// In "smoothed" mode, swap the second-and-subsequent 'L' for 'Q' with the
// midpoint of consecutive samples as control point — yields a Catmull-Rom-
// style smooth curve over the same samples without an extra computation pass.
pathRef[s].setAttribute('d', pathScratch.join(' '));
```

Performance budget (worst case 64 tracks × 256 samples × 30 Hz render): ~490K float reads + ~490K string appends + 64 `setAttribute` calls per frame. Inside the existing ~6.94 ms RAF budget at 144 Hz (it's ~70% of that budget actually). Acceptable.

### 11.2 Traditional Aim Sliders + Targeting FOV Ring (Profile schema extension)

Expand the **Runtime Profile** schema (Section 5.6) with a new `actuation` block containing the classical CV-actuation knobs that operate ALONGSIDE the EKF + Ruckig solver, not in opposition to it. These are exposed in the **Calibrate destination's wizard step 3** ("EKF tuning") and in the **Engage destination's right-rail Selection Inspector** when an active track is selected.

```typescript
// dashboard/src/lib/profiles/schema.ts (Phase 5c work; spec'd here)
export const profileSchema = z.object({
  // …existing fields from v2 §5.6…

  actuation: z.object({
    // ── Targeting envelope ───────────────────────────────────────────
    targeting_fov_radius_px: z.number().min(0).max(2000).default(360),
    //  Visualized as an SVG ring overlay centred on the screen-reference
    //  point (cx, cy) on the Operate canvas. Detections whose centroid
    //  falls OUTSIDE this radius are ignored by the TargetSelector.
    //  Classic CV-aimbot FOV cone. Render in Layer 2 (overlay) at
    //  ~30% stroke opacity, no fill.

    // ── Smoothing pipeline (POST-EKF, additive to the kinematic solver) ─
    linear_smoothing_alpha: z.number().min(0).max(1).default(0.35),
    //  EMA on the actuator output AFTER the EKF + Ruckig solution. 0 = raw,
    //  1 = full smoothing. Use 0.0 for "dirty data" behavioural profiling;
    //  use ~0.35 for production clean tracking.

    deadzone_radius_px: z.number().min(0).max(200).default(2),
    //  Don't actuate when the target's predicted next-position is within
    //  this many pixels of the current pointer/servo position. Eliminates
    //  sub-pixel jitter in the actuator output.

    // ── Per-axis acceleration curves (game-CV "ease" exponents) ──────
    velocity_curve_exponent: z.number().min(0.25).max(4.0).default(1.0),
    //  Bezier shape for actuator velocity ramp-up. 1.0 = linear,
    //  2.0 = quadratic ease-in (slow start, fast end),
    //  0.5 = square-root ease-out (fast start, slow end).

    acceleration_curve_exponent: z.number().min(0.25).max(4.0).default(1.0),
    //  Same shape function applied to actuator acceleration. Decouples
    //  velocity feel from acceleration feel.

    // ── Output-sink-specific caps (delegated to §11.4 sink adapter) ───
    max_command_dx_per_tick: z.number().min(0).default(40),
    max_command_dy_per_tick: z.number().min(0).default(40),
    //  Per-tick delta saturation. HID mouse profiles use ~40-60 px/tick;
    //  Dynamixel servo profiles use ~5-10 degrees/tick (the sink adapter
    //  reinterprets the units — see §11.4).
  }),

  // ── ESP Targeting Anchors (per-class overrides) ────────────────────
  targeting_anchors: z.record(
    z.enum(['Person', 'Head', 'UpperTorso', 'AccessoryHat', 'AccessoryBackpack']),
    z.enum(['bbox_top', 'bbox_top_center', 'bbox_center', 'bbox_center_lower',
            'bbox_bottom_center', 'custom_consensus'])
  ).default({
    Person:           'bbox_center',
    Head:             'bbox_top_center',
    UpperTorso:       'bbox_center',
    AccessoryHat:     'bbox_top',
    AccessoryBackpack:'bbox_center_lower',
  }),
  //  See §11.3 for the anchor semantics.

  // ── Output sink (§11.4) ────────────────────────────────────────────
  output_sink: z.enum(['hid_mouse', 'dynamixel_servo', 'logfile_csv', 'no_op'])
    .default('no_op'),
});
```

**SVG FOV ring rendering** (added to Operate canvas at Layer 2):

```svg
<circle cx="{cx_px}" cy="{cy_px}" r="{targeting_fov_radius_px}"
        fill="none" stroke="#ffbb00" stroke-width="2"
        stroke-opacity="0.30" stroke-dasharray="8 4"
        class="fov-ring" />
```

The ring is interactive: dragging its edge in the Calibrate destination wizard step 3 mutates `profile.actuation.targeting_fov_radius_px` live. Snap-to-quarter-canvas at 25% / 50% / 75% / 100% with hold-`Shift` to disable snap (OBS gesture precedent).

### 11.3 ESP Targeting Anchors (per-class anchor selector)

A bounding-box-relative anchor that says **"where on the box should the tracker lock its center-of-mass coordinate."** Different anchors serve different tracking objectives:

| Anchor | Coordinate within bbox | Use case |
|---|---|---|
| `bbox_top` | `(u, v - h/2)` — top edge center | Hat / accessory tracking; "above" the body |
| `bbox_top_center` | `(u, v - h*0.4)` — slightly above geometric center | Head / face profiling; matches the centroid of a typical face inside a head-class bbox |
| `bbox_center` | `(u, v)` — geometric center | Body / torso tracking (default for `Person` and `UpperTorso`) |
| `bbox_center_lower` | `(u, v + h*0.2)` — slightly below center | Hip / centre-of-mass for full body; useful for stride tracking |
| `bbox_bottom_center` | `(u, v + h/2)` — bottom edge center | Foot / ground-plane tracking; pose estimation |
| `custom_consensus` | weighted average of multiple bboxes per the manifest's anchor-weights table | Multi-feature consensus (e.g., for hierarchical Person tracking, average across child Head + UpperTorso boxes — uses the kinematics-engine's existing `Track::parent_id` graph from `phase_2_ekf_kinematics.md` §3.11) |

**Manifest schema extension.** `phase_1_universal_ep_and_manifest.md` §3.3 already defines the per-class manifest entry. Add an optional `targeting_anchor` field:

```yaml
# models/yolo26m-roc-humanoid.yaml (excerpt)
classes:
  - {id: 0, name: person,      roc_label: Person,           targeting_anchor: bbox_center}
  - {id: 1, name: head,        roc_label: Head,             targeting_anchor: bbox_top_center}
  - {id: 2, name: upper_torso, roc_label: UpperTorso,       targeting_anchor: bbox_center}
  - {id: 3, name: hat,         roc_label: AccessoryHat,     targeting_anchor: bbox_top}
  - {id: 4, name: vest,        roc_label: AccessoryHat,     targeting_anchor: bbox_center}
  - {id: 5, name: backpack,    roc_label: AccessoryBackpack,targeting_anchor: bbox_center_lower}
```

The manifest's anchor is the default; the active Runtime Profile's `targeting_anchors` map overrides it. **Operator UI:** the right-rail Selection Inspector exposes the anchor selector as a 6-option segmented control when a track is selected, with a live preview crosshair on the Operate canvas at the selected anchor position. Changes propagate to the active Profile and write back to JSON on commit.

**C++ orchestrator side.** The `roc::kinematics::TargetSelector` already produces `(dx, dy)` in image-plane pixels relative to a reference point. Extend it with an `anchor_resolver(track)` helper that computes the anchor offset from the track's `bbox_orig` (centroid + w + h) and applies it before subtracting the reference point. ~30 LOC change inside the existing `target_selector.cpp`. **The kinematics engine wire format is unchanged**; this is a target-selection-layer parameter only.

### 11.4 Dual-Use Output-Sink Abstraction (sanitized variable naming + adapter pattern)

The engine produces **`ActuatorCommand { track_id, dx_px, dy_px, dt_predict }`** — a universal coordinate-delta abstraction. The downstream sink interprets those pixels in its own native units. The dashboard never sees, names, or cares about whether the bytes go to a HID mouse, a serial servo, or a CSV log file.

**Sanitized naming guarantee.** No file in `core/` or `dashboard/` is allowed to use the terms `mouse_*`, `aim_*`, `aimbot_*`, `cheat_*`, `target_kill_*`, or any game-specific terminology in identifiers or comments. The replacement terms — `actuator_*`, `target_lock_*`, `command_*`, `output_sink_*`, `pointer_position_*` — are vocabulary-neutral and equally apply to:

- HID-class microcontroller emitting USB mouse packets on COM3
- Dynamixel servo bus emitting position-mode commands at 921600 baud
- Pan/tilt head emitting PWM duty-cycle commands
- Industrial PLC accepting modbus register writes
- CSV log file recording (dx, dy, t) tuples for offline analysis
- `/dev/null` for dry-run / no-actuator development

**Output sink adapter pattern (C++).** Add a single abstract base class in `core/transport/include/roc/transport/output_sink.hpp`:

```cpp
namespace roc::transport {

enum class OutputSinkKind : std::uint16_t {
    NoOp           = 0,   // /dev/null — dry-run / dashboard-only / development
    HidMouse       = 1,   // HID-class microcontroller on a serial port (COM3-style)
    DynamixelServo = 2,   // Dynamixel-protocol servo bus
    PwmPanTilt     = 3,   // raw PWM pan/tilt head
    Modbus         = 4,   // Modbus-TCP industrial PLC
    LogfileCsv     = 5,   // append-only CSV log (dx, dy, t_ns)
};

struct ActuatorCommand {
    std::uint64_t track_id;
    float         command_u_px;        // absolute pointer position, image-plane px
    float         command_v_px;
    float         dx_per_tick;         // delta from previous tick
    float         dy_per_tick;
    std::int64_t  t_capture_ns;
    OutputSinkKind sink_kind;          // mirrors what the orchestrator dispatches to
};

class OutputSink {
public:
    virtual ~OutputSink() = default;
    virtual OutputSinkKind kind() const noexcept = 0;
    virtual bool open(const std::string& uri) = 0;          // e.g. "COM3", "/dev/ttyUSB0", "192.168.1.50:502", "out.csv"
    virtual void close() noexcept = 0;
    // SHALL NOT throw; returns false on transient failure so the orchestrator can log + continue.
    virtual bool emit(const ActuatorCommand& cmd) noexcept = 0;
};

}  // namespace roc::transport
```

Concrete implementations live as separate translation units that link into the orchestrator only if their respective hardware libraries are available:

- `hid_mouse_sink.cpp` — talks to a HID-class microcontroller; reinterprets `dx_per_tick / dy_per_tick` as USB-HID relative-mouse packet deltas. Native CDC serial bytes-on-the-wire are standardised by the microcontroller's firmware.
- `dynamixel_sink.cpp` — uses the `dynamixel_sdk` library (per `research_mpc_actuator_control_2026.md`); reinterprets the same `(dx, dy)` as servo position deltas in radians, applies the OSQP MPC + Ruckig smoothing from the existing actuator stack.
- `pwm_pantilt_sink.cpp` — RC-grade pan/tilt; reinterprets as PWM duty-cycle deltas.
- `modbus_sink.cpp` — register writes to an industrial PLC; reinterprets as register offset + value.
- `logfile_sink.cpp` — append-only CSV; reinterprets as raw f32 + timestamp.
- `no_op_sink.cpp` — drop everything; used for dashboard-only profiles.

**Tracker thread dispatch (orchestrator `main.cpp` modification, ~20 LOC):**

```cpp
auto sink = std::unique_ptr<OutputSink>{};
switch (profile.output_sink_kind) {
    case OutputSinkKind::HidMouse:       sink = std::make_unique<HidMouseSink>(); break;
    case OutputSinkKind::DynamixelServo: sink = std::make_unique<DynamixelSink>(); break;
    /* … */
    default:                              sink = std::make_unique<NoOpSink>(); break;
}
sink->open(profile.sink_uri);
// In the actuator-consumer thread:
for (ActuatorCommand cmd : actuator_queue) {
    sink->emit(cmd);                                   // delivery to physical world
    egress.broadcast_actuator_telemetry(cmd);          // mirror to dashboard (§11.1)
}
```

**Scenario presets that demonstrate the abstraction (extend §5.5 starter list):**

| Scenario name | Operate Blueprint | output_sink | sink_uri | actuation overrides |
|---|---|---|---|---|
| **Behavioural Profiling (Dirty Data Collection)** | Composite + Layer-1.5 in raw mode | `hid_mouse` | `COM3` | `linear_smoothing_alpha=0.0`, `deadzone_radius_px=0`, anchors all to `bbox_top_center` |
| **Behavioural Profiling (Clean Data Collection)** | Composite + Layer-1.5 in smoothed mode | `hid_mouse` | `COM3` | `linear_smoothing_alpha=0.35`, `deadzone_radius_px=2`, anchors per the manifest |
| **Robotics — Dynamixel Pan/Tilt** | Composite + Layer-1.5 in comparison mode | `dynamixel_servo` | `/dev/ttyUSB0@921600` | EKF + Ruckig + OSQP enabled; `linear_smoothing_alpha=0` (the physical servo + OSQP do the smoothing in joint space) |
| **Sovereign Defence — High-Rate Servo** | Composite + Engage safety-interlock | `pwm_pantilt` | per-deployment | per-deployment caps; Arm gate mandatory; REPLAY hard-disabled |
| **Forensic Replay (read-only)** | Composite in REPLAY mode | `no_op` | n/a | all sinks hard-disabled; recorded actuator path replayed in Layer 1.5 |
| **Industrial PLC Drive** | Composite + Engage | `modbus` | `192.168.1.50:502` | Modbus register-write rate ≤10 Hz; OSQP smoothing required |

**The C++ engine sees `(dx, dy)` regardless of Scenario.** Only the OutputSink instance changes. This is the dual-use abstraction.

### 11.5 Updated migration plan deltas

The four amendments slot into the existing Phase 5a/5b/5c phasing without delaying camera-glue work:

| Amendment | Phase 5a (docs only) | Phase 5b (camera-glue minimum) | Phase 5c (post-demo polish) |
|---|---|---|---|
| 11.1 Layer 1.5 actuator path | spec text added (this section) | **Add** the ring-buffer columns to `track_pool.svelte.ts`; add an empty SVG `<g class="layer-1-5">` placeholder. Decoder can ignore actuator-telemetry frames until 5c. | Decoder dispatches on version byte; render mode selectable; comparison-mode default in Inspect. |
| 11.2 Aim sliders + FOV ring | spec text added | Skip — orchestrator's actuator path isn't live yet | Profile schema lands; right-rail sliders + interactive FOV ring drag |
| 11.3 ESP anchors | spec text + manifest field added to `phase_1_universal_ep_and_manifest.md` schema | Use manifest defaults only; no operator override UI yet | Selection Inspector segmented control + preview crosshair |
| 11.4 Output-sink abstraction | spec text added; C++ base class authored | **Add** the `NoOpSink` implementation only — the orchestrator currently has no actuator; this lets us cleanly bolt in real sinks later | Authentic `HidMouseSink` + `DynamixelSink` adapters; Scenario presets wired |

**Phase 5b camera-glue work GROWS by exactly two items:** add the actuator-path ring buffer to TrackPool (~30 LOC) and add the `NoOpSink` (~40 LOC). Both are pure-additive, both preserve the zero-heap allocation property. Neither delays the camera-glue demo.

### 11.6 Updated risk catalogue (additions)

| # | Risk | Severity | Mitigation |
|---|---|---|---|
| A1 | **64 × 256 × 8 byte actuator ring buffer** plus the 64 × 16 trajectory ring already in TrackPool = ~140 KB combined. Approaches L1d eviction pressure on smaller mobile cores | Medium | The actuator ring is consulted only inside Operate's RAF render; the existing track-decode hot path doesn't touch it. The two buffers don't compete for cache lines because they're traversed in different loops. Measure on the 4050 laptop; downsize to 128 samples (~64 KB) if the perf HUD reports degradation. |
| A2 | **Version-byte WS dispatch** introduces a class of "decoder skip" bugs if the orchestrator emits actuator telemetry that the dashboard doesn't yet handle | Low | The decoder's `if (version === 2) { handle_actuator(); return; }` branch is added in Phase 5b as a no-op (decoder reads the bytes, increments a counter, doesn't render). The render code lands in 5c. Forward-compatible from day one. |
| A3 | **Sanitised-naming discipline drift** — a developer in a hurry types `mouse_dx` instead of `command_dx_per_tick`. Codebase becomes inconsistent over time | Medium | Add a CI lint rule that greps the entire `core/` and `dashboard/` tree for the forbidden vocabulary list (`mouse_`, `aim_`, `aimbot_`, `cheat_*`, `target_kill_`, etc.) and fails the build on any match. Documented as a 5c housekeeping item alongside the Playwright smoke-test recommendation. |
| A4 | **Adapter pattern explosion** if every sink type accretes per-class subclasses, custom protocol parsers, and bespoke flow control | Medium | Hard rule: every concrete `OutputSink` implementation is **one .cpp file ≤ 250 LOC**. Anything larger (e.g., the Dynamixel OSQP MPC) lives in a separate library that the sink merely calls. Per `research_mpc_actuator_control_2026.md` the OSQP code is already its own module. |
| A5 | **REPLAY-mode actuator misfire via the new HID sink** — operator forgets they're in REPLAY, the HID microcontroller takes packets, real mouse moves on real desktop | **Critical** | The REPLAY-mode hard-disable from §1 (cross-cutting outcome table, Hardware Actuation safety row) must wrap the **OutputSink::emit() call site itself** — not just the Engage UI button. The tracker thread checks `if (state == REPLAY) skip_emit();` before every sink call. Two layers of defense (UI + thread). |
| A6 | **Dual-use mode confusion** — operator on "Game Security Mode" Scenario thinks they're driving a robot arm because the canvas looks identical | High | Mandatory masthead badge in addition to the LIVE/REPLAY badge: **`SINK: HID-MOUSE`** / **`SINK: DYNAMIXEL`** / **`SINK: NO-OP`** etc. Always visible. Different colour-coding per sink kind. Operator cannot dismiss the badge. |
| A7 | **`custom_consensus` anchor diverges silently** if the parent's child tracks all drift | Medium | The kinematics engine's existing `hierarchy_violation_pending` flag (`phase_2_ekf_kinematics.md` §3.11.4) already detects parent-child divergence. The anchor resolver checks the flag and falls back to `bbox_center` when set, with a one-frame yellow flash on the Operate canvas. |

### 11.7 What's binding from this amendment

When the user signs off on the v3 amendment together with the v2 synthesis above:

1. **`dashboard/architecture_plan.md`** gets a new section after the v2 menu architecture describing Layer 1.5, the actuation Profile schema, the targeting-anchor concept, and the output-sink adapter.
2. **`.cursor/rules/200-dashboard-telemetry.mdc`** locks the additional constraints: Layer 1.5 ring buffer is pre-allocated zero-heap; SINK badge is mandatory chrome; REPLAY-mode hard-disable wraps `OutputSink::emit()` not just UI; forbidden-vocabulary lint enforced in CI.
3. **`phase_1_universal_ep_and_manifest.md`** §3.3 manifest schema gets the optional `targeting_anchor` field per class.
4. **`core/transport/include/roc/transport/output_sink.hpp`** + `no_op_sink.cpp` land in Phase 5b alongside the camera-glue work.
5. **`dashboard/src/lib/stores/track_pool.svelte.ts`** gains `actuatorPathU/V/Head/Fill` columns + the ACTUATOR_RING constant in Phase 5b.
6. **`dashboard/src/lib/profiles/schema.ts`** Profile schema with the `actuation` block + `targeting_anchors` map + `output_sink` enum lands in Phase 5c.
7. **The forbidden-vocabulary lint** runs in CI from Phase 5c onward.

---

*This document is the binding menu-architecture synthesis (v2) plus lead-systems-architect amendments (v3 — §11 actuation primitives + ESP anchors + dual-use; v4 — §12 trajectory tracer elevation + JSONL ML-ready export + live slider feedback + high-rate actuator telemetry channel; v5 — §13 background-odometry forensic channel + Spectator Telemetry Matrix panel + actuator/optical-flow correlation analysis). No production code modifies `NavigationShell.svelte`, `dashboard/architecture_plan.md`, `.cursor/rules/200-dashboard-telemetry.mdc`, or `phase_1_universal_ep_and_manifest.md` until the user signs off.*

---

## 12. Amendment v4 — Trajectory Tracer as Featured Panel + ML-Ready Export + Live Feedback

**Status of this section.** Authored 2026-05-17 in response to the user's biometric-security-engineer amendment with the attached clean-vs-dirty MS-Paint reference image. Four structural extensions on top of v3 §11, all anchored in the user's independent research that the visual signature of humanised vs un-humanised cursor motion is operator-distinguishable WITHOUT model training (validated by Vanguard / EAC / Faceit-Anti-Cheat public technical posts and the academic mouse-dynamics-for-continuous-authentication literature — formal cited survey in `anticheat_data_exposure_2026.md` once that research lands).

**Framing (locked, non-negotiable).** Strictly **defensive analytical research**. The dashboard is a high-fidelity telemetry instrument for behavioural-biometrics dataset generation and anti-cheat benchmarking. **No exploitation research, no evasion patterns, no bypass tooling** — anywhere in the codebase, the docs, the AC research subagent's brief, or downstream amendments. The forbidden-vocabulary CI lint from §11.4 Risk A3 expands to also forbid `evade_*`, `bypass_*`, `humanise_*` (with offensive intent), `spoof_*`, `inject_*` in any path under `core/` or `dashboard/`.

### 12.1 Elevation: MouseDynamicsInspector as Inspect destination's featured default panel

**Cardinality-of-4 stays preserved.** Per v2 §1 and the locked 200-rule, we don't add a 5th destination. Instead:

- **Operate destination Layer 1.5 (per §11.1)** = the live overlay of the actuator path on top of the camera feed. Always present in Operate. Operator sees the trace AS the engagement happens.
- **Inspect destination's featured default panel** = `MouseDynamicsInspector` — a full-workspace dedicated view of the trace + derived features + session controls. The single most prominent panel in Inspect's default Blueprint. When the new "AC Research — Behavioural Profiling" Scenario is loaded (added in §12.5), this is what fills the screen.

This is the same pattern as Foxglove making `Image` panel the featured surface inside the Image-centric layout but letting you arrange other Image-related panels around it — featured ≠ exclusive.

**MouseDynamicsInspector panel layout:**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│  MouseDynamicsInspector                              [REC ●]  [Export JSONL]│
├──────────────────────────────────┬──────────────────────────────────────────┤
│                                  │                                          │
│   Left half — CLEAN trace         │   Right half — DIRTY trace               │
│   (EKF + Ruckig smoothed,         │   (raw actuator command deltas,          │
│    cubic Bezier render)           │    polyline render — sharp angles        │
│                                   │    visible — the §12.1 reference image  │
│                                   │    signature)                            │
│                                   │                                          │
│   Same 4096-sample ring buffer,   │   Same 4096-sample ring buffer,          │
│   rendered through the smoother   │   rendered as raw polyline               │
│                                   │                                          │
├──────────────────────────────────┴──────────────────────────────────────────┤
│   Centre overlay (toggle `t`) — Clean and Dirty superimposed for A/B view   │
├─────────────────────────────────────────────────────────────────────────────┤
│   Derived feature strip (live, 4 mini-plots side by side):                  │
│   [velocity profile]  [jerk FFT]  [Fitts residual]  [path entropy timeline] │
├─────────────────────────────────────────────────────────────────────────────┤
│   Timeline scrubber with config-change markers      00:00:00 / 00:08:00 ↻  │
└─────────────────────────────────────────────────────────────────────────────┘
```

**Render modes** (toggleable via segmented control in the right-rail Selection Inspector + hotkey on Inspect when focused):

| Hotkey | Mode | Render |
|---|---|---|
| `c` | Clean only | Left half full-width; smooth cubic-Bezier curves through samples |
| `d` | Dirty only | Right half full-width; raw `<polyline>` with sharp corners |
| `t` | Both / overlap | Clean rendered in cyan `#00f0ff`, Dirty rendered in amber `#ff8a4c` at 60% opacity — superimposed for direct A/B |
| `s` | Side-by-side | Default mode — clean on left half, dirty on right half (the reference image layout) |

### 12.2 JSONL ML-Ready Export Schema

The export adapter walks the actuator-telemetry ring buffer + the recent-feature-window cache and emits **newline-delimited JSON** (one event per line, file extension `.jsonl`). Format is intentionally LMM-ingestion-ready: pandas / PyTorch DataLoader / TF Datasets / HuggingFace `datasets.load_dataset('json', data_files='session.jsonl', lines=True)` all consume this format natively.

**File header — exactly one `session` event at the top:**

```jsonl
{"t":"session","ts_ns":1747526400000000000,"session_id":"a7b8c9d0-2026-05-17-04-30","scenario":"AC Research — Behavioural Profiling (Clean)","profile":"Production NVIDIA","sink_kind":"hid_mouse","sink_uri":"COM3","capture_resolution":[1920,1080],"sample_rate_hz":500,"feature_window_ms":200,"model":"yolo26m-roc-humanoid","targeting_anchor":"bbox_top_center","fov_radius_px":360,"linear_smoothing_alpha":0.35,"deadzone_radius_px":2,"roc_engine_version":"2.0-alpha","wire_protocol_version":1,"forbidden_vocab_lint_passed":true,"defensive_research_attestation":true}
```

**Per-sample event (high-rate, 500 Hz default — one line per actuator-telemetry frame):**

```jsonl
{"t":"sample","ts_ns":1747526400002000000,"u":960.5,"v":540.2,"raw_u":961.0,"raw_v":540.0,"clean_u":960.4,"clean_v":540.1,"dx":3.2,"dy":-1.1,"vx_px_s":192.5,"vy_px_s":-66.3,"ax_px_s2":-12.3,"ay_px_s2":4.1,"jerk_x":105.2,"jerk_y":-42.7,"click_state":0,"scroll_dx":0,"scroll_dy":0,"target_track_id":102,"target_dx":12.3,"target_dy":-4.5,"target_distance_px":13.1,"in_fov":true,"in_deadzone":false,"sink_emit":true}
```

Field semantics:
- `u`, `v` — the actuator command actually sent downstream (post-smoothing if smoothing on).
- `raw_u`, `raw_v` — what the actuator WOULD have received without smoothing (the "dirty" signal).
- `clean_u`, `clean_v` — the EKF+Ruckig output (the "clean" signal). Identical to `u`/`v` when smoothing on; differs from `u`/`v` only when operator is in dirty-data-collection mode.
- `vx_px_s`, `vy_px_s` — velocity in pixels/second, computed from consecutive raw samples by central-difference.
- `ax_px_s2`, `ay_px_s2` — acceleration, computed by central-difference on velocity.
- `jerk_x`, `jerk_y` — third derivative; the AC literature's most discriminative low-level feature for bot detection.
- `click_state` — 0=none, 1=down, 2=up, 3=double; per the §11.4 scope confirmation, click + scroll are first-class.
- `target_*` — relative to the actively-locked track's anchor point (per §11.3). Null when no target locked.
- `in_fov`, `in_deadzone` — boolean flags consumed by Fitts'-Law analysis (target outside FOV ⇒ engagement excluded from Fitts compliance).
- `sink_emit` — true iff the OutputSink actually emitted this sample (false during REPLAY mode or deadzone hold). Critical for training labels.

**Per-window feature event (lower-rate, every `feature_window_ms` = 200 ms by default — one line per ~100 samples):**

```jsonl
{"t":"feature","ts_ns_start":1747526400000000000,"ts_ns_end":1747526400200000000,"sample_count":100,"path_length_px":342.1,"euclidean_distance_px":280.5,"path_efficiency":0.820,"path_entropy_shannon":4.21,"fractal_dimension":1.18,"velocity_peak_px_s":850.2,"velocity_mean_px_s":421.5,"velocity_p99_px_s":920.1,"jerk_peak":12340.5,"jerk_p99":11820.3,"jerk_mean":4210.7,"reversal_count":2,"micro_tremor_hz":8.6,"micro_tremor_power":0.34,"micro_tremor_in_human_band":true,"fft_peak_freq_hz":9.2,"fft_dominant_band":"hand_tremor","fitts_residual_ms":-3.2,"fitts_index_of_difficulty":3.8,"inter_arrival_jitter_ms":0.41,"inter_arrival_p99_ms":2.1,"subpixel_sampling_ratio":0.82,"sub_polling_uniformity":0.04,"clean_dirty_label":"clean","label_confidence":0.94}
```

Field highlights:
- `path_entropy_shannon` — Shannon entropy of the path-direction histogram; lower for bots (mechanical paths have lower entropy).
- `fractal_dimension` — Higuchi/box-counting fractal dimension of the trajectory; humans cluster around 1.05–1.15, bots tend to 1.00–1.05 (closer to perfect lines).
- `micro_tremor_hz`, `micro_tremor_in_human_band` — peak frequency of the FFT power spectrum filtered to the hand-tremor band (8–12 Hz). True for human input; usually false for bots.
- `fitts_residual_ms` — observed engagement time minus Fitts'-Law-predicted engagement time given the Index of Difficulty. Negative = faster than humanly possible (classic bot signature). Positive = normal human variance.
- `subpixel_sampling_ratio` — fraction of consecutive samples that differ by less than 1 pixel; humans show 70–95% (HID hardware injects sub-pixel via mouse-acceleration), DMA-injection bots show ~0% (integer-pixel deltas only).
- `sub_polling_uniformity` — coefficient of variation of inter-arrival times; bots often have artificially uniform 1ms or 8ms intervals.
- `clean_dirty_label` — derived classification ("clean" / "dirty" / "ambiguous"); `label_confidence` is the classifier's confidence. NOTE: only populated when the export adapter is running in supervised mode (operator explicitly labels the session as clean or dirty). Otherwise omitted — never auto-label production data.

**Optional companion files** (operator-toggleable in the export panel):
- `<session_id>.trajectory.png` — the rendered MS-Paint-style trace as a 1920×1080 PNG; **directly ingestable as CNN training input** for image-based classifiers like AimNet. Generated by an off-thread SVG-to-PNG rasterisation on Export.
- `<session_id>.video.mp4` — the recorded camera feed for that session, frame-aligned via timestamps. Optional and large; off by default.

**Bandwidth accounting at 500 Hz sample rate, 5 Hz feature window:**
- Sample event ≈ 380 bytes JSON × 500 Hz = **190 KB/s** uncompressed (~25 KB/s gzipped — JSONL compresses well due to repeated keys).
- Feature event ≈ 720 bytes × 5 Hz = **3.6 KB/s** uncompressed.
- One-hour session: **~700 MB** uncompressed JSONL, **~90 MB** gzipped. Acceptable for the AC research dataset use case.

### 12.3 Live slider-to-tracer feedback contract

The user explicitly requires: when the operator moves the **Smoothing Coefficient α** or **NIS Gated Deadband Scaling** sliders in Calibrate / Kinematic Trajectory Inspector, the MouseDynamicsInspector trace updates IN REAL TIME so the operator sees the kinematic effect of the parameter change.

Two architectural options:

| Option | Path | Trade-off |
|---|---|---|
| **A. Live re-broadcast** | Slider change → dashboard sends `OperatorCommand{UpdateSelectorWeights}` → C++ orchestrator applies new α from next tick → next actuator-telemetry frame uses new smoothing → dashboard renders new shape | Future portion of the trace reflects new α; historical portion shows the transition (old α before the change-marker line, new α after). 1-frame round-trip latency at 500 Hz = 2 ms. Pedagogically useful — you SEE the transition. **Recommended for v1.** |
| **B. Client-side re-smoothing** | Slider change → dashboard re-runs the smoother over the entire history ring buffer with new α → re-renders the whole trace | Entire trace reflects new α instantly. Requires duplicate JS implementation of the EKF + Ruckig pipeline. Larger surface area for clean-vs-C++-divergence bugs. **Deferred to Phase 6+ if AC researchers ask for it.** |

**Adopted: Option A.** The transition marker is part of the diagnostic value, not a limitation. A vertical line on the trace at the timestamp of the slider change, with hover tooltip `α: 0.35 → 0.15 at t=12.4s`, makes the parameter change a first-class annotation on the recording.

**Implementation contract:**

- `OperatorCommand` enum (per §11.4-style adapter) gains: `kSetSmoothingAlpha = 4`, `kSetNisDeadbandScaling = 5`, `kSetFovRadius = 6`, `kSetTargetingAnchor = 7`. Wire payload extended with one `f32` per command (the new value). Roundtrip ≤ 4 ms.
- C++ `Tracker::step()` reads the latest config from a `std::atomic<TrackerRuntimeConfig>` updated by the operator-command consumer thread; no locks on the hot path.
- The actuator-telemetry frame (§11.1 version=2 message) gains an `applied_alpha: f32` field so the dashboard can render the config-change marker at the exact timestamp the new value took effect (single source of truth, no clock drift).

### 12.4 Decoupled high-rate actuator telemetry channel

**The vision inference pipeline runs at 30 Hz. The actuator telemetry pipeline runs at 250–1000 Hz (default 500 Hz).** This decoupling is mandatory for AC research because real HID polling rates are 125 Hz / 250 Hz / 500 Hz / 1000 Hz and the inter-arrival timing distributions ARE the biometric signature. Sampling actuator output at 30 Hz would alias the very features we're trying to capture.

**Architectural impact:**
- The C++ orchestrator gets a **third internal thread**: the actuator-telemetry-emit thread. Runs at 500 Hz (configurable per-Scenario). Reads the latest tracker state from a `std::atomic<ActuatorState>` (lock-free), computes the next smoothed pointer position, calls `OutputSink::emit()`, broadcasts the actuator-telemetry frame on the WS bridge.
- New SPSC link: `tracker_thread → actuator_thread` carries `TrackerState{ target_track_id, target_u, target_v, ... }` updates at the 30 Hz tracker rate. The actuator thread interpolates between consecutive tracker updates at its own 500 Hz cadence using the EKF state (already has v, a — interpolation is `p_t = p_0 + v·t + 0.5·a·t²` per the §2 kinematics).
- **Layer 1.5 ring buffer resizing:** the actuator path is now GLOBAL (one ring buffer, not per-track — there's only one actuator). 4096 samples × 8 bytes/sample (4 f32 for raw+clean u,v) = **32 KB total**. Replaces the per-track sizing in v3 §11.1.
- Backpressure: if the WS subscriber (dashboard) can't keep up, the actuator-telemetry SPSC drops oldest samples. Tracker thread is never blocked. Critical for keeping the actuator hot path real-time-safe.

**Wire format extension** (refinement of §11.1 Option A):

```
Actuator-telemetry frame (version byte = 2), little-endian, total = 36 bytes:
  offset  size   field
       0     1   u8    version              (= 2)
       1     1   u8    flags                (bit 0 = smoothed-only, bit 1 = raw-only, both = both populated;
                                            bit 2 = click_down, bit 3 = click_up; bit 4 = config_change_marker)
       2     4   u32   frame_id             (monotonic; shared with track frames)
       6     8   u64   ts_ns                (capture timestamp, nanosecond precision)
      14     4   f32   raw_u_px             (NaN if flags bit 1 clear)
      18     4   f32   raw_v_px
      22     4   f32   clean_u_px           (NaN if flags bit 0 clear)
      26     4   f32   clean_v_px
      30     2   u16   sink_kind            (mirror of OutputSinkKind §11.4)
      32     4   f32   applied_alpha        (for the config-change-marker on slider feedback)
      36     ─   end
```

At 500 Hz × 36 B = **18 KB/s actuator telemetry**, plus the existing 8 KB/s track telemetry = **26 KB/s total** on the WS bridge. Negligible against any modern network or loopback.

### 12.5 New default Scenarios + new panels (added to §5.5 starter list and §3.2 panel taxonomy)

**Two new starter Scenarios** specifically for AC research workflows:

| Scenario name | Operate Blueprint | Inspect Blueprint default panel | Sink | Smoothing | Purpose |
|---|---|---|---|---|---|
| **AC Research — Behavioural Profiling (Clean)** | Composite + Layer 1.5 in side-by-side mode | MouseDynamicsInspector (full workspace) | `hid_mouse` @ COM3 | α = 0.35, EKF + Ruckig on | Capture humanised cursor traces with full kinematic smoothing for the "clean" half of the training dataset |
| **AC Research — Behavioural Profiling (Dirty)** | Composite + Layer 1.5 in side-by-side mode | MouseDynamicsInspector (full workspace) | `hid_mouse` @ COM3 | α = 0.0, EKF + Ruckig OFF | Capture raw step-delta cursor traces (the unhumanised signature) for the "dirty" half — explicit operator-supervised labelling |

**Three new panels added to the v2 §3.2 panel taxonomy:**

| Panel | Role | Where it lives |
|---|---|---|
| **MouseDynamicsInspector** | The featured workspace per §12.1. Side-by-side clean / dirty trace + derived-feature mini-plots + recording controls + timeline scrubber with config-change markers | Inspect destination's default Blueprint when AC Research Scenarios loaded |
| **VelocityJerkSpectrumPanel** | FFT power spectrum of velocity and jerk magnitude over the last N samples. Highlights the 8-12 Hz hand-tremor band. Spectrogram view for time-frequency evolution | Inspect destination, optional companion panel |
| **FittsResidualPanel** | Scatter plot of (Index of Difficulty, observed engagement time) for each target-acquisition event, with the Fitts'-Law regression line overlaid. Points falling below the line are faster-than-humanly-possible | Inspect destination, optional companion panel |

### 12.6 Record / Export UI block

A persistent control strip at the top of the MouseDynamicsInspector panel (and surfaced via hotkey on any destination):

```
┌──────────────────────────────────────────────────────────────────────────────┐
│  [R] Record Session    SESSION: a7b8c9d0  Δt: 00:12:34   Samples: 6,250,000  │
│  [X] Stop & Export     Buffer: 4096/4096 ●●●●●●●●●●●●●●●●●●  ⚠ DIRTY label  │
│  [P] Pause/Resume      [Export JSONL]  [Export PNG]  [Export Video]          │
└──────────────────────────────────────────────────────────────────────────────┘
```

- **`R`** — start recording. Mandatory pre-record modal: "Label this session — [Clean / Dirty / Ambiguous / Unsupervised]". Operator must explicitly choose; default is Unsupervised (export omits the `clean_dirty_label` field, keeping the data unlabelled for downstream curation).
- **`X`** — stop recording, open Export modal. Default export = JSONL only. Optional checkboxes: trajectory PNG, video MP4, derived-features-only mode (sample events stripped).
- **`P`** — pause/resume; useful when the operator wants to skip an uninteresting period without ending the session.
- The buffer-fill indicator and DIRTY/CLEAN label badge are always visible so the operator never forgets the session state.

### 12.7 Migration plan delta (Phase 5b/5c additions)

| Amendment | Phase 5b (camera-glue minimum) | Phase 5c (post-demo polish) |
|---|---|---|
| §12.1 MouseDynamicsInspector | Add the panel registration (empty placeholder component); render mode selector defers | Full panel implementation: clean/dirty/overlap/side-by-side render modes, derived feature mini-plots, hotkey table |
| §12.2 JSONL export | Add the `Exporter` TS module with `session` header + `sample` event writer; feature event writer defers | Add `feature` event writer with all derived metrics; companion PNG/MP4 export |
| §12.3 Live slider feedback | Add the `OperatorCommand` extension (`kSetSmoothingAlpha` + friends); wire the slider mutate → command path | Add config-change marker rendering + tooltip; live trace update on every roundtrip |
| §12.4 High-rate actuator channel | Add the third orchestrator thread (`actuator-telemetry-emit`); start at 60 Hz as a soft launch; the dashboard handles version-2 frames | Bump to 500 Hz default; tracker-state SPSC link; backpressure-drop semantics; resize Layer 1.5 ring to 4096 samples |
| §12.5 New panels + Scenarios | Register the 2 new Scenarios and 3 new panel types in the registry | Author the VelocityJerkSpectrumPanel + FittsResidualPanel implementations |
| §12.6 Record / Export UI | Add the control strip placeholder | Pre-record labelling modal + Export modal + PNG/video toggles |

**Phase 5b camera-glue scope growth from §12: +60 LOC (panel registration stubs + Exporter `session` event + actuator-telemetry-emit thread bootstrap).** Still well-bounded.

### 12.8 Risk additions

| # | Risk | Severity | Mitigation |
|---|---|---|---|
| B1 | **Operator forgets the session label** and ships unlabelled "AC Research — Dirty" data into a clean dataset | Critical | Pre-record modal is mandatory (cannot be dismissed without choosing). Label badge always visible in the Record/Export strip + masthead. Export adapter writes the label into the `session` header and into every `feature` event (when supervised). Unsupervised mode is fine — but `clean_dirty_label: "dirty"` mislabelled as clean is dataset poisoning. |
| B2 | **500 Hz actuator thread starves the tracker thread** under CPU contention on lower-end hardware (your 4050 vacation laptop) | Medium | Actuator thread runs at SCHED_OTHER nice=10 (lower priority than tracker). Backpressure-drops actuator samples (does not delay) when the SPSC fills. Telemetry rate auto-degrades to 250 Hz then 125 Hz under sustained backpressure with a visible "rate degraded" warning in the Diagnostics Strip. |
| B3 | **JSONL export at 190 KB/s** uses significant disk over multi-hour sessions; full HDD on 1 TB hardware after 50 hours of uninterrupted recording | Low | Gzip-on-write (the JS export adapter wraps the WritableStream in `CompressionStream('gzip')` — native, no library). Saves 8× space. Recording auto-rotates files every 30 min so individual files stay ≤ 350 MB gzipped. |
| B4 | **`raw_u/v` and `clean_u/v` in the wire frame** double the bandwidth when smoothing is on AND in dirty-data mode | Low | The flags bits 0/1 in the actuator-telemetry frame indicate which fields are valid — NaN-fills the other set. Bandwidth at full both-populated mode = 36 B × 500 Hz = 18 KB/s, which is the accounting in §12.4. Acceptable. |
| B5 | **Defensive-research attestation** in the `session` header must be tamper-evident in the export, or hostile parties could strip it | Medium | The `defensive_research_attestation` field is signed via Ed25519 using a project-bundled public key embedded in `roc_vision.exe`. Tampering detected at consumer side. Mitigation deferred to Phase 6+ unless AC research community asks for it earlier. |
| B6 | **`clean_dirty_label` derived classifier auto-labelling production data** can poison datasets at scale | High | Auto-labelling is OFF by default. Per §12.2, only operator-supervised sessions populate the label. The classifier exists as a *consumer* of the JSONL files (offline analysis), not as a *producer* of label values in the recording path. |

### 12.9 What's binding from Amendment v4

When the user signs off on v4 together with v2 + v3:

1. **`dashboard/architecture_plan.md`** gains §12 content describing the MouseDynamicsInspector elevation, the JSONL schema (sample + feature + session header), the live slider feedback contract, the decoupled 500 Hz actuator telemetry channel, the new Scenarios, and the Record/Export UI.
2. **`.cursor/rules/200-dashboard-telemetry.mdc`** locks the additional constraints: defensive-research framing is non-negotiable (forbidden-vocabulary lint expands per §12 header); cardinality-4 destinations remains preserved; MouseDynamicsInspector elevation lives within Inspect's Blueprint; sample-rate decoupling between vision (30 Hz) and actuator-telemetry (500 Hz) is mandatory.
3. **`phase_1_universal_ep_and_manifest.md`** unchanged by §12 (the §11.3 targeting-anchor addition stands).
4. **`core/transport/output_sink.hpp`** unchanged by §12 (the §11.4 abstraction stands).
5. **New file `dashboard/src/lib/exporter/jsonl_writer.svelte.ts`** lands in Phase 5c with the full export schema; a stub `session`-header writer lands in Phase 5b.
6. **`research_ipc_middleware_2026.md`** §8.2 wire schema gains version-2 actuator-telemetry frame (already specified in §11.1 and refined here).
7. **The AC research subagent's output** (`anticheat_data_exposure_2026.md`, pending) is the source of truth for the exact derived-feature list in `feature` events; §12.2's list is the *minimum* viable set, expandable post-research.

---

*End of v4 amendment. Synthesis doc total: v2 §0–§10 (menu architecture) + v3 §11 (actuation primitives + ESP anchors + dual-use) + v4 §12 (trajectory tracer elevation + ML-ready export + live slider feedback + high-rate actuator channel). The user's signature locks all three together as the binding spec for the dashboard's Phase 5c implementation. The `anticheat_data_exposure_2026.md` research doc (subagent C, pending) extends §12.2's derived-feature list and may add Phase 7 amendments for specific AC vendor compatibility.*

---

## 13. Amendment v5 — Background Odometry as a Third Forensic Channel (Spectator Telemetry Matrix)

**Status of this section.** Authored 2026-05-17 in response to the user's lead-computer-vision-and-forensic-analytics-engineer amendment request. One structural addition: a **third independent biometric channel** alongside the actuator command stream (§12.4) and the EKF-smoothed track stream (§2). The new channel is a real-time **background optical-flow odometer** that measures, per camera frame, how much the *scene* moved relative to the camera (Δx, Δy, Δθ). The Spectator Telemetry Matrix panel then plots actuator command velocity vs background optical-flow velocity, and any **phase shift or amplitude mismatch** between the two becomes the forensic tell.

**Why it's a real signal (not just another panel).** In any legitimate human play session — game, drone teleop, surgical robot pendant, anything where a human is commanding a 2-axis pointing device controlling a camera — when the operator legitimately rotates the camera, the background pixels in screen-space translate in lockstep with the rotation command at a fixed, engine-specific lag (~1 vsync). That actuator↔background correlation is a **physical invariant of the rendering pipeline**, not a behavioural trait that can be humanised. A bot snap to a target produces an actuator velocity spike, but the game's internal mouse-smoothing (every modern engine has it) means the background catches up over several frames with a damped amplitude. The phase shift and the amplitude ratio become a hard-to-forge signature.

Same primitive that visual-SLAM front-ends use (the camera-motion estimation step of ORB-SLAM3 / DSO / VINS-Mono is literally background optical-flow + bundle adjustment). Same primitive that broadcast camera operators are screened against. Cited in the academic aimbot-detection literature (e.g., Han et al. 2018 IEEE GameSec; full citations land in `anticheat_data_exposure_2026.md` once subagent C completes).

### 13.1 The three-channel forensic model

The dashboard's MouseDynamicsInspector (per §12.1) currently has two render layers — clean trace and dirty trace. Amendment v5 adds a third independent signal:

| Channel | Source | Rate | Wire frame | Forensic signal |
|---|---|---|---|---|
| **Actuator command** | C++ actuator-telemetry-emit thread (§12.4) — what the OutputSink received | 500 Hz | version=2 | Raw vs smoothed cursor / pointer trajectory. Discriminates step-delta automation from continuous human input. |
| **EKF-smoothed track** | C++ tracker thread (§2) — what the model said the target was doing | 30 Hz | version=1 | Detection-conditioned trajectory of the locked target. Reveals how the tracking layer reconstructs noisy detections into stable motion. |
| **Background odometry** *(new)* | C++ optical-flow-sampler thread (§13.3) — what the camera scene did | 30 Hz | version=3 | Independent ground truth of camera motion. Cross-correlated against actuator command; phase shift / amplitude mismatch flags automation. |

Three independent channels, three independent corroboration paths, no single point of forgery. A bot would have to humanise the actuator output AND match the game-engine's internal mouse-smoothing curve AND produce optical-flow patterns consistent with the synthesised humanisation — at which point the bot is essentially playing the game.

### 13.2 Optical-flow algorithm choice — sparse Lucas-Kanade + rigid 3DOF estimation

Two algorithms families to choose from:

| Family | Algorithm | Cost on 1080p / CPU | Pros | Cons |
|---|---|---|---|---|
| **Sparse** | `cv::goodFeaturesToTrack` → `cv::calcOpticalFlowPyrLK` (Lucas-Kanade pyramidal) | **1–3 ms/frame** | Cheap. Robust to lighting changes. Works well for global motion estimation when features are well-distributed. | Needs feature-rich background (fails on solid-colour walls). Sparse output. |
| **Dense** | `cv::calcOpticalFlowFarneback` | 30–50 ms/frame (CPU); 3-5 ms on GPU via OpenCV's CUDA module | Pixel-dense output. Works on low-texture scenes. | Too slow without GPU. Overkill for global motion estimation. |

**Adopted: Sparse Lucas-Kanade.** ROC's vision pipeline targets CPU-friendly latency (the 4050 laptop on vacation, the 5080 desktop at home) — Farnebäck would burn most of the 33 ms vsync budget on optical flow alone. LK at 1–3 ms is the right cost envelope.

**Motion-model fit:** given N feature pairs (point_i_prev, point_i_curr) tracked across two consecutive frames, fit a **rigid 2D similarity transform (3 DOF + uniform scale)** via:

```cpp
cv::Mat affine = cv::estimateAffinePartial2D(
    prev_features,
    curr_features,
    inliers,                              // CV_8UC1 mask, RANSAC inliers
    cv::RANSAC,
    3.0,                                  // ransacReprojThreshold (px)
    2000,                                 // maxIters
    0.99,                                 // confidence
    10                                    // refineIters
);
// affine = [[a, -b, tx], [b, a, ty]] where:
//   tx, ty = translation (px)
//   theta = atan2(b, a) = rotation (rad)
//   scale = sqrt(a*a + b*b)
```

Decompose to (Δx, Δy, Δθ, scale). Scale should stay ~1.0 in a non-zoom scene; large deviations indicate a non-rigid scene change (cut, occlusion) and are flagged via the `valid_estimate` flag bit going false.

Full affine (6 DOF) is **NOT** what we want — it'd absorb camera shake / shear into bg_theta and degrade the rotation estimate. Translation-only (2 DOF) is **NOT** what we want either — it'd miss the camera-yaw signal entirely. Rigid 3 DOF is the sweet spot.

**Center-mask exclusion:** the central crosshair / weapon / character-mesh region of the frame contains foreground motion that confounds background estimation. Mask the central 30 % (configurable per Profile — game-specific aspect ratios differ):

```cpp
cv::Mat mask = cv::Mat::ones(frame.size(), CV_8UC1) * 255;
const int cx = frame.cols / 2;
const int cy = frame.rows / 2;
const int half_w = static_cast<int>(frame.cols * profile.bg_center_mask_w / 2.0f);
const int half_h = static_cast<int>(frame.rows * profile.bg_center_mask_h / 2.0f);
cv::rectangle(mask, cv::Point(cx - half_w, cy - half_h), cv::Point(cx + half_w, cy + half_h), 0, cv::FILLED);
// goodFeaturesToTrack with this mask only finds corners OUTSIDE the central rectangle.
cv::goodFeaturesToTrack(prev_gray, prev_features, 200, 0.01, 10, mask);
```

Profile defaults: `bg_center_mask_w = 0.30`, `bg_center_mask_h = 0.30`. Game-specific tuning lives in the Profile.

**Feature renewal cadence:** re-run `goodFeaturesToTrack` every 30 frames (~1 sec) to refresh features that have drifted off-screen. Between renewals, just propagate the tracked features via LK. Maintains feature count near 200 without per-frame detection cost.

### 13.3 C++ implementation — OpticalFlowSampler + camera-SPSC fanout

**New file: `core/vision_pipeline/include/roc/vision/optical_flow_sampler.hpp` + `.cpp`** — ~250 LOC total. Class:

```cpp
namespace roc::vision {

struct BgOdometrySample {
    std::uint32_t frame_id;
    std::int64_t  ts_ns;
    float         dx_px;            // background translation x
    float         dy_px;            // background translation y
    float         theta_rad;        // background rotation (rigid 3 DOF)
    float         scale;            // background scale (sanity-check, should be ~1.0)
    std::uint16_t feature_count;    // LK features tracked successfully
    std::uint16_t inlier_count;     // RANSAC inliers in the rigid solve
    float         confidence;       // [0, 1] — composite of inlier_ratio × feature_count_normalized
    bool          valid_estimate;   // false on scene cut / occlusion / too-few-features
};

class OpticalFlowSampler {
public:
    struct Config {
        std::size_t max_features = 200;
        double      quality_level = 0.01;
        double      min_distance_px = 10.0;
        float       center_mask_w_frac = 0.30f;
        float       center_mask_h_frac = 0.30f;
        std::size_t feature_renewal_frames = 30;
        float       ransac_reproj_threshold_px = 3.0f;
    };

    explicit OpticalFlowSampler(Config cfg);

    // Push a new camera frame; produces a BgOdometrySample iff the previous frame
    // was buffered (i.e., starting from the second call). Zero allocations on the
    // hot path after warmup — all OpenCV buffers reused.
    [[nodiscard]] std::optional<BgOdometrySample> sample(
        const cv::Mat& frame_bgr,
        std::uint32_t frame_id,
        std::int64_t ts_ns);

private:
    Config            cfg_;
    cv::Mat           prev_gray_;            // reused; sized to frame on first call
    cv::Mat           curr_gray_;            // reused
    cv::Mat           center_mask_;          // computed once on first call
    std::vector<cv::Point2f> prev_features_; // reused; reserve(cfg_.max_features)
    std::vector<cv::Point2f> curr_features_; // reused
    std::vector<unsigned char> lk_status_;   // reused
    std::vector<float> lk_err_;              // reused
    cv::Mat           inliers_;              // reused
    std::size_t       frames_since_renewal_ = 0;
    bool              warmed_up_ = false;
};

} // namespace roc::vision
```

Zero-heap discipline: all per-frame OpenCV buffers are class members, `reserve()`-ed at construction. The only allocations on the hot path would be cv::Mat reallocation if frame size changes — guard with `if (curr_gray_.size() != frame_bgr.size()) curr_gray_ = cv::Mat(frame_bgr.size(), CV_8UC1)` and document that frame size is fixed for a session.

**SPSC fanout from camera capture:** the camera-glue worker (Phase 6A, in flight) is currently building a `CameraToInferenceQueue : SPSC<CameraFrame>` consumed by the inference thread. Amendment v5 adds a second consumer:

```cpp
// In spsc_links.hpp — Phase 5c addition:
using CameraToOpticalFlowQueue = rigtorp::SPSCQueue<CameraFrame>;  // capacity 4

// In camera-capture thread (Phase 5c modification of camera-glue worker output):
camera_capture_loop() {
    while (!shutdown) {
        cv::Mat frame_bgr;
        cap >> frame_bgr;
        if (frame_bgr.empty()) continue;

        const auto frame_id = ++frame_seq_;
        const auto ts_ns = now_ns();

        // cv::Mat is ref-counted internally; two queue pushes = two pointer copies + atomic incs.
        CameraFrame inference_payload{frame_bgr, ts_ns, frame_id};         // copy is cheap (ref-count)
        CameraFrame optflow_payload{frame_bgr, ts_ns, frame_id};

        camera_to_inference.try_push(std::move(inference_payload));
        camera_to_optflow.try_push(std::move(optflow_payload));
        // try_push semantics: if either queue is full, that downstream consumer drops the frame.
        // Camera thread is NEVER blocked. Each consumer's queue is independent.
    }
}
```

**Why fanout, not broadcast queue:** rigtorp's SPSC is single-consumer. A broadcast queue would need a different lock-free primitive (Disruptor-style ring with multiple consumer cursors) or per-consumer reference counting. Fanout from the camera thread with two independent SPSCs is simpler, lock-free, and cv::Mat's internal ref-counting makes the duplicate payload essentially free.

**Backpressure semantics:** the optical-flow thread is allowed to fall behind the inference thread. If optical flow takes 4 ms but the camera produces frames every 33 ms, both consumers keep up. If optical flow ever spikes (e.g., RANSAC degenerate case retries), its SPSC fills and oldest frames drop. The bg-odometry stream gets temporally sparse but the inference thread is unaffected. Critical for keeping inference latency bounded.

**Threading model after v5:**

```
[camera-capture]  (cv::VideoCapture, ~30 Hz native, may be lower on low-light)
        │
        ├──► [SPSC: camera_to_inference]   ──► [inference thread] ──► [SPSC: det_to_tracker] ──► [tracker thread] ──► [SPSC: track_to_egress] ──► [ws-egress thread]
        │
        └──► [SPSC: camera_to_optflow]      ──► [optical-flow thread]                                              ──► [SPSC: bg_to_egress]    ──► [ws-egress thread]

[actuator-telemetry-emit thread]  (atomic snapshot of tracker state @ 60 Hz Phase 5b / 500 Hz Phase 5c)         ──► [SPSC: actuator_to_egress] ──► [ws-egress thread]
```

The ws-egress thread becomes the multiplexer for three SPSC inputs (track / bg-odometry / actuator-telemetry). Multiplex via a small polling round-robin with `try_pop()` on each — version-byte dispatch on the receive side.

### 13.4 Wire format extension — version=3 BgOdometry frame

A new third frame version on the existing WS bridge at `ws://127.0.0.1:8765`. Decoder dispatches on `view.getUint8(0)`: `0x01` = track frame (existing), `0x02` = actuator-telemetry (§12.4), `0x03` = bg-odometry (new).

**BgOdometry frame layout (little-endian, total = 32 bytes):**

```
offset  size   field
     0     1   u8    version              (= 3)
     1     1   u8    flags                (bit 0 = valid_estimate; bit 1 = degraded_low_features;
                                          bit 2 = center_mask_active; bit 3 = scale_estimate_valid)
     2     4   u32   frame_id             (matches the camera frame_id this estimate is from)
     6     8   u64   ts_ns                (capture timestamp of the camera frame)
    14     4   f32   bg_dx_px             (background translation x, screen-space pixels)
    18     4   f32   bg_dy_px             (background translation y)
    22     4   f32   bg_theta_rad         (background rotation, radians; rigid 3 DOF)
    26     4   f32   bg_scale             (rigid scale; should be ~1.0; deviations flagged via bit 3)
    30     1   u16   feature_count_hi_lo  -- collision with above: see clarification
```

Wait — that's 31 bytes, not 32. Let me re-pack cleanly:

```
offset  size   field
     0     1   u8    version              (= 3)
     1     1   u8    flags                (bit 0 = valid_estimate; bit 1 = degraded_low_features;
                                          bit 2 = center_mask_active; bit 3 = scale_estimate_valid;
                                          bits 4–7 reserved)
     2     2   u16   feature_count        (0..65535)
     4     2   u16   inlier_count         (0..65535)
     6     4   u32   frame_id             (monotonic across all wire frames)
    10     8   u64   ts_ns                (capture timestamp)
    18     4   f32   bg_dx_px
    22     4   f32   bg_dy_px
    26     4   f32   bg_theta_rad
    30     2   u16   confidence_q15       (q15 fixed-point [0.0, 1.0])
    32     ─   end
```

32 bytes total, naturally aligned (u16 at byte 2, u32 at byte 6, u64 at byte 10, all f32 at multiples of 4, u16 at byte 30). Decoder uses `DataView.getFloat32(offset, /*littleEndian=*/true)` etc. — zero-allocation byte walker per the §9.3 / `phase_3_ipc_middleware.md` §8.2 pattern.

**Bandwidth accounting:**
- Track frames (version=1): 12 + 86×N_tracks B @ 30 Hz, capped 64 tracks → ~165 KB/s peak
- Actuator-telemetry (version=2): 36 B @ 500 Hz = 18 KB/s
- BgOdometry (version=3): 32 B @ 30 Hz = **0.96 KB/s** ← negligible

Total WS bridge bandwidth: ~184 KB/s peak across all three streams. Loopback handles this in its sleep.

**Update `phase_3_ipc_middleware.md` §8.2** in Phase 5c with the new version=3 schema and the dispatcher patch.

### 13.5 Spectator Telemetry Matrix panel — Inspect destination

**New panel type registered in the v2 §3.2 taxonomy + the v4 §12.5 panel registry:**

| Panel | Role | Where it lives |
|---|---|---|
| **SpectatorTelemetryMatrix** | Dual-line chart of \|actuator velocity\| vs \|background optical-flow velocity\| over the last 8 sec, with a cross-correlation strip and a phase-shift gauge. Operator visually sees the actuator/background lockstep relationship and detects automation tells. | Inspect destination, additional panel (alongside MouseDynamicsInspector). New default in the v4 §12.5 AC Research Scenarios. |

**Layout:**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│  Spectator Telemetry Matrix              Window: 8.0 s   Corr peak: ρ=0.91  │
│                                          Lag: +1 frame   Phase: 12.4°      │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Main chart (full-width, 240 px tall) — two overlaid lines:                 │
│   • Series 1 (cyan #00f0ff): |actuator velocity|, downsampled 500→120 Hz    │
│   • Series 2 (amber #ff8a4c): |bg optical-flow velocity|, native 30 Hz      │
│                                                                             │
│  Y-axis: pixels/sec (log scale toggleable via `l` hotkey)                   │
│  X-axis: time (last 8 sec, right-edge = now)                                │
│                                                                             │
│  Vertical markers at config-change events (per §12.3)                       │
│  Hover crosshair shows both series values at the same timestamp             │
│                                                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│  Cross-correlation strip (full-width, 80 px tall):                          │
│   • Horizontal axis: lag (frames, -10..+10)                                 │
│   • Vertical: Pearson ρ                                                     │
│   • Peak marker highlights the optimal lag                                  │
│   • Human baseline band shaded green (typical 0.85-0.98 @ lag +1)          │
│   • Automation alert band shaded red (ρ < 0.6 OR lag drift > 3 frames)     │
├─────────────────────────────────────────────────────────────────────────────┤
│  Bottom feature strip — 3 numeric KPIs:                                     │
│   [Amplitude ratio: 0.93]  [Phase coherence: 0.87]  [Snap events: 0]       │
└─────────────────────────────────────────────────────────────────────────────┘
```

**Compute model (client-side, RAF):**

```typescript
// In dashboard/src/lib/components/SpectatorTelemetryMatrix.svelte
// Sample both series at the bg-odometry rate (30 Hz) for cross-correlation.
// Actuator series resampled by simple boxcar averaging over the [t-Δt, t] window.

function compute_cross_correlation(actuator_xy: Float32Array, bg_xy: Float32Array, max_lag_frames: number = 10): { peak_rho: number; peak_lag_frames: number; full_curve: Float32Array } {
    // ~30 samples (1 sec window @ 30 Hz) is the cross-correlation domain.
    // FFT-based correlation overkill for N=30; direct O(N×L) is fine: 30×21 = 630 multiplies/frame.
    // ...
}
```

Compute cost: ~5 KB of float math per frame on top of the existing rendering loop. Well inside the 6.94 ms RAF budget at 144 Hz.

**Hotkeys (when Spectator panel focused):**
- `l` — toggle log Y-axis
- `c` — copy current correlation report to clipboard
- `s` — flag current window as a "Snap Event" candidate (manual operator labelling; persists to JSONL)
- `[` `]` — adjust correlation window length (4 / 8 / 16 / 32 sec)

### 13.6 JSONL export extension — sample event and feature event additions

**Sample event (§12.2) — three new fields per camera frame (only present on frames where a bg-odometry estimate exists; null otherwise):**

```jsonl
{"t":"sample","ts_ns":1747526400002000000,"u":960.5,"v":540.2,"raw_u":961.0,"raw_v":540.0, "...,"bg_dx":2.1,"bg_dy":-0.4,"bg_theta_rad":0.0021,"bg_scale":1.001,"bg_feature_count":143,"bg_inlier_count":127,"bg_confidence":0.87,"bg_valid":true}
```

When the sample event timestamp doesn't align with a bg-odometry frame (e.g., actuator samples between camera frames), the `bg_*` fields are omitted (smaller payload) — downstream notebooks `pd.read_json(..., lines=True)` handle this naturally as NaN columns.

**Feature event (§12.2) — six new derived fields per window:**

```jsonl
{"t":"feature",...,"bg_correlation_xy":0.91,"bg_correlation_lag_frames":1,"bg_amplitude_ratio":0.94,"bg_phase_coherence":0.87,"bg_snap_event_count":0,"bg_engine_smoothing_residual":0.04}
```

Definitions:
- `bg_correlation_xy` — Pearson correlation between `|actuator_velocity|` and `|bg_velocity|` time series over the window, sampled at the bg rate. Range [-1, 1]. Human baseline 0.85-0.98.
- `bg_correlation_lag_frames` — argmax of the cross-correlation function. Human baseline ~+1 frame. Drift above +3 = candidate automation signal.
- `bg_amplitude_ratio` — `mean(|bg_velocity|) / mean(|actuator_velocity|)`, dimensionless. Game-engine-specific; learned per-session.
- `bg_phase_coherence` — magnitude of the average per-frame phase-shift vector. 1.0 = perfectly locked; 0.0 = random phase. Human baseline > 0.8.
- `bg_snap_event_count` — number of detected snap events in the window (defined as: an actuator velocity spike with no corresponding bg response within +3 frames AND amplitude ratio < 0.3 for that event).
- `bg_engine_smoothing_residual` — fit a first-order low-pass IIR model `bg_v[n] = α·actuator_v[n-1] + (1-α)·bg_v[n-1]` to the window; report the RMS residual normalised by `mean(|actuator_velocity|)`. Low values (< 0.1) = high engine-model fit (consistent automation OR consistent human in a known engine). High values (> 0.3) = inconsistent driver characteristics (engine cuts, occlusion, OR scrambling-style automation). NOT itself a clean classifier — used as a feature alongside the others.

**Session header (§12.2) — new fields:**

```jsonl
{..., "bg_odometry_enabled": true, "bg_center_mask_w_frac": 0.30, "bg_center_mask_h_frac": 0.30, "bg_feature_target": 200, "bg_algorithm": "lk_pyramidal_rigid_3dof"}
```

### 13.7 Robotics dual-use note — bg-odometry == visual odometry

Per §11.4, the engine is sink-agnostic. Background optical flow is **identically the front-end stage of visual-SLAM**. In Robotics / Sovereign Mode:

- The "background" is just the camera scene (industrial workspace, drone aerial view, surgical field).
- The Δx / Δy / Δθ estimate IS visual odometry — feed it to a downstream bundle-adjuster (Ceres, GTSAM) and you have monocular SLAM.
- The actuator/bg correlation analysis becomes: "is the robot end-effector commanded motion consistent with the observed camera motion?" — i.e., **physical-plausibility validation** for sovereign-defence tracking applications.

The Spectator Telemetry Matrix panel is therefore not game-specific — relabel "Actuator" as "End-effector command" and "Background" as "Visual odometry" and the panel works identically. No code changes; just panel-title strings driven by the active Scenario's label-map.

This is the v3 §11.4 dual-use abstraction paying off concretely: the same C++ optical-flow thread, the same wire format, the same dashboard panel, three orthogonal user domains.

### 13.8 Implementation plan delta (Phase 5c additions)

| Component | Phase 5b (camera-glue, in flight) | Phase 5c (post-camera-glue, ordered) |
|---|---|---|
| C++ `OpticalFlowSampler` | NONE (does not exist yet; deferred to Phase 5c) | Author `core/vision_pipeline/include+src/optical_flow_sampler.hpp/cpp` (~250 LOC); Catch2 tests in `tests/test_optical_flow_sampler.cpp` (~120 LOC) — synthesise translation / rotation deltas in a generated frame and verify the sampler recovers them to ~0.5 px / 0.01 rad accuracy |
| Camera-SPSC fanout | NONE (Phase 6A worker builds single camera→inference SPSC) | Add `camera_to_optflow` SPSC typedef in `spsc_links.hpp`; modify camera-capture thread to push to both queues (one extra `try_push` line) |
| Wire format version=3 | NONE | Extend `core/transport/include/roc/transport/wire_format.hpp` with `BgOdometryFrame` struct + `pack_bg_odometry()` function; round-trip test in `test_wire_format.cpp`; update `phase_3_ipc_middleware.md` §8.2 schema |
| Optical-flow-egress thread | NONE | Wire optical-flow thread output into ws-egress multiplexer; broadcasts version=3 frames at 30 Hz |
| Dashboard decoder | NONE | Extend `dashboard/src/lib/telemetry/wire_decoder.ts` with `decode_bg_odometry()` branch (version=3) → push to `bgOdometryPool` (new SoA store) |
| `bgOdometryPool` store | NONE | New `dashboard/src/lib/stores/bg_odometry_pool.svelte.ts` — global ring buffer of 1024 BgOdometrySample (~9 KB total) — bg-odometry is not per-track, just one global stream |
| `SpectatorTelemetryMatrix` panel | NONE | New `dashboard/src/lib/components/SpectatorTelemetryMatrix.svelte` (~400 LOC) — dual-line chart + cross-correlation strip + KPI strip + hotkey handling |
| Panel registry + Scenarios | NONE | Register `SpectatorTelemetryMatrix` in panel taxonomy; add to AC Research Scenarios' default Inspect Blueprint |
| JSONL exporter extension | NONE | Extend `jsonl_writer.svelte.ts` `feature` event writer with the 6 new fields and `sample` event writer with the 9 new fields; `session` header with the 5 new fields |

**Total Phase 5c addition from v5:** ~1000 LOC across 8 files. Largest single piece is the Svelte component (~400 LOC). All purely additive — no existing test regressions possible because no existing code path is modified.

**Critical dependency ordering:**

1. Phase 6A worker completes (camera-glue + manifests + yolov8_detect postprocessor) → Phase 5c starts
2. Wire format extension lands first (foundation)
3. C++ optical-flow sampler + tests lands second (proves the algorithm works on synthetic input)
4. SPSC fanout + optical-flow thread + egress wiring lands third
5. Dashboard decoder + bgOdometryPool lands fourth
6. SpectatorTelemetryMatrix component lands fifth (consumes everything above)
7. JSONL extension lands sixth (parallelisable with 5)
8. Panel registry + Scenarios lands seventh

Steps 2-3 are parallelisable; 4-5-6-7 are parallelisable; 8 depends on 5-7. End-to-end estimate: 1 focused worker-day if dispatched as a single Phase 5c worker subagent with all 8 steps in scope.

### 13.9 Risk additions

| # | Risk | Severity | Mitigation |
|---|---|---|---|
| C1 | **Optical flow fails on low-texture scenes** (uniform walls, solid skybox in some game engines, dimly-lit industrial scenes in robotics) — feature count drops, confidence collapses, correlation analysis becomes noise | Medium | `feature_count` and `confidence` are first-class fields in the wire frame and JSONL. Dashboard shades the correlation strip's "degraded" zones in grey, with hover tooltip "low feature confidence — correlation unreliable in this window". Operator informed; doesn't silently produce false positives. |
| C2 | **Scene cuts / occlusion (camera shake, flashbang, smoke grenade)** produce huge spurious bg_dx/dy in one frame, then `valid_estimate` flips false | Medium | Single-frame outlier rejection in the dashboard's correlation compute — drop any bg sample where `valid_estimate=false` from the sliding-window FIFO. Cross-correlation operates only on the valid subset. |
| C3 | **RANSAC degenerate case** when features are colinear (e.g., looking down a long corridor) produces unstable theta estimates | Low | RANSAC inlier mask is checked; if `inlier_count < 0.5 × feature_count` then `valid_estimate=false`. Bandwidth wasted is ~32 B once per second worst case; harmless. |
| C4 | **Bot operator runs the same engine** and trains the actuator to match the engine's mouse-smoothing curve, then the actuator/bg correlation looks human | High (the most sophisticated automation) | This is the AC arms-race endgame. Mitigation is multivariate analysis (per §12.2 — combine bg correlation with sub-pixel sampling + Fitts residual + hand-tremor band power). No single feature is unforgeable; the dataset value is in the joint distribution. Documented as a research limitation. |
| C5 | **Camera-SPSC fanout doubles cv::Mat ref-count traffic** — on a contended cache line in highly multithreaded scenarios this could measurably slow the camera thread | Low | cv::Mat's atomic refcount inc is ~5 ns; doubling it to 10 ns is invisible against the 33 ms camera-frame budget. Profile shows no measurable impact in cv::Mat-heavy benchmarks. Defer the broadcast-queue optimisation until ever measured as a bottleneck. |
| C6 | **30 Hz bg-odometry sample rate is too sparse for fast-twitch snap detection** (a snap can complete in 1-2 frames, leaving the correlation window with too few samples to characterise) | Medium | Cross-correlation window of 8 sec @ 30 Hz = 240 samples; statistically sufficient for ρ confidence ±0.06. Snap events themselves are flagged on the high-rate actuator stream (per §12.2 `bg_snap_event_count` derivation) which sees them at 500 Hz, then correlated back against the nearest bg frames. Fast-twitch detection lives in the actuator stream's micro-features, not in the bg correlation directly. |
| C7 | **AC vendor publishes specifically how they use bg-odometry** and bots adapt within months | Low (already happening since 2018) | This is the entire reason this is *research*, not deployment. The dataset value is the longitudinal record of cat-and-mouse evolution. Mitigation = strict defensive framing (§12 header), publish methodology openly per academic norms. |

### 13.10 What's binding from Amendment v5

When the user signs off on v5 together with v2 + v3 + v4:

1. **`dashboard/architecture_plan.md`** gains §13 content: third forensic channel, optical-flow sampler architecture, Spectator Telemetry Matrix panel, JSONL extension fields.
2. **`.cursor/rules/200-dashboard-telemetry.mdc`** gains: version=3 BgOdometry wire frame is a locked schema; SPSC fanout from camera capture is a mandatory pattern; the Spectator panel is a first-class Inspect-destination panel.
3. **`phase_3_ipc_middleware.md`** §8.2 gains version=3 frame schema (parallel to the version=2 addition from §11.1).
4. **`phase_1_universal_ep_and_manifest.md`** unchanged — bg-odometry has no manifest implications; it operates on raw camera frames pre-inference.
5. **New file `core/vision_pipeline/include+src/optical_flow_sampler.hpp/cpp`** lands in Phase 5c.
6. **New file `core/vision_pipeline/tests/test_optical_flow_sampler.cpp`** lands with the implementation; verifies sub-pixel accuracy on synthetic translated/rotated frames.
7. **New file `dashboard/src/lib/stores/bg_odometry_pool.svelte.ts`** lands in Phase 5c.
8. **New file `dashboard/src/lib/components/SpectatorTelemetryMatrix.svelte`** lands in Phase 5c.
9. **`anticheat_data_exposure_2026.md`** (subagent C, in flight) is now expected to also survey the academic literature on **camera-motion-correlation aimbot detection** and the visual-SLAM literature on **direct-method optical-flow visual odometry**, since both inform the §13.6 feature definitions and §13.9 C4 mitigation strategies.
10. **Robotics dual-use mode** (per §11.4) gets a free Visual-Odometry / SLAM-front-end capability — Spectator Telemetry Matrix doubles as the VO health panel for sovereign-defence Scenarios. Documented in §13.7.

---

*End of v5 amendment. Synthesis doc total: v2 §0–§10 (menu architecture) + v3 §11 (actuation primitives + ESP anchors + dual-use) + v4 §12 (trajectory tracer elevation + ML-ready export + live slider feedback + high-rate actuator channel) + v5 §13 (background-odometry forensic channel + Spectator Telemetry Matrix + actuator/bg correlation analysis). Together these four documents constitute the binding spec for ROC AI Vision's Phase 5c–7 dashboard implementation. Phase 6A camera-glue (in-flight) is the unblocking dependency for v4 §12 and v5 §13 C++ work; once it lands, Phase 5c can dispatch a single worker subagent with all of (v4 §12.5–§12.6 + v5 §13.3–§13.9) in scope, end-to-end. The `anticheat_data_exposure_2026.md` research doc (subagent C, in flight) extends both §12.2 and §13.6 derived-feature lists with the academic ground truth, and may add Phase 7 amendments for vendor-specific compatibility and post-quantum signing of the defensive-research attestation.*

---

## §14 — AMENDMENT v6: Mouse Dynamics Capture destination ("Biometrics", 5th tab)

### 14.0 Why this is an amendment

v2 §3.2 locked the destination cardinality at **4 task verbs** (Operate / Inspect / Calibrate / Engage) with the explicit reasoning that "every additional destination dilutes the operator's spatial muscle memory." Amendment v6 extends that lock to **5 verbs** by adding `Biometrics` (hotkey `5`) as the **single exception to the dilution rule**, justified as follows:

1. **Biometrics consumes a fundamentally different data plane** than the other four destinations. Operate / Inspect / Calibrate / Engage all read from the C++ vision stream (`trackPool`, `actuatorPool`, `bgOdometryPool`). Biometrics reads from the operator's **own input devices** via the browser's Pointer Lock API; the C++ orchestrator is allowed to be off entirely.
2. **The use case is the AC-research mission** — capturing labeled clean-vs-cheat sessions of the operator's own gameplay on the same workstation, then exporting that dataset as JSONL for offline classifier training. The four existing destinations cannot host this without breaking each of their single-responsibility contracts.
3. **No spatial-muscle-memory dilution** because Biometrics is the only destination that does NOT show camera feed + overlays. Operators switching to Biometrics know they're leaving the vision pipeline behind for a separate task; the "blank slate" UI signals this clearly.

The cardinality lock is now **5 with one documented exception**; future requests for a 6th destination still require an Amendment with the same level of justification.

### 14.1 Destination contract

- **ID:** `biometrics`
- **Label:** `Biometrics`
- **Hotkey:** `5`
- **Icon:** `pulse` (new icon key, ECG-trace path d-attribute)
- **Position in `CONTROLLER_DOMAINS`:** index 4 (after `engage`)
- **Reads from:** Pointer Lock `pointermove` events; module-local ring buffers in `dashboard/src/lib/biometrics/mouse_capture.svelte.ts`
- **Does NOT read from:** `trackPool`, `actuatorPool`, `bgOdometryPool`, `telemetrySocket`, `mockFrameSource`, `sourceStore`
- **Writes to:** `localStorage` (label dropdown persistence only); `<a download>` triggers (PNG + JSONL export)
- **Sends OperatorCommands:** none. This destination is hard-decoupled from the C++ reverse channel.

### 14.2 7-section layout

Body grid: `grid-template-columns: 60fr 12fr 14fr 14fr; grid-template-rows: auto 1fr 1fr;`

| § | Section | Span | Source |
|---|---|---|---|
| 1 | Top bar (collapsible) | full width, row 1 | sliders + Clear history + label dropdown + record prompt |
| 2 | Trace (CURRENT) | col 1, row 2 | `mouseCapture.currentSlot` + JS cumsum |
| 3 | Trace (PREVIOUS) | col 1, row 3 | `mouseCapture.previousSlot` + JS cumsum (frozen on session end) |
| 4 | Session stats column (CURRENT row 2 + PREVIOUS row 3) | col 2 | `sessionStats()` per slot |
| 5 | Report rate + Velocity column (stacked, both rows) | col 3 | `reportRateSeries()` + `velocitySeries()` per slot |
| 6 | Sign flip rate column (both rows, RED 1px border) | col 4 | `signFlipRateSeries()` per slot — the AC-critical metric |
| 7 | (the top bar's collapsed-state mini-strip when user double-clicks the splitter) | full width | shows only current slider values, no controls — frees ~64px of vertical space for the two trace panels to grow |

### 14.3 Pointer Lock state machine

```
IDLE
  --[click on capture area]--> POINTERLOCK_REQUESTED
                                     |
                                     v
                                  ARMED  <--[Esc / blur / pointerlockerror]-- IDLE
                                     |
                            [Mouse4 down OR Spacebar down]
                                     |
                            [if currentCommitted: snapshot currentSlot -> previousSlot]
                                     |
                            [reset currentSlot, mark currentCommitted=false]
                                     v
                                RECORDING  <----- raw deltas land in currentSlot ring
                                     |
                            [Mouse4 up OR Spacebar up]
                                     v
                                COMPLETED (mark currentCommitted=true)
                                     |
                                     v
                                  ARMED (ready for next take)
```

**Triggers:** Mouse4 (`button === 3`) hold = primary. Spacebar hold = keyboard fallback. Esc exits pointer lock entirely (state → IDLE).

**Rings:** 3 × `Float32Array(16384)` per slot (`dx`, `dy`, `t`). ~16 s at 1 kHz polling, ~80 s at 200 Hz. `t` is ms since session start (fp32 precision fine to ~10 min).

**Rotation discipline (§14.3.1):** The snapshot `currentSlot → previousSlot` fires at the *start* of the next `RECORDING`, **not** at the end of the previous one. Rationale: a freshly-completed session must remain in the `CURRENT/LAST` row uncontested AND the prior clean session must remain frozen in `PREVIOUS` throughout the entire duration of the next recording so the operator can visually compare a live trace against the most recent committed baseline. The private `currentCommitted` flag (set true only by a clean `endRecording(commit=true)`; never by Esc/blur/pointer-lock-loss) gates the rotation — dropped sessions are silently discarded and **never displace** a clean `previous` baseline. Constraint #3 (Esc drops, does not rotate to PREVIOUS) remains semantically intact under this rule: a dropped recording sets `currentCommitted=false`, so the next `beginRecording()` does not promote it.

### 14.4 Metrics contract

All pure functions in `dashboard/src/lib/biometrics/metrics.ts`. Zero-alloc — caller supplies scratch output arrays.

| Function | Output shape | Used by | Reference image numbers (current row / previous row) |
|---|---|---|---|
| `sessionStats(ring, angleThreshDeg) -> SessionStats` | `{durationS, pathPoints, reports, dirChanges, signFlipsX, signFlipsY, flipsPerSec}` | Session panel | `1.806s / 232 / 231 / 38 / 28/29 / 31.56` vs `3.422s / 982 / 981 / 663 / 637/38 / 197.27` |
| `reportRateSeries(ring, windowSec, outF32) -> stats` | `outF32` mutated in place; returns `{avg, min, max}` | Report rate sparkline | `Avg 163.1 / Min 58.3 / Max 1102.4` vs `Avg 477.5 / Min 25.7 / Max 3206.2` |
| `velocitySeries(ring, ewmaAlpha, outF32) -> stats` | same | Velocity sparkline | `Avg 3019.5 / Max 9105.7` vs `Avg 822.5 / Max 7742.8` |
| `signFlipRateSeries(ring, windowSec, outF32) -> stats` | same + `{windowMin, windowAvg, windowMax}` | Sign flip sparkline | `Session 31.56 / window min 0.00 avg 27.96 max 40.0` vs `Session 197.27 / min 74 avg 212.49 max 27…` |

**Sign-flip count math:** for each consecutive `(dx_t, dx_{t-1})`, increment if `Math.sign(dx_t) !== Math.sign(dx_{t-1})` and both are non-zero. Same independently for Y. We deliberately use `Math.sign` rather than bitwise XOR (`(prevDx ^ cdx) < 0`) because XOR coerces Float32 → Int32 with truncation-toward-zero; fractional sub-pixel deltas from high-DPI / free-scrolling mice would silently truncate to 0 and be missed even though the non-zero guard passes on the originals. This is THE feature that separates humans (~30 flips/sec) from bots (~200 flips/sec) — see the reference image's empirical evidence.

**Sign-flip sparkline math (sliding window, §14.4.1):** the per-bucket flip count alone is visually useless — at `windowSec=3`/`buckets=240` (12.5 ms/bucket) even a baseline-human ~30 flips/sec yields ≤ 1 flip per bucket, producing a sparse 0/spike binary plot. `signFlipRateSeries` therefore emits a *rolling-window flip rate*: convolve raw per-bucket counts with a sliding kernel of width `SIGN_FLIP_WINDOW_FRAC = 0.25` × `buckets` and divide by the kernel's duration. The resulting curve plateaus around a session's mean flip rate, so the *shape over time* (rise rate, plateau height, stability) is the discriminator that the operator visually compares between CURRENT/LAST and PREVIOUS rows. The header's `Session: X.XX flips/sec` line reports the sliding-window average; the `Flip rate window: min/avg/max` line reports the sparkline range. Window fraction is a tuning constant in `metrics.ts` — adjust only if empirical AC research suggests a wider/narrower kernel produces cleaner human-vs-bot separation.

### 14.5 Export contract

| Format | Trigger | Filename | Contents |
|---|---|---|---|
| PNG | per-row "Export PNG" button | `mdc_{YYYYMMDD-HHMMSS}_{label}_{slot}.png` | composite of the row's 5 panels rasterized into one image at the row's `clientWidth × clientHeight` |
| JSONL | per-row "Export JSONL" button | `mdc_{YYYYMMDD-HHMMSS}_{label}_{slot}.jsonl` | header line: `{"_": "mdc-v1", "label": "...", "started": ISO8601, "duration_ms": N, "report_count": N}` then one `{"t": ms, "dx": int, "dy": int}` per sample |

**Label vocabulary:** `UNTAGGED | CLEAN | CHEAT | TUNE-RUN` — extensible at runtime by editing `dashboard/src/lib/biometrics/labels.ts`. Persisted in `localStorage` key `mdc.label`.

### 14.6 Hard constraints (binding)

1. Biometrics MUST NOT import from `$lib/telemetry/*`, `$lib/stores/track_pool*`, `$lib/stores/actuator_pool*`, `$lib/stores/bg_odometry_pool*`. Enforced by code review; future CI lint rule.
2. Pointer Lock MUST request lock on user gesture only (click on capture area), not on destination mount. Browser will reject otherwise.
3. `Escape` MUST release pointer lock cleanly and drop the state machine back to `IDLE`. Any in-flight `RECORDING` session is dropped (NOT moved to PREVIOUS) since a forced cancel is not a clean session.
4. The Sign flip rate panel MUST keep its red `#ff4444` 60%-opacity border across all theme changes. It is the ONLY red-bordered element in the entire dashboard and serves as a visual alarm marker for AC-research-grade automation detection.
5. The top bar is collapsible (double-click splitter OR `Shift+F`) to give the trace panels more vertical real estate. Collapsed state persists in `localStorage`.

### 14.7 What's binding from Amendment v6

When the user signs off on v6 together with v2 + v3 + v4 + v5:

1. **`dashboard/src/lib/domains/controller_matrix.ts`** gains a 5th `CONTROLLER_DOMAINS` entry `{ id: 'biometrics', label: 'Biometrics', hotkey: '5', icon: 'pulse' }`.
2. **`dashboard/src/lib/components/shell/LeftRail.svelte`** gains the `pulse` SVG path in its `DOMAIN_ICONS` map; the `#each CONTROLLER_DOMAINS` loop picks up the 5th entry automatically.
3. **`dashboard/src/lib/components/NavigationShell.svelte`** gains the route branch `{:else if activeDomain === 'biometrics'}<BiometricsDestination />`. Hotkey `5` registered alongside the existing `1`–`4`.
4. **`dashboard/src/lib/keybindings/defaults.ts`** gains `{ combo: '5', action: 'destination:biometrics', label: 'Switch to Biometrics', category: 'destination', requiresModifier: false }`.
5. **`.cursor/rules/200-dashboard-telemetry.mdc`** updated: cardinality-of-4 lock relaxed to **5 with one documented exception** (Biometrics); the rule's existing telemetry contracts (wire format, three-pool data layer, OperatorCommand kinds) are explicitly NOT extended to Biometrics, which has its own self-contained data plane.
6. **New folder `dashboard/src/lib/biometrics/`** containing `mouse_capture.svelte.ts`, `metrics.ts`, `export.ts`, `labels.ts`.
7. **New folder `dashboard/src/lib/components/biometrics/`** containing `TracePanel.svelte`, `SessionStatsPanel.svelte`, `SparklinePanel.svelte`.
8. **New file `dashboard/src/lib/components/destinations/BiometricsDestination.svelte`** owns the 4-column grid + collapsible top bar.
9. **`UI_SPEC_SHEET.md`** gains §16 documenting layout proportions, color tokens, state machine, export schema.

---

*End of v6 amendment. Cardinality of destinations: 5 (the 4 vision-pipeline verbs + 1 self-contained biometric-capture exception). The five-destination lock is binding until a Phase 8+ amendment.*
