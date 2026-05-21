---
name: NASA-Grade Dashboard Build Manifest
overview: Complete build manifest for the ROC AI Vision dashboard, control-by-control, field-by-field, extracted from all 1354 lines of the binding spec (v2 ss0-10 + v3 ss11 + v4 ss12 + v5 ss13). Every UI element, every input, every readout, every hotkey, every state transition specified.
todos:
  - id: s1
    content: "Session 1 (~1200 LOC) ROBOFLOW-GRADE OPERATE: class_colors palette + bbox_renderer helpers + render_settings_store + selection_store + OperateCanvas 6-layer composite (per-class colored bboxes + Roboflow-style class labels with confidence + track ID badges + anchor crosshair + velocity arrow + uncertainty ellipse + hierarchy edges + selection outline) + Layer Visibility Toolbar (6 toggles + opacity slider + blend mode + pixel alpha) + Model Card overlay (mAP/Prec/Recall + live INF p50/p99) + Object Count Badge with per-class breakdown + RightRail (Detection JSON Feed live cards + Thresholds & Display panel + Selected Track Inspector with 6-option anchor selector + 120x80 minimap) + NavigationShell grid update + TopChrome Profile dropdown + DiagnosticsStrip hover tooltips with sparklines + profile_store"
    status: pending
  - id: s2
    content: "Session 2 (~600 LOC) ENGAGE COCKPIT: shared OperateCanvas reuse + 48px Safety Interlock Bar (Camera/Tracker/Auth/Armed lights with reactive bindings) + Armed two-stage toggle with PENDING pulse state + REPLAY-disabled red state + Engage Right Rail (Active Locks list with UNDO + Locked Track Inspector + Hardware Control config) + Bottom Hardware Bar (serial status pill + slew arrow + EMERGENCY STOP red button) + Target Lock Confirmation Modal (Skydio Pause-slider 800ms hold) + Alt+L/Space/g/Esc wiring"
    status: pending
  - id: s3
    content: "Session 3 (~1100 LOC) INSPECT FOXGLOVE-GRADE: Inspect grid layout + HierarchyExplorerDrawer Palantir Gotham tree (parent_id graph + right-click pivot menu + violation row highlighting) + MouseDynamicsInspector full implementation (header bar with R/X/P + 4 trace modes + buffer/label badges + PreRecordModal flow) + Derived Feature Strip 4 mini-plots (P1 VelocityProfile + P2 JerkSpectrum log-Y + P3 FittsResidual scatter+OLS+outliers + P7 SubPixel big-number+bands+sparkline) + SpectatorTelemetryMatrix full ss6.2 (KPI header + dual-line chart + cross-correlation strip + bottom KPIs) + PipelineGraph SVG nodes + ModelInspector sortable table + TopicGraph + EventLogDock collapsible 32px/240px with level filter + per-panel right-rail settings + P4/P5/P6/P8 stub registrations + focused-panel hotkeys"
    status: pending
  - id: s4
    content: "Session 4: Calibrate wizard interactive — 5+1 steps: Step 1 intrinsics 8 fields, Step 2 extrinsics 3x4 matrix, Step 3 full ss11.2 (alpha+NIS+FOV+deadzone+velocity/accel curves+max dX/dY+presets+reset), Step 4 model selector, Step 4.5 R11 sub-pixel verification (F16 gate>=0.40), Step 5 save-as-scenario with localStorage"
    status: pending
  - id: s5
    content: "Session 5: 6 scenarios + 4 profiles + profile dropdown + profile_store + localStorage auto-load + Alt+Z overlay mode (collapse rails, 75% opacity, 120ms) + diag strip hover sparklines + time cursor placeholder + BEACON export bundle (jsonl.gz+trajectory.png+hardware_info+croissant) + wire Alt+R/Tab/[/]/g + radar panel stub"
    status: pending
isProject: false
---

# ROC AI Vision — Dashboard Build Manifest

This is not a summary. This is a field-level build manifest extracted from every section of `dashboard_menu_architecture_2026.md` plus `phase_5_console_robotics_ml.md`, `dashboard_game_cv_overlay_ux_2026.md`, `dashboard_pro_console_ux_2026.md`, and `anticheat_data_exposure_2026.md`. An executing agent reads this and builds exactly what is described, nothing more, nothing less.

The dashboard must look and feel like a real product comparable to Roboflow + Foxglove + Mission Control. Every square inch earns its place. The camera feed is the center of the universe.

---

## CHARTER: VISUAL DENSITY AND POLISH STANDARDS (read this BEFORE any code)

These are NON-NEGOTIABLE rules that apply to every component, every state, every pixel.

### No Empty Space Rule
- Every panel card has data the moment it mounts. If live data isn't available yet, show a skeleton schema with field names + dashes (e.g. "x: --- y: --- w: --- h: ---") in mono font, faded 30%. Never a blank box. Never the words "Coming Soon" or "Phase 6+" as primary content (those go in subtle footer text).
- Every right-rail section, drawer, and overlay shows real content or schema-shaped placeholders. The user must be able to look at any surface and understand what data will populate it.
- Empty `<div>` is forbidden in any visible surface. Use `min-height` plus skeleton placeholder, not empty containers.

### Loading and Skeleton States
- Initial mount: subtle shimmer animation on placeholder values (cream-to-grey 1.5s ease infinite)
- Awaiting first frame: schema-shaped layout with `---` placeholders, NOT a centered "Loading..." spinner
- Frame stale (no update for > 200ms): values dim to 50% opacity, status dot flips grey/stale
- Error state: red border on the section, error text in mono font with the actual error

### Cockpit Instrumentation Feel
- Status dots have subtle glow when active: `box-shadow: 0 0 4px {colour}` at 40% alpha. No glow when stale
- Critical events get a red pulse animation: 1.2s ease-in-out infinite for 3 cycles, then steady red
- Every interactive element has a 150ms ease-out transition on state change (background, border, color)
- Hover reveals (tooltips, expanded details): 80ms delay, 100ms fade-in
- Every value is mono font (`SF Mono`, `Cascadia Code`, `Consolas`) - data is mono, labels are sans
- Labels: 9-11px sans, uppercase, letter-spaced 0.06em, grey #878787
- Values: 11-13px mono, color-coded by status, right-aligned where comparable across rows
- Numbers always have consistent decimal places per metric (FPS = 0, latency = 1, ratio = 3)

### Information Density Targets
- Diagnostics Strip: 6+ metrics in 26px height
- Right rail: 280px wide must hold detection JSON + threshold controls + selected track inspector simultaneously
- Inspect destination at 1920x1080: minimum 8 distinct data surfaces visible without scrolling
- No surface bigger than necessary; if a panel needs more space, it earns it through density not padding

### Color System (extracted, applied consistently)
- App bg: #0e0e10 (near-black)
- Surface bg: #161618 (dark grey)
- Card bg: #1a1a1e (slightly lighter)
- Border: #27272a (subtle)
- Border-active: #f5a623 (orange accent)
- Text primary (cream): #d4d0c8
- Text secondary (grey): #878787
- Text tertiary (muted): #555
- Text mono (data): #d4d0c8
- Accent (orange): #f5a623 (active states, brand)
- Status OK (green): #34d399
- Status Warn (orange): #f5a623
- Status Err (red): #ef4444
- Status Stale (grey): #444
- Cyan accent (clean trace, EKF): #00f0ff
- Amber accent (dirty trace, raw): #ff8a4c

### Per-Class Detection Colors (NEW POLICY - applies to bbox stroke + label background)
- Unknown (0): #6b6b6f (grey)
- Person (1): #00f0ff (cyan)
- Head (2): #ff6dc7 (magenta)
- UpperTorso (3): #34d399 (green)
- AccessoryHat (4): #f5a623 (orange/amber)
- AccessoryBackpack (5): #ff8a4c (warmer amber)

---

## CHARTER: DETECTION RENDERING SPEC (the heart of the Operate canvas)

This is how every detection becomes pixels on the canvas. Maps directly to wire frame v1 fields decoded into `trackPool`.

### Bounding Box (Layer 2)
- SVG `<rect>`: x = (u - w/2), y = (v - h/2), width = w, height = h (centroid-to-topleft conversion already done in TelemetryCanvas)
- Stroke: 2px solid, color from per-class palette above based on `pool.label[slot]`
- Stroke-dasharray modulated by status flags:
  - confirmed (bit 0): solid
  - boundary (bit 1): `8 4` dashed + amber pulse animation
  - stationary (bit 2): smaller "S" badge at top-right corner of bbox
  - hierarchy_violation (bit 3): RED override stroke + flash animation 3 frames, red glow
- Fill: none (boxes are outlines only, like Roboflow)
- Corner notches optional (per BBox Style setting)

