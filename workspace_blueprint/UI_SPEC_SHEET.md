# ROC AI Vision — Dashboard UI Spec Sheet (Living Document)

**Audience:** UI reviewers (Claude Code in browser, design collaborators) who need to give precise, codebase-aligned feedback on the dashboard.

**Purpose:** Single source of truth for visual tokens, component tree, interaction patterns, wire format, and safety logic. Any UI suggestion can be expressed in terms of these tokens and patterns so the implementation maps 1:1.

**Repo:** `d:\roc-ai-vision\dashboard\src\lib\`
**Stack:** SvelteKit 5 (runes) · SPA mode (`+layout.ts: export const ssr = false`) · TypeScript strict · Vite 8 · No external CSS framework (hand-rolled `<style>` blocks per component)

**Binding spec (the math + architecture):** `workspace_blueprint/research_notes/dashboard_menu_architecture_2026.md` (v2 ss0-10 + v3 ss11 + v4 ss12 + v5 ss13). The doc you are reading distills the UI side; the binding spec is canonical for any conflict.

---

## 1. Design Tokens

### 1.1 Color System (use these names, not hex, in suggestions)

| Token | Hex | Usage |
|-------|-----|-------|
| `--app-bg` | `#0e0e10` | App background (body) |
| `--surface-bg` | `#161618` | Card surfaces, panels |
| `--surface-hi` | `#1a1a1e` | Slightly elevated cards, hover states |
| `--surface-low` | `#111113` | Inset surfaces (input fields, status pills) |
| `--border` | `#27272a` | Default card borders |
| `--border-subtle` | `#1d1d20` | Internal dividers |
| `--border-active` | `#f5a623` | Active/focus borders (orange accent) |
| `--text-cream` | `#d4d0c8` | Primary text |
| `--text-grey` | `#878787` | Secondary text, labels |
| `--text-muted` | `#666` | Tertiary text |
| `--text-mute2` | `#555` | Disabled labels, footer hints |
| `--text-mute3` | `#444` | Stale dots, separators |
| `--accent-orange` | `#f5a623` | Brand, active states, value emphasis |
| `--status-ok` | `#34d399` | Green/healthy state |
| `--status-warn` | `#f5a623` | Amber/warning state |
| `--status-err` | `#ef4444` | Red/error state |
| `--status-stale` | `#444` | No data / disconnected |
| `--accent-cyan` | `#00f0ff` | EKF trajectories, "clean" trace, INFO level |
| `--accent-amber` | `#ff8a4c` | "Dirty" raw trace, warm warning |
| `--accent-purple` | `#a78bfa` | REPLAY mode badge, GLOBAL hotkey category |
| `--accent-blue` | `#60a5fa` | LAYER hotkey category |

### 1.2 Per-class Detection Color Palette

Mirrors `Label` enum at `core/kinematics_engine/include/roc/kinematics/types.hpp` and `dashboard/src/lib/render/class_colors.ts`. Single source of truth — used by bbox stroke, label pill bg, detection card dot, class visibility checkbox swatches, hierarchy tree dots, object count badge dots.

| Label ID | Class Name | Color | Hex | Used For |
|---|---|---|---|---|
| 0 | Unknown | grey | `#6b6b6f` | Unrecognized class |
| 1 | Person | cyan | `#00f0ff` | Primary subject (body) |
| 2 | Head | magenta | `#ff6dc7` | Anatomic part |
| 3 | UpperTorso | green | `#34d399` | Anatomic part |
| 4 | AccessoryHat | orange | `#f5a623` | Accessory (top) |
| 5 | AccessoryBackpack | amber | `#ff8a4c` | Accessory (carried) |

### 1.3 Type Scale

| Token | Size | Weight | Family | Usage |
|-------|------|--------|--------|-------|
| `text-h2` | 18px / 16px | 600 | sans | Destination titles ("Inspect") |
| `text-card-title` | 11px | 600 | sans uppercase letter-spaced 0.04em | Panel card headers |
| `text-section-title` | 9-10px | 700 | sans uppercase letter-spaced 0.06-0.08em | Right rail section headers ("DETECTIONS", "THRESHOLDS & DISPLAY") |
| `text-label` | 9-11px | 500 | sans uppercase letter-spaced 0.04-0.06em | Tile labels, button labels |
| `text-body` | 11-12px | 400-500 | sans | Body copy, card content |
| `text-value` | 11-13px | 600 | mono | Numeric values, IDs |
| `text-value-big` | 36-48px | 700-800 | mono | Hero numbers (sub-pixel ratio) |
| `text-hint` | 9-10px | 400 italic | sans | Footer hints, empty-state copy |
| `text-keycap` | 10-11px | 600 | mono | Hotkey badges in cheatsheet |
| `text-trace-id` | 10-11px | 500 | mono | Track IDs, hex values |

**Font families:**
- Sans stack: `-apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif`
- Mono stack: `ui-monospace, 'SF Mono', 'Cascadia Code', Consolas, monospace`

### 1.4 Spacing Scale

| Token | Value | Usage |
|-------|-------|-------|
| `space-xs` | 2-3px | Inline gaps, dot spacing |
| `space-sm` | 4-5px | Tight inline padding |
| `space-md` | 6-8px | Standard inline gaps |
| `space-lg` | 10-12px | Card body padding |
| `space-xl` | 14-16px | Card padding, section gutters |
| `space-2xl` | 20-24px | Destination content padding |

**Component-level dimensions (current state):**

| Component | Dimension |
|-----------|-----------|
| Top Chrome | 42px height |
| Diagnostics Strip | 26px height |
| Left Rail | 56px width |
| Right Rail | 280px width |
| Right Rail (collapsed tab) | 14px width |
| Bottom Time Cursor placeholder | 30px height |
| HierarchyExplorerDrawer (Inspect) | 240px width |
| Wizard Rail (Calibrate) | 220px width |
| Event Log Dock (collapsed / expanded) | 30px / 240px |
| Safety Interlock Bar | ~96px (lights + summary) |
| Hardware Bar (Engage bottom) | 48px |

### 1.5 Border Radii + Shadows + Transitions

| Token | Value | Usage |
|-------|-------|-------|
| `radius-sm` | 2-3px | Pills, small chips, inline badges |
| `radius-md` | 4-5px | Buttons, input fields, segmented controls |
| `radius-lg` | 6-8px | Panel cards, modals |
| `radius-pill` | 14-50% | Status dots, count badges |
| `transition-snap` | 80-100ms ease | Hover/active state changes |
| `transition-default` | 120-150ms ease | Color/border transitions |
| `transition-slow` | 200ms ease | Panel transforms, drawer slide |
| `glow-status` | `0 0 4-6px rgba(R,G,B,0.4-0.5)` | Status dot glow when active |
| `shadow-popover` | `0 8px 24px rgba(0,0,0,0.5)` | Floating menus, popovers |
| `shadow-modal` | `0 16px 48px rgba(0,0,0,0.5)` | Center modal dialogs |

### 1.6 Animation Patterns

| Pattern | Spec | Usage |
|---------|------|-------|
| `pulse-armed` | `1.2s ease-in-out infinite`, opacity 0.6↔1.0 + glow shift | Armed dot when system is armed |
| `pulse-pending` | `0.8s ease-in-out infinite`, opacity 0.5↔1.0 | "CONFIRM ARM?" pending text |
| `pulse-replay` | `1.6s ease-in-out infinite`, opacity 0.7↔1.0 | LIVE/REPLAY badge in REPLAY mode |
| `pulse-banner` | `1.2s ease-in-out infinite`, opacity 0.65↔1.0 | Click-to-lock mode banner |
| `pulse-critical` | `1.5s ease-in-out infinite`, bg flash | Event log critical row |
| `shimmer` | `1.4s ease infinite`, bg gradient sweep | Skeleton loading lines |
| `lock-pulse` | `1.2s ease-in-out infinite`, opacity 0.5↔1.0 | Target lock crosshair |
| `tooltip-fade` | `100ms ease`, opacity + translateY(2px) | Hover tooltips |
| `slideIn` | `150ms ease`, translateX(-20px → 0) + opacity | Drawer entrance |

---

## 2. Component Tree

### 2.1 Top-Level Shell

```
NavigationShell.svelte                       (src/lib/components/)
├── TopChrome.svelte                         (.shell .top-chrome — 42px)
│   ├── brand-group (ROC + AI Vision + v2.0-alpha)
│   ├── selector-group (Scenario dropdown + Profile dropdown)
│   └── badge-group (LIVE/REPLAY badge + SINK badge)
│
├── DiagnosticsStrip.svelte                  (.diag-strip — 26px)
│   └── 6 tiles: INF | FPS | TRK | WS | ACT | EKF
│       (each with status dot, label, value, hover tooltip with sparkline)
│
└── shell-body (flex row)
    ├── LeftRail.svelte                      (.left-rail — 56px)
    │   ├── destinations (4 buttons: Operate/Inspect/Calibrate/Engage)
    │   ├── rail-separator
    │   └── drawers (2 buttons: Models / Settings)
    │
    ├── content-viewport (flex 1)
    │   └── ONE OF:
    │       ├── OperateCanvas + overlays + RightRail
    │       ├── InspectDestination
    │       ├── CalibrateDestination
    │       └── EngageDestination
    │
    └── RightRail.svelte                     (.right-rail — 280px)
        (content varies per destination)

Overlays (mounted at shell root):
├── ModelsDrawer.svelte                      (slides from left)
├── SettingsDrawer.svelte                    (slides from left)
├── CommandPalette.svelte                    (Ctrl+K center modal)
└── Cheatsheet                               (? center modal)
```

### 2.2 Operate Destination Tree

`src/lib/components/destinations/OperateCanvas.svelte` is the centerpiece.

```
OperateCanvas (.operate-canvas-root  ← display: grid; place-items: center)
├── .canvas-stage  ← aspect-ratio: VIEW_W / VIEW_H; max-width/height: 100%; overflow: hidden
│   ├── Layer 0a: fallback grid (.grid-bg.layer-0-fallback, z-index 0)
│   ├── Layer 0b: live MJPEG <img class="layer-0-video" object-fit: fill, z-index 1>
│   ├── Layer 0c: awaiting/connecting chip (bottom-center pill)
│   └── SVG composite (.layer-svg, viewBox="0 0 VIEW_W VIEW_H", z-index 2)
│       ├── Layer 1: trajectories (per-class colored polylines, 16-sample ring)
│       ├── Layer 1.5: actuator path history (raw polyline + smoothed Bezier)
│       ├── Layer 2: detections (bbox + label pill + track ID badge + FOV ring + ellipses)
│       ├── Layer 3: hierarchy edges (parent_id graph, dashed grey lines)
│       └── Layer 4: anchor crosshairs + velocity arrows + selection rings + lock crosshairs
│   ⚠ Every stroked overlay primitive carries vector-effect="non-scaling-stroke"
│     so stroke width stays in CSS pixels regardless of source resolution.
│   ⚠ overflow: hidden hard-clips overlay geometry to the visible video rect —
│     FOV ring, bbox strokes, anchor crosshairs CANNOT spill into gutters.
│
├── canvas-status-bar (top-left, semi-transparent pill — OUTSIDE the stage)
│   └── SRC | SEQ | ACT | NIS | BND | WS
│
├── click-to-lock-banner (when active — OUTSIDE the stage)
└── replay-mode-banner (when REPLAY profile active — OUTSIDE the stage)

Mounted siblings within Operate destination:
├── LayerVisibilityToolbar.svelte           (floating bottom-left, 6 toggles)
├── ModelCard.svelte                         (floating top-right, 260x80px)
├── ObjectCountBadge.svelte                  (floating bottom-right, pill)
└── bottom-time-cursor placeholder           (30px disabled bar)

RightRail (during Operate):
├── Section 1: DETECTIONS (40% height)
│   ├── header: title + count + Copy JSON button
│   └── scrollable detection-card list (one per active track)
├── Section 2: THRESHOLDS & DISPLAY (variable)
│   ├── Confidence slider (0-100%)
│   ├── Overlap (IoU) slider (0-100%)
│   ├── Label Display dropdown
│   ├── BBox Style segmented (Solid/Dashed/Corners/Outline)
│   ├── Class Visibility (6 checkboxes with swatches + live counts)
│   └── Display Toggles grid (ID/Trail/Anchor/Vel/Ellipse/Hier/FOV/Trace)
└── Section 3: SELECTED TRACK (30% height)
    ├── id row + class
    ├── stat rows (conf, bbox, pos, vel, acc, uncert, parent)
    ├── flag pills
    ├── Targeting Anchor 6-option segmented (T/TC/C/CL/BC/CS)
    └── actions (Lock/Export/Mark)
```

### 2.3 Inspect Destination Tree

`src/lib/components/destinations/InspectDestination.svelte`

```
InspectDestination (.inspect-dest)
├── inspect-main (flex row)
│   ├── HierarchyExplorerDrawer.svelte      (240px left)
│   │   ├── header: TRACK HIERARCHY
│   │   ├── filter input + filter dropdown (All/Confirmed/Tentative/Violations)
│   │   ├── tree of TreeNodes (recursive, parent_id graph)
│   │   └── footer: counts (total / confirmed / violations)
│   │
│   └── inspect-content (flex 1, scroll)
│       ├── header: "Inspect" + Show/Hide Event Log button
│       ├── featured-panel: MouseDynamicsInspector (full width, FEATURED tag)
│       ├── derived-strip (4-col grid):
│       │   ├── VelocityProfilePanel (P1 · F01)
│       │   ├── FittsResidualPanel (P3 · F21)
│       │   ├── SubPixelPanel (P7 · F16)
│       │   └── JerkSpectrumPanel placeholder (P2 · F03/F09)
│       └── companion-grid (2x2):
│           ├── SpectatorTelemetryMatrix (ss13.5)
│           ├── PipelineGraphPanel
│           ├── ModelInspectorPanel
│           └── TopicGraphPanel
│
└── EventLogDock.svelte                      (collapsed 30px / expanded 240px)
    ├── collapsed: last-event bar
    └── expanded: tabs (Console/Network/Performance) + filter + scrolling rows
```

### 2.4 Engage Destination Tree

`src/lib/components/destinations/EngageDestination.svelte`

```
EngageDestination (.engage-destination, grid-rows: auto 1fr auto)
├── interlock-bar (top, ~96px)
│   ├── interlock-lights (4 lights horizontal)
│   │   ├── Camera (reactive to telemetrySocket.state)
│   │   ├── Tracker (reactive to trackPool.activeCount)
│   │   ├── Auth (placeholder amber — Phase 6+)
│   │   └── Armed (two-stage toggle — see ss5 Safety Logic)
│   └── interlock-summary text (red until all 4 green)
│
├── engage-canvas-wrap (1fr, center)
│   ├── OperateCanvas (SHARED component, same 6 layers)
│   ├── LayerVisibilityToolbar
│   ├── ModelCard
│   ├── ObjectCountBadge
│   └── slew-indicator (top-center, 80x80px SVG, only when all 4 green)
│
└── hardware-bar (bottom, 48px)
    ├── hw-left
    │   ├── hw-status-pill (COM3 @ 921600)
    │   ├── PORT dropdown
    │   └── BAUD dropdown
    └── EMERGENCY STOP button (red, right)

Modal (when target lock pending):
└── TargetLockModal.svelte                  (Skydio Pause-slider 800ms hold)

RightRail (during Engage):
└── Same 3 sections as Operate (DETECTIONS / THRESHOLDS / SELECTED TRACK)
    ⚠ Future Phase 6+: replace SELECTED TRACK with ACTIVE LOCKS list
```

### 2.5 Calibrate Destination Tree

`src/lib/components/destinations/CalibrateDestination.svelte`

```
CalibrateDestination (.calibrate-dest, flex row)
├── wizard-rail (220px left)
│   ├── wizard-header: CALIBRATE
│   └── 6 step buttons (clickable):
│       1. Camera Intrinsics                — red dot until loaded
│       2. Camera Extrinsics                — red dot until loaded
│       3. EKF Tuning                       — green dot (defaults available)
│       4. Model Selection                  — green dot if model selected
│       4.5. Sub-Pixel Verify (R11)         — red until F16 >= 0.40
│       5. Save as Scenario                 — green if any saved
│
└── wizard-content (flex 1, scroll, max-w 720px)
    └── ONE OF (per activeStep):
        ├── Step 1: 8 read-only intrinsic fields + Load Synthetic button
        ├── Step 2: 3x4 R|t matrix + Load Pose button
        ├── Step 3: 6 sliders + 2 number inputs + preset dropdown + reset
        │   - Alpha, NIS, FOV, Deadzone, VelExp, AccExp, MaxDx, MaxDy
        ├── Step 4: model radio list + Apply Model
        ├── Step 4.5: live F16 readout + gauge + 5s sweep button + status
        └── Step 5: name + description + include checkboxes + save + saved list
```

---

## 3. Wire Format

The dashboard consumes binary WebSocket frames from `ws://127.0.0.1:8765`. The decoder dispatches on byte 0 (`version`).

### 3.1 Version 1 — Track Frame (live)

Header 12 bytes + N × 86 bytes per track.

**Header (12 B):**

| Offset | Size | Field | Type | Notes |
|---|---|---|---|---|
| 0 | 1 | `version` | u8 | = 1 |
| 1 | 1 | `flags` | u8 | bit 0 = NIS-degraded · bit 1 = boundary-present |
| 2 | 4 | `frame_id` | u32 LE | Monotonic frame counter |
| 6 | 2 | `num_tracks` | u16 LE | Loop bound |
| 8 | 4 | `t_capture_ms_offset` | u32 LE | Unused on dashboard side currently |

**Per track (86 B):**

| Offset (from track start) | Size | Field | Type | Notes |
|---|---|---|---|---|
| 0 | 4 | `track_id_lo` | u32 LE | Low half of u64 ID |
| 4 | 4 | `track_id_hi` | u32 LE | High half of u64 ID |
| 8 | 4 | `parent_id_lo` | u32 LE | 0 if no parent |
| 12 | 4 | `parent_id_hi` | u32 LE | 0 if no parent |
| 16 | 1 | `label` | u8 | 0..5 per Label enum (see ss1.2) |
| 17 | 1 | `status_flags` | u8 | See bit table below |
| 18 | 4 | `confidence` | f32 LE | 0.0..1.0 |
| 22 | 12 | `p_world[3]` | f32 LE ×3 | x, y, z position (meters) |
| 34 | 12 | `v_world[3]` | f32 LE ×3 | velocity (m/s) |
| 46 | 12 | `a_world[3]` | f32 LE ×3 | acceleration (m/s²) |
| 58 | 12 | `P_pos_diag[3]` | f32 LE ×3 | Position covariance diagonal (for uncertainty ellipse) |
| 70 | 4 | `bbox_u` | f32 LE | Centroid X in capture-frame px (e.g. 1920px wide) |
| 74 | 4 | `bbox_v` | f32 LE | Centroid Y in capture-frame px |
| 78 | 4 | `bbox_w` | f32 LE | Width px |
| 82 | 4 | `bbox_h` | f32 LE | Height px |

**Status flag bits (`status_flags` byte):**

| Bit | Value | Name | Visual treatment |
|---|---|---|---|
| 0 | `0x01` | CONFIRMED | Normal stroke |
| 1 | `0x02` | BOUNDARY | Amber dashed stroke `8 4` |
| 2 | `0x04` | STATIONARY | Subtle 6% fill tint |
| 3 | `0x08` | HIERARCHY_VIOLATION | Red stroke + 3-frame flash + red row in hierarchy tree |

### 3.2 Version 2 — Actuator Telemetry (500 Hz, optional)

36 bytes per frame. Consumed by `actuatorPool` ring (4096 samples) and rendered as Layer 1.5 actuator trace + MouseDynamicsInspector clean/dirty paths.

| Offset | Size | Field | Notes |
|---|---|---|---|
| 0 | 1 | `version` | = 2 |
| 1 | 1 | `flags` | bit 0=smoothed, bit 1=raw, bit 2=click_down, bit 3=click_up, bit 4=config_change_marker |
| 2 | 4 | `frame_id` | Monotonic, shared with v1 |
| 6 | 8 | `ts_ns` | Capture timestamp ns |
| 14 | 4 | `raw_u_px` | NaN if bit 1 clear |
| 18 | 4 | `raw_v_px` | |
| 22 | 4 | `clean_u_px` | NaN if bit 0 clear |
| 26 | 4 | `clean_v_px` | |
| 30 | 2 | `sink_kind` | u16 mirroring OutputSinkKind enum |
| 32 | 4 | `applied_alpha` | f32 for slider feedback marker |

### 3.3 Version 3 — Background Optical-Flow Odometry (30 Hz)

32 bytes per frame. Consumed by `bgOdometryPool` ring (1024 samples), rendered by SpectatorTelemetryMatrix.

| Offset | Size | Field | Notes |
|---|---|---|---|
| 0 | 1 | `version` | = 3 |
| 1 | 1 | `flags` | bit 0=valid · bit 1=degraded_low_features · bit 2=center_mask_active · bit 3=scale_valid |
| 2 | 2 | `feature_count` | u16 |
| 4 | 2 | `inlier_count` | u16 |
| 6 | 4 | `frame_id` | u32 |
| 10 | 8 | `ts_ns` | Capture timestamp |
| 18 | 4 | `bg_dx_px` | Background translation X |
| 22 | 4 | `bg_dy_px` | Background translation Y |
| 26 | 4 | `bg_theta_rad` | Background rotation |
| 30 | 2 | `confidence_q15` | u16 q15 fixed-point [0..1] |