### Class Label + Confidence (above the bbox, Roboflow style)
- Position: just above the bbox (y - 18)
- Layout: `<g>` containing a `<rect>` background pill and `<text>` overlay
- Background pill: fill = class color, rx = 2, padding 4px horizontal
- Text: 11px mono bold, dark text color (#0e0e10) when on bright pill, white when on dark
- Format depends on Label Display Mode setting:
  - "Draw Confidence": `Person 89.7%`
  - "Draw Labels": `Person`
  - "Draw Both": `Person 89.7%` (same as Confidence)
  - "None": label hidden entirely
- Confidence percentage: `(confidence * 100).toFixed(1) + '%'`
- Position smart: if box near top of frame, label goes BELOW the box instead

### Track ID Badge (corner of bbox)
- Position: top-right corner inside the bbox
- Format: `0x{trackIdLo.toString(16).padStart(8, '0')}` (first 8 hex digits)
- Style: 9px mono, color-matched to class but 70% opacity
- Optional toggle in Threshold Controls panel

### Trajectory Tail (Layer 1)
- Per slot: render the 16-sample ring as `<polyline>`
- Color: class color, opacity gradient 0.15 (oldest) → 0.6 (newest)
- Stroke-width: 1.5px
- Stroke-dasharray: `2 2` (light dotted)
- Tail extension: 3-frame forward extrapolation from last 2 samples in same color at 0.8 opacity (already exists in TelemetryCanvas)
- Toggle: Show Trajectory in Threshold Controls

### Anchor Crosshair (Layer 4)
- Per spec ss11.3, draw at the resolved anchor point per active profile
- Resolved position depends on `targeting_anchors[label]` setting:
  - `bbox_top`: (u, v - h/2)
  - `bbox_top_center`: (u, v - h*0.4)
  - `bbox_center`: (u, v)
  - `bbox_center_lower`: (u, v + h*0.2)
  - `bbox_bottom_center`: (u, v + h/2)
- Render: small diamond (4x4px) + cross (8px arms) at the anchor point
- Color: bright orange #f5a623 with 2px stroke + 1px outer white glow for visibility on any background
- Only visible when track is selected OR Show Anchor toggle is on

### Velocity Arrow (Layer 1 / 4)
- Per spec ss9 Q2: arrow from centroid → predicted position 100ms ahead
- Predicted position: (u + vWorld[0] * 0.1, v + vWorld[1] * 0.1) using image-plane velocity
- SVG `<line>` with `<polygon>` arrowhead
- Color: bright cyan #00f0ff at 80% opacity
- Stroke-width: 2px
- Length proportional to |v| (capped at 200px to avoid overflow)
- Optional 1s opacity-fade trailing history of past arrow positions
- Toggle: Show Velocity Arrow

### EKF Uncertainty Ellipse (Layer 2)
- Per spec uses `pPosDiag[3]` (x, y, z diagonal of covariance)
- SVG `<ellipse>` centered on (u, v), rx = sqrt(pPosDiag[0]) * scale, ry = sqrt(pPosDiag[1]) * scale
- Scale factor maps world units to image px (currently 1.0 placeholder until camera intrinsics load)
- Stroke: class color at 30% opacity, 1px dashed `4 2`, no fill
- Toggle: Show Uncertainty Ellipse

### Hierarchy Edge (Layer 3)
- When `parent_id` != 0, draw dashed line from this bbox centroid to parent bbox centroid
- SVG `<line>` 1px stroke, color #555, dasharray `6 3`
- Parent slot lookup: `_idToSlot[parent_id]` from trackPool (uses parentIdLo/parentIdHi composite)
- Only rendered if parent slot is currently active
- Toggle: Show Hierarchy Edges

### Selection Highlight (special)
- When user clicks a bbox: orange 3px stroke outline around the bbox at 100% opacity
- Selected slot index stored in a `$state` in `selection_store.svelte.ts`
- All right-rail content keys off this selection
- Tab/Shift+Tab cycles selectedSlot through active tracks

### Hover State (in JSON feed → highlight on canvas)
- When mouse hovers detection card in right rail: that bbox on canvas gets temporary orange outline (1.5px) and slight glow
- Implemented via `hoveredSlot` $state shared between right rail and canvas

### Frame-Level Status Indicators (top-left of canvas, existing)
- `SRC: MOCK` / `SRC: LIVE` / `SRC: FILE` / `SRC: REPLAY` color-coded
- `SEQ: {frameSeq}` mono
- `ACT: {activeCount}` mono with status dot
- `NIS: OK` / `NIS: DEGRADED` from frameFlags bit 0, red when DEGRADED
- `BND: OK` / `BND: PRESENT` from frameFlags bit 1, amber when PRESENT
- `WS: {connected}/{attempts}` with green/red dot
- All in monospace, 10px, semi-transparent dark bg pill

---

## SHELL CHROME (wraps all destinations)

### Top Chrome Bar (42px, pinned, never scrolls)

Left zone:
- Brand: "ROC" (orange #f5a623, 800 weight) + "AI Vision" (cream #d4d0c8, 500 weight) + "v2.0-alpha" (grey #666, mono, 10px)

Center zone:
- Scenario dropdown: `<select>` bound to `scenarioStore.activeScenarioId`. Lists all scenarios by `label`. On change: loads the scenario's profile overrides, inspect panel list, sink kind, recording defaults. Styled: dark bg #111113, cream text, orange border on focus
- Profile dropdown: `<select>` bound to `profileStore.activeProfileId`. Lists: Default (dev), Production NVIDIA, Production AMD/Intel, Replay. On change: updates actuation block, EP order, recording flag. **THIS DOES NOT EXIST YET — MUST ADD**

Right zone:
- LIVE/REPLAY badge: green bg+text when LIVE, red bg+text when REPLAY. Reads from a global `$state` `appMode: 'live' | 'replay'`. REPLAY triggers when time cursor drags past NOW (Phase 6+); for now always LIVE
- SINK badge: shows `SINK: {sinkLabel}` where sinkLabel mirrors the active scenario's `sinkKind` mapped to display names (NO-OP, HID-MOUSE, DYNAMIXEL, PWM, MODBUS, CSV). Colour-coded per sink type (grey for noop, orange for hid, amber for dynamixel)

### Diagnostics Strip (26px, pinned below Top Chrome)

6 tiles, always visible, horizontal. Each tile has:
- Coloured dot (6px circle): green #34d399 = OK, orange #f5a623 = Warn, red #ef4444 = Err, grey #444 = Stale
- Label (9px, mono, uppercase, grey)
- Value (11px, mono, coloured by status)
- **Hover tooltip** (missing — must add): shows threshold definition, source binding name, last 30 values as a sparkline
- **Click** cycles text-only / text+graph mode (spec ss6)

Exact tiles and their data sources + thresholds:

- **INF** (Inference latency ms): source = time between consecutive v1 frame arrivals in `wire_decoder`. Threshold: value > 30ms = Warn, > 50ms = Err. Currently shows "---" — must compute from `trackPool.lastFrameTs` delta
- **FPS** (Dashboard render FPS): source = RAF tick counter. Threshold: < 30 = Warn, < 10 = Err. Already works
- **TRK** (Track count): source = `trackPool.activeCount`. Threshold: informational (always OK if > 0, stale if 0). Already works
- **WS** (Wire egress rate): source = `telemetrySocket.framesReceived` derivative (frames/sec). Threshold: < 20 = Warn, < 5 = Err. Currently shows CONN/OFF — must compute rate
- **ACT** (Actuator ack lag ms): source = future actuator round-trip. Threshold: > 25ms = Warn. Currently shows "---" — placeholder OK
- **EKF** (Measurement age ms): source = `performance.now() - lastFrameReceiveTime`. Threshold: > 200ms = Warn. Currently shows OK/--- — must compute staleness

### Left Rail (56px wide, vertical, pinned)

4 destination buttons stacked:
- Each: 44x44px, icon (20px SVG) + label (8px, uppercase) + hotkey hint (9px, mono, top-right corner)
- Active state: bg #27272a, text+icon colour #f5a623, left 3px inset border #f5a623
- Hover: bg #27272a, text #d4d0c8
- Icons: crosshair (Operate), activity-wave (Inspect), sliders (Calibrate), concentric-circles (Engage)

Separator line (1px, #2a2a2e, 10px vertical margin)

2 drawer buttons:
- Models: box/cube icon, opens Models drawer
- Settings: gear icon, opens Settings drawer
- Drawer buttons pinned to bottom of rail (`margin-top: auto`)
- Both 44x44px same styling as destination buttons minus the active state

### Right Rail (260px wide, collapsible via `]` hotkey) — **DOES NOT EXIST YET**

**When Operate or Engage is active — Track Selection Inspector:**
- Header: "Selected Track" or "No Selection"
- If a track is selected (future: click on canvas box):
  - Track ID (hex, mono)
  - Class label (from manifest mapping)
  - Age (frames since first detection)
  - Confidence (0.00-1.00, 2 decimal)
  - BBox dimensions (w x h px)
  - Targeting Anchor selector: 6-option segmented control (bbox_top, bbox_top_center, bbox_center, bbox_center_lower, bbox_bottom_center, custom_consensus) per ss11.3. Active anchor highlighted orange. Change sends `OperatorCommand{kSetTargetingAnchor}` to C++
  - Minimap: 120x80px thumbnail showing selected track position on canvas
- If no track selected: "Click a detection on the canvas to inspect" message

**When Inspect is active — Focused Panel Settings:**
- Header: name of the last-clicked panel card
- Panel-specific settings (per panel type):
  - MouseDynamicsInspector: render mode selector (c/d/t/s segmented), recording label override, buffer window size
  - SpectatorTelemetryMatrix: correlation window dropdown (4/8/16/32 sec), log Y-axis toggle, amplitude ratio threshold input
  - VelocityProfile: time window (4/8/16 sec), smoothing toggle
  - FittsResidual: minimum acquisitions before regression (default 5), outlier rejection toggle
  - SubPixel: window size (500/1000/2000 samples), threshold bands display toggle

**When Calibrate is active — Step Help:**
- Shows contextual help text for the active wizard step
- Step 3 (EKF Tuning): "Adjust smoothing alpha and deadband. Changes apply in real-time via OperatorCommand. The actuator trace on the Operate canvas updates within 2ms."

### Bottom Time Cursor (32px, Operate destination only) — **Phase 6+ placeholder**
- Horizontal track spanning full content width
- Draggable handle at [NOW] position
- Label: `00:00:00 / 00:30:00` with elapsed / total
- Drag handle left = enter REPLAY mode (badge flips, Engage controls disable)
- For now: render the bar with "LIVE — time scrubber available in Phase 6" text, disabled appearance

---

## DESTINATION 1: OPERATE (hotkey `1`) — THE ROBOFLOW-GRADE MISSION VIEW

**Mental model:** Camera feed is dead-center, filling 100% of available middle viewport. Right rail (280px) shows the live Detection JSON Feed + Threshold Controls + Selected Track Inspector. Floating overlays on the canvas show Model Card (top-right) and Object Count (bottom-right). Layer visibility toolbar floats bottom-left. This is the screen the operator stares at all day.

### Visual Layout

```
+--------------------------------------------------------------------+----------+
|                                                                    |          |
|  [SRC:MOCK SEQ:1247 ACT:6 NIS:OK BND:OK WS:0/1]   [MODEL CARD]   | RIGHT    |
|                                                                    | RAIL    |
|     <Composite Camera Canvas — fills 100% of center column>       |          |
|                                                                    | Detection|
|     +-----+   +-----+                                              | JSON     |
|     |Person|  |Head |  <- bboxes drawn per Detection Rendering    | Feed     |
|     |89.7%|  |92.1%|     Spec, per-class colored                  |          |
|     +-----+   +-----+                                              | Threshold|
|         crosshair + anchor                                          | Controls |
|                                                                    |          |
|  [Layer Toolbar: v0 v1 v1.5 v2 v3 v4]      [OBJ COUNT: 8 obj]    | Selected |
+----------------------------------------------------------------+----| Track   |
|  [Bottom Time Cursor placeholder — LIVE, Phase 6+]                | Inspector|
+--------------------------------------------------------------------+----------+
```

### Composite Canvas (6-layer z-stack, see CHARTER: DETECTION RENDERING SPEC for per-layer detail)

- **Layer 0 — Camera feed:** `<video>` element (Phase 6+) OR animated grid placeholder when SRC=MOCK. Placeholder = subtle radial gradient + 64px grid lines at 8% opacity + "AWAITING VIDEO FEED" 10px mono semi-transparent center text. Toggleable via `v`
- **Layer 1 — Trajectories:** SVG `<polyline>` per slot from `trajU/trajV` ring (16 samples). Per-class color with opacity gradient
- **Layer 1.5 — Actuator path history:** SVG from `actuatorPool` ring (4096 samples). Raw polyline (amber #ff8a4c 50%) + Smoothed Bezier (cyan #00f0ff 80%). Render mode per active profile
- **Layer 2 — Detection boxes + FOV ring:** per-class colored bboxes per Detection Rendering Spec. Plus FOV ring (dashed orange circle centered at cx,cy radius = profile.actuation.targeting_fov_radius_px). EKF uncertainty ellipses overlay per slot
- **Layer 3 — Hierarchy edges:** dashed grey lines from each track to its parent track via parent_id lookup
- **Layer 4 — Selection + Crosshair:** orange outline on selected slot, anchor crosshair per ss11.3 resolution, target-lock crosshair (cyan unlocked, red armed) plus slew indicator arrow when actuator engaged

### Layer Visibility Toolbar (floating bottom-left of canvas)
- 6 toggle buttons, each 32x32px, icon-only
- Eye-open / eye-closed icons via Foxglove Image Overlays convention (R4)
- Per layer: name tooltip + hotkey hint
- Hotkeys: `v` = Layer 0, `o` = Layers 1-3 (group toggle for "overlay")
- Active state: orange tint
- Right of toggles: 3 controls per Foxglove Image Overlays (R4):
  - Opacity slider (0-100%) — applies to selected layer
  - Blend mode dropdown: Alpha / Add
  - Pixel alpha checkbox (for mask layers)

### Floating Model Card (top-right of canvas, Roboflow pattern)
- 280x80px floating card, dark bg #161618cc with backdrop-blur, 1px border #27272a, rounded 6px
- Layout:
  ```
  yolo26m-roc-humanoid v0.1
  84.1% mAP  92.7% Prec  77.4% Recall
  INF p50: 4.2ms  p99: 18.7ms
  [Switch model...]
  ```
- Stats: mAP / Precision / Recall pulled from manifest YAML (when available, else "---")
- Inference latency: live computed from inter-frame deltas in trackPool
- Switch model button: opens Models drawer
- Mono font for all numbers, sans for labels

### Object Count Badge (bottom-right of canvas)
- 200x32px floating pill, dark bg with backdrop-blur
- Layout: `[●●●●●● 8 objects detected]`
- Color dots = per-class count (one dot per active track, color-coded)
- Number = `trackPool.activeCount`
- Click → opens/focuses Detection JSON Feed
- Hover → per-class breakdown popover: "Person: 3, Head: 3, UpperTorso: 2"

### Canvas Status Bar (top-left, semi-transparent dark pill)
- Per CHARTER: DETECTION RENDERING SPEC (existing fields enhanced with color-coded NIS/BND)

---

### Operate Right Rail (280px, always visible)

#### Section 1: DETECTIONS (Roboflow-style live JSON feed) — top 40% of rail

Header bar:
- "DETECTIONS" 10px mono uppercase letter-spaced
- Count badge right-aligned: "8 OBJ" mono orange
- Auto-scroll toggle icon
- Copy JSON button (clipboard icon)

Body: scrollable list of detection cards, one per active track in `trackPool`. Each card 88px tall:
```
+--------------------------------------+
| ● Person          89.7%             |  <- color dot + name + confidence right-aligned mono
| 0xa3f240e1  age 47  hits 45         |  <- mono small grey
| u 635  v 152  w 74  h 78            |  <- bbox in mono, centroid form
| vel 12.4,-2.1   anchor: bbox_center |  <- velocity + anchor selector inline
+--------------------------------------+
```

Card states:
- Default: dark bg #1a1a1e, 1px border #222
- Hover: orange left border 3px + canvas bbox gets temporary orange outline
- Selected: orange left border 4px + filled orange tint 5% bg
- Stale (not updated last frame): 60% opacity
- Hierarchy violation (status flag bit 3): red left border + flash animation

Click card: selects that track, fills Section 3 (Selected Track Inspector) below
Right-click card: context menu (Replay last 30s / Mark for review / Export track history / Lock actuator) per Palantir Gotham pattern

Empty state: 4 skeleton cards with `---` field placeholders + "Awaiting detections..." mono center text

#### Section 2: THRESHOLDS (Roboflow-style filter controls) — middle 30% of rail

Header: "THRESHOLDS & DISPLAY"

Controls (each row tightly packed at 28px height):
- **Confidence Threshold** slider (0-100%, default from manifest `thresholds.score * 100`, e.g. 25%). Drag → instantly filter visible bboxes on canvas. Value display right-aligned mono orange "50%"
- **Overlap (IoU) Threshold** slider (0-100%, default 45%). Drag → controls NMS preview filtering. Value mono "45%"
- **Label Display Mode** dropdown:
  - Draw Confidence
  - Draw Labels
  - Draw Both (default)
  - None
- **BBox Style** segmented control (4 buttons): Solid | Dashed | Corners | Outline
- **Class Visibility** — 6 checkbox rows with class color swatch + name + live count:
  ```
  [✓] ● Person         (3)
  [✓] ● Head           (3)
  [✓] ● UpperTorso     (2)
  [ ] ● AccessoryHat   (0)
  [ ] ● AccessoryBackpack (0)
  [ ] ● Unknown        (0)
  ```
- **Display Toggles** (compact 2-column grid of icon toggles):
  - Show Track ID
  - Show Trajectory
  - Show Anchor Crosshair
  - Show Velocity Arrow
  - Show Uncertainty Ellipse
  - Show Hierarchy Edges
  - Show FOV Ring
  - Show Actuator Trace

#### Section 3: SELECTED TRACK (drill-down inspector) — bottom 30% of rail

If no selection: skeleton card with field schema, dimmed
If track selected (from canvas click or JSON feed card):
- Header: track_id (hex mono) + class name + close button
- Fields stacked vertically, each 20px row, label left + value right (mono):
  - Class: `Person`
  - Confidence: `89.7%`
  - Age: `47 frames`
  - Hits/Misses: `45/2`
  - BBox (px): `635, 152, 74, 78`
  - Position (m): `2.31, 1.08, 5.42` from pWorld
  - Velocity (m/s): `0.12, -0.02, 0.03` from vWorld
  - Acceleration: `0.01, 0.00, 0.00` from aWorld
  - Uncertainty diag: `0.04, 0.04, 0.16` from pPosDiag
  - Parent: `0xb1c2...` (clickable) or `none`
  - Status flags: pill badges per active bit (CONFIRMED / BOUNDARY / STATIONARY / HIERARCHY)
- **Targeting Anchor** 6-option segmented control per ss11.3:
  ```
  [top] [top-center] [center] [center-low] [bottom-center] [consensus]
  ```
  Active highlighted orange. Change sends `OperatorCommand{kSetTargetingAnchor=7}` to C++. Live preview crosshair updates on canvas immediately
- **Minimap** (120x80px): thumbnail showing track position on canvas (orange dot on dark grid)
- Footer buttons: `[Lock Actuator]` (disabled unless Engage+Armed) | `[Export History]` | `[Mark for Review]`

### Interaction Model on Canvas (per R5 + Foxglove)
- **Single-click bbox:** SELECT that track (orange outline + fills Section 3)
- **Double-click bbox:** LOCK target (Engage destination only, requires Armed, triggers confirmation modal)
- **Click empty canvas:** DESELECT
- **Right-click bbox:** context menu (Palantir Gotham style)
- **Tab / Shift+Tab:** cycle through selectedSlot
- **Alt+L:** toggle lock on selected
- **Esc:** cancel selection / dismiss modal
- **`g`:** enter click-to-lock cursor mode (crosshair cursor, next click locks)
- **Foxglove numeric scrub:** click-drag horizontal on any number input in right rail scrubs the value
- **Hover detection in JSON feed:** highlights bbox on canvas with orange outline temporarily

### Bottom Time Cursor (Operate only, Phase 6+ placeholder)
- 32px bar spanning content width
- Foxglove playback styling: track + draggable handle at "NOW"
- Label: `LIVE 00:14:22 / 00:30:00`
- Disabled drag handle for now with tooltip "Time scrubber available in Phase 6+"
- Hotkeys reserved: Space (play/pause), ←/→ (100ms seek), Shift+←/→ (10ms), Alt+←/→ (500ms), Home/End

---

## DESTINATION 2: INSPECT (hotkey `2`) — THE FOXGLOVE-GRADE DEBUG WORKSPACE

**Mental model:** This is where the operator drills into tracking quality, model health, pipeline state, hierarchy graph, biometric signatures. NOT just a panel grid. Has its own **left sub-drawer** (Hierarchy Explorer, Palantir Gotham style) + **bottom Event Log** + **featured biometric panel** + companion grid.

### Visual Layout

```
+--------+-------------------------------------------+----------+
|HIERARCH|                                            | RIGHT    |
|EXPLORE |  +--------- FEATURED PANEL ---------+    | RAIL     |
|DRAWER  |  |   MouseDynamicsInspector         |    |          |
|(left   |  |   (full-width, ~50% height)       |    | Focused  |
| sub)   |  |   [c|d|t|s] [R] [X] [P]          |    | Panel    |
|        |  +------------------------------------+    | Settings |
|+ Track |  | Velocity | Jerk | Fitts | SubPixel|   |          |
|  hier  |  +-----------+------+--------+--------+   | (changes |
|  tree  |                                            |  per     |
|        |  +---------------+---------------+        |  active  |
|        |  | Spectator     | Pipeline      |        |  panel)  |
|        |  | TelemetryMtrx | Graph         |        |          |
|        |  +---------------+---------------+        |          |
|        |  | ModelInspector| TopicGraph    |        |          |
|        |  +---------------+---------------+        |          |
+--------+-------------------------------------------+----------+
|          EVENT LOG / Console Bottom Dock (collapsible 32px↔240px)|
+--------------------------------------------------------------+
```

### Hierarchy Explorer Left Sub-Drawer (Palantir Gotham style, 240px, collapsible via `[`)

Header: "TRACK HIERARCHY" + filter dropdown (All / Confirmed / Tentative / Violations)

Body: tree view of all tracks with parent_id graph. Indented rows:
```
▼ 0xa3f2 Person   age:47 conf:0.89  ✓
  ├ 0xb4c1 Head        age:45 conf:0.92  ✓
  ├ 0xc5d2 UpperTorso  age:46 conf:0.84  ✓
  └ 0xd6e3 AccessoryBackpack age:30 conf:0.71  ⚠ HIERARCHY VIOLATION
▼ 0xe7f4 Person   age:12 conf:0.66
  └ 0xf8g5 Head        age:10 conf:0.55
```

Per-row fields: track_id (hex mono) | class (color dot) | age | confidence | status icon
Hierarchy violation rows: RED left border + red row bg at 8% alpha + warning icon

Right-click on any row: pivot menu (Palantir Gotham pattern):
- "Replay this track's last 30s" (Phase 6+)
- "Lock actuator on this track" (Defensive Engagement profile only, opens confirmation)
- "Mark for review" (adds note + timestamp to event log)
- "Export track history" (downloads JSON with full track lifecycle)
- "Focus on canvas" (selects in Operate canvas)

Search input at top: filter by hex track_id substring

Footer: "{n} tracks / {m} confirmed / {v} violations" mono counter

### Featured Panel: MouseDynamicsInspector (per ss6.1, expanded)

Card 100% width of Inspect content area, ~50% height. Prominent header bar (44px tall):

```
+-----------------------------------------------------------------------+
| MOUSE DYNAMICS INSPECTOR  [c|d|t|s]  α 0.430  [● R] [X] [P]  00:14 |
|                                              SAMPLES: 87,500          |
|                                              BUFFER: ▓▓▓▓▓▓▓░░ 73%   |
|                                              [DIRTY]                  |
+-----------------------------------------------------------------------+
```

Header zones:
- Left: Panel title 11px mono uppercase
- Center: Mode segmented control (4 buttons, see ss6.1)
- Center-right: Alpha readout "α 0.430" from latest v2 frame, mono orange
- Right: Recording strip — `[R]` (idle/pulsing red), `[X]` (disabled when not recording), `[P]` (pause toggle), session timer mono, sample counter mono updating at 10 Hz, buffer fill bar (80px progress), session label badge

Body per mode (see ss6.1 spec). Below trace: derived feature strip (4 panels horizontally, see next section)

### Derived Feature Strip (4 mini-plots, beneath MouseDynamicsInspector, each ~25% width)

Per ss6.3 with detailed specs:

**P1 VelocityProfilePanel** (240px tall mini):
- Header: "VELOCITY PROFILE (P1)" + window selector (100/250/500/1000ms inline)
- Canvas 2D chart, 8s rolling window, F01 from actuatorPool
- Y-axis: px/s, X-axis: time (right-edge = now)
- Vertical bands at click events (from flags bit 2/3) in dim yellow
- Right-hand mini-readout: "peak: 920.1" / "mean: 421.5" / "p99: 920.1" mono small

**P2 JerkSpectrumPanel** (240px tall mini, v1 simplified):
- Header: "JERK + TREMOR (P2)" + log-Y toggle
- v1: Canvas 2D line chart of jerk magnitude, last 4s, log-Y
- 8-12 Hz tremor band: cyan horizontal stripe annotation
- Mini-readout: "peak: 12340" / "tremor: 0.34" / "in-band: ✓ HUMAN"
- Phase 6+: full spectrogram

**P3 FittsResidualPanel** (240px tall mini):
- Header: "FITTS RESIDUAL (P3)" + acquisitions counter
- Canvas 2D scatter (ID on X, MT on Y)
- OLS regression line when N >= 5
- Outliers (>3σ below): red filled circles
- Mini-readout: "N=12 / a=85 / b=210 / r²=0.84"

**P7 SubPixelPanel** (240px tall mini):
- Header: "SUB-PIXEL RATIO (P7)"
- Big number 36px mono orange: "0.353"
- Below: horizontal bar gauge with Human/Bot/Ambiguous bands (color-coded backdrop)
- Below gauge: sparkline 60s of ratio values
- Footer: "Human: 0.70-0.95 / Bot: 0.00-0.10"

### Companion Panel Grid (2 columns, below Featured + Derived strip)

**SpectatorTelemetryMatrix** (full ss13.5 layout):
- KPI header strip 32px (Window / Corr peak / Lag / Phase)
- Main dual-line chart 240px (actuator vs bg-odometry velocity)
- Cross-correlation strip 80px (lag -10..+10, Pearson rho, baseline bands)
- Bottom KPI strip 24px (Amplitude ratio / Phase coherence / Snap events)

**PipelineGraph Panel** (NEW per phase_5_console_robotics_ml.md §5.1 Rec 2):
- Header: "PIPELINE GRAPH"
- Directed graph rendered as SVG:
  ```
  [Camera] →30 fps→ [Inference] →28 fps→ [Tracker] →28 fps→ [Output Sink]
              ●OK              ●OK               ●OK              ●NO-OP
              4.2ms p50        12.7ms p50        0.8ms p50        ---
  ```
- Each node: rounded rect, name + status dot (● green/amber/red), latency tooltip on hover
- Each edge: FPS label above arrow
- Run/Stop/Pause node badges (Phase 6+ control via OperatorCommand)
- Click node: opens node-specific config in right rail (model details, tracker params, sink config)

**ModelInspector Panel** (NEW per phase_5_console_robotics_ml.md §5.1 Rec 3):
- Header: "MODEL INSPECTOR" + currently loaded model name
- Body: scrollable table of layers, sortable columns:
  - Layer Name (mono)
  - Op Type (Conv2D / Linear / Softmax / etc)
  - Input Shape
  - Output Shape
  - EP Assignment (color-coded: CUDA green, DML blue, OpenVINO purple, CPU grey)
  - Latency ms (mono right-aligned, bar chart background)
- Top summary: Total Latency / Memory / Parameter Count / Quantization
- Phase 6+: real ORT profile JSON; v1: schema placeholder rows derived from manifest

**TopicGraph Panel** (NEW per phase_5_console_robotics_ml.md §5.1 Rec 7):
- Header: "WS TOPIC GRAPH" + reconnect button
- Visual: Publisher↔subscriber map of WS channels with opcode labels
- Nodes: C++ Orchestrator (publisher) + Dashboard panels (subscribers)
- Edges: opcode (v1 tracks / v2 actuator / v3 bg-odometry) + frame rate
- Foxglove TopicGraph pattern

### Event Log Bottom Dock (collapsible 32px ↔ 240px, hotkey: `\``)

Collapsed state (32px): single mono line showing last event
```
[14:23:45.123] INFO  track 0xa3f2 spawned (Person, conf 0.89)
```

Expanded state (240px tall):
- Header bar: tabs Console | Network | Performance + auto-scroll toggle + search filter + level dropdown (DEBUG/INFO/WARN/ERR)
- Body: scrolling event list, each row 18px:
  - Timestamp (mono grey)
  - Level pill (DEBUG=grey, INFO=cyan, WARN=orange, ERR=red, CRITICAL=red+bold pulse)
  - Source tag (in brackets: [orchestrator] / [ws] / [decoder] / [tracker] / [user])
  - Message (mono)
- Events surfaced:
  - `track_spawned`, `track_lost`, `hierarchy_violation`
  - `nis_degraded`, `boundary_present`
  - `config_change` (slider adjustments with old → new value)
  - `recording_start`, `recording_stop`, `export_complete`
  - `scenario_switched`, `profile_switched`, `model_loaded`
  - `ws_connected`, `ws_disconnected`, `ws_error`
- Right-click event: jump time cursor to that timestamp (Phase 6+) / copy to clipboard / pin event

### Inspect Right Rail (260px) — Per-Panel Settings

When user clicks any panel card or focuses via `,` hotkey, right rail shows:
- Header: panel name + cog icon
- Body: panel-specific settings per ss6.3 control specs (smoothing toggle, window size, regression type, log-Y toggle, etc.)
- Empty state when no panel focused: "Click a panel header to view settings" + list of available panels with quick-jump links

### Bottom Time Cursor: hidden in Inspect (only in Operate per spec)

---

## DESTINATION 3: CALIBRATE (hotkey `3`)

**Layout:** Wizard sidebar (200px) left + content area right

**Wizard sidebar — 5 steps, vertically stacked:**
Each step is a clickable row:
- Step number in a circle (22px diameter, mono font)
  - Active step: orange bg #f5a623, dark text
  - Inactive: dark bg #222, grey text
- Step label (12px, cream when active, grey when inactive)
- Status dot (8px, right-aligned):
  - Green #34d399 = step complete (has valid data)
  - Red #ef4444 = step incomplete (needs configuration)

Clicking a step switches `activeStep` state and renders that step's content.

**Step 1: Camera Intrinsics** (red dot — always incomplete until Phase 6+)
- Header: "Camera Intrinsics"
- Subtitle: "Lens distortion model and focal length parameters"
- Fields (all read-only display for now, labelled inputs):
  - Focal Length X (px): `<input type="number" readonly>` showing "---"
  - Focal Length Y (px): `<input type="number" readonly>` showing "---"
  - Principal Point X (px): `<input type="number" readonly>` showing "---"
  - Principal Point Y (px): `<input type="number" readonly>` showing "---"
  - Distortion K1: `<input type="number" readonly>` showing "0.0"
  - Distortion K2: `<input type="number" readonly>` showing "0.0"
  - Distortion P1: `<input type="number" readonly>` showing "0.0"
  - Distortion P2: `<input type="number" readonly>` showing "0.0"
- Button: "Load from calibration file..." (disabled stub)
- Footer text: "Camera calibration loads from a YAML file generated by OpenCV's calibrateCamera(). Phase 6+ will add a live calibration wizard with checkerboard detection."

**Step 2: Camera Extrinsics** (red dot)
- Header: "Camera Extrinsics"
- Subtitle: "Camera-to-world rotation and translation"
- Display: 3x4 matrix grid showing rotation (3x3 identity) + translation (3x1 zeros)
  - 12 cells in a CSS grid, mono font, read-only
- Button: "Load from pose file..." (disabled stub)
- Footer text: "Extrinsic parameters define the camera's position and orientation in world space. Required for 3D reconstruction and multi-camera setups."

**Step 3: EKF Tuning** (green dot — has defaults)
- Embeds the existing `KinematicTrajectoryControllerPanel` content:
  - Smoothing Coefficient (alpha) slider: range 0.05-1.00, step 0.05, orange thumb, value display in mono orange. `onchange` sends `OperatorCommand{kSetSmoothingAlpha}` to C++ via `telemetrySocket.setSmoothingAlpha()`
  - NIS Gated Deadband Scaling slider: range 0.5-5.0, step 0.1, value display "{n}x"
  - Target Profile Presets dropdown: Standard Kinematic Match / High-Acceleration Tracking / Smooth Interpolation Blend
  - **NEW controls to add (from ss11.2):**
    - FOV Radius (px) slider: range 0-2000, step 10, default 360. Shows live value. `onchange` sends `OperatorCommand{kSetFovRadiusPx}`
    - Deadzone Radius (px) slider: range 0-200, step 1, default 2. `onchange` sends `OperatorCommand{kSetDeadbandRadiusPx}`
    - Velocity Curve Exponent slider: range 0.25-4.0, step 0.25, default 1.0. Labelled: "1.0 = linear, 2.0 = quadratic ease-in, 0.5 = sqrt ease-out"
    - Acceleration Curve Exponent slider: range 0.25-4.0, step 0.25, default 1.0
    - Max Command dX/tick input: number, default 40
    - Max Command dY/tick input: number, default 40
  - Reset to Defaults button

**Step 4: Model Selection** (red dot)
- Header: "Model Selection"
- Subtitle: "Select the ONNX model and manifest for inference"
- List of available manifests (same data as Models drawer but inline):
  - Each model row: name (mono), class count, input shape, EP hint, radio button to select
  - Active model highlighted with orange left border
- Currently loaded model display: name + status badge
- Button: "Apply Model" (sends model path to C++ orchestrator — stub for now)

**Step 5: Save as Scenario** (red dot)
- Header: "Save as Scenario"
- Subtitle: "Persist the current calibration + tuning as a reusable Scenario"
- Fields:
  - Scenario Name: `<input type="text" placeholder="My Custom Scenario">`
  - Description: `<textarea placeholder="Optional description...">`
  - Includes checkboxes: Intrinsics, Extrinsics, EKF Tuning, Model, Recording defaults (all checked by default)
- Button: "Save Scenario" — writes to `scenarioStore` + localStorage
- Below: list of existing saved scenarios with delete buttons

---

## DESTINATION 4: ENGAGE (hotkey `4`) — THE SAFETY-CRITICAL ACTUATION CONSOLE

**Mental model:** Identical canvas to Operate (same component instance, not duplicated) PLUS the safety-interlock chrome wrapped around it. Until all 4 lights green, every actuator control is hard-disabled, not just greyed. This is the "armed" mode where commands actually leave the dashboard.

### Visual Layout

```
+--------------------------------------------------------------------+----------+
| [●Camera CONN] [●Tracker 6trk] [●Auth NotCfg] [●Armed OFF]  AC OFF| RIGHT    |
+--------------------------------------------------------------------+ RAIL     |
|                                                                    |          |
|     [SAME COMPOSITE OPERATE CANVAS — fills middle column]         | Active   |
|     6-layer z-stack with bboxes, anchors, FOV ring, etc.           | Locks    |
|     Plus: target-lock crosshair becomes RED when armed             | List     |
|     Plus: slew indicator arrow on canvas                           |          |
|                                                                    | Locked   |
|                                                                    | Track    |
|                                                                    | Inspector|
+--------------------------------------------------------------------+----------+
| HARDWARE CONFIG (collapsed by default; expands ONLY when 4 lights green)     |
| Serial: COM3 | Baud: 921600 | [EMERGENCY STOP] | Slew: ◢ 12px/tick           |
+------------------------------------------------------------------------------+
| TARGET LOCK CONFIRMATION (shown only when Alt+L pending)                     |
| Lock target 0xa3f2 Person (anchor: bbox_center, dist 13.1px)?               |
| [HOLD TO CONFIRM (Skydio pause-slider, 800ms hold)]   [CANCEL Esc]          |
+------------------------------------------------------------------------------+
```

### Safety Interlock Bar (48px, top of Engage content, ALWAYS visible)

4 lights arranged horizontally with full status text per light:

**Camera Light:**
- Source: `telemetrySocket.state === 'connected'`
- States: ● CONNECTED (green glow) / ● DISCONNECTED (grey)
- Latency badge inline: "12ms" mono when connected

**Tracker Light:**
- Source: `trackPool.activeCount > 0 && !hasHierarchyViolation`
- States: ● HEALTHY (green) / ● {n} TRACKS (green) / ● NO TRACKS (grey) / ● VIOLATION (red)

**Auth Light:**
- Source: `profileStore.active.authenticated`
- States: ● AUTHENTICATED (green) / ● NOT CONFIGURED (amber - v1 default) / ● FAILED (red)
- v1: always amber "NOT CONFIGURED" with link to Settings drawer Auth tab

**Armed Toggle (the actual button):**
- Default state OFF on mount, every layout change, every page load (mandatory per spec)
- States:
  - OFF: dark bg, red dot, "● ARM SYSTEM" text, click triggers two-stage confirm
  - PENDING: amber pulse 800ms, "CONFIRM ARM?" + inline [Confirm] [Cancel] buttons
  - ARMED: green bg, green dot + 4px green glow pulse, "● ARMED" text, click → instant disarm
  - REPLAY-DISABLED: dark red bg, red dot, "DISABLED IN REPLAY" non-interactive

Above the bar (1px line below): green text "ACTUATOR ENABLED" when all 4 green, red italic "All 4 lights required for actuation" otherwise

### Center: Composite Canvas (shared with Operate)

Same `<OperateCanvas>` component, same 6 layers, same controls. Key differences when in Engage:
- Layer 4 target-lock crosshair: cyan when not locked, RED with pulse when armed + locked
- Slew indicator: SVG arrow from canvas center to (canvas_center + actuator_command_delta * scale), shows current actuator output direction
- FOV ring: more prominent (60% opacity instead of 30%)

### Engage Right Rail (260px)

**Section 1: ACTIVE LOCKS** (per game-CV §R5 "active locks panel with immediate undo")
- List of currently-locked tracks (one card per lock)
- Each lock card: track_id + class + anchor + lock-duration timer + `[UNDO]` button
- When no locks: "No active locks" + skeleton card

**Section 2: LOCKED TRACK INSPECTOR**
- Same Track Inspector as Operate right rail Section 3, but shows the LOCKED track instead of selected
- Adds: "Distance from anchor" / "Slew rate to target" / "Predicted intercept time" mono fields
- `[Release Lock]` button at bottom

**Section 3: HARDWARE CONTROL** (only when all 4 lights green)
- Serial Port dropdown (COM1-4, /dev/ttyUSB0-1, /dev/ttyACM0)
- Baud Rate dropdown (115200, 230400, 460800, 921600)
- Output Sink dropdown (no_op, hid_mouse, dynamixel_servo, pwm_pantilt, modbus, logfile_csv)
- Sink URI input (e.g. "COM3", "/dev/ttyUSB0@921600", "192.168.1.50:502")
- Test Hardware Diagnostic Ping button
- Last ping result display (mono)

### Bottom Hardware Bar (when armed, 48px collapsible)

Compact horizontal layout when Section 3 active:
- Serial connection status pill: "COM3 @ 921600 / OK 12ms ack"
- Slew indicator: SVG arrow showing current actuator vector (from latest v2 frame raw_u - clean_u, raw_v - clean_v)
- EMERGENCY STOP button: red, 80px wide, always prominent
  - Click: sends `OperatorCommand{EmergencyStop}` immediately, no confirmation
  - Also bound to keyboard kill switch (Phase 6+)
  - Pulse-red animation when actuator is actively moving

### Target Lock Confirmation Modal (Skydio Pause-slider, anti-fat-finger)

Shown when user presses Alt+L on a selected track (Engage + Armed only):
```
+-------------------------------------------------------+
|  LOCK TARGET                                          |
|  Track: 0xa3f240e1 Person                             |
|  Anchor: bbox_center                                  |
|  Distance: 13.1px from canvas center                  |
|  Predicted intercept: 240ms                           |
|                                                       |
|  ┌─────────────────────────────────────────────┐    |
|  │ [HOLD TO CONFIRM] ████████░░░░░░░░░░░░░░░░░ │    |  <- 800ms hold slider
|  └─────────────────────────────────────────────┘    |
|                                                       |
|              [CANCEL (Esc)]                           |
+-------------------------------------------------------+
```

- HOLD TO CONFIRM: mouse-down starts 800ms fill animation. Release before complete = cancel. Complete = lock confirmed
- Esc cancels at any time (per R5: "modal Esc never sends actuator")
- Anti-fat-finger: cannot bypass via single click

### Hotkey wiring (Engage-specific)
- `Alt+L`: open Target Lock Confirmation modal for currently selected track
- `Space`: centre actuator on selected track (when armed, sends OperatorCommand)
- `Esc`: release lock / cancel any pending action (always safe-side)
- `g`: enter click-to-lock cursor mode

---

## DRAWERS

### Models Drawer (slides out from left rail, 340px wide)
- Header: "Models" + close X button
- Search input: filter by name (fuzzy substring match)
- View toggle: Tile / List buttons (top-right of header)
- **List view (default):** each model as a row card:
  - Name (mono, 13px)
  - Status badge: "Ready" (green) if manifest exists + ONNX file found, "Needs Training" (orange) if manifest exists but no ONNX, "Missing" (red) if neither
  - Meta line: "{n} classes - {shape} - {ep_hint}"
  - Click: expands inline to show full class list with targeting anchors per class
- **Tile view:** 2-column grid of compact cards showing name + status + class count
- Models listed: all 5 from `models/*.yaml`, properly parsed (class lists, EP hints, anchor defaults)
- "Load" button on each model (stub — will send model path to orchestrator)

### Settings Drawer (slides out from left rail, 320px wide)
- Header: "Settings" + close X button
- 3 section tabs: Profile | Hotkeys | Preferences

**Profile tab:**
- Active Profile display: name + description badge
- Smoothing Alpha slider + value readout
- Deadband Scaling slider + value readout
- FOV Radius (px) slider + value readout
- Output Sink selector: dropdown (no_op, hid_mouse, dynamixel_servo, logfile_csv)
- Active Profile label (read-only tag)
- EP Order display: ordered list showing current execution provider priority

**Hotkeys tab:**
- Full binding table: combo (keycap styled) | action label | category badge (coloured)
- Each row is the complete DEFAULT_BINDINGS list (16 bindings)
- "Rebind" button per row (stub — shows "Rebinding available in Phase 6+")
- Warning text at bottom: "Ctrl+R, Ctrl+W, Ctrl+S, Ctrl+F are reserved by the browser and cannot be rebound"

**Preferences tab:**
- Render Rate display: "144 Hz (vsync)" read-only
- Theme display: "Dark (Cursor)" read-only
- Wire Bridge URL: `ws://127.0.0.1:8765` read-only mono display
- Diagnostics Strip threshold tuning (future — placeholder text)
- Auto-load toggle: "Load last Scenario + Profile on startup" checkbox (wired to localStorage)

---

## OVERLAYS

### Command Palette (Ctrl+K)
Already built. Verify: fuzzy search works across destinations, scenarios, panels, actions. Arrow keys navigate, Enter selects, Esc dismisses

### Hotkey Cheatsheet (?)
Already built. Verify: shows all 16 bindings with coloured categories, Esc dismisses

### Operator Overlay (Alt+Z)
- Toggle: left rail collapses to 40px (icons only, no labels), right rail hides, top chrome shrinks to brand+badges only (dropdowns hidden), diag strip stays
- Canvas fills remaining viewport
- Remaining chrome at 75% opacity
- 120ms transition on all affected elements
- State persists in localStorage
- Esc or Alt+Z again exits

---

## SCENARIOS (6 — expand from current 2)

| ID | Label | Default Profile | Sink | Inspect Panels | Purpose |
|---|---|---|---|---|---|
| surveillance-shift | Surveillance Shift | Default | no_op | MouseDynamics, SpectatorTelemetry, VelocityProfile | Active humanoid tracking (DEFAULT on first run) |
| calibration-session | Calibration Session | Default | no_op | (none — Calibrate wizard active) | Setting up new rig |
| forensic-review | Forensic Review | Replay | no_op | MouseDynamics, SpectatorTelemetry, FittsResidual, SubPixel | Post-incident analysis |
| defensive-engagement | Defensive Engagement | Default | no_op | MouseDynamics, SpectatorTelemetry | Trained-operator actuator session |
| ac-research-clean | AC Research — Behavioural Profiling (Clean) | (alpha=0.35) | noop | MouseDynamics, SpectatorTelemetry, VelocityProfile, FittsResidual, SubPixel | Clean dataset capture |
| ac-research-dirty | AC Research — Behavioural Profiling (Dirty) | (alpha=0.15) | noop | MouseDynamics, SpectatorTelemetry, VelocityProfile, FittsResidual, SubPixel | Dirty dataset capture |

---

## PROFILES (4)

| ID | Label | EP Order | Recording | Actuator | Notes |
|---|---|---|---|---|---|
| default | Default (dev) | CPU, DML, OpenVINO | off | absent | Local development |
| prod-nvidia | Production NVIDIA | CUDA, DML, CPU | local MP4 | full caps | Requires CUDA EP build |
| prod-amd-intel | Production AMD/Intel | DML, OpenVINO, CPU | local MP4 | full caps | Cross-vendor |
| replay | Replay | (none) | playback only | hard-disabled | Pinned to REPLAY mode |

---

## HOTKEYS (complete table — verify all wired)

| Combo | Action | Category | Wired? |
|---|---|---|---|
| 1 | Operate | destination | YES |
| 2 | Inspect | destination | YES |
| 3 | Calibrate | destination | YES |
| 4 | Engage | destination | YES |
| v | Toggle video layer | layer | NO — must wire to canvas layer visibility |
| o | Toggle overlay layers | layer | NO — must wire |
| Ctrl+K | Command Palette | global | YES (intercepted by browser in Cursor — works in standalone) |
| Alt+Z | Overlay Mode | global | NO — must implement |
| Alt+L | Toggle target lock | global | NO — stub for Phase 6+ |
| Alt+R | Expand perf HUD | global | NO — stub |
| ? | Cheatsheet | global | YES |
| Escape | Dismiss | global | YES |
| r | Record | panel | registered but not wired to recording flow |
| x | Stop recording | panel | registered but not wired |
| c/d/t/s | Trace render modes | panel | wired in MouseDynamicsInspector already |
| , | Panel settings | global | NO — must wire to right rail |
| [ / ] | Toggle left/right rail | global | NO — must wire |
| Tab/Shift+Tab | Cycle tracked entities | global | NO — stub |
| Space | Centre on selected | global | NO — stub for Engage |
| g | Click-to-lock picker | global | NO — stub |

---

## 6. HIGH-END BIOMETRIC PANELS — FULL SPECIFICATION (anticheat_data_exposure_2026.md + Amendments v3, v4, v5)

Every panel below is registered in `panel_registry.ts`, wired to real SPSC ring buffer stores, and renders from the zero-GC typed-array pools. No mocks.

---

### 6.1 MouseDynamicsInspector (ss12.1 + anticheat P1-P8 featured surface)

**Registry ID:** `mouse-dynamics-inspector` | **Destination:** Featured full-width in Inspect | **Hotkey when focused:** `c`/`d`/`t`/`s`

**Data source:** `actuatorPool` global ring buffer (4096 samples, v2 wire frames at 500 Hz). Fields consumed per sample: `raw_u_px`, `raw_v_px`, `clean_u_px`, `clean_v_px`, `ts_ns`, `flags`, `applied_alpha`, `sink_kind`.

**Header bar controls (left to right):**
- Panel title "Mouse Dynamics Inspector"
- Mode segmented control: 4 buttons — `c` Clean (cyan cubic Bezier) | `d` Dirty (amber polyline) | `t` Both (overlap 60%/80% opacity) | `s` Side-by-side (split viewport, default)
- Alpha readout: `alpha live: {applied_alpha}` from latest v2 frame, mono, orange
- Recording controls:
  - `[R] Record` button: idle=dark, recording=pulsing red dot. On click: opens `PreRecordModal` — mandatory label selection (Clean / Dirty / Ambiguous / Unsupervised) before recording starts. Cannot dismiss without choosing (spec ss12.6)
  - `[X] Stop` button: disabled when not recording. On click: stops recording, triggers JSONL.gz + ZIP export via `JsonlWriter`
  - `[P] Pause/Resume`: toggle, amber when paused
  - Session timer: `00:00:00` mono counter
  - Sample counter: `{n}` mono, updates at 10 Hz (not per sample)
  - Buffer fill bar: thin 80px progress bar showing `actuatorPool.head / actuatorPool.capacity`
  - Session label badge: "CLEAN" (green) / "DIRTY" (red) / "AMBIGUOUS" (amber) / "UNSUPERVISED" (grey) — always visible so operator never forgets

**Trace viewport (per mode):**
- `s` (default): left half = clean trace (cubic Bezier through `clean_u/v` samples, cyan #00f0ff, 1.5px), right half = dirty trace (polyline through `raw_u/v`, amber #ff8a4c, 1.5px). Vertical divider
- `c`: full-width clean trace only
- `d`: full-width dirty trace only
- `t`: both superimposed — clean at 80% opacity, dirty at 60% opacity

**Derived feature strip** (docked below trace, 4 mini-plots in horizontal row, each ~25% width):
- See ss6.3 below for exact specs per mini-plot

---

### 6.2 SpectatorTelemetryMatrix (ss13.5 — Background Odometry Cross-Correlation)

**Registry ID:** `spectator-telemetry-matrix` | **Destination:** Companion panel in Inspect grid | **Hotkeys when focused:** `l` log-Y, `c` copy report, `s` snap flag, `[`/`]` window size

**Data sources:**
- `actuatorPool` ring (4096 samples, 500 Hz) — compute |velocity| = sqrt(dx^2 + dy^2) from consecutive-sample displacement, downsample to 120 Hz via boxcar average
- `bgOdometryPool` ring (1024 samples, 30 Hz) — read `bg_dx_px`, `bg_dy_px`, compute |bg velocity| = sqrt(bg_dx^2 + bg_dy^2)

**Layout (top to bottom):**

**KPI header strip (32px):**
- `Window: {n} s` — active correlation window (4/8/16/32 sec, cycled via `[`/`]`)
- `Corr peak: rho={value}` — Pearson rho at optimal lag, 3 decimal. Green if 0.85-0.98, red if < 0.6
- `Lag: +{n} frame` — argmax of cross-correlation. Green if ~+1, red if > +3 drift
- `Phase: {n}deg` — phase coherence converted to degrees

**Main dual-line chart (240px tall):**
- Series 1: |actuator velocity| (cyan #00f0ff), downsampled 500->120 Hz
- Series 2: |bg optical-flow velocity| (amber #ff8a4c), native 30 Hz
- Y-axis: px/sec. `l` hotkey toggles linear/log scale
- X-axis: time, last 8 sec (right edge = now), scrolling
- Vertical markers at config-change events (slider adjustments, shown as dashed grey lines with hover tooltip `alpha: 0.35 -> 0.15 at t=12.4s`)
- Hover crosshair: shows both series values at same timestamp

**Cross-correlation strip (80px tall):**
- Horizontal axis: lag in frames, -10..+10
- Vertical: Pearson rho, 0.0..1.0
- Bar chart with peak marker (orange dot on highest bar)
- Human baseline band: shaded green at rho 0.85-0.98 @ lag +1
- Automation alert band: shaded red at rho < 0.6 OR lag > +3

**Bottom KPI strip (24px):**
- `Amplitude ratio: {value}` — mean(|bg_vel|) / mean(|actuator_vel|)
- `Phase coherence: {value}` — magnitude of average per-frame phase-shift vector, 0.0-1.0
- `Snap events: {n}` — actuator velocity spikes with no bg response within +3 frames AND amp ratio < 0.3

**Compute:** Direct O(N*L) cross-correlation, 30 samples * 21 lags = 630 multiplies/frame. Inside RAF budget.

---

### 6.3 Derived Feature Mini-Plots (anticheat P1, P2, P3, P7 — biometric strip)

4 mini-plots docked beneath MouseDynamicsInspector, each ~25% width of the panel.

**6.3.1 VelocityProfilePanel (P1)**
- **Registry ID:** `velocity-profile` | **Hotkey:** `v` when focused
- **Data:** F01 tangential velocity `V = sqrt(Vx^2 + Vy^2)` from `actuatorPool` consecutive-sample central difference
- **Render:** Canvas 2D line chart, 8-second rolling window (4000 samples at 500 Hz), right-edge = now
- **Controls (right-rail):** smoothing band overlay toggle; window size 100/250/500/1000 ms
- **Visual encoding:** vertical bands at click events (from `flags` bit 2/3); bell-curve velocity = human, rectangular = bot
- **JSONL binding:** `sample.vx_px_s`, `sample.vy_px_s`, `sample.ax_px_s2`, `sample.ay_px_s2`

**6.3.2 FittsResidualPanel (P3)**
- **Registry ID:** `fitts-residual` | **Hotkey:** `f` when focused
- **Data:** F21 Fitts engagement-time residual. Per target-acquisition event: `ID = log2(2A/W)`, `MT` = observed engagement time. Event detected by transition from no-anchor to anchor-on-track in `actuatorPool` flags
- **Render:** Scatter plot (ID on X, MT on Y). Live OLS regression line `MT = a + b*ID` when N >= 5 acquisitions
- **Controls (right-rail):** regression type: linear / power-law / per-user; bubble size by `target_distance_px`; outlier threshold (default 3 sigma)
- **Outliers:** points > 3 sigma below regression line rendered red (faster-than-humanly-possible)
- **JSONL binding:** `feature.fitts_residual_ms`, `feature.fitts_index_of_difficulty`

**6.3.3 SubPixelPanel (P7)**
- **Registry ID:** `subpixel` | **Hotkey:** `s` when focused
- **Data:** F16 sub-pixel sampling ratio = fraction of consecutive `(u,v)` deltas where `|delta_u| < 1 OR |delta_v| < 1`. Rolling window of 500 samples
- **Render:** Big-number display 0.00-1.00 (36px mono, orange). Horizontal bar gauge. Sparkline (last 60 sec of ratio values)
- **Threshold bands:** Human 0.70-0.95 (green stripe), Bot 0.00-0.10 (red stripe), Ambiguous 0.10-0.70 (grey)
- **JSONL binding:** `feature.subpixel_sampling_ratio`
- **R11 integration:** During Calibrate wizard step 4.5, this panel's F16 value is checked: > 0.40 = unlock AC Research scenarios, < 0.40 = warning modal about pointer precision settings

**6.3.4 JerkSpectrumPanel (P2) — Phase 6+ full implementation, v1 placeholder with live jerk readout**
- **Registry ID:** `jerk-spectrum`
- **Data:** F03 jerk magnitude, F08 FFT peak frequency, F09 8-12 Hz tremor band power
- **v1 render:** Jerk magnitude time series (line chart, last 4 sec, log-Y). Header shows `Tremor band: {power}` readout from the windowed compute. 8-12 Hz band highlighted as a cyan stripe label
- **Full render (Phase 6+):** 2D spectrogram (time x freq x log-power), Hann window 256 samples, 50% overlap, update 10 Hz. Tremor band 8-12 Hz highlighted
- **Controls (right-rail):** linear vs log magnitude toggle; tremor highlight on/off
- **JSONL binding:** `feature.jerk_peak`, `feature.jerk_p99`, `feature.fft_peak_freq_hz`, `feature.micro_tremor_hz`, `feature.micro_tremor_in_human_band`

---

### 6.4 Additional Research Panels (P4, P5, P6, P8 — Phase 6+ per R7 build order)

These are registered in the panel registry NOW (component stubs) but full implementation deferred per R7.

**P4 ClickLandingPanel:** Target-relative scatter + Gaussian ellipses (50/90/99%). Data: F18 click landing vs target centroid from `sample.click_state`, `target_dx`, `target_dy`. Colour by target class (Head/UpperTorso/etc per ss11.3 anchors). Hotkey `c`.

**P5 PathEntropyPanel:** Shannon entropy H time series (F12) + curvature heatmap inset (F15). Bins 8/16/32 selectable. Higuchi k_max=8 toggle for F13 fractal dimension. Hotkey `h`. Data: `feature.path_entropy_shannon`, `feature.path_efficiency`, `feature.reversal_count`, `feature.fractal_dimension`.

**P6 InterArrivalHistogramPanel:** Log-x histogram of inter-arrival times (F05, F17). Bins at 1/125, 1/250, 1/500, 1/1000 sec (polling rate harmonics). Overlay N sessions. Hotkey `i`. Data: `feature.inter_arrival_jitter_ms`, `feature.sub_polling_uniformity`.

**P8 MultimodalAlignmentPanel:** Two-row Gantt of `vision_frame_index` aligned to `sample.ts_ns`. Toggle target-locked-only view. Hotkey `m`. Data: `sample.vision_frame_index`.

---

### 6.5 Updated Runtime Profiles (ss5.6 — corrected per user directive)

| ID | Label | EP Order | Recording | Actuator | Notes |
|---|---|---|---|---|---|
| default | Default (dev) | CPU, DML | Off | Hard-disabled | Local development, no hardware risks |
| prod-nvidia | Prod-NVIDIA | CUDA, TensorRT, CPU | JSONL + MP4 | Armed and Configured | Full hardware acceleration |
| prod-amd-intel | Prod-AMD/Intel | DML, OpenVINO, CPU | JSONL + MP4 | Armed and Configured | Cross-vendor acceleration |
| replay | Replay/Forensic | (none - offline) | Read-only | Hard-disabled | Strict playback mode |

### 6.6 Calibrate Step 4.5: Sub-Pixel Verification Protocol (R11)

Inserted between Model Selection (step 4) and Save as Scenario (step 5):
1. Prompt: "Move cursor slowly across the canvas for 5 seconds"
2. System computes F16 over the sweep window
3. If `F16 > 0.40`: green checkmark, AC Research scenarios unlocked
4. If `F16 < 0.40`: red warning modal — "Sub-pixel resolution insufficient. Enable 'Enhance Pointer Precision' in Windows mouse settings or check HID polling rate." Cannot proceed to step 5 until resolved or explicitly overridden
5. Session header writes `subpixel_verification_ratio: f32`

### 6.7 BEACON-Compatible Export Bundle (R4, R6, R8)

Per-session folder structure generated by `JsonlWriter` + JSZip:

```
<session_id>/
  session.jsonl.gz          # ss12.2 header + sample (500 Hz) + feature (5 Hz) events, gzip-on-write
  trajectory.png            # 224x224 greyscale (R5 default), last 4096 samples, polyline intensity proportional to local velocity
  hardware_info.json        # MAC/IP/hostname SCRUBBED per BEACON convention (C10)
  croissant.jsonld           # Google ML Croissant 1.0 per-session record
  video.mp4                 # OPTIONAL, off by default, ts_ns-aligned (C9 privacy)
```

Export modal checkboxes: JSONL (always on) | trajectory PNG | video MP4 | features-only (strips sample events)

### 6.8 Derived Feature Computation Specs (F01-F25 data pipeline)

**Live tier (inside actuator-telemetry-emit thread at 500 Hz, <=12 mul-adds/sample):**
- F01 tangential velocity: central difference on (u,v), 4 mul-adds
- F02 tangential acceleration: central difference on velocity
- F03 jerk: 5-point stencil `Jx[n] = (Vx[n+2] - 8*Vx[n+1] + 8*Vx[n-1] - Vx[n-2]) / (12*dt)`
- F16 sub-pixel ratio: `|delta_u| < 1 OR |delta_v| < 1` comparator, EMA with alpha=0.05
- F19 click-while-moving: `|v| > threshold` at click event

**Windowed tier (JSON-writer thread, 200ms windows = 100 samples):**
- F04 velocity profile asymmetry: segment detection, `alpha = t_peak / t_total`
- F05 inter-arrival CV: `sigma(dt) / mean(dt)`
- F06 click-after-stop latency: time from `velocity < 10 px/s` to `click_state=1`, human 80-250ms, bot 5-25ms
- F07 dwell time: total time at `velocity < 10 px/s`
- F08 FFT peak: 256-sample Hann window, `argmax(|FFT(vx)|^2)`. ~12k ops/window
- F09 tremor band: sum FFT power 8-12 Hz bins
- F10 sub-polling uniformity: CV of FFT amplitude across lower-half spectrum
- F11 path efficiency: `D / sum(|p_{i+1}-p_i|)`
- F12 Shannon entropy: `-sum(p_k * log2(p_k))` over 16-bin direction-angle histogram
- F14 reversal count: zero-crossings of Vx, Vy
- F15 curvature: `(x'*y'' - y'*x'') / (x'^2 + y'^2)^(3/2)`
- F17 polling harmonics: FFT amplitude at 125/250/500/1000 Hz bins
- F18 click landing: `(delta_u, delta_v) = click_pos - target_anchor_pos`
- F21 Fitts residual: `ID = log2(2A/W)`, live once (a,b) known
- F22 TCM: `(1/S_n) * sum(t_{i+1} * D_i)`
- F23 scattering: `(1/S_n) * sum(t_{i+1}^2 * D_i) - TCM^2`

**Offline tier (Python `roc_offline_features.py`):**
- F13 fractal dimension: Higuchi, k_max=8, window >= 256
- F24 approximate entropy: SampEn(m=2, r=0.2*sigma, N=100)
- F25 multi-pattern embedding: 1D-ResNet + GRU trained sequence

---

## EXECUTION ORDER

5 focused sessions. Each produces a visually verifiable, type-clean checkpoint.

### Session 1 — Operate Cockpit: Canvas + Right Rail + Model Card + Threshold Filters (~1200 LOC)

This session delivers the Roboflow-grade Operate destination, the heart of the product. By the end the user sees a real cockpit with bboxes drawn per class color, live JSON feed in the right rail, threshold sliders that visually filter the canvas, and a model card overlay.

**Deliverables:**

**A. Detection Rendering Foundation (CRITICAL — without this it still looks like wireframes)**
- `src/lib/render/class_colors.ts`: per-class color palette map (Unknown/Person/Head/UpperTorso/AccessoryHat/AccessoryBackpack), exported for use across canvas + JSON feed + threshold checkboxes
- `src/lib/render/bbox_renderer.ts`: pure helpers — `renderBbox(slot, opts)`, `renderLabel(slot, mode)`, `renderTrackIdBadge(slot)`, `renderAnchorCrosshair(slot, anchor)`, `renderVelocityArrow(slot)`, `renderUncertaintyEllipse(slot)`. Zero-alloc, mutates pre-allocated SVG nodes
- `src/lib/stores/render_settings.svelte.ts`: $state for confidence threshold, IoU threshold, label display mode, class visibility map, bbox style, all display toggles. Persisted to localStorage
- `src/lib/stores/selection_store.svelte.ts`: $state for selectedSlot, hoveredSlot, locked tracks list. Shared between canvas + right rail

**B. OperateCanvas.svelte (~500 LOC)**
- 6-layer z-stack composite: Layer 0 camera placeholder (animated grid + "AWAITING VIDEO FEED"), Layer 1 trajectories (per-class colored, opacity gradient), Layer 1.5 actuator trace (raw + clean from actuatorPool), Layer 2 detection boxes (per-class colored, per-status flags styling) + FOV ring, Layer 3 hierarchy edges (parent_id dashed lines), Layer 4 anchor crosshair + selection outline + target-lock crosshair + velocity arrow
- Reads from render_settings_store to filter what's drawn (confidence threshold, class visibility, display toggles)
- Reads from selection_store to highlight selected/hovered slot
- Click bbox: sets selectedSlot. Double-click: locks (Engage only). Right-click: opens context menu
- Tab/Shift+Tab cycles selectedSlot through active tracks
- Per-class colored bboxes following CHARTER: DETECTION RENDERING SPEC exactly
- Smart label placement (above bbox normally, below if near top of frame)
- Frame-level status bar top-left with NIS/BND color coding

**C. Layer Visibility Toolbar (floating bottom-left)**
- 6 toggle buttons + opacity slider + blend mode dropdown + pixel alpha checkbox per Foxglove Image Overlays R4
- `v` hotkey toggles Layer 0, `o` hotkey toggles Layers 1-3 group

**D. Model Card Overlay (floating top-right of canvas)**
- 280x80px floating card with mAP/Precision/Recall (from manifest), live INF p50/p99 (from frame timing), Switch model button
- Data sources: active manifest YAML parsed into models_manifest.ts + live computed inference latency

**E. Object Count Badge (floating bottom-right)**
- Pill showing trackPool.activeCount + colored dots per class + hover popover with per-class breakdown

**F. RightRail.svelte (~400 LOC, 280px)**
- Section 1 (40%): Live Detection JSON Feed — scrollable cards per active track with class color dot, name, confidence, hex track_id, age, bbox, velocity, anchor selector inline. Hover card highlights bbox on canvas. Click selects. Right-click opens context menu
- Section 2 (30%): Thresholds & Display — Confidence slider + IoU slider + Label Display Mode dropdown + BBox Style segmented + 6 class visibility checkboxes with live counts + 8 display toggle icons
- Section 3 (30%): Selected Track Inspector — class/conf/age/hits/bbox/pWorld/vWorld/aWorld/pPosDiag/parent/flags mono fields + 6-option Targeting Anchor segmented control (sends OperatorCommand{kSetTargetingAnchor}) + 120x80 minimap + Lock/Export/Mark footer buttons
- Collapsible via `]` hotkey, focusable via `,` hotkey
- Empty states everywhere are skeleton-shaped data, never blank

**G. NavigationShell.svelte grid update**
- `grid-template-columns: 56px 1fr 280px` (left rail + content + right rail)
- OperateCanvas + Operate Right Rail render together in Operate destination
- Add Bottom Time Cursor placeholder (32px disabled bar with "LIVE 00:00:00 / 00:00:00" + reserved hotkey hints)

**H. TopChrome.svelte enhanced**
- Add Profile dropdown next to Scenario dropdown, bound to new profile_store
- Profile shows current EP order + recording flag in tooltip
- LIVE/REPLAY badge stays, SINK badge gets per-sink color glow

**I. DiagnosticsStrip.svelte upgraded**
- All 6 tiles compute real values (not "---")
- Hover tooltip per tile: threshold definition + source name + 80x20 sparkline canvas of last 30 values + min/max/avg readout
- Click tile cycles text-only / text+graph mode (RTSS pattern)
- Zero-allocation RAF counter for FPS

**J. profile_store.svelte.ts**
- 4 profiles (Default / Prod-NVIDIA / Prod-AMD-Intel / Replay) per ss6.5
- $state activeProfileId + reactive `active` derived
- localStorage persistence

**Visual verification at end of Session 1:**
- Open the dashboard, see Operate destination
- Camera-feed-placeholder grid fills middle, mock data renders bboxes per class color
- Right rail shows live JSON feed updating every frame with all detection details
- Drag Confidence slider 25% → 80%, see bboxes disappear/reappear instantly
- Toggle Show Trajectory off, trails disappear
- Click a bbox, orange outline appears + right rail Section 3 fills with that track's data
- Tab cycles selection through tracks
- Model card top-right shows model name + stats + latency
- Object count badge bottom-right shows "6 objects detected"
- Diagnostics strip shows real FPS / track count / WS rate / inference latency

### Session 2 — Engage Destination Rebuild (~500 LOC)

**Deliverables:**
- Rewrite Engage section: compact 48px safety-interlock bar at top (4 lights: Camera/Tracker/Auth/Armed with reactive bindings to `telemetrySocket.state`, `trackPool.activeCount`)
- Armed toggle: actual two-stage button — click shows "Confirm Arm?" inline with [Confirm]/[Cancel]. Defaults OFF on mount. Disabled+red in REPLAY mode
- Center: embed shared `<OperateCanvas>` component (identical to Operate, same 6 layers)
- Bottom collapsible hardware panel: only visible when all 4 lights green. Contains: serial port dropdown, baud rate dropdown, kill switch button ("EMERGENCY STOP" — red, calls `telemetrySocket.sendCommand(EmergencyStop)`)
- Slew indicator: SVG arrow overlay on canvas showing current actuator command direction + magnitude from latest v2 frame `(raw_u - clean_u, raw_v - clean_v)` as vector
- Target-lock confirmation area: when track locked via Alt+L, shows track ID + class + anchor + distance + [Confirm Lock] / [Cancel] buttons (Skydio anti-fat-finger pattern)
- Wire `Alt+L` hotkey (Engage only, requires Armed=green): selects currently-hovered track for lock confirmation
- Wire `Space` hotkey (Engage only, Armed): centres actuator on selected track

### Session 3 — Inspect Destination: Foxglove-Grade Debug Workspace (~1100 LOC)

This session delivers the multi-panel debug surface with biometric panels, pipeline graph, model inspector, hierarchy explorer drawer, and event log bottom dock.

**Deliverables:**

**A. Inspect Layout Skeleton (~150 LOC)**
- Inspect destination uses CSS Grid: left HierarchyExplorerDrawer (240px collapsible) + center content (1fr) + right per-panel-settings rail (260px)
- Center content has 2 stacked sections: top = featured MouseDynamicsInspector + derived feature strip, bottom = 2-col companion grid
- Bottom EventLogDock absolute positioned 32px collapsed / 240px expanded, hotkey `\``

**B. HierarchyExplorerDrawer.svelte (~250 LOC) — Palantir Gotham style**
- Tree view rendering from trackPool parent_id graph
- Indented rows per nesting level (Person → Head → UpperTorso → AccessoryHat etc)
- Per row: track_id hex mono | class color dot | age | conf | status icon
- Hierarchy violation rows: red left border + red bg 8% alpha + warning icon
- Right-click row: pivot menu (Replay last 30s / Lock actuator / Mark for review / Export track history / Focus on canvas)
- Search input top, filter dropdown (All / Confirmed / Tentative / Violations)
- Footer counter: "{n} tracks / {m} confirmed / {v} violations"

**C. MouseDynamicsInspector.svelte (FULL implementation per ss6.1, replaces existing minimal stub) (~250 LOC)**
- Header bar 44px: title + mode segmented (c/d/t/s) + alpha readout + recording strip (R/X/P + timer + sample counter + buffer fill + label badge)
- Body 4 modes (clean cubic Bezier / dirty polyline / both overlap / side-by-side)
- Reads `actuatorPool.raw_u_px`, `raw_v_px`, `clean_u_px`, `clean_v_px`, `applied_alpha`, `flags`, `sink_kind`
- PreRecordModal flow: R click → mandatory label selection modal → recording starts on choose → updates buffer-fill bar + sample counter live
- X click → stops, triggers JsonlWriter export with BEACON folder bundle

**D. Derived Feature Strip (~250 LOC, 4 mini-plots per ss6.3 in horizontal row beneath MouseDynamicsInspector)**
- VelocityProfilePanel (P1): Canvas 2D 8s rolling line + window selector + mini-readout (peak/mean/p99)
- JerkSpectrumPanel (P2): Canvas 2D log-Y jerk time series + 8-12Hz cyan stripe + tremor readout (peak/in-band)
- FittsResidualPanel (P3): Canvas 2D scatter + OLS regression line + N=12/a=85/b=210/r²=0.84 readout + 3-sigma outliers in red
- SubPixelPanel (P7): big 36px mono number + horizontal bar gauge with Human/Bot/Ambiguous color bands + 60s sparkline + threshold footer

**E. SpectatorTelemetryMatrix.svelte (FULL ss6.2 layout, ~250 LOC)**
- KPI header strip 32px: Window / Corr peak / Lag / Phase
- Main dual-line chart 240px: actuator velocity (cyan, downsampled 500→120Hz boxcar) + bg-odometry velocity (amber, native 30Hz)
- Cross-correlation strip 80px: lag -10..+10, Pearson rho bars, peak marker, human baseline band (green) + automation alert band (red)
- Bottom KPI strip 24px: Amplitude ratio / Phase coherence / Snap events
- Hotkeys when focused: l (log-Y), c (copy report), s (snap flag), [/] (window 4/8/16/32s)

**F. PipelineGraph Panel (~120 LOC) — per phase_5_console_robotics_ml.md §5.1 Rec 2**
- SVG-rendered directed graph: Camera → Inference → Tracker → Output Sink
- Per node: rounded rect + class status dot + latency tooltip
- Per edge: FPS label
- Run/Stop/Pause node badges (Phase 6+ control)
- Click node: opens config in right rail

**G. ModelInspector Panel (~150 LOC) — per phase_5_console_robotics_ml.md §5.1 Rec 3**
- Header: model name + total latency
- Scrollable sortable table: layer name | op type | input shape | output shape | EP (color-coded) | latency ms (bar chart bg)
- v1: schema rows derived from manifest YAML; Phase 6+: real ORT profile JSON
- Memory footprint + parameter count summary at top

**H. TopicGraph Panel (~80 LOC) — per phase_5_console_robotics_ml.md §5.1 Rec 7**
- Foxglove TopicGraph pattern: pub-sub map of WS opcodes
- Nodes: C++ Orchestrator (publisher) + Dashboard subscribers
- Edges: v1 tracks / v2 actuator / v3 bg-odometry + frame rate

**I. EventLogDock.svelte (~200 LOC) — Bottom dock across destinations**
- Collapsed 32px: shows last event line mono
- Expanded 240px: tabs Console | Network | Performance + auto-scroll + search + level dropdown (DEBUG/INFO/WARN/ERR/CRITICAL)
- Body: scrolling 18px event rows with timestamp + level pill + source tag + message
- Event types: track spawned/lost, hierarchy violation, NIS spike, recording start/stop, scenario switched, profile switched, model loaded, ws connected/disconnected/error, config change with old → new
- Right-click event: jump time cursor (Phase 6+) / copy / pin
- Hotkey `\`` toggles
- Critical events get red pulse animation

**J. Per-panel right rail settings**
- When user clicks any panel card or focuses via `,`, right rail shows panel-specific settings per ss6.3 control specs

**K. Stub panels P4/P5/P6/P8**
- Each renders panel name + schema description + "Phase 6+" footer
- Registered in panel_registry so they show up in palette/scenarios

**L. Panel-specific hotkeys via focused-panel guard**
- v (P1 velocity), j (P2 jerk), f (P3 Fitts), s (P7 sub-pixel), l (Spectator log-Y), c (Spectator copy), h (P5 entropy when implemented), i (P6 inter-arrival), m (P8 multimodal)

**Visual verification at end of Session 3:**
- Open Inspect destination, see MouseDynamicsInspector featured at top with c/d/t/s controls + recording strip
- Below it: 4 mini-plots showing live data (velocity, jerk, fitts, subpixel)
- Below those: 2-col grid with SpectatorTelemetryMatrix + PipelineGraph (top row), ModelInspector + TopicGraph (bottom row)
- Left side: HierarchyExplorerDrawer collapsible with track tree
- Bottom: EventLogDock collapsed showing last event
- Press `\`` to expand event log, see events streaming
- Right-click a hierarchy row, see Palantir Gotham context menu
- Click a panel, right rail fills with that panel's settings

### Session 4 — Calibrate Wizard Complete (~500 LOC)

**Deliverables:**
- Make `activeStep` a `$state(3)`, clicking any step switches content. Active step has orange number circle, left accent border
- **Step 1 Camera Intrinsics:** 8 read-only numeric fields (fx, fy, cx, cy, k1, k2, p1, p2) in a 2x4 grid. "Load from calibration file" button (disabled stub). Footer explaining OpenCV calibrateCamera() pipeline
- **Step 2 Camera Extrinsics:** 3x4 matrix display (R|t) in CSS grid, 12 mono read-only cells. "Load from pose file" button (disabled stub)
- **Step 3 EKF Tuning (ss11.2 full control set):**
  - Smoothing Coefficient alpha: range 0.05-1.00, step 0.05, onchange sends OperatorCommand{kSetSmoothingAlpha=4}
  - NIS Gated Deadband Scaling: range 0.5-5.0, step 0.1
  - FOV Radius (px): range 0-2000, step 10, default 360. onchange sends OperatorCommand{kSetFovRadiusPx=6}. Labelled with live canvas preview (FOV ring updates)
  - Deadzone Radius (px): range 0-200, step 1, default 2. onchange sends OperatorCommand{kSetDeadbandRadiusPx=5}
  - Velocity Curve Exponent: range 0.25-4.0, step 0.25, default 1.0. Label: "1.0=linear, 2.0=quadratic ease-in, 0.5=sqrt ease-out"
  - Acceleration Curve Exponent: range 0.25-4.0, step 0.25, default 1.0
  - Max Command dX/tick: number input, default 40
  - Max Command dY/tick: number input, default 40
  - Target Profile Presets: dropdown (Standard / High-Acceleration / Smooth Interpolation)
  - Reset to Defaults button
- **Step 4 Model Selection:** inline model list (same data as Models drawer), radio select per model, "Apply Model" button, currently loaded model display
- **Step 4.5 Sub-Pixel Verification (R11):** prompt for 5s cursor sweep, compute F16 live, gate >= 0.40 to unlock AC Research scenarios, warning modal if insufficient, writes `subpixel_verification_ratio` to session header
- **Step 5 Save as Scenario:** name input, description textarea, include-checkboxes (Intrinsics/Extrinsics/EKF/Model/Recording), "Save Scenario" button writes to scenarioStore + localStorage, list of existing saved scenarios with delete
- Step status dots reactive: green when step has valid data, red when incomplete

### Session 4 — Calibrate Wizard: Mission Setup Workflow (~600 LOC)

Per existing Calibrate Wizard spec but ADD: source selector card at top of every step (Webcam / File / URL / Mock / Live tile row), step navigation breadcrumb at top, "previous step / next step" buttons at bottom, sub-pixel verification step 4.5 gate, save-as-scenario localStorage persistence, every input field has live preview on canvas (Calibrate also shows the canvas as a small overlay top-right so user sees changes immediately).

### Session 5 — Scenarios + Profiles + Overlay + Export + Polish (~600 LOC)

**Deliverables:**
- **6 Scenarios** in scenarios.ts: Surveillance Shift (default), Calibration Session, Forensic Review, Defensive Engagement, AC Research Clean, AC Research Dirty. Each with: profile overrides, inspect panel list, sink kind, recording defaults, model preference
- **4 Profiles** in new profile_store.svelte.ts: Default (CPU/DML, off, hard-disabled), Prod-NVIDIA (CUDA/TensorRT/CPU, JSONL+MP4, armed), Prod-AMD/Intel (DML/OpenVINO/CPU, JSONL+MP4, armed), Replay (none, read-only, hard-disabled)
- **Profile dropdown** added to TopChrome.svelte next to Scenario dropdown, bound to profileStore.activeProfileId
- **localStorage auto-load** (ss5.7): on mount, read last (scenarioId, profileId) pair. Default: (surveillance-shift, default). Write on every change
- **Alt+Z Overlay Mode:** toggle collapse — left rail shrinks to 40px icon-only, right rail hides, top chrome shows brand+badges only (dropdowns hidden), diag strip stays. Canvas fills viewport. Chrome at 75% opacity. 120ms CSS transition. State persists in localStorage. Esc or Alt+Z exits
- **DiagnosticsStrip hover tooltips:** each tile on hover shows: threshold definition, source binding name, 30-value sparkline (canvas 80x20px in tooltip). Click cycles text-only / text+graph mode
- **Bottom time cursor placeholder** (Operate only): 32px bar with "LIVE — time scrubber available in Phase 6" text, disabled drag handle, timestamp display
- **BEACON export integration:** update JsonlWriter to produce the per-session folder structure (session.jsonl.gz + trajectory.png stub + hardware_info.json scrubbed + croissant.jsonld stub). Export modal with checkboxes per ss6.7
- **Wire remaining hotkeys:** Alt+R (expand perf HUD), Tab/Shift+Tab (cycle entities), `[`/`]` (toggle left/right rail), `g` (click-to-lock picker mode)
- **Multi-feature radar placeholder:** register in panel_registry for Phase 6+ — normalized F04/F09/F11/F12/F21 radar polygon per session