### 3.4 Reverse Channel — Operator Command (22 B)

Dashboard → C++. Same WebSocket, opposite direction. Used by sliders + anchor selector + future lock controls.

| Offset | Size | Field | Notes |
|---|---|---|---|
| 0 | 1 | `kind` | OperatorCommandKind enum |
| 1 | 1 | `reserved` | 0 |
| 2 | 8 | `target_track_id` | u64 (split lo/hi as u32 ×2) |
| 10 | 4 | `value_or_w_proximity` | f32 — repurposed per command kind |
| 14 | 4 | `w_confidence` | f32 |
| 18 | 4 | `w_stability` | f32 |

**OperatorCommandKind enum:**

| Value | Kind | Wire payload |
|---|---|---|
| 0 | None | |
| 1 | LockTarget | target_track_id |
| 2 | ReleaseLock | |
| 3 | UpdateSelectorWeights | weight_proximity, weight_confidence, weight_stability |
| 4 | SetSmoothingAlpha | f32 alpha (0.05..1.00) |
| 5 | SetDeadbandRadiusPx | f32 radius px |
| 6 | SetFovRadiusPx | f32 radius px |
| 7 | SetTargetingAnchor | f32 ordinal (0..5) |

---

## 4. State Stores

All in `src/lib/stores/` as Svelte 5 runed singletons (`.svelte.ts` modules).

### 4.1 Hot-path stores (typed-array backing, zero per-frame allocation)

| Store | Capacity | Reactive surface | Consumers |
|-------|----------|------------------|-----------|
| `trackPool` | 64 slots, 16-sample trajectory ring per slot | `frameSeq`, `frameFlags`, `activeCount`, `presenceVersion` $state | OperateCanvas, RightRail JSON feed, HierarchyExplorerDrawer, all panels |
| `actuatorPool` | 4096 sample ring (global) | `fill`, `head`, `presenceVersion` | Layer 1.5 actuator trace, MouseDynamicsInspector, derived feature panels |
| `bgOdometryPool` | 1024 sample ring | (read directly) | SpectatorTelemetryMatrix |

### 4.2 Configuration / UI-state stores

| Store | Purpose | Persistence |
|-------|---------|-------------|
| `renderSettings` | Confidence/IoU thresholds, label mode, bbox style, class visibility, all display toggles | localStorage `roc.render_settings.v1` |
| `selectionStore` | `selectedSlot`, `hoveredSlot`, `locks[]`, `clickToLockMode` | None (resets on reload) |
| `profileStore` | Active profile (Default / Prod-NVIDIA / Prod-AMD-Intel / Replay) | localStorage `roc.profile.activeId` |
| `scenarioStore` | Active scenario (6 scenarios) | localStorage `roc.scenario.activeId` |
| `overlayMode` | Alt+Z overlay mode enabled | localStorage `roc.overlay_mode.enabled` |
| `engageState` | Armed state machine, lock confirm modal, emergency stop latch | None (resets on mount per SAFETY) |
| `eventLog` | Append-only event ring (512 cap, preserves pinned) | None |
| `aimingConfig` | Smoothing alpha, deadband scaling slider state | None |
| `hardwareConfig` | Serial port, baud rate, crop region | None |

### 4.3 Telemetry transport

| Module | Purpose |
|--------|---------|
| `telemetrySocket` | WebSocket client to `ws://127.0.0.1:8765`, exponential backoff reconnect, sends OperatorCommands |
| `wire_decoder` | Dispatches on version byte, routes to appropriate pool |
| `mock_frame_source` | Synthetic v1/v2/v3 frame generator (active when WS disconnected) |

---

## 5. Safety Interlock Logic (Engage Destination ARM SYSTEM)

Per `dashboard_menu_architecture_2026.md` ss1 (Hardware Actuation safety) — this is the critical UX pattern for everything actuator-related.

### 5.1 The 4 Lights

All four must be GREEN for any actuator command to be enabled. Until then, every hardware control is hard-disabled (not just greyed — `pointer-events: none`).

| Light | Source binding | Green when | Otherwise |
|-------|----------------|-------------|-----------|
| **Camera** | `telemetrySocket.state` | `=== 'connected'` | grey (stale) "DISCONNECTED" |
| **Tracker** | `trackPool.activeCount` | `> 0` | grey (stale) "NO TRACKS" |
| **Auth** | `profileStore.active.authenticated` (Phase 6+) | `true` | amber (warn) "NOT CONFIGURED" |
| **Armed** | `engageState.armed` | `=== 'armed'` | red (err) "ARM SYSTEM" button OR amber (warn) "CONFIRM ARM?" pending OR locked red "DISABLED · REPLAY" |

### 5.2 Armed State Machine

State machine: `off → pending → armed`. Three explicit user gestures.

```
[off]
   │  click ARM SYSTEM
   ▼
[pending]  ─ shows "CONFIRM ARM?" + [Confirm] [Cancel] inline buttons
   │
   ├─ click Confirm → [armed]
   │                     │
   │                     ├─ click again → [off]
   │                     ├─ 30s timer → [off] (auto-disarm)
   │                     └─ Emergency Stop → [off] + emergencyStopLatched=true
   │
   └─ click Cancel → [off]

[off]  ⟵ on every mount, every layout change, every page reload (MANDATORY)

[REPLAY mode]
   └─ profileStore.isReplay === true → armed forced to off, toggle hard-disabled
```

**File:** `src/lib/stores/engage_state.svelte.ts` lines 33-72 implement this.

### 5.3 Target Lock Modal (Skydio Pause-Slider, 800ms hold)

After Armed, the user can press `Alt+L` on a selected track to open the lock confirmation modal:

- Modal blocks UI, backdrop click cancels
- Big red [HOLD TO CONFIRM] button
- User must press AND HOLD for `HOLD_DURATION_MS = 800ms` to commit
- Release before 100% = cancel (no commit)
- Esc cancels at any time (the spec is explicit: "modal Esc never sends actuator")
- Progress bar fills inside button during hold
- On 100%: lock added to `selectionStore.locks`, modal closes

**File:** `src/lib/components/overlays/TargetLockModal.svelte`

### 5.4 Emergency Stop

- One-click, no confirmation (safety-side action)
- Sets `emergencyStopLatched = true`
- Immediately disarms (`armed = 'off'`)
- Calls `telemetrySocket.releaseLock()` to push command downstream
- Closes any pending lock modal
- "ARM SYSTEM" becomes greyed with "E-STOP LATCHED" text
- Must explicitly click "Reset" to unlatch

### 5.5 REPLAY Mode

When `profileStore.activeProfileId === 'replay'` (or future time-cursor scrubs past NOW):
- Top chrome LIVE badge flips to REPLAY (purple, pulsing)
- `engageState.canArm` returns false
- ARM toggle hard-disabled, shows "DISABLED · REPLAY"
- Lock crosshair on canvas tinted differently (Phase 6+)
- All actuator emit paths skipped at the C++ side (defense in depth)

---

## 6. Hotkey Map

Full table is in `src/lib/keybindings/defaults.ts`. Press `?` in app to see the live cheatsheet.

| Combo | Action | Category | Notes |
|-------|--------|----------|-------|
| `1` | Switch to Operate | destination | bare key, blocked when input focused |
| `2` | Switch to Inspect | destination | same |
| `3` | Switch to Calibrate | destination | same |
| `4` | Switch to Engage | destination | same |
| `v` | Toggle video layer (Layer 0) | layer | |
| `o` | Toggle overlay layers (1-3) | layer | |
| `Tab` | Cycle next track in selection | selection | |
| `Shift+Tab` | Cycle previous track | selection | |
| `g` | Enter click-to-lock cursor mode | selection | |
| `]` | Toggle right rail | global | |
| `` ` `` | Toggle EventLogDock (Inspect only) | global | bare key |
| `Ctrl+K` | Command Palette | global | modifier-required |
| `Alt+Z` | Toggle Overlay Mode | global | |
| `Alt+L` | Toggle target lock on selected | global | Engage + Armed only |
| `Alt+R` | Toggle perf HUD expansion | global | |
| `?` | Cheatsheet | global | bare key |
| `Escape` | Dismiss / Cancel / Deselect | global | priority: palette → cheatsheet → drawer → overlay → click-to-lock → selection |
| `r` | Start Recording | panel | MouseDynamicsInspector focused |
| `x` | Stop Recording | panel | |
| `p` | Pause / Resume Recording | panel | |
| `c` | Clean trace mode | panel | |
| `d` | Dirty trace mode | panel | |
| `t` | Both traces overlap | panel | |
| `s` | Side-by-side traces | panel | |

**Input guard:** Bare-key bindings (`1-4`, `v`, `o`, `r`, etc.) check `document.activeElement.tagName` and skip if `INPUT` / `TEXTAREA` / `SELECT` is focused. Modifier-prefixed bindings always fire.

**Modal gate:** When `keybindingManager.isModalOpen === true`, only `global:dismiss` (Esc) fires.

---

## 7. Interaction Patterns (existing)

### 7.1 Canvas → Right Rail Selection Sync

1. User clicks a bbox on canvas → `selectionStore.select(slot)`
2. Bbox gets orange outline (Layer 4 selection ring)
3. RightRail Section 3 (Selected Track Inspector) fills with that track's data
4. Detection card in Section 1 (JSON feed) gets orange left border
5. Targeting anchor selector becomes interactive

### 7.2 Right Rail → Canvas Hover

1. User hovers a detection card in Section 1 JSON feed
2. `selectionStore.setHover(slot)` fires
3. Layer 4 hover ring renders around that bbox on canvas (1.5px orange)
4. On mouse leave: ring disappears, selection (if any) remains

### 7.3 Slider → C++ Round-Trip

1. User drags Smoothing Coefficient slider in Calibrate Step 3
2. `onchange` fires (not `oninput` — debounced to slider release)
3. `telemetrySocket.setSmoothingAlpha(value)` writes 22-byte OperatorCommand frame
4. C++ orchestrator's tracker thread reads new alpha from `std::atomic<TrackerRuntimeConfig>`
5. Next v2 actuator-telemetry frame includes new `applied_alpha`
6. MouseDynamicsInspector shows config-change marker at that timestamp

### 7.4 Recording Flow (MouseDynamicsInspector)

1. User clicks `[R] Record` button in MouseDynamicsInspector header
2. **PreRecordModal** opens — mandatory label selection (Clean / Dirty / Ambiguous / Unsupervised)
3. User must explicitly choose (no Esc dismiss, no click-outside)
4. On choice: JsonlWriter starts capturing samples + features
5. Buffer fill bar updates, sample counter ticks at 10 Hz
6. Label badge becomes visible (color-coded: green/red/amber/grey)
7. User clicks `[X] Stop`
8. **Export modal** opens with checkboxes (JSONL always on, optional PNG / MP4 / features-only)
9. On confirm: download triggers via `<a download>` blob URL

---

## 8. Camera/Hardware Source Architecture (Planning Discussion)

**Current state:** Source-related controls are spread across destinations:
- **Calibrate Step 1**: Camera Intrinsics (calibration params)
- **Calibrate Step 4**: Model Selection
- **Engage Hardware Bar**: Serial Port + Baud Rate + Emergency Stop
- **Models Drawer**: ONNX manifest browser

**User's question:** Should source selection (laptop camera vs capture card vs file vs WebRTC) live in its own dedicated place?

**Recommended addition (not yet built):** A **Models drawer expansion** OR a **5th drawer slot** below Models/Settings titled "Sources" (or a "Source" tab within the Models drawer).

The Sources panel would unify:
- Camera device selector (dropdown enumerating MSMF / DirectShow / V4L2 devices via `roc_vision.exe --enumerate-cameras` returning JSON)
- Capture card detection (capture cards show up as additional camera indices)
- File-based source ("Open MP4...") for Forensic Review scenario
- Future: WebRTC URL input, RTSP URL input
- Resolution + FPS selector per device
- Live preview thumbnail of current source
- "Apply Source" button → sends `kSetCameraSource` OperatorCommand (would need new wire format kind 8)

**Why a drawer, not a destination:**
- The spec ss1 binding rule: cardinality of 4 destinations is FIXED. Adding a 5th destination requires explicit amendment.
- Sources are cross-cutting (used in Operate AND Engage AND Calibrate), not a workflow itself.
- Drawer pattern matches Models / Settings (Foxglove sidebar precedent).
- Source change should be a one-shot action that closes the drawer and re-renders the current destination.

**Where the hardware actuation config lives:**
Stays in Engage (the destination that USES it). Per spec ss5.3 Engage owns "Hardware Configuration" because actuator commands only fire from Engage. Moving it would split the safety-interlock context from the controls it gates.

**Where MJPEG video feed enters the picture:**
The MJPEG/WebRTC `<video>` source URL is a Sources-drawer concern. When the user picks "Laptop Camera 0" → Sources drawer sets:
- `mjpegUrl = 'http://127.0.0.1:8765/mjpeg'` (assumes orchestrator serves it)
- OperateCanvas Layer 0 `<video>` element's `src` updates
- Layer 0 video starts streaming behind the SVG overlay

---

## 9. Known UI Polish Opportunities (Claude — please scrutinize and add)

These are areas where someone with fresh eyes can immediately add value. Cite them by the design tokens above so the fix maps to code.

1. **Diagnostics Strip alignment** — tiles use loose `gap: 2px`; tile contents internally have `gap: 5px`. Values are not column-aligned across tiles, which can feel jittery as numbers change.

2. **BBox label legibility on cyan backgrounds** — `Person 92.0%` label pill uses `--text-cream` body text on a `#0a1014` dark text color picked from `CLASS_LABEL_TEXT[1]`. The contrast is intentional (high) but the orange `#f5a623` confidence percentage on the same dark text loses some weight at small sizes (11px mono).

3. **Empty-state copy in Selected Track panel** — currently "Click a detection on the canvas or in the feed above" (compact, line-wrapped). Could be more inviting / actionable.

4. **Right rail Detection card padding density** — currently 8px vertical / 10px horizontal, 4 meta rows. Feels readable at 1080p but might be too cramped on smaller viewports.

5. **Featured panel in Inspect has FEATURED · ss12.1 tag** — fine for developer audience, looks like an internal reference for a user-facing dashboard. Decide on the audience.

6. **Calibrate wizard step numbers** — Step 4.5 uses `"4.5"` as the displayed number; visually inconsistent with the others (single digits). Could be styled differently (smaller, prefixed dot) or renumbered (3.5, 4, 5 = 5, 5.5, 6).

7. **Engage interlock "All 4 lights required for actuation"** message — red italic when off, green non-italic when on. The font-style switch is subtle; might be clearer with iconography (× / ✓ prefix).

8. **Right rail collapse tab** — currently 14px wide with a tiny chevron. Visually disappears against the dark canvas. Could be more discoverable.

9. **Hover tooltips on Diagnostics Strip tiles** — appear as floating cards with sparkline. Good information density but positioning is fixed below the tile — clips on the leftmost tile (INF) at viewport edge.

10. **Model Card "Switch model..." button** — uses orange outline style consistent with the accent. But the click action opens the Models drawer which slides from the left edge; the visual flow from top-right (card) to left edge (drawer) is jarring. Could highlight the Models button in the left rail during the transition.

---

## 10. Files Reference

If Claude wants to read source for any component, here are the paths:

```
src/lib/components/
├── NavigationShell.svelte               ← shell root
├── shell/
│   ├── TopChrome.svelte                 ← masthead
│   ├── DiagnosticsStrip.svelte          ← 6-tile strip
│   ├── LeftRail.svelte                  ← destination + drawer rail
│   ├── RightRail.svelte                 ← Operate/Engage right column
│   └── CommandPalette.svelte            ← Ctrl+K palette
├── destinations/
│   ├── OperateCanvas.svelte             ← the 6-layer composite
│   ├── InspectDestination.svelte        ← hier drawer + panels + event log
│   ├── CalibrateDestination.svelte      ← 5+1 step wizard
│   └── EngageDestination.svelte         ← safety interlock + canvas + hardware bar
├── overlays/
│   ├── LayerVisibilityToolbar.svelte    ← 6 layer toggles
│   ├── ModelCard.svelte                 ← top-right floating model stats
│   ├── ObjectCountBadge.svelte          ← bottom-right object count
│   └── TargetLockModal.svelte           ← Skydio 800ms hold
├── drawers/
│   ├── ModelsDrawer.svelte              ← ONNX model browser
│   └── SettingsDrawer.svelte            ← profile + hotkeys + prefs
├── panels/
│   ├── MouseDynamicsInspector.svelte    ← featured in Inspect
│   ├── SpectatorTelemetryMatrix.svelte  ← bg-odom cross-corr
│   ├── VelocityProfilePanel.svelte      ← P1
│   ├── FittsResidualPanel.svelte        ← P3
│   ├── SubPixelPanel.svelte             ← P7
│   ├── PreRecordModal.svelte            ← mandatory label selection
│   └── inspect/
│       ├── HierarchyExplorerDrawer.svelte  ← Palantir-style tree
│       ├── PipelineGraphPanel.svelte       ← Camera → Inf → Tracker → Sink
│       ├── ModelInspectorPanel.svelte      ← per-layer ORT profile
│       ├── TopicGraphPanel.svelte          ← WS pub-sub map
│       └── EventLogDock.svelte             ← bottom collapsible dock
└── domains/
    └── KinematicTrajectoryControllerPanel.svelte  ← legacy, embedded in Calibrate step 3

src/lib/stores/
├── track_pool.svelte.ts                 ← 64-slot SoA
├── actuator_pool.svelte.ts              ← 4096-sample ring
├── bg_odometry_pool.svelte.ts           ← 1024-sample ring
├── render_settings.svelte.ts            ← all threshold/display state
├── selection_store.svelte.ts            ← selected/hovered/locks
├── profile_store.svelte.ts              ← 4 runtime profiles
├── engage_state.svelte.ts               ← arm state machine + lock modal
├── overlay_mode.svelte.ts               ← Alt+Z toggle
├── event_log.svelte.ts                  ← 512-event ring
├── aiming_config.svelte.ts              ← Calibrate Step 3 state
└── hardware_config.svelte.ts            ← serial + crop

src/lib/render/
└── class_colors.ts                      ← per-class palette + sink palette + status flags

src/lib/profiles/
├── scenario_store.svelte.ts             ← 6 scenarios + localStorage
├── scenarios.ts                         ← 6 scenario defs
├── schema.ts                            ← RocProfile + DEFAULT_PROFILE
└── panel_registry.ts                    ← panel ID → component map

src/lib/telemetry/
├── telemetry_socket.svelte.ts           ← WS client + OperatorCommand encoder
├── wire_decoder.ts                      ← v1/v2/v3 dispatcher
└── mock_frame_source.ts                 ← synthetic data generator

src/lib/keybindings/
├── defaults.ts                          ← binding table
└── manager.svelte.ts                    ← central dispatcher with input guard

src/lib/domains/
└── controller_matrix.ts                 ← 4 destination defs (Operate/Inspect/Calibrate/Engage)
```

---

## 11. Binding References

For deeper context Claude can pull from when needed:

- `workspace_blueprint/research_notes/dashboard_menu_architecture_2026.md` — full binding spec (1354 lines, v2+v3+v4+v5)
- `workspace_blueprint/research_notes/anticheat_data_exposure_2026.md` — biometric panel definitions (P1-P8, F01-F25)
- `workspace_blueprint/research_notes/dashboard_pro_console_ux_2026.md` — pro-console UX patterns (Foxglove, RViz, Datadog, Anduril)
- `workspace_blueprint/research_notes/dashboard_game_cv_overlay_ux_2026.md` — overlay UX (NVIDIA, AMD, OBS, RTSS)
- `workspace_blueprint/research_notes/phase_5_console_robotics_ml.md` — robotics ML pipeline introspection
- `workspace_blueprint/research_notes/phase_2_ekf_kinematics.md` — kinematics math, wire format origin
- `workspace_blueprint/research_notes/phase_1_universal_ep_and_manifest.md` — manifest schema for Models drawer
- `core/main.cpp` — C++ orchestrator (camera capture, inference, tracker, ws egress threads)
- `core/kinematics_engine/include/roc/kinematics/types.hpp` — Label enum (authoritative)
- `core/transport/include/roc/transport/wire_format.hpp` — wire layout (authoritative)
- `core/transport/include/roc/transport/operator_command.hpp` — reverse channel (authoritative)
- `.cursor/rules/200-dashboard-telemetry.mdc` — cardinality-of-4 lock + safety mandates

---

---

## 12. Update Log — Polish Pass 1 + Sources Drawer (2026-05-17 late)

After Claude Code's UI review (the doc-aligned polish round + Sources drawer architecture validation), the following landed:

### Polish items applied (Claude review)

| # | Item | File · change |
|---|------|---------------|
| 1 | Diagnostics strip tiles as discrete bordered chips | `shell/DiagnosticsStrip.svelte` — `.diag-tile` now uses `grid-template-columns: 8px 28px 1fr` with `1px rgba(255,255,255,0.06)` border + 8px horizontal padding · gap 5px between tiles |
| 3 | AWAITING VIDEO FEED bottom-center chip | `destinations/OperateCanvas.svelte` — replaced centered watermark with `.awaiting-chip` floated at `bottom: 56px`, pulsing orange dot, dark backdrop pill |
| 4 | Detection card metadata 2-column micro-grid | `shell/RightRail.svelte` — `.dc-meta-grid` uses `dl/dt/dd` with `grid-template-columns: 32px 1fr` so id/flags/bbox/vel column-align across cards |
| 5 | Selected Track empty state with target icon + orange word | `shell/RightRail.svelte` — `.empty-target` block with 48x48 dashed SVG crosshair + "Click a `<span class='accent-word'>detection</span>`" |
| 6 | ARM SYSTEM disabled grey, orange only when readyToArm | `destinations/EngageDestination.svelte` — added `readyToArm` derived state · `.arm-btn` is grey until 3 of 4 lights green, then `.arm-ready` flips to orange |
| Big1 | BBox dark halo for MJPEG legibility | `destinations/OperateCanvas.svelte` — added `bboxHaloRefs[]` array · renders dark 4px stroke at 45% opacity under the 2px colored bbox · same x/y/w/h, set in same RAF tick |
| Obs2 | Targeting Anchor T/TC/C/CL/BC/CS tooltips + hint | `shell/RightRail.svelte` — each `<button>` has `title` with full description · persistent `.anchor-hover-hint` text shows active anchor's meaning below the segmented control |

### New: Sources Drawer (per spec ss8)

**Files added:**
- `src/lib/stores/source_store.svelte.ts` (~165 LOC) — `SourceStore` runed singleton
- `src/lib/components/drawers/SourcesDrawer.svelte` (~430 LOC)

**Wiring:**
- `shell/LeftRail.svelte` — added 3rd drawer button "Sources" (above Models · video-camera icon), drawer type signature widened to `'models' | 'sources' | 'settings'`
- `NavigationShell.svelte` — `activeDrawer` widened, `sourceStore.hydrate()` added, `<SourcesDrawer>` mounted at root
- `destinations/OperateCanvas.svelte` — Layer 0 now renders `<img class="layer-0-video" src={sourceStore.mjpegUrl} />` when `mjpegUrl` is set, falls back to grid placeholder otherwise. Canvas status bar SRC reads from `sourceStore.sourceLabel`

**Drawer contents:**

| Section | Controls |
|---------|----------|
| Header | Title "Sources" · refresh icon (calls `/sources` HTTP endpoint) · close X |
| AVAILABLE DEVICES | 5 starter device cards (Mock · Webcam 0 · Webcam 1 · Capture Card 2 · File Replay). Each card: type icon + label + kind badge + meta (idx/drv/res/status). Click selects (orange left border). Cards show `offline` until /sources confirms `online` |
| RESOLUTION + FPS | 6 preset buttons (640x480@30, 1280x720@30, 1280x720@60, 1920x1080@30, 1920x1080@60, 3840x2160@30) + custom W/H/FPS spinbuttons |
| MJPEG URL OVERRIDE | Text input · default `http://127.0.0.1:8766/mjpeg/{deviceIndex}` · hint text explains Layer 0 binding |
| Footer | "Apply Source → updates Layer 0" big orange CTA button |

**Persistence:** `sourceStore.activeSourceId` saved to `localStorage` key `roc.source.activeId`.

**HTTP enumeration endpoint (Phase 6+ C++ work):**
- Endpoint: `http://127.0.0.1:8766/sources` (separate port from WS at 8765)
- Response: `{ "devices": [SourceDevice[]] }` JSON
- Drawer fetches with 2-second AbortController timeout
- Falls back silently to built-in device list when unreachable
- `lastEnumerationError` surfaced as orange-bordered note in drawer

**MJPEG endpoint URL convention (Phase 6+ C++ work):**
- `http://127.0.0.1:8766/mjpeg/{cameraIndex}` serves multipart MJPEG stream of `cv::VideoCapture(index).read()` re-encoded as JPEG
- Dashboard `<img>` element consumes natively (no JS decoder needed)
- WebRTC path reserved for Phase 7+ (would use `<video>` + MediaStream)

### New OperatorCommandKind for source switching (Phase 6+ wire format extension)

```ts
SetCameraSource = 8  // payload: { device_index: u32, width: u16, height: u16, fps: u16 }
```

When the user clicks "Apply Source" in the drawer, the dashboard would send this command to the C++ orchestrator so it can re-init `cv::VideoCapture` mid-session without restarting. v1 implementation: dashboard side only updates `sourceStore`; C++ wiring deferred.

### Updated component tree (additions to ss2)

```
NavigationShell.svelte
└── shell-body
    └── LeftRail.svelte
        └── drawers (now 3 buttons)
            ├── Sources (NEW — video-camera icon)
            ├── Models
            └── Settings

OperateCanvas.svelte
└── .layer-0-camera
    └── ONE OF:
        ├── <img class="layer-0-video" src={mjpegUrl}/>  ← NEW when mjpegUrl set
        └── <div class="grid-bg"/> + <div class="awaiting-chip"/>  ← else placeholder
```

### Updated stores (additions to ss4)

| Store | Purpose | Persistence |
|-------|---------|-------------|
| `sourceStore` | Camera/capture/file source devices + MJPEG URL + resolution override | localStorage `roc.source.activeId` |

### Carry-over open items (not yet addressed)

These were on Claude's list but pending more design discussion:
- **#2 BBox label stacking when Person/Head/Torso overlap vertically** — current naive label placement above-or-below bbox top. Two options on the table: (a) auto-stagger horizontally on y-collision (Roboflow-style), or (b) collapse child labels into parent chip "Person 92% · Head 88% · Torso 95%" and only break out when parent selected. Decision pending.
- **#7 Calibrate STEP HELP keyboard hints** — would add `← prev · → next · S save` row at bottom of right rail when Calibrate active. Wire arrow keys to step navigation.
- **#9 Left rail activity badges** — small pulse/dot on inactive destination icons when that destination has new activity (Engage when interlocks become ready, Inspect when new violation, etc.). Needs a generic `notificationStore` per destination.
- **C++ MJPEG endpoint** — `roc_vision.exe` needs new `--mjpeg-port 8766` flag + tiny HTTP server module + per-frame JPEG encode. Estimated 1-2 hours C++.
- **C++ /sources endpoint** — JSON enumeration of available `cv::VideoCapture` indices via OpenCV `cv::CAP_ANY` probe-and-list. Estimated 30 min.
- **v1 wire frame inference latency field** — add 4-byte `last_inference_ms` to header so the INF diag tile becomes authoritative instead of estimated from inter-frame deltas. Estimated 30 min split between C++ and TS.

---

---

## 13. Update Log — Polish Pass 2 + C++ MJPEG Server (2026-05-17 night)

After Claude Code's full review (he's lost from the chat but his suggestions stand), Pass 2 + the C++ MJPEG endpoint landed.

### Polish items applied (Claude's carry-over list — done)

| Claude # | Item | Files · change |
|---|------|----------------|
| 2 | BBox label vertical-stacking collision auto-stagger | `destinations/OperateCanvas.svelte` — added `placedLabels[]` scratch array reset per RAF tick · per-slot AABB collision check vs all previously placed labels · horizontal shift first, vertical stack second when off-canvas. Resolves the Person/Head/Torso pile-up case |
| 7 | Calibrate STEP HELP keyboard hints + ←/→/S nav | `destinations/CalibrateDestination.svelte` — added `nextStep`/`prevStep`/`handleKeydown` · `←`/`→` cycles steps, `S` saves on step 5 · `.wizard-header` shows kbd hints `← →` step · `S` save |
| 9 | Left rail activity badges | `shell/LeftRail.svelte` — added `engageReady` / `inspectHasViolation` / `operateRecentActivity` derived states · `.activity-badge` 7px pulsing dot top-left of inactive icon · level=ready (green), warn (red), info (cyan) · tooltips explain |

### Additional dashboard wiring (this round)

| Item | File · change |
|---|---|
| Source row in Settings Preferences | `drawers/SettingsDrawer.svelte` — new "Active Source" row reading from `sourceStore.sourceLabel` + MJPEG Endpoint readout |
| Sources drawer auto-refresh on first open | `drawers/SourcesDrawer.svelte` — `hasAutoRefreshed` guard calls `sourceStore.enumerate()` once per session when drawer first opens |
| Inspect right rail per-panel settings | `stores/focused_panel.svelte.ts` (NEW · 50 LOC) + `shell/RightRail.svelte` — when activeDomain=inspect, right rail shows panel-specific controls (mode segmented, window size, regression type, smoothing toggle, etc) per focused panel. Click any Inspect card to focus it; orange outline highlights focus |
| `[` hotkey for left rail toggle | `keybindings/defaults.ts` — added `[` binding · `stores/rail_state.svelte.ts` (NEW) consolidates left+right collapse state · `NavigationShell.svelte` renders 14px collapsed tab when leftCollapsed |
| Scenario → Profile auto-switch | `profiles/scenario_store.svelte.ts` — `setScenario()` now also calls `profileStore.setProfile(scenario.defaultProfileId)` so picking Forensic Review flips to Replay profile automatically |
| MJPEG `<img>` graceful onerror | `destinations/OperateCanvas.svelte` — Layer 0 `<img>` has onerror/onload handlers that hide the broken-image icon when the endpoint is unreachable, falls back to the grid placeholder + "MJPEG ENDPOINT OFFLINE" chip |

### NEW: C++ MJPEG-over-HTTP server

**Files added:**
- `core/transport/include/roc/transport/mjpeg_server.hpp` (~95 LOC)
- `core/transport/src/mjpeg_server.cpp` (~420 LOC)

**CMake wiring:**
- `core/transport/CMakeLists.txt` — added `src/mjpeg_server.cpp` to lib sources, header to public list, `find_package(OpenCV ... imgcodecs)` (needs cv::imencode)

**Orchestrator integration (`core/main.cpp`):**
- New CLI flags:
  - `--mjpeg-port PORT` (default 8766)
  - `--no-mjpeg` (disable entirely)
  - `--mjpeg-quality Q` (default 80, range 1-100)
- `OrchestratorOptions` extended with `mjpeg_port`, `enable_mjpeg`, `mjpeg_quality`
- `camera_capture_thread` signature widened with `MjpegServer*` + `camera_index_for_mjpeg` params; calls `push_jpeg_frame()` after each `cap.read()`
- `main()` constructs `roc::transport::MjpegServer` (when enabled), calls `set_sources({CameraSourceInfo{...}})` with the currently-opened camera, starts the server, prints stream URLs
- Graceful shutdown: `mjpeg->stop()` called alongside `server.stop()`

**HTTP endpoints served (all loopback only — `127.0.0.1` peer enforcement):**

| Path | Method | Response |
|------|--------|----------|
| `/health` | GET | `text/plain` "ok" — fast liveness probe |
| `/sources` | GET | `application/json` `{"devices":[CameraSourceInfo[]]}` — drives dashboard's Sources drawer |
| `/mjpeg/{cameraIndex}` | GET | `multipart/x-mixed-replace; boundary=rocframe` — Motion-JPEG stream, one frame per camera capture |

**Architecture:**
- 1 accept thread + 1 thread per active client (small N expected: dashboard's `<img>` element + occasional `/sources` probes)
- Per-camera `FrameSlot` with `std::mutex` + `std::condition_variable` — capture thread `notify_all()` after each `push_jpeg_frame()`; client threads wait_for(250ms) to pick up the latest frame
- Best-effort delivery: slow clients drop frames silently; capture thread is never backpressured
- `cv::imencode(".jpg", frame, ..., {IMWRITE_JPEG_QUALITY, 80})` per frame on capture thread (~5-10ms on 1920x1080)
- CORS `Access-Control-Allow-Origin: *` so dashboard browser can fetch `/sources` without proxy
- Loopback enforcement: `accept()` checks `peer.sin_addr` is in `127.0.0.0/8`, closes non-loopback connections immediately
- Cross-platform sockets (Winsock on Windows, BSD sockets on POSIX) via macros — same code compiles MinGW + g++ + clang

**Verified live:**
- `roc_vision.exe --camera 0 --model X --manifest Y --demo-duration 30` ran 30s on real laptop webcam
- 455 camera frames captured, 289 inference runs (CPU EP), 3845 actuator telemetry frames, 4566 WS broadcasts
- MJPEG server listened on `127.0.0.1:8766` cleanly, `/sources` returned the active webcam, exit code 0

**Dashboard consumption (already wired in Polish Pass 1):**
- `sourceStore.enumerate()` calls `/sources` with 2s AbortController timeout; populates `sourceStore.devices` when reachable
- `sourceStore.mjpegUrl` returns `http://127.0.0.1:8766/mjpeg/{deviceIndex}` for the active source
- `OperateCanvas.svelte` Layer 0 `<img src={mjpegUrl}>` consumes the multipart stream natively (no JS decoder)
- On endpoint offline: `<img onerror>` hides broken icon, fallback grid + "MJPEG ENDPOINT OFFLINE" chip stays visible

### Carry-over items (not done this round)

- **v1 wire frame inference latency field** — touching the wire format requires C++ + dashboard decoder + spec doc update; deferred to avoid breaking compatibility mid-session. The current dashboard INF tile estimates latency from inter-frame deltas which is accurate enough for now
- **C++ camera enumeration** — `/sources` currently only reports the camera the orchestrator opened (1 device). Future: probe `cv::VideoCapture(N)` for N=0..7 with short timeout, return list. Adds ~50 LOC + setup time
- **Sources drawer "Apply Source" → C++ live switch** — currently the drawer updates the dashboard's source_store but doesn't send a command to the C++ side. Adding `kSetCameraSource = 8` to OperatorCommand enum + handler in main.cpp would let the operator switch cameras mid-session without restarting. Requires C++ wire format change

### Updated files reference (additions to ss10)

```
src/lib/stores/
├── focused_panel.svelte.ts            (NEW · 50 LOC · Inspect panel focus tracking)
├── rail_state.svelte.ts               (NEW · 60 LOC · left+right rail collapse state)
└── source_store.svelte.ts             (existing · enumerate() now has 2s timeout)

src/lib/keybindings/defaults.ts        (added `[` for left rail toggle)

core/transport/include/roc/transport/
└── mjpeg_server.hpp                   (NEW · MjpegServer class + CameraSourceInfo)

core/transport/src/
└── mjpeg_server.cpp                   (NEW · 420 LOC · accept loop + per-client handler + MJPEG streaming)

core/main.cpp                          (added --mjpeg-port/--no-mjpeg/--mjpeg-quality flags · MjpegServer construction + lifecycle · camera thread pushes JPEG frames)

core/transport/CMakeLists.txt          (added mjpeg_server.cpp source + imgcodecs OpenCV component)
```

### How to use the live pipeline

**Terminal 1 — orchestrator:**
```powershell
$env:PATH = "C:\msys64\mingw64\bin;C:\msys64\usr\bin;" + [System.Environment]::GetEnvironmentVariable('PATH', 'Machine')
cd d:\roc-ai-vision\core
.\build\roc_vision.exe --camera 0 `
  --model "C:\Users\heave\Desktop\onnx_collection\1.5kR6.onnx" `
  --manifest "d:\roc-ai-vision\models\person-only-yolov8.yaml"
```

**Terminal 2 — dashboard:**
```powershell
cd d:\roc-ai-vision\dashboard
npm run dev
```

Open `http://localhost:5173/`, switch to Operate destination. The canvas Layer 0 will receive live MJPEG from `http://127.0.0.1:8766/mjpeg/0`, and the SVG overlay above it draws detection bboxes from the YOLOv8 inference at ~10-30 Hz depending on EP. The Diagnostics Strip turns green (WS = CONN), and the Sources drawer shows the active camera marked `online`.

---

**Last updated: 2026-05-17 night · After Polish Pass 2 + C++ MJPEG-over-HTTP server landed.**
*If you change a design token, update this doc first, then the code (component CSS variables to follow).*

---

## 14. Stage-div overlay pattern (binding)

**Binding research:** `workspace_blueprint/research_notes/research_svg_overlay_alignment_2026.md`
**Binding rule:** `.cursor/rules/220-operate-canvas-stage.mdc` (always-applied, glob `dashboard/**/*.svelte`)
**First implementation:** `dashboard/src/lib/components/destinations/OperateCanvas.svelte` (2026-05-18)

### 14.1 Why this exists

Any component that overlays SVG (or `<canvas>`) on top of an `<img>` / `<video>` raster MUST use the canonical aspect-ratio stage-div pattern. This is the only zero-JS, layout-thrash-free way to guarantee that:

1. Source-pixel coordinates (e.g. 640×480 detector output) map **1:1** to the rendered pixels of the visible raster content.
2. Overlay geometry that legitimately extends beyond the source rect (FOV halos, range rings, off-screen indicators) is **hard-clipped** to the visible raster — no spillover into the dark gutters.
3. The same DOM works for 4:3, 16:9, 21:9, and arbitrary source aspect ratios with zero code branching.

Failure mode before this pattern landed: an SVG with `viewBox="0 0 640 480"` sitting at `inset: 0; width: 100%; height: 100%` over a `<img object-fit: contain>` would `meet`-fit its viewBox to the same letterboxed rect — BUT the SVG element itself extended past the letterboxed area, so any geometry drawn at user coords outside the viewBox (e.g. an FOV `<circle r="720">` inside a 640×480 viewBox) rendered into the gutters. The stage-div pattern eliminates this by making the SVG element's CSS box exactly source-AR-shaped and clipping with `overflow: hidden`.

### 14.2 Canonical CSS (copy verbatim into any new overlay component)

```css
.panel-root {
  position: relative;
  width: 100%;
  height: 100%;
  min-width: 0;       /* required so flex/grid ancestors don't break shrink */
  min-height: 0;
  background: #06080c;
  overflow: hidden;
  display: grid;
  place-items: center;
}

.canvas-stage {
  position: relative;
  max-width: 100%;
  max-height: 100%;
  overflow: hidden;
  /* aspect-ratio is set inline from source W/H — reactive to active source:
     style="aspect-ratio: {VIEW_W} / {VIEW_H};" */
}

.canvas-stage > img,
.canvas-stage > svg,
.canvas-stage > .grid-bg {
  position: absolute;
  inset: 0;
  width: 100%;
  height: 100%;
  display: block;
}

.canvas-stage > img    { object-fit: fill; }   /* stage already enforces AR */
.canvas-stage > svg    { pointer-events: none; cursor: crosshair; }
```

```svelte
<div class="panel-root">
  <div class="canvas-stage" style="aspect-ratio: {VIEW_W} / {VIEW_H};">
    <div class="grid-bg"></div>
    <img class="layer-0-video" src={mjpegUrl} alt="" />
    <svg
      class="layer-svg"
      viewBox="0 0 {VIEW_W} {VIEW_H}"
      preserveAspectRatio="xMidYMid meet"
    >
      <!-- Every stroked element MUST carry vector-effect="non-scaling-stroke" -->
      <rect x={d.x} y={d.y} width={d.w} height={d.h}
            fill="none" stroke="lime" stroke-width="2"
            vector-effect="non-scaling-stroke" />
    </svg>
  </div>
  <!-- Status pills, banners, OS chrome live OUTSIDE the stage, at panel-root level -->
</div>
```

### 14.3 Hard rules (lint-enforceable in spirit)

| # | Rule | Rationale |
|---|------|-----------|
| 1 | The stage MUST declare `aspect-ratio: {VIEW_W} / {VIEW_H}` inline so it tracks the active source reactively (no CSS-var indirection — Svelte 5 runes write the style string each render). | Capture cards / camera switches in the Sources drawer change `sourceStore.active.{width,height}` mid-session; the stage must follow without a remount. |
| 2 | The stage MUST set `overflow: hidden`. | Primary clip guard. Without this, all the other guards become advisory. |
| 3 | The raster `<img>` / `<video>` MUST use `object-fit: fill`. | The stage already enforces source AR; `contain` would re-letterbox inside the stage and break 1:1 pixel mapping. |
| 4 | The SVG MUST use `viewBox="0 0 VIEW_W VIEW_H"` with `preserveAspectRatio="xMidYMid meet"`. | `meet` is a no-op when the SVG's CSS box is already source-AR-shaped; preserves invariance if some future ancestor breaks the stage's AR enforcement. |
| 5 | Every stroked overlay element MUST carry `vector-effect="non-scaling-stroke"`. | Stroke width stays in CSS pixels across source resolutions — a 2 px bbox stroke on a 480p source matches a 2 px stroke on a 4 K source visually, instead of becoming a hairline on 4 K. |
| 6 | Overlay user coords MUST stay within `[0, VIEW_W] × [0, VIEW_H]`. Geometry that legitimately needs to exceed (FOV halos, range rings, off-screen indicators) MUST be capped at `min(VIEW_W, VIEW_H) * 0.5` OR drawn inside an explicit `<clipPath>`. | Belt-and-suspenders. `overflow: hidden` is the primary guard; the radius cap is a secondary guard in case a future change accidentally drops the stage clip. |
| 7 | OS chrome (status bars, banners, floating toolbars, modals) MUST live OUTSIDE the stage at panel-root level. | These elements legitimately need to overlap the dark gutters — putting them inside the stage would force the gutters to be visually empty and waste the operator's real estate. |

### 14.4 FOV ring radius cap (current implementation)

`OperateCanvas.svelte`'s FOV ring is currently driven by `aimingConfig.smoothingCoefficient` as a Phase 5 slider-feedback demo. The radius is capped per Rule 6 above:

```ts
// TODO(phase-6-fov-system): replace with profile.actuation.targeting_fov_radius_px once profile schema lands
const fovRadius = Math.min(
  200 + aimingConfig.smoothingCoefficient * 520,
  Math.min(VIEW_W, VIEW_H) * 0.48
);
```

The `200` / `520` / `0.48` constants are intentionally magic for Phase 5 — when the real configurable FOV system lands (Phase 6+, bound to `profile.actuation.targeting_fov_radius_px` per `dashboard_menu_architecture_2026.md` §11.3), the cap is replaced with profile-driven min/max bounds. The stage's `overflow: hidden` remains the primary guard either way.

### 14.5 Migration checklist for new overlay components

When adding a panel that draws SVG/Canvas over a raster source:

1. Wrap the raster + overlay in `.canvas-stage` with inline `aspect-ratio: {srcW} / {srcH}`.
2. Promote the outer container to `display: grid; place-items: center` with `overflow: hidden` and `min-width/height: 0`.
3. Make the raster `position: absolute; inset: 0; width/height: 100%; object-fit: fill`.
4. Make the SVG `position: absolute; inset: 0; width/height: 100%` with `viewBox` set to source pixel dimensions and `preserveAspectRatio="xMidYMid meet"`.
5. Add `vector-effect="non-scaling-stroke"` to every stroked overlay primitive (rect, circle, ellipse, line, path, polyline).
6. Cap any user-coord geometry that could exceed source bounds at `min(W, H) * 0.5` OR wrap in `<clipPath>`.
7. Keep OS chrome (status bars, banners) at the panel root, NOT inside the stage.

---

## §15 — Virtual cursor controller (Phase 6-part-A stand-in)

### 15.1 Why this exists

Per `dashboard_menu_architecture_2026.md` §12.4 ("Actuator telemetry emit"), the C++ `actuator_telemetry_emit_thread` was designed to interpolate between tracker frames using a jerk-limited Ruckig controller, emitting `clean_u/v` as the post-controller cursor position and `raw_u/v` as the pre-controller setpoint. That C++ controller is **Phase 6 part B** and is not built yet — the current emit thread just snapshots `selector_cx + dx_px` into both `raw_u/v` and (EMA-smoothed) `clean_u/v`.

Until Phase 6 part B lands, **the dashboard runs its own jerk-limited approach controller in JavaScript** and uses ITS cursor history as the source for Layer 1.5's CLEAN trail. The C++ stream's `raw_u/v` is treated purely as the controller's setpoint (= "where the tracker says the target is right now"). This makes the visualisation conceptually honest for the anti-cheat research mission: the CLEAN trail is what a humanised aim-assist would produce, the DIRTY trail is what an unhumanised bot would produce, and both are anchored at the canvas center (= the in-game crosshair anchor).

### 15.2 Controller model

Pure 2-D point-mass with velocity limit and acceleration limit. Per RAF tick (~60 Hz):

```
target = (raw_u_newest, raw_v_newest)            // from actuatorPool[head-1]
error  = target - cursor
dist   = ||error||

// Desired velocity: gain × distance, capped at MAX_V.
// Gain × dist gives critical-damping-ish behaviour: fast far from target,
// slow at lock-on. Direction is unit vector toward target.
v_desired_mag = min(MAX_V, dist × GAIN)
v_desired     = (error / dist) × v_desired_mag   if dist > 0.5 else (0, 0)

// Jerk limit: bound the change in velocity per tick. This is the constraint
// that produces the swooping arcs from the reference image (the green curve);
// without it the controller would be a pure first-order low-pass and produce
// straight chords between target updates.
dv     = v_desired - v
|dv|   = clip(|dv|, 0, MAX_A × dt)
v     += dv

cursor += v × dt
cursor  = clamp(cursor, viewBox)                 // pin to visible image bounds
```

Defaults (`OperateCanvas.svelte`):

| Constant | Value | Rationale |
|---|---|---|
| `CURSOR_MAX_V_PX_S` | `1500` | Upper bound of pro FPS aim sustained velocity in screen-space pixels per second. |
| `CURSOR_MAX_A_PX_S2` | `12000` | Jerk-limited approach. ~8 ms 0→1500 px/s ramp — fast enough to keep up with a sprinting target, slow enough to look human. |
| `CURSOR_APPROACH_GAIN` | `8` | Maps distance to desired velocity. Higher = snappier on close lock, slower far away. Tuned by visual inspection vs the reference image. |
| `CURSOR_TRAIL_CAP` | `240` samples | ~4 s of trail at 60 Hz RAF. Matches the binding spec's 8 s trail history halved for visual clarity (long enough to see the controller's arc shape, short enough that old chaos isn't on screen). |

`dt` is **clamped to `[0.001, 0.05]` s** so a tab-switch / GC pause cannot teleport the cursor by integrating a multi-second `dt`.

### 15.3 SVG model

Layer 1.5 now has FOUR primitives (was 2):

| Element | Source | Style | Meaning |
|---|---|---|---|
| `actuatorRawPathRef` (polyline) | `actuatorPool.rawU/V` newest 60 samples, prepended with canvas center | orange `#ff8a4c`, 1.5 px, 0.5 opacity | DIRTY trail. Raw target-snap signature. What an unhumanised bot produces. |
| `actuatorCleanPathRef` (path) | JS controller `cursorTrailX/Y` ring | cyan `#00f0ff`, 2 px, 0.8 opacity | CLEAN trail. Jerk-limited cursor history. What a humanised aim-assist produces. |
| `cursorOriginRef` (g — crosshair + dot) | Fixed at `(VIEW_W/2, VIEW_H/2)` | grey `#7a8a9a`, 1 px, 0.45 opacity | Origin marker. The virtual cursor's birthplace = the in-game crosshair anchor. |
| `cursorMarkerRef` (g — ring + dot) | JS controller `(cursorX, cursorY)` | cyan `#00f0ff`, 1.5 px outer / 2 px solid inner dot | Live cursor marker. The head of the CLEAN trail. |

### 15.4 Lifecycle

- Controller state (`cursorX/Y`, `cursorVX/VY`, ring) is module-local to `OperateCanvas.svelte`. One instance per canvas.
- `$effect` watches `sourceStore.activeSourceId`; flipping the camera resets the controller (different image plane → old trail invalid).
- `cursorInit` flag delays the first tick until both VIEW_W and VIEW_H are bound to the active source's resolution — prevents a 1920×1080-anchored cursor flashing for one frame on a 640×480 source.
- When `settings.showActuatorTrace = false`, all four primitives hide. Controller still ticks (so the trail is correct on toggle-back-on) but does no DOM work.

### 15.5 Migration to Phase 6 part B (C++ Ruckig)

When the C++ controller lands, this entire JS controller block is **deleted in one PR**:

1. Remove the JS controller state + `tickCursorController` from `OperateCanvas.svelte`.
2. Replace `cursorTrailX/Y` reads with direct walks of `actuatorPool.cleanU/V` (oldest→newest, length 240).
3. Keep `cursorMarkerRef` — bind its `(x, y)` to the newest `actuatorPool.cleanU/V` sample.
4. Keep `cursorOriginRef` — same logic, fixed at `(VIEW_W/2, VIEW_H/2)`.
5. Keep the DIRTY trail polyline as-is — `actuatorPool.rawU/V` is the contract for "tracker setpoint" in both Phase 6-A and Phase 6-B.

The visual model is **unchanged across the migration** — only the source of the smoothed cursor stream moves from JS RAF to the C++ 500 Hz emit thread, gaining (a) sub-RAF-tick resolution and (b) byte-for-byte parity with the actual hardware actuator command stream.

### 15.6 Validation

`tickCursorController` is testable in isolation. A canonical fixture for Phase 6-A → Phase 6-B regression:

```ts
// Setpoint sequence: hold center 0.5s, jump to (800, 300), hold 1s, jump back.
// Expected: clean trail arcs smoothly, no overshoot >5%, settling time <300ms
// at GAIN=8, MAX_V=1500, MAX_A=12000.
```

When the C++ side ships, the dashboard test harness compares the JS controller's trail against `cleanU/V` from a synthetic C++ telemetry capture; tolerance is `≤ 4 px` end-point error over a 2-second jump-and-hold sequence. Anything looser indicates the C++ side picked different limits and they need reconciliation against the binding spec.

---

**Last updated: 2026-05-18 · After §15 virtual cursor controller landed in `OperateCanvas.svelte` (Phase 6-part-A stand-in).**
*Future overlay components — copy §14.2 verbatim. The rule at `.cursor/rules/220-operate-canvas-stage.mdc` is enforced for every `dashboard/**/*.svelte` file. The cursor controller §15.2 is the canonical reference for the C++ Ruckig port (Phase 6 part B).*

---

## §16 — Biometrics destination (5th tab, MDC)

### 16.1 What & why

A standalone mouse-dynamics capture tool living as the 5th nav destination (`hotkey 5`). Captures raw mouse-input deltas via the Pointer Lock API, computes AC-signature features in real-time, renders the exact 7-section layout from the reference image, and exports labeled datasets as PNG + JSONL. **Completely decoupled from the C++ vision pipeline** — usable for capturing labeled clean-vs-cheat sessions on the same workstation with the camera/orchestrator off entirely.

Binds to `dashboard_menu_architecture_2026.md` §14 (Amendment v6) — the **single documented exception** to the cardinality-of-destinations lock.

### 16.2 Layout grid (exact-match to reference image)

```
+======================================================================+
| [1] TOP BAR (full width, ~96px tall, collapsible via Shift+F)         |
|     prompt + 3 sliders + Clear history + label dropdown               |
+-----------------------------------+--------+-------------+------------+
|                                   |        |             |            |
|  [2] TRACE current/last           |  [4]   |  [5]        |  [6]       |
|       (60% width, 50% height)     | Session| Report rate | Sign flip  |
|                                   | stats  +-------------+ rate       |
|                                   | text   |  [5b]       | (RED bdr)  |
|                                   |        | Velocity    |            |
+-----------------------------------+--------+-------------+------------+
|                                   |        |             |            |
|  [3] TRACE previous               |  [4']  |  [5']       |  [6']      |
|       (60% width, 50% height)     | Session| Report rate | Sign flip  |
|                                   | stats  +-------------+ rate       |
|                                   | text   |  [5b']      | (RED bdr)  |
|                                   |        | Velocity    |            |
+-----------------------------------+--------+-------------+------------+
```

CSS: `grid-template-columns: 60fr 12fr 14fr 14fr; grid-template-rows: auto 1fr 1fr;`

User-counted "7 sections" map to: top bar + 2 trace slots + 4 metric column types (Session, Report rate, Velocity, Sign flip rate).

### 16.3 Color tokens

| Element | Token | Rationale |
|---|---|---|
| Destination background | `#0e0e10` (`--surface-0`) | Matches existing app background |
| Panel background | `#161618` (`--surface-1`) | Matches other destinations' panel cards |
| Panel border (default) | `#27272a` (`--border-subtle`) | Standard |
| Trace stroke | `#00f0ff` (`--accent-cyan`) | Matches Layer 1.5 in OperateCanvas |
| Trace start dot | `#34d399` (`--success-green`) | Matches Engage interlock OK light |
| Trace end/head dot | `#ff8a4c` (`--warning-orange`) | Reserve true red for alarms |
| Sparkline stroke | `#00f0ff` cyan | Visual consistency with trace |
| Sparkline fill (area under curve) | `rgba(0, 240, 255, 0.08)` | Subtle, matches reference's bar-chart-ish look |
| **Sign-flip rate panel border** | `rgba(255, 68, 68, 0.6)` 1px | **ONLY red element in the dashboard, intentional alarm marker** |

> §16.3.1 Layout enforcement: the sign-flip column (col 4) MUST fill the full row height — its inner `SparklinePanel` carries an explicit `flex: 1` rule. Without this, the panel collapses to its header's intrinsic height and produces a visually unbalanced L-shape next to the stacked Report-Rate / Velocity column on its left. The sign-flip column MUST always equal the combined Report-Rate + Velocity column height per row, per the reference image.

> §16.3.2 Sign-flip sparkline math: per-bucket raw counts are convolved with a sliding window of `0.25 × buckets` width and divided by the window's duration in seconds. The resulting flips/sec curve plateaus around the session's mean flip rate, so the *shape over time* (rise rate, plateau height, stability) is the human-vs-bot discriminator. Raw per-bucket counts are NOT shown — they produce sparse 0/spike binary plots that are visually useless at typical capture rates. See `dashboard/src/lib/biometrics/metrics.ts :: signFlipRateSeries` and `dashboard_menu_architecture_2026.md §14.4.1`.
| Slider track | `#27272a` filled, `#3a3a3e` track | Matches Calibrate sliders |
| Label dropdown | text `#d4d0c8`, bg `#1a1a1e`, border `#333` | Matches Settings drawer inputs |
| Mono text (stats) | `ui-monospace, 'SF Mono', Consolas, monospace` | Matches diagnostics strip |

### 16.4 State machine

Pointer Lock + `pointermove` `movementX/movementY`. Mouse4 (button 3) hold = primary trigger; Spacebar hold = keyboard fallback.

```
IDLE → [click capture area] → POINTERLOCK_REQUESTED → ARMED ↔ (Esc/blur)
  ARMED → [Mouse4 down OR Space down] → (rotate prior current→previous if committed) → RECORDING
  RECORDING → [Mouse4 up OR Space up] → COMPLETED (mark currentCommitted=true) → ARMED
```

**Rotation discipline (§16.4.1):** `previous` is populated at the *start* of the next recording (rotate-on-begin), not at the end of the current one. Rationale: keeping the prior committed session frozen in `previous` for the entire duration of a new recording lets the operator visually compare a live trace against their most recent clean baseline. The internal `currentCommitted` flag gates rotation — dropped sessions (Esc / window blur / pointer lock loss) are never promoted into `previous` and never displace a prior clean baseline.

### 16.5 Wire format (JSONL export)

Header line:
```json
{"_": "mdc-v1", "label": "CLEAN", "started": "2026-05-18T05:30:00.000Z", "duration_ms": 1806, "report_count": 231}
```
Per-sample lines:
```json
{"t": 0.000, "dx": 3, "dy": -1}
{"t": 1.247, "dx": 2, "dy": 0}
...
```
Directly trainable; loadable in Python via `pandas.read_json(path, lines=True)`.

### 16.6 Hard constraints (enforced by code review)

1. Biometrics destination MUST NOT import `$lib/telemetry/*`, `$lib/stores/track_pool*`, `$lib/stores/actuator_pool*`, `$lib/stores/bg_odometry_pool*`. Independent data plane.
2. Pointer Lock MUST be requested on user gesture (click), never on mount.
3. Esc MUST cleanly release pointer lock + drop SM to IDLE; in-flight RECORDING is dropped, NOT rotated to PREVIOUS.
4. Sign-flip panel red border is binding across all theme changes.
5. The top bar is collapsible via `Shift+F` and double-click on the splitter handle; collapsed state persists in `localStorage`.

### 16.7 File layout (relative to `dashboard/src/lib/`)

```
biometrics/
├── mouse_capture.svelte.ts     ($state ring buffers + Pointer Lock SM)
├── metrics.ts                  (4 zero-alloc pure compute functions)
├── export.ts                   (PNG composite + JSONL serializer)
└── labels.svelte.ts            (label vocabulary + localStorage persistence)

components/
├── biometrics/
│   ├── TracePanel.svelte       (SVG path with auto-fit viewBox + start/end dots)
│   ├── SessionStatsPanel.svelte (vertical-stack labels/values; 10 Hz refresh)
│   └── SparklinePanel.svelte   (canvas sparkline; reused 6× — 3 metrics × 2 rows)
└── destinations/
    └── BiometricsDestination.svelte (owns the 4-column grid + collapsible top bar)
```

> §16.7.1 — `SessionStatsPanel` uses a vertical per-stat stack (label on top, value below) NOT a 2-column grid. The session column is intentionally narrow (12fr of the 100fr row) and the longest label exceeds the cell width; a horizontal grid would push values off the right edge into `overflow: hidden` (the original bug, fixed 2026-05-18 with Amendment v7). Future contributors MUST preserve the stack layout — the values column must never be a CSS grid track wider than the cell.

---

## §17 — Models & FOV drawers (Amendment v7)

Two new left-rail drawers landed alongside Sources / Settings:

### 17.1 Models drawer (real, server-backed)

Backed by `model_store.svelte.ts` (REST client for the orchestrator's
`/api/models` + `/api/model/active`). On open the drawer issues a refresh
which populates `modelStore.models[]` with every `.yaml` manifest found
in the orchestrator's `--models-dir` (defaults to the parent of the
boot manifest). Each card shows:

- name + Active / Ready / Broken status pill
- purpose + head + input W×H + class count
- inline status note for broken manifests (parse error or missing .onnx)
- Switch button (disabled when card is the active model, OR the manifest
  is broken, OR a swap is already in flight)

Switching POSTs `{name: "..."}` to `/api/model/active`. The C++ side runs
`InferenceEngine::try_swap_model()` which builds the new ORT session on
the calling thread, validates I/O shapes against the new manifest, then
atomically swaps under `swap_mtx_`. Inference is paused for exactly one
Ort::Run call duration. On failure the prior session remains active and
the drawer banners the explanatory error.

### 17.2 FOV drawer (operator-driven inference crop)

Backed by `fov_store.svelte.ts` (REST client for `/api/fov`). The
"inference crop" is a SQUARE centred on (cxNorm × W, cyNorm × H) with
half-extent `radiusPx`. When enabled, the inference thread cv::Mat-ROIs
the source frame to this rect BEFORE running the model; detections are
translated back to source-frame coordinates by adding the crop offset.
This means the dashboard's overlays + tracker + actuator all continue to
see source-frame coordinates and need no changes.

Controls:

- Enable toggle (master)
- Follow-target toggle (Phase 7) — when on AND a track is locked, the
  tracker thread overrides cx/cy_norm each frame with an EMA (α=0.35) of
  the selected target's EKF pixel position. Operator cx/cy are ignored
  while follow-target is on.
- cxNorm, cyNorm, radiusPx sliders
- Re-centre button
- Diagnostics block — source frame size, last crop rect the inference
  thread published, pixel-count percent. The dashed orange rectangle on
  the Operate canvas (`fovCropRectRef` in OperateCanvas.svelte) is bound
  to these diagnostic fields, so the operator can visually verify the
  EXACT region the model saw last frame.

Performance contract: enabling FOV at half-frame radius reduces inference
compute by roughly 4× (radius² × 4 / W × H), translating to a
proportional speedup once preprocess+inference is GPU-bound.

### 17.3 LeftRail icon assignment

| Drawer | Icon path concept |
|---|---|
| Sources | film/feed icon (existing) |
| Models | cube outline (existing) |
| **FOV** | **crosshair-in-square (scope reticle inside source frame)** |
| Settings | gear (existing) |

---

---

## §18 — SourcesDrawer desktop-capture section (Amendment v8)

The SourcesDrawer gained a new DESKTOP CAPTURE section below the source
device list. Backed by `capture_sources_store.svelte.ts` which polls
`GET /api/capture-sources` to populate live monitor + window lists.

### 18.1 UI flow

- Kind selector (4 pills): `primary` / `monitor` / `window` / `region`
- Conditional sub-form based on kind:
  - `monitor` → dropdown listing every enumerated monitor with index +
    primary flag + dimensions + virtual-desktop position
  - `window` → dropdown listing every visible top-level window with title
    + dimensions. Empty selection means "capture whichever window is
    foreground at the time of each frame"
  - `region` → 4 number inputs (X, Y, W, H) in virtual-desktop coordinates
- Common options: FPS cap, downscale-max-dim (0 = native), include-cursor checkbox
- Synthesised CLI block — a monospace `<code>` rendering the exact
  orchestrator flags the operator would need to restart with, plus a
  Copy-to-Clipboard button. Live source-swap is on the Phase-8.5 wishlist.

### 18.2 Backend contract

The C++ `MjpegServer::ApiHandlers::list_capture_sources` returns a
`CaptureSourcesSnapshot` containing arrays of `CaptureMonitorInfo`
(index, x, y, w, h, isPrimary, deviceName) and `CaptureWindowInfo`
(hwnd, title, x, y, w, h, minimized). The dashboard filters minimised
windows and windows smaller than 200×150 to keep the dropdown actionable.

### 18.3 CLI flag synthesis (binding)

The `synthesizeCliFlags(args)` helper is the single source of truth for
the dashboard's CLI rendering. Any future change to the orchestrator's
flag names MUST be made in both places (`core/main.cpp` parser AND
`capture_sources_store.svelte.ts` synthesizer). A unit test is on the
Phase-8.5 wishlist to enforce that.

---

**Last updated: 2026-05-18 · After §18 Desktop capture section landed (Amendment v8).**
*The 5-destination cardinality remains binding. Drawers are NOT destinations — they are modal control surfaces that overlay any destination.*
