# Professional Operator-Console UX — Survey & Menu-Architecture Recommendations

> **ROC AI Vision** · UX Research Scientist (subagent deliverable)
> **Scope:** `dashboard/` — top-level menu architecture, panel taxonomy, live-feed placement
> **Sister docs:** `research_dashboard_tech_2026.md` (transport/render stack), `phase_5_telemetry_ui.md` (wire protocol), `dashboard/architecture_plan.md` (locked 4-domain matrix)
> **Cross-cutting rule files:** `.cursor/rules/200-dashboard-telemetry.mdc` (LOCKED, do not modify), `.cursor/rules/300-research-standards.mdc`
> **Last updated:** May 2026
> **Status:** Read-only research deliverable. Parent agent will synthesise with the parallel game-CV-overlay research output before any rules / architecture get touched.

---

## 0. Method

I surveyed twelve professional operator consoles spanning robotics telemetry, video-management systems (VMS), defence C2, drone GCS, live-broadcast composition, in-game overlay, observability, and reference CV / SOC tooling. For every system I used primary product documentation, GitHub READMEs, or peer-reviewed / industry-press write-ups, and recorded:

1. **Information architecture** — top-level navigation, panel taxonomy, persistence model (layout / scene / profile).
2. **Live-stream placement** — whether live capture is its own destination or a composited overlay surface.
3. **Operator ergonomics** — hotkeys, drag-resize, dock-undock, multi-window, multi-monitor.
4. **Configuration scope split** — what lives in "hot path" UI vs "config" UI vs "preset/profile" persistence.

Caching of all scraped primary sources is under `.firecrawl/dashboard_ux/` (15 files, ~430 KB markdown). Existing prior research caches in `.firecrawl/console-research/` (RViz user guide, Foxglove panel index, Milestone XProtect, Genetec Security Center, Avigilon, Hanwha, Hikvision, Palantir Gotham, Anduril, QGroundControl) were also mined.

**Cross-references to existing ROC research.** This doc deliberately *complements*, rather than overlaps, the existing dashboard-tech research:
- `research_dashboard_tech_2026.md` — covers *how* the dashboard renders (WebTransport vs WebSocket, SVG vs Canvas vs WebGL, Svelte 5 runes zero-GC store, SAB + Atomics, Foxglove + Rerun *implementation* architecture, Service-Worker caching, performance-profiling methodology). I treat that as solved.
- `phase_5_telemetry_ui.md` — covers the binary wire-protocol schema, the `trackPool` pool architecture, and the SVG render loop. I treat that as solved.
- This doc covers *what* the dashboard surfaces to the operator — the IA, panel taxonomy, hotkey contract, and the live-capture placement question that the brief explicitly asks. The two layers (how to render vs. what to render) are decoupled; recommendations here do not touch the render-stack layer.

**What this doc is NOT.** It is not a visual design spec (no mock screenshots), not a Svelte component design (no component-tree diagrams), and not a usability study (no operator interviews — that's a different deliverable). It is a *pattern survey + IA recommendation* designed to be input to the parent synthesis agent.

Every quantitative claim below is either (a) from a primary product doc, (b) a count I performed against the cited reference page, or (c) marked **observed** (qualitative). The exact accessed-date is in §11 References.

---

## 1. Executive Summary

1. **The "tabs as top-level domains" pattern that ROC currently uses is rare in the professional set.** Foxglove, RViz2, OBS, Grafana, QGroundControl, Skydio Remote Ops, ReShade, Datadog — all of these instead use a **persistent shell** (titlebar + sidebar + viewport + dock) and route the operator's *task* to a **named, savable workspace** (layout / scene collection / view / mission / preset). Tabs, where they exist, are either ① browser-style document tabs *inside* a workspace (Milestone XProtect, Genetec Security Desk) or ② grouping containers inside a single layout (Foxglove Tab panel). They are *not* the primary IA.
2. **Live capture is virtually never its own peer tab next to "Spatial Canvas".** It is the **center pane** of one workspace and is *composited* with detection overlays via z-ordered transparent layers. Tesla Autopilot, Skydio, QGC, DeepStream `tiled-display + OSD` plugin, RViz Camera display, Foxglove Image panel, Milestone XProtect Live tab — all treat raw video and overlay as the **same surface**, with the overlay rendered on top using either OpenGL/Skia (Tesla, Skydio AR), GStreamer OSD (DeepStream), or DOM-on-canvas (Foxglove Image annotations). The two-tab "video vs spatial" split that ROC's current matrix would imply is **not** a pattern that exists in production CV consoles. (See §10 addendum.)
3. **There is a hard separation between hot-path render surfaces and human-speed configuration.** This is *not* unique to ROC. Foxglove has its panel settings sidebar (`,` key) separate from the panel viewport. RViz has the Displays / Tool Properties / Views panels separate from the 3D canvas. OBS has Sources/Audio/Mixer docks separate from the preview/program. ROC's `TelemetryCanvas` vs `aiming_config` split is therefore **architecturally correct**, but the *naming* and *count* of the domains is what needs fixing.
4. **Layouts / scene-collections / presets are universal.** Every system in the survey can save a named workspace and reload it. Foxglove personal + organization layouts, RViz `.rviz` config, OBS Scene Collections + Profiles, Grafana Library Panels + Dashboard JSON, QGC Plan files, ReShade `.ini` presets, Datadog dashboard JSON. The operator-grade discriminator is that the *same operator* uses different presets for different *tasks*, not different settings within one session. ROC currently has no preset system; this is the single largest pattern gap.
5. **The current 4-domain matrix is structurally close to right, but the labels are wrong.** "Observer Matrix / Spatial Projection Canvas / Kinematic Trajectory Controller / Hardware Actuation Interface" maps poorly onto operator mental models from the surveyed systems. The closest real-world equivalent is RViz's three-pane shell (Displays · Viewport · Tools/Views) or Foxglove's (Sidebar · Layout · Settings) — neither uses 4 peer tabs. **Recommendation: collapse to one persistent shell with a primary "Operate" workspace + 3–4 secondary task workspaces (`Calibrate`, `Review`, `Models`, `Settings`) selected via a left rail, and a saved-layout / preset system layered on top.** Full detail in §6.

---

## 2. System-by-System Survey

### 2.1 Foxglove Studio

**Primary source:** Foxglove docs — Panels (`docs.foxglove.dev/docs/visualization/panels`) and Layouts (`docs.foxglove.dev/docs/visualization/layouts`).

| Aspect | Foxglove pattern |
|---|---|
| Top-level IA | Persistent shell: left **Sidebar** (Layouts, Topics, Variables, Performance, Studio settings) · center **Layout** (tiled panels) · right **Panel Settings** sidebar (cog icon, `,` key). |
| Panel taxonomy | ~22 first-class panels: **3D, Image, Plot, Raw Messages, Table, State Transitions, Indicator, Gauge, Map, Log, Diagnostics, Parameters, Variable Slider, Tab, Service Call, Topic Graph, TF Tree, Teleop, Audio, Data Source Info, Publish, Markdown.** ([Foxglove panels map](https://docs.foxglove.dev/docs/visualization/panels)) |
| Live capture placement | `Image` panel with image annotations is the live-camera surface; overlays are composited *in the same panel*. There is **no separate "live tab".** Same panel handles raw frames + 2D annotations. |
| Persistence | "Layouts" — personal + organization, exportable as JSON, version history (Pro/Enterprise tier). Multi-select via `Cmd`-click; batch move-to-folder; in-tab grouping via the `Tab` panel. |
| Hotkeys | `,` open panel settings; `Cmd-A` select all panels; ``Cmd-` `` show panel shortcuts; click-and-drag on numeric inputs to scrub values. |
| Layout customisation | Drag-resize tiles, drag-and-drop topics from sidebar into panels, drag a series from one Plot's settings into another to duplicate. |
| What works | (a) Settings sidebar is separate from the render surface — keeps the canvas zero-jank; (b) one canonical "Add panel" menu instead of N hard-coded tabs; (c) layouts shared at org level act as task presets; (d) drag-from-sidebar onto a panel is the dominant data-source-binding gesture. |
| What hurts | (a) React virtual DOM forced a `done()` callback backpressure pattern to prevent unbounded state growth ([phase_5_telemetry_ui.md cross-ref](research_dashboard_tech_2026.md#54-performance-patterns-worth-adopting)); (b) BSL license means we can borrow the *patterns* but not the code; (c) panel count >12 makes new users get lost — they introduced [organization layouts](https://docs.foxglove.dev/docs/visualization/layouts#organization-layouts) explicitly because raw panel libraries overwhelm. |

### 2.2 RViz2 (ROS 2 Jazzy)

**Primary source:** `docs.ros.org/en/jazzy/Tutorials/Intermediate/RViz/RViz-User-Guide/RViz-User-Guide.html`.

| Aspect | RViz2 pattern |
|---|---|
| Top-level IA | **Three-panel shell**: left **Displays** list (tree of enabled visualisations), center **3D viewport** + bottom **toolbar** (10 tools — Interact, Move Camera, Select, Focus, Measure, 2D Pose Estimate, 2D Nav Goal, Publish Point), right **Views** panel + **Tool Properties** + **Time** panel (collapsible). |
| Display taxonomy | 23 built-in display types: Axes, Camera, Effort, Grid, Grid Cells, Image, InteractiveMarker, Laser Scan, Map, Markers, Path, Point, Pose, Pose Array, Point Cloud(2), Polygon, Odometry, Range, RobotModel, TF, Wrench, Twist. |
| Live capture placement | `Camera` display **overlays detected geometry on top of the live image inside a single panel** (the user guide explicitly says "overlays the image on top of it"). No separate live-tab. The `Image` display is the same render surface, just without the `CameraInfo` reprojection. |
| Persistence | `.rviz` config files. Loadable / saveable per task. Contains: displays + properties, tool properties, viewpoint settings. Custom **named views** (Orbital, FPS, Top-down Ortho, XY Orbit, Third-Person Follower) saved per user. |
| Hotkeys | Single-letter tool switches: `i` Interact · `m` Move Camera · `s` Select · `c` Focus · `n` Measure · `p` 2D Pose · `g` 2D Nav Goal · `u` Publish Point · `f` focus on current selection. |
| Fixed-frame paradigm | Single most important config: the *fixed frame* (typically `map` or `world`). Everything else is transformed into it via the `tf2` tree. Critically, changing it clears existing displays. |
| What works | (a) Status colour-coding per display (`OK`/`Warning`/`Error`/`Disabled`) — operator sees data-source health in the displays tree without leaving the canvas; (b) toolbar is a *modal tool* (current mouse behaviour), not a navigation tab — operator stays in the same workspace; (c) Add-Display dialog mirrors Foxglove's Add-Panel menu — one canonical entry point. |
| What hurts | (a) Add-display dialog list is long and unsearchable; (b) Time panel is "mostly useless without simulation" (user guide's own words) — dead UI real-estate in live operation; (c) no first-class layout/preset versioning. |

### 2.3 NVIDIA DeepStream / Metropolis

**Primary source:** `docs.nvidia.com/metropolis/deepstream/dev-guide/text/DS_ref_app_deepstream.html` (9.0).

| Aspect | DeepStream pattern |
|---|---|
| Operator surface | The `deepstream-app` reference application is a **GStreamer pipeline driven entirely by a key-file config** (no GUI binding to a tabbed dashboard). Its visualisation is a **tiled-display** sink with the **`Gst-nvdsosd`** plugin drawing detection boxes / text on top of the composited frame. ([DS_ref_app](https://docs.nvidia.com/metropolis/deepstream/dev-guide/text/DS_ref_app_deepstream.html)) |
| Source taxonomy | Multi-source: `[source0]`, `[source1]`, … config groups feed the **`Gst-nvstreammux`** plugin which batches buffers; the **`Gst-nvmultistreamtiler`** plugin composes them in a `rows × columns` 2D array (e.g., `rows=5 columns=6` = 30-camera wall). |
| Live capture placement | The same surface holds frame + OSD overlay + pre-defined ROIs (green boxes for `Gst-nvdspreprocess` ROIs). The OSD plugin is canonical: text/box drawing happens *inside the pipeline*, not in a separate UI layer. |
| Persistence | Per-deployment `.txt` config (freedesktop key-file format) with named groups: `[application]`, `[tiled-display]`, `[source0..N]`, `[streammux]`, `[primary-gie]`, `[secondary-gie0..N]`, `[tracker]`, `[osd]`, `[sink0..N]`, `[message-converter]`, `[message-consumer0..N]`, `[nvds-analytics]`, `[tests]`. ~13 config groups in the reference app. |
| What works | (a) **Config groups map 1:1 to pipeline stages** — operator never wonders which knob controls which stage; (b) tiled-display with on-screen-display is the de-facto pattern for production multi-camera CV (Metropolis customers run hundreds of these in retail / smart-city deployments); (c) `enable-perf-measurement=1` + `perf-measurement-interval-sec=10` are a built-in performance HUD path. |
| What hurts | (a) No GUI — config is `.txt` only, requiring a text editor + restart; (b) `Gst-nvmsgconv` / `Gst-nvmsgbroker` sends analytics to cloud, expecting that the *operator UI* is a separate downstream (Streamlit / web dashboard built on Kafka). DeepStream itself is the inference graph, not the operator surface — important context that the user's *operator dashboard* shouldn't try to replicate DeepStream-the-pipeline-tool. |

### 2.4 Anduril Lattice / Lattice for Mission Autonomy

**Primary sources:** Anduril product pages (bot-blocked by Cloudflare on scrape; meta + descriptions captured via search), `ex2.com.au/in-detail-anduril-launches-extended-lattice-os/` (May 2023 long-form industry article), Anduril public videos (`youtube.com/watch?v=RpFFScTovII`).

| Aspect | Lattice pattern |
|---|---|
| Top-level IA | Single-screen **management terminal** that runs on a laptop. Hardware-agnostic — the same UI commands AUVs (Ghost Shark), counter-UAS (Tower), loitering munitions (ALTIUS), and surveillance aircraft. ([ex2.com.au, 2023](https://www.ex2.com.au/uncategorized/in-detail-anduril-launches-extended-lattice-os/)) |
| Operator abstraction | "Mission intent" — operator declares the *outcome* ("score a touch-down against this team"); Lattice dynamically re-tasks assets. Operator's job is supervisory check-ins at decision points, *not* per-asset joystick. |
| Live video / sensor surface | Multi-sensor fused situational-awareness map is the *primary* surface; individual sensor feeds are composited overlays selected from a sidebar (asset list). The conductor metaphor — Arnott (Anduril SVP Engineering) explicitly calls it "the conductor of an orchestra". |
| Persistence | Mission packages + asset profiles; software refresh "every few days or weeks, hardware every few months or years" per Anduril. Mission state hands off dynamically between command nodes (e.g., laptop on ground → laptop on E-7 Wedgetail AEW&C). |
| What works | (a) **Single operator, many platforms** is the explicit design centre — a strong existence proof that one screen can manage 50+ entities if the IA collapses correctly; (b) the C2 architecture is **node-portable** — same UI runs on a laptop, ship, ground vehicle, or aircraft. |
| What hurts (observed) | (a) Limited public screenshots; classified deployments dominate; (b) the "intent abstraction" is overkill for a single-camera surveillance rig — ROC operators *want* the per-track view, not the mission-level view. We borrow the *node-portable single-screen* concept, not the intent-only abstraction. |

### 2.5 Skydio Remote Ops + DJI Pilot 2

**Primary source:** `skydio.com/software/remote-ops`, QGroundControl `Fly View` docs.

| Aspect | Skydio / DJI pattern |
|---|---|
| Top-level IA | **Fly view is the only "tab" that matters in operation.** It is a full-screen viewport with a **toolbar** at the top, **instrument panel** to one side, **attitude/compass widget** floating on the video, and a **mission-action button cluster** as the only persistent navigation. Settings and mission-planning are *separate views* on a top-level mode switcher (Fly · Plan · Settings — 3 modes max, often 2). ([QGC Fly View](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/fly_view/fly_view.html)) |
| Live capture placement | **Video and map share a single switcher widget** — pressing the switcher swaps which is foreground; the other becomes a picture-in-picture inset. The switcher is detachable into a free-floating OS window for multi-monitor operation. This is the only "tab-like" gesture and it is **inside** the Fly view. |
| Operator input multiplexing | Skydio Remote Ops explicitly supports "keyboard, mouse, controller, or a combination" simultaneously. Every operator action has a binding in each input modality. AR overlays (street names, addresses, mission markers) are *composited on the live video*. |
| Persistence | Mission paths saved + scheduled; "powerful mission planning and scheduling tools". |
| What works | (a) **Video ↔ map switcher with detachable PiP** is the canonical pattern for "I need both, but one is foreground" — this directly answers ROC's live-camera-vs-spatial-canvas question; (b) AR overlays directly on video build operator trust without splitting attention; (c) multi-input parity (kbd + mouse + controller) is operator-grade ergonomics. |
| What hurts | (a) Fly view becomes cluttered with widgets — QGC explicitly lists ~6 widget zones overlaid on a single video surface, which is the upper bound of what's readable; (b) emergency Pause is a *slider* (anti-fat-finger), not a button — fine for vehicles, possibly overkill for ROC's actuator. |

### 2.6 OBS Studio

**Primary source:** `obsproject.com/kb/sources-guide`, `obsproject.com/kb/scene-collections`, `obsproject.com/kb/profiles`.

| Aspect | OBS pattern |
|---|---|
| Top-level IA | Single-window shell with **docks**: `Scenes` (list) · `Sources` (z-ordered list with eye-toggle) · `Audio Mixer` · `Scene Transitions` · `Controls`. Docks are draggable, resizable, dockable to edges, and savable in **profiles**. **No tabs** — the operator switches *scenes* (workspaces) from the Scenes dock, not tabs. |
| Source taxonomy | ~10 source types: Audio · Browser · Color · Display Capture · Game Capture · Image · macOS Screen Capture · Media · Text · Video Capture · Window Capture. Sources are **z-ordered** in the dock (top of list = top of preview). Per-source visibility toggle (eye icon). |
| Live capture placement | "Video Capture" source is composited into the preview alongside game capture, browser overlays, text, etc. The preview is the **single live surface** — no separate "Camera Tab". |
| Persistence (two-tier) | **Scene Collections** save scenes + sources + audio routing ("the workspace layout"). **Profiles** save output / encoder settings (bitrate, encoder, recording path). The split is deliberate: you swap *what you're streaming* (Scene Collection: podcast vs. game vs. recording-only) independently of *how you're streaming* (Profile: Twitch vs. YouTube vs. local 4K). |
| Hotkeys | Per-action hotkey table (assignable in Settings → Hotkeys). Transform hotkeys: `Ctrl-E` edit transform · `Ctrl-R` reset · `Ctrl-F` fit · `Ctrl-S` stretch · `Ctrl-D` centre. Hold `Alt` to crop. Hold `Ctrl` to disable edge-snap. |
| What works | (a) **Two-tier preset system (Scene Collection × Profile) is the pattern ROC needs** — the equivalent for us is `Scene Collection = camera + tracker config + UI layout` (e.g., "Surveillance shift", "Calibration session", "Forensic review") × `Profile = inference EP + recording settings + actuator caps`. Combinatorial without proliferation; (b) z-ordered source list is the canonical "layered composition" metaphor; (c) eye-toggle to hide-without-deleting is universally understood. |
| What hurts | (a) Docks can be re-arranged into a mess; OBS ships canonical layouts as a fallback; (b) Scene Collection import has *three* different legacy formats (OBS Classic, XSplit, Streamlabs) — a maintenance tax we should avoid by versioning our own schema from day 1. |

### 2.7 ReShade

**Primary source:** `framedsc.com/ReshadeGuides/setupreshade.htm`, `reshade.me/forum/addons-discussion/8463-custom-imgui-rendering`.

| Aspect | ReShade pattern |
|---|---|
| Top-level IA | **Single in-game overlay** opened by a hotkey (default `HOME` or `Shift-F2`). Overlay is **a floating tabbed window** (ImGui-based): `Home` · `Settings` · `Statistics` · `Add-ons` · sometimes `D3D9/D3D10/D3D11/D3D12` (depth-buffer picker). The overlay is **dockable to either screen edge or floats** — operator drags tabs off the dock to detach. |
| Effect taxonomy | Each shader = a "technique"; techniques are listed top-to-bottom on the Home tab in *execution order* (first listed runs first, output piped into next). Operator toggles per-technique with a checkbox or hotkey; parameters appear below the splitter as sliders / drag-numbers. |
| Persistence | `.ini` preset files. Each preset is a named selection of enabled techniques + parameter values + per-technique hotkey bindings. Operators swap presets between screenshots / scenes. |
| Hotkeys | Per-technique hotkey assignment via right-click → `Toggle Key`. Global hotkeys for menu open + screenshot capture in Settings → Screenshots. |
| What works | (a) **Overlay opens on-demand and dismisses to zero-pixel cost** — the render surface (the game) is never interrupted; (b) drag-numeric values (click + drag horizontally to scrub) is the same pattern Foxglove and OBS use — universal for operator-grade tuning; (c) preset files are plain INI and editable in any text editor; (d) `Hold Shift` while dragging a slider = fast scrub, `Hold Ctrl + click` = type a value outside the slider range. Operator-grade ergonomics. |
| What hurts | (a) The `Home` tab does five different things (toggle techniques, show parameters, edit preprocessor defines, screenshot, tutorial) — overloaded; (b) depth-buffer selection (D3D11 tab) is a tutorial-required step that hides behind a vendor-specific tab name — bad first-run UX. |

### 2.8 Datadog + Grafana

**Primary sources:** `grafana.com/docs/grafana/latest/visualizations/dashboards/build-dashboards/`, `docs.datadoghq.com/dashboards/`.

| Aspect | Datadog / Grafana pattern |
|---|---|
| Top-level IA | Persistent **left rail** ("Dashboards", "Notebooks", "Monitors", "APM", "Logs", "Synthetics", "RUM", "Profiler", "Security", "Cloud Cost", "Service Catalog" — ~12 sections in Datadog 2026) · **dashboard list** view as the inner home · individual **dashboard** is a **grid of panels** with drag-resize. Settings live inside each panel's own kebab-menu. |
| Panel/widget taxonomy | Grafana lists ~30 widget types (`Time series`, `Stat`, `Gauge`, `Bar gauge`, `Table`, `Pie chart`, `Heatmap`, `Geomap`, `Status history`, `Logs`, `Histogram`, `Candlestick`, …); Datadog publishes a separate "Widget Types" reference page with ~25 types. Custom panels via plugins / library panels. |
| Live update model | Auto-refresh interval (5s · 10s · 30s · 1m · …) chosen per-dashboard. Time-range picker (relative or absolute) is the *single global* control that drives every panel. |
| Persistence | Dashboards are JSON, versioned, importable / exportable, multi-tier (`Personal` / `Team` / `Org`). Library panels — reusable widget definitions shared across dashboards. Template variables — `$env`, `$service` style — let one dashboard parametrically display N hosts. |
| What works | (a) **Grid of resizable panels with one global time-range** is the de-facto pattern for performance HUDs and analytics; (b) library panels = "reusable widget definitions" — the right pattern for ROC's per-track-metric tiles; (c) template variables are the right pattern for ROC's *per-track* metrics view (`$track_id`). |
| What hurts | (a) Datadog left rail has ~12 sections; without onboarding, operators get lost; (b) Grafana's per-panel cog buried in a kebab is a recurring complaint — Foxglove improved on this with the `,` hotkey. |

### 2.9 OpenCV `cv2.imshow` + cvui

**Primary sources:** `dovyski/cvui` (GitHub README), `learnopencv.com/cvui-gui-lib-built-on-top-of-opencv-drawing-primitives/`.

| Aspect | cvui pattern |
|---|---|
| Top-level IA | **No top-level IA at all.** `cv2.imshow("title", frame)` is a *single OS window* per image; cvui draws ImGui-style buttons / sliders / trackbars *into the same Mat buffer* using OpenCV drawing primitives. Header-only C++ (and Python bindings); no Qt / OpenGL dependency. |
| Operator surface | The image and the controls live on the same surface, drawn into the same pixel buffer. Multiple cvui windows are independent OS windows. |
| Live capture placement | The frame *is* the control surface. There is no concept of "live tab" — every cv2.imshow window is live. |
| Persistence | None built-in — operator saves screenshots manually. |
| What works | (a) **Floor pattern.** This is what bare-metal CV inspection looks like: a video surface with `text`, `rectangle`, `circle`, `line` OpenCV calls overlaid. The minimal viable operator surface for a single-camera tracker is **exactly this** — one window with detections drawn on top; (b) confirms that **video + overlay = same surface** is the universal CV pattern. |
| What hurts | (a) Doesn't scale past one operator + one window; (b) no panel management, no presets, no event log — fine for a debug session, untenable for production surveillance. |

### 2.10 Tesla Autopilot / FSD Visualisation

**Primary sources:** `teslarati.com/tesla-autopilot-visualization-big-upgrade-new-additions/` (Oct 2025, citing the [@greentheonly](https://x.com/greentheonly/status/1977590457476776302) Tesla-hacker reverse-engineering), `teslarati.com/tesla-just-fixed-four-year-old-bug-with-full-self-driving-visualization/`.

| Aspect | Tesla AP/FSD pattern |
|---|---|
| Top-level IA | Single full-screen center display. Driving viz occupies the left third of the screen (or the entire screen in some "Bird's Eye" overrides). **No tabs** in the visualisation itself — the driving viz is *always on* when the car is driving. |
| Visualisation content | World-frame 3D scene rendered behind the ego-car asset: detected vehicles (now ~15+ vehicle archetypes including ambulance, firetruck, schoolbus, European semi, golf cart, three-wheeler, trailer, train, tram), pedestrians (now with stroller, wheelchair, skateboard, scooter renders), animals, lane lines, traffic cones, path-prediction polyline. |
| Live video | Tesla **does not show raw camera frames on the operator display in normal driving** — the visualisation is the *abstracted scene-graph reconstruction*. Raw camera feed is only surfaced in service mode, parking-cam, and the Dashcam viewer. |
| Persistence | None operator-facing — visualisation is an immutable render, no "viz preset" UI. |
| What works | (a) **The abstracted reconstruction is the load-bearing surface, not raw video.** This is the defining Tesla UX choice and is the opposite of what most CV inspection tools (cvui, OpenCV imshow) do; (b) richer visualisations (Tesla now planning Unreal Engine driver visualisation per [Teslarati Oct 2025](https://www.teslarati.com/tesla-plans-unreal-engine-driver-visualization-crazy-upgrade/)) build trust; (c) "smoother visualisation" was a published-bug priority — confirms operators care about *jitter* of overlays, which is exactly ROC's defensive-engagement use case (target lock + serial actuator pointing at a clicked person — overlay smoothness directly affects operator confidence in the lock). |
| What hurts | (a) Loss of raw camera ground truth on the operator display means *the driver can't verify what the perception system actually sees* — this is the explicit failure mode behind several FSD-disengagement incident reports. **ROC must provide a raw-camera toggle.** |

### 2.11 Ultralytics YOLO `predict` mode + Ultralytics HUB

**Primary source:** `docs.ultralytics.com/modes/predict`, `hub.ultralytics.com/`.

| Aspect | Ultralytics pattern |
|---|---|
| Top-level IA (CLI) | `model("source").show()` — pops a single OpenCV window per result with rectangles + class labels drawn in-frame. Optionally `save=True` writes annotated frames to disk. |
| Top-level IA (HUB) | Web app: left rail (Models · Datasets · Projects · Tasks · Train · Predict tab inside a model) · main pane = upload widget + result canvas with bounding-box overlays. The **Predict tab** is *inside* a model — i.e., model identity is the primary IA, prediction is a verb on a model. |
| Live capture placement | "Upload an image, use an example, or open your webcam; **View predictions instantly with bounding box overlays.**" Single canvas surface; overlay composited via standard 2D `Canvas` API. No separate live tab. |
| Real-world overlay reference | The Ultralytics docs ship per-domain reference images (Manufacturing/Sports/Safety) showing the canonical CV-overlay style: thin coloured rectangles, class label + confidence above the box, no trails by default (trails are a `solutions/`-level feature, not in `predict`). |
| What works | (a) Confirms the "**one canvas, overlay composited in same surface**" pattern is what the YOLO community ships by default; (b) `Predict` as a *verb on a model*, not a top-level destination — operator picks model first, then runs inference. |
| What hurts | (a) No multi-camera abstraction in `predict` mode — single-source assumption; (b) Streamlit-based live-inference demos ([Ultralytics + Streamlit YouTube tutorial](https://www.youtube.com/watch?v=Fm72tfuQG70)) are toys, not operator consoles. |

### 2.12 VMS / SOC Operator Consoles — Milestone XProtect, Genetec Security Desk, Palantir Gotham

**Primary sources:** `doc.milestonesys.com` (XProtect Smart Client `User interface overview` + `Tabs explained`), `techdocs.genetec.com` (Security Desk, `About-Security-Desk`, `Monitoring-in-Security-Desk`), `palantir.com/platforms/gotham/`.

This category is where public-domain documentation thins out the fastest: defence-grade and EW operator consoles (e.g., Sierra Nevada / L3Harris EW dashboards) are not documented publicly. The cross-referenceable surrogates are commercial SOC video-management systems (Milestone, Genetec, Avigilon, Hanwha, Hikvision) and Palantir Gotham — all of which serve the same archetype of operator (analyst monitoring N feeds while alarms route to actionable workflows).

| Aspect | VMS / SOC pattern |
|---|---|
| Top-level IA — **Milestone XProtect Smart Client** | Browser-style **top tabs**: `Views`, `Exports`, `Search`, `Alarm Manager`, `System Monitor`, **plus default tabs** like `Live`, `Playback`, `Access Control` (the user-manual lists ~5–8 default tabs depending on installed integrations). |
| Top-level IA — **Genetec Security Desk** | **Task-based design**: Monitoring, Maps, Alarm monitoring, Cameras, Door management, License plate management, plus a *task bar* that lets operators send a task to a different physical monitor on a video wall. The operator can keep N tasks open simultaneously. |
| Live capture placement | The `Live` / `Monitoring` tab is **a grid of camera tiles** (e.g., 2×2 to 8×8), with PTZ controls and tile-level overlays. Each tile is one camera. **Separate `Playback` tab** for recorded forensic work — this is the only place in the survey where live and playback are clearly distinct top-level destinations. |
| Operator workflow | Pre-built **layouts** (named tile arrangements) selectable from a sidebar; alarms route to the Alarm Manager tab with playback context; investigation workflow goes from Live → Bookmark → Search → Export. |
| Persistence | Per-operator default tab set; per-operator camera layouts; per-role permissioning (e.g., XProtect Alarm Manager tab hidden if user not in the right Group). |
| What works | (a) **Live and Playback are clearly two different cognitive modes** — operators are either *watching now* or *investigating the past*. SOC consoles encode this in IA. **This is the single strongest argument for keeping a separate "Replay" workspace in ROC, even if Live and Spatial Canvas merge**; (b) the `System Monitor` tab in XProtect is precedent for a built-in performance HUD; (c) per-role tab visibility — fine-grained but useful for ROC's multi-operator-profile concept. |
| What hurts | (a) Tab counts in XProtect can balloon to ~10+ when integrations are enabled — the user-manual workarounds amount to "hide tabs you don't use"; (b) Genetec's video-wall multi-monitor task push is operator-power-user only — overkill for ROC's single-machine deployment. |

**Palantir Gotham specifically** introduces a fourth IA primitive worth calling out: the **Object** (Person / Vehicle / Event / Document) as the unit of analysis, with workflows that pivot between objects (a person of interest → their associated vehicle → its last-known location → events that match) rather than between fixed dashboards. The [Gotham product page](https://www.palantir.com/platforms/gotham/) describes "informed decisions, maximizing effectiveness of your assets in even the most dynamic operational environments" — i.e., the IA is *the entity graph itself*. For ROC, the analogue is the `parent_id` hierarchy graph (`person → head / upper_torso / accessories`) from `phase_2_ekf_kinematics.md` §3.11, and the operator workflow "click a person, see their head + torso + accessory children, walk that subtree to confirm tracking quality" maps cleanly onto Gotham's pivot pattern. **The Hierarchy Explorer feature from the brief is therefore Gotham-style entity-graph navigation, not a generic tree-view.**

---

## 3. Comparison Matrix

| System | Live-video panel | Telemetry / overlay | Recording / Replay | Calibration | Model selection | Performance HUD | Hotkeys (count, dominant chord) | Layout customisation |
|---|---|---|---|---|---|---|---|---|
| **Foxglove Studio** | `Image` panel + annotations (same surface) | `3D`, `Plot`, `State Transitions`, `Indicator`, `Gauge` | MCAP playback inside same panel set | External per-sensor | Not a concept (data-source agnostic) | `Performance` sidebar | ~15 documented (`,` `Cmd-A` `Cmd-B` ``Cmd-` `` `Cmd-click`) | Drag-tile resize, Tab-panel grouping, JSON layouts |
| **RViz2** | `Camera` display (image + overlay same surface) | 23 display types (Markers, Path, PoseArray, Map, Grid …) | Bag playback via `ros2 bag play` (external) | `2D Pose Estimate` tool seeds localiser | Not a concept | Status colours in display tree | 10 single-letter (`i m s c n p g u f`) | Saveable `.rviz` configs + named Views |
| **DeepStream** | Tiled-display sink + `Gst-nvdsosd` overlay (same surface) | OSD plugin overlays box/text per-frame | `[sink]` group can record encoded file | External | Per-`[primary-gie]` `.txt` cfg | `enable-perf-measurement=1` | None (config-file driven) | Config groups; no GUI |
| **Anduril Lattice** | Multi-sensor fused C2 map (composited) | Asset state on map | Mission packages | Per-asset onboarding | Hardware-agnostic — drop-in | Node-portable laptop terminal | Unpublished | Mission package hand-off |
| **Skydio / QGC** | Fly-view video ↔ map switcher (detachable PiP) | Instrument panel + attitude/compass widgets | On-camera + on-GCS `.mkv` recording | Per-vehicle setup wizard | N/A | Toolbar status icons | Pause + mission actions | Detachable video window |
| **OBS Studio** | Preview = composited live capture sources | Scene = z-ordered source list | Recording profile + Replay Buffer | Per-source transform | N/A | Stats dock | Per-action assignable; `Ctrl-E/R/F/S/D`, `Alt`-crop | Dockable panes + Scene Collection JSON |
| **ReShade** | Game framebuffer = live + overlay (same buffer) | Per-technique parameter sliders | None (use external screenshot tool) | Depth-buffer picker tab | Shader library = "models" | `Statistics` tab | `HOME` open · per-technique `Toggle Key` · `Shift`-drag fast scrub | Floating undockable ImGui tabs |
| **Datadog / Grafana** | N/A (metrics-only) | Grid of resizable panels | Time-range scrub | N/A | N/A | Built-in (it *is* the performance HUD) | Per-dashboard customisable | Drag-resize grid + library panels |
| **OpenCV imshow / cvui** | Single OS window = frame + overlay | Drawing primitives | None | None | None | None | OpenCV `waitKey` only | None |
| **Tesla AP/FSD** | Center display = scene-graph viz (no raw video by default) | World-frame reconstruction | Dashcam viewer (separate UI) | N/A (auto) | OTA | Reverse-engineered only | Steering-wheel scroll | None |
| **Ultralytics** | `result.show()` opens OpenCV window with overlay | Bounding boxes + labels in-frame | `save=True` writes annotated frames | N/A | First-class — model is the primary IA | N/A | N/A | HUB web app: standard SaaS panes |
| **Milestone XProtect / Genetec** | `Live` tab = tile grid (per-camera, w/ PTZ overlay) | Per-tile detections, PTZ controls | Separate `Playback` tab (key IA decision) | Per-camera install wizard | N/A | `System Monitor` tab | Per-role configurable | Per-operator named layouts |

**Reading the matrix:** ten of the twelve systems composite live video and overlay onto a single surface. The two exceptions are (a) Tesla, which replaces raw video with an abstracted scene reconstruction by design, and (b) Milestone/Genetec, which split *Live* from *Playback* not because video is on a different tab, but because *forensic search* is a different cognitive mode.

---

## 4. Pattern Extraction — what shows up in ≥ 3 systems

| # | Pattern | Systems exhibiting it (≥ 3 required) | Why it generalises |
|---|---|---|---|
| **P1** | **Single render surface for video + overlay (z-ordered layers in one panel).** | Foxglove (`Image`), RViz (`Camera`), DeepStream (`Gst-nvdsosd`), OBS (sources stack), Skydio (AR overlays), Ultralytics (`result.show`), cvui (drawing on Mat), VMS Live tab. | Operator cognitive load is lower when *what the camera sees* and *what the model thinks it sees* are co-registered without an alt-tab. |
| **P2** | **Persistent shell + named workspace presets** (not top-level tabs as IA). | Foxglove (Layouts), RViz (`.rviz` configs + Views), OBS (Scene Collections + Profiles), Grafana (Dashboards), ReShade (presets), Anduril (mission packages), DeepStream (config groups). | Operator's *task* (calibration vs. surveillance vs. forensic review) varies more than their *view philosophy*. Presets encode tasks; shells encode philosophy. |
| **P3** | **Sidebar with hierarchical list of data sources / displays, drag-into-panel to bind.** | Foxglove (Topics sidebar), RViz (Displays panel), OBS (Sources dock), Skydio (Asset list). | When data sources are dynamic (cameras come and go, tracks spawn / die, models swap), a sidebar enumeration beats a hard-coded panel set every time. |
| **P4** | **Settings sidebar is separate from render surface; cog icon or `,` opens it.** | Foxglove (`,`), RViz (Tool Properties panel), OBS (Properties dialog), Grafana (panel cog), ReShade (Settings tab). | Tuning is human-speed; rendering is hot-path. The two must not share a render loop. ROC's locked rule `200-dashboard-telemetry.mdc` already encodes this — confirm by external pattern. |
| **P5** | **Single-key tool/mode hotkeys** (i, m, s, c, n in RViz; `,` in Foxglove; HOME in ReShade; per-action in OBS). | RViz, Foxglove, ReShade, OBS, vim-style across the survey. | Operator-grade tools live at the hands. A mouse-only console is a tell that the designers haven't deployed it themselves. |
| **P6** | **Drag-numeric inputs** (click + drag horizontally to scrub a numeric parameter). | Foxglove ("Click input + Drag right – Increment numeric panel setting values"), ReShade (slider drag, `Shift` for fast, `Ctrl-click` for type), OBS (transform numerics), Grafana. | Mouse-wheel-on-a-stepper is too slow; type-in-a-box too modal. Drag-scrub is the unanimous compromise. |
| **P7** | **Status colour-coding embedded in the navigation tree** (not a separate health panel). | RViz (`OK / Warning / Error / Disabled` colour on each display tile), Skydio (toolbar status icons), VMS Alarm Manager tile colours. | Operator must see *which subsystem is sick* without leaving the canvas. ROC's planned per-track confidence colours map directly. |
| **P8** | **Live vs. Replay as a hard mode-switch** (not a tab — a mode that re-binds the global time scrubber). | XProtect (Live tab vs Playback tab), Genetec (Monitoring vs Investigation tasks), Foxglove (`live` vs MCAP file as data source), Datadog (live mode vs custom time-range). | Forensic review is fundamentally different from live operation — operator *expects* the entire UI behaviour (auto-refresh, alerts, scrubbing) to swap. |
| **P9** | **Z-ordered source / layer list with eye-toggle for visibility, no-delete.** | OBS (eye icon), RViz (display checkbox), Foxglove (topic visibility in panel settings), Photoshop genealogy. | Operators rapidly A/B overlays without losing the configuration — toggle-off ≠ remove. |
| **P10** | **Two-tier preset: workspace × profile** (compose what you do vs. how you do it). | OBS (Scene Collection × Profile), Grafana (Dashboard × Template Variables), Datadog (Dashboard × time-range), DeepStream (`[application]` × `[source]` × `[sink]` config groups). | Without this split, presets explode combinatorially (N tasks × M output configs = NM duplicated workspaces). |

---

## 5. Findings against ROC's locked architecture

Before recommending, I check the locked rules so the synthesis agent doesn't have to.

| Locked element | Source rule / file | Survey pattern alignment |
|---|---|---|
| `TelemetryCanvas.svelte` — hot-path zero-GC SVG only | `.cursor/rules/200-dashboard-telemetry.mdc` | **Consistent with P4.** Foxglove, RViz, OBS, Grafana all split the render surface from configuration. Keep. |
| `NavigationShell.svelte` — 4-Domain Controller Matrix (`Observer / Spatial / Kinematic / Hardware`) with `{#if}` unmounts | `.cursor/rules/200-dashboard-telemetry.mdc`, `dashboard/architecture_plan.md` | **Partially consistent with P2.** The shell + workspace-switch is right; but the names "Observer Matrix / Kinematic Trajectory Controller / Hardware Actuation Interface" don't map onto any operator mental model from the survey. **Operators in the survey think in tasks** (Watch, Calibrate, Review, Configure), not in mathematical-layer names. **The rule's mandate to keep 4 stops short of saying *which* 4** — and lets the parent synthesise the labels. |
| Domain configuration panels — `aiming_config` / `hardware_config` only, never coupled to `trackPool` or RAF loop | `.cursor/rules/200-dashboard-telemetry.mdc` | **Consistent with P4.** Every system in the survey has this split. Keep. |
| `TelemetryCanvas` unmounts off spatial domain | `.cursor/rules/200-dashboard-telemetry.mdc` | **Consistent.** RViz tools modal-switch the canvas, Foxglove panels unmount on layout change, OBS scenes swap render graph. Keep. |

**The locked rules don't forbid:** (a) a left-rail navigation instead of top tabs, (b) more than 4 destinations *as long as 4 of them are first-class*, (c) a saved-layout/preset layer, (d) a video-and-spatial *composited* surface inside the existing `Spatial Projection Canvas` domain. These are the levers the parent agent has.

---

## 6. Concrete Recommendations for ROC AI Vision

> Numbered, ranked by importance. Every recommendation cites the survey pattern that justifies it.

### Rec 1 — Keep 4 first-class top-level destinations, but re-label them around operator tasks (not mathematical layers).

The locked rule mandates 4; the survey shows operators think in tasks. Reconcile by re-labelling within the 4-count constraint:

| Current (locked label) | Proposed re-label | Operator's job in this destination | Survey precedent |
|---|---|---|---|
| **Spatial Projection Canvas** | **Operate** | Live tracking — single composited surface, video underneath, EKF overlay on top, target-lock click, actuator slew indicator | Foxglove `Image` + annotations · RViz `Camera` display · VMS `Live` tab · Tesla AP viz · OBS preview |
| **Observer Matrix** | **Inspect** | Per-track state (EKF covariance, NIS regime, confidence-over-time, occlusion events, hierarchy explorer) | Foxglove `State Transitions` + `Plot` panels · Datadog template variables · RViz Displays status |
| **Kinematic Trajectory Controller** | **Calibrate** | Camera intrinsics/extrinsics, EKF tuning (jerk PSD, gates), aiming-config smoothing/deadband, model selector | RViz `2D Pose Estimate` tool · OBS Properties dialogs · ReShade Settings + per-technique sliders |
| **Hardware Actuation Interface** | **Engage** | Pan/tilt joystick, target-lock confirmation, safety interlocks, serial config, recording controls | Skydio Pause-slider · QGC `Vehicle Actions` · OBS recording controls · Foxglove `Teleop` panel |

The names are a synthesis target — the parent agent owns the final wording. **What matters mathematically is the cardinality (4) and the workspace-per-task IA** (P2 in §4).

### Rec 2 — Make live capture and the EKF spatial overlay the SAME canvas, not separate tabs.

This is the user's explicit question. The unanimous survey verdict (10 of 12 systems, P1 in §4) is **composited single-surface**. The two outliers (Tesla — scene-graph instead of raw; VMS — Live vs Playback) don't argue for splitting live video from overlay; they argue for *abstracting* (Tesla) or *separating from forensic playback* (VMS), neither of which means giving live video its own peer destination.

Architecturally inside `Operate`:

```
Operate workspace (the renamed "Spatial Projection Canvas")
└── single full-canvas surface, z-ordered:
    ├── Layer 0 (bottom) :  <video> element bound to the camera-stream MediaSource
    │                       (or hidden raw-frame fallback when offline)
    ├── Layer 1          :  trajectory paths (Phase B: <canvas>; Phase A: SVG <path>)
    ├── Layer 2          :  bounding boxes + class labels + confidence (SVG)
    ├── Layer 3          :  EKF uncertainty ellipses + hierarchy edges (SVG)
    └── Layer 4 (top)    :  reticle / target-lock crosshair + actuator slew indicator
```

The video layer must be **toggleable** (operator hotkey: `v`) so that defensive-engagement training and forensic review can fall back to an abstracted scene (the Tesla pattern as escape hatch — addresses the Tesla failure-mode where operator loses raw ground truth). See §10 addendum for the full recommendation.

### Rec 3 — Add a saved-layout / preset system, organised as `Scenario × Profile`.

The survey's most universal pattern after composited-surface is preset persistence (P2, P10). ROC has zero presets today; the 11 future features (per the brief) cannot fit on 4 destinations without explosion unless we layer a preset system on top.

Concretely:

- **`Scenarios`** (Scene Collection equivalent): named bundles of (panel arrangement × default workspace × default camera × default model × default actuator-cap). Suggested defaults shipped with v2.0: **Surveillance Shift**, **Calibration Session**, **Forensic Review**, **Defensive Engagement**, **Behavioural Analysis**, **Bring-Up / Dev**.
- **`Profiles`** (OBS Profile equivalent): named bundles of (execution-provider preference × recording format/path × actuator safety caps × hotkey bindings). Operator picks one once at session start.
- **Storage:** JSON under `dashboard/data/scenarios/*.json` + `dashboard/data/profiles/*.json`, importable / exportable.
- **UI:** dropdown in the Operate masthead, similar to QGC's mode selector. No new top-level destination needed — presets are a *cross-cutting* concept.

### Rec 4 — Replace top tabs with a left-rail destination switcher.

Top tabs work when the destination count is fixed and ≤ 5 (XProtect's exact threshold before tab balloon). With ROC's 11 stated future features, the math says we'll cross that line. Survey systems with ≥ 8 destinations universally use a **left rail** (Datadog, Grafana, Skydio, Anduril); systems with ≤ 5 use top tabs (current ROC, XProtect, OBS, ReShade). A left rail also reads better on widescreen monitors at 1440p+ where vertical pixels are scarce.

**Concrete:** keep the 4 first-class destinations as left-rail icons + labels; reserve the bottom of the rail for the **`Models` browser** (per-Rec 6) and the **`Settings`** drawer (Profile selection, hotkey rebinds, dashboard preferences). Total persistent left-rail entries: 4 + 2 = **6**, still well under XProtect's empirical-fail threshold.

### Rec 5 — Build a model manifest browser as a destination-level drawer, not a tab.

The user's 113 ONNX files on the desktop need a UI. Survey reference: Ultralytics HUB makes the model the *primary IA* (model identity → predict verb); ReShade lists techniques on the Home tab; DeepStream's `[primary-gie]` config group is THE per-pipeline model knob. ROC needs the model-selector to be:

- Reachable from anywhere (left-rail bottom icon: `Models`).
- A side drawer (not a dedicated full-canvas destination) — operator selects, drawer closes, current workspace re-renders with the new model.
- Backed by the manifest schema already in `phase_1_universal_ep_and_manifest.md` — class mapping, EP availability badges, input shape, expected post-process strategy.
- Searchable + filterable (113 files demands it; Foxglove's panel search is precedent).

### Rec 6 — Hotkey table is operator-grade ergonomics, not a nice-to-have.

Every survey system with operator users has hotkeys; every system without them (cvui, Ultralytics CLI) doesn't scale past a debug demo. Proposed minimum table:

| Key | Action | Survey precedent |
|---|---|---|
| `1 / 2 / 3 / 4` | Switch destination (Operate / Inspect / Calibrate / Engage) | Browser tabs · VMS task switcher |
| `v` | Toggle video layer on the Operate canvas | New (addresses Tesla failure-mode) |
| `o` | Toggle EKF overlay on the Operate canvas | New |
| `,` | Open panel-settings sidebar (whichever panel is focused) | Foxglove |
| `space` | Pause / resume tracker (Engage workspace only) | DAW + media-player convention |
| `r` | Toggle recording | OBS / QGC |
| `g` | Focus camera-target picker (click-to-lock) | RViz `2D Nav Goal` precedent |
| `?` / `F1` | Open hotkey-cheatsheet overlay | Universal |

**Implementation:** a single `KeyboardShortcut.svelte` registry component that lives at the shell level; per-destination handlers register/unregister on mount. Use existing `keydown` listeners on `window`, with the focused-input guard (don't fire when a `<input>` / `<textarea>` has focus).

### Rec 7 — Recording / Replay is a *mode swap* on Operate, not a 5th destination.

Survey result: VMS systems give Live and Playback their own tabs (P8). However, those systems serve forensic analysts who spend 80% of their time in Playback. ROC's primary operator runs surveillance shifts — Live is the dominant mode. The cleaner pattern is to **bind the global time-cursor to either "now" or "scrubbed past"** (Datadog / Grafana / Foxglove pattern). Concretely:

- In Operate, a time-cursor strip at the bottom (Grafana time-range picker style) defaults to "Live".
- Dragging it into the past *automatically swaps the data source* from `ws://127.0.0.1:8765` to the most recent recording matching that timestamp, and the EKF overlay re-binds to the recorded `trackPool`.
- The masthead shows a prominent `LIVE` ⇄ `REPLAY` badge so the operator never confuses the two (XProtect / Genetec lesson).
- Defensive-engagement actuator commands are *hard-disabled* in REPLAY (safety interlock per §6 Engage workspace).

This avoids the 5th-destination cost while preserving the cognitive-mode separation that VMS systems prove operators need.

### Rec 8 — Performance HUD belongs inside Inspect, not as a top-level destination.

Datadog / Grafana exist *as* performance HUDs because they have nothing else. ROC has tracking work to do. The performance HUD (frame timing, model latency, GPU util, JS-GC pressure) is one panel-grid inside the Inspect workspace, surfaced by hotkey `i` (Inspect default panel). Survey precedent: XProtect `System Monitor` tab, RViz status-tree colours, DeepStream `enable-perf-measurement`.

### Rec 9 — Operator profile / preset surfacing follows OBS, not Datadog.

OBS's Profile concept is the right shape — "select profile at session start, never think about it during operation". Datadog's per-user dashboard sharing is a multi-user enterprise pattern that's overkill for ROC's single-machine deployment. Implement Profile as a startup modal + a left-rail Settings drawer. No mid-shift profile-switching UI needed.

### Rec 10 — Hierarchy Explorer is a side-drawer on Inspect, not a destination.

Survey reference: Palantir Gotham's entity-graph navigation, Foxglove's `TF Tree` panel (transform-frame tree as a side panel, not a tab), and the file-tree-on-side IDE convention. ROC's `parent_id` hierarchy graph is the data model. Implementation:

- Inspect workspace gains a left side-drawer (collapsible) listing every confirmed parent track (typically `person`) as a row; expanding a row reveals child sub-features (`head`, `upper_torso`, `accessory:hat`, `accessory:backpack`).
- Clicking a node highlights it in the Operate canvas (cross-workspace highlight, surviving navigation).
- Right-click on a node opens the "Pivot to" menu: *Replay this track's last 30 s*, *Lock actuator on this track* (only in Defensive Engagement profile), *Mark for review*, *Export track history*.
- The drawer is also where `hierarchy_violation` events (from `phase_2_ekf_kinematics.md` §3.11.4) surface as red-bordered rows — operator can investigate without leaving Inspect.

### Rec 11 — Calibration must surface intrinsics + extrinsics + EKF Q_c tuning + manifest selection in one workspace.

Calibrate (renamed from Kinematic Trajectory Controller) is the workspace that the survey shows is *most* prone to "where the hell is that knob" sprawl. Mitigation: structure Calibrate as a **stepwise wizard** (RViz `2D Pose Estimate` + Skydio per-vehicle setup wizard precedent), top-to-bottom:

1. **Camera intrinsics** — `K`, distortion coefficients (file picker for `.yaml`).
2. **Camera extrinsics** — `R_CW`, `t_CW` against the world frame; live re-projection error read-out.
3. **EKF tuning** — `Q_c` per-class (CWNJ defaults from `phase_2_ekf_kinematics.md` §2.4), NIS regime thresholds, $\kappa_\text{disagree}$ from §3.10.1.
4. **Model manifest selection** — opens the Models drawer (Rec 5).
5. **Save as new Scenario** — promotes the calibration session into a reusable preset.

Each step is collapsible; an unfilled step shows a red-dot indicator (P7 status colouring).

### Rec 12 — Treat the 11 future features as content for 4 destinations + 2 drawers + a hotkey table — not as 11 new tabs.

Mapping every brief-named future feature to a concrete home:

| Feature (from brief) | Home in proposed IA | Survey precedent |
|---|---|---|
| 1. Live camera feed | **Operate** canvas, Layer 0 (z-ordered under overlay) | Foxglove `Image` · RViz `Camera` · OBS Video Capture |
| 2. EKF state visualisation | **Operate** canvas, Layer 2–3 (overlay on the live surface) | RViz Markers / Pose / Path · DeepStream OSD |
| 3. Model selector / manifest browser | Left-rail **Models** drawer (Rec 5) | Ultralytics HUB · ReShade preset list |
| 4. Recording / Replay | **Operate** time-cursor REPLAY mode (Rec 7) | XProtect Playback tab (mode-swap analogue) |
| 5. Calibration tools | **Calibrate** wizard (Rec 11) | RViz `2D Pose Estimate` · Skydio per-vehicle setup |
| 6. Actuator control + safety interlocks | **Engage** workspace (Rec 1) | QGC Vehicle Actions · Skydio Pause slider |
| 7. Hierarchy explorer (parent → child) | **Inspect** side-drawer (Rec 10) | Palantir Gotham objects · Foxglove TF Tree |
| 8. Per-track metrics + history | **Inspect** panel grid (Datadog template variables) | Datadog `$track_id` · Foxglove State Transitions |
| 9. Operator profile / preset system | Startup modal + left-rail **Settings** drawer (Rec 3 + Rec 9) | OBS Profile · Grafana Library Panels |
| 10. Hotkey / quick-switch UI | Global key registry (Rec 6) + `?` overlay | RViz toolbar · ReShade Toggle Keys · OBS Hotkeys |
| 11. Performance HUD (frame timing, GPU util, GC) | **Inspect** panel grid (one of the default panels) | XProtect System Monitor · DeepStream perf-measurement |

Eleven features, six homes. None require a 5th destination.

---

## 7. End-to-end Operator Workflow Walkthroughs (proof the recs hold up)

**A. Surveillance shift (8 hours, 50 tracks peak).**

1. Operator launches `roc_vision.exe`; dashboard opens to **Operate** with the `Surveillance Shift` scenario auto-selected from Profile.
2. Single canvas shows live camera feed (Layer 0) + bbox+label (Layer 2) + EKF ellipses (Layer 3). Operator scans visually.
3. Anomalous behaviour spotted on track 17 → operator hits `i` to jump to **Inspect**, track 17 highlights with per-track confidence-over-time plot and NIS regime histogram. Operator confirms: track is healthy.
4. Hits `1` to return to Operate. Continues scanning.

**B. Calibration session (after rig re-mount).**

1. Operator hits `3` → **Calibrate** wizard starts. Step 1 file-picks the new intrinsics YAML; step 2 lets them click 4 known-world points on a test target in the live canvas; step 3 leaves EKF defaults; step 4 keeps current model. Save → "ROC-LobbyCam-2026-05-18" Scenario.
2. Switches to that Scenario from the masthead dropdown. Done.

**C. Forensic review (post-event).**

1. Operator hits `1` → **Operate**, then drags the bottom time-cursor into the past. Masthead badge flips `LIVE → REPLAY`. Engage hotkeys disabled (safety).
2. Same composited canvas plays back recorded video + recorded `trackPool`. Operator scrubs.
3. Identifies that target lock briefly snapped between head and torso → hits `i` for **Inspect** → reads the `hierarchy_violation` event log (per `phase_2_ekf_kinematics.md` §3.11.4).
4. Marks the moment with a bookmark, exports a 10-second clip from a recording controls panel inside Inspect.

**D. Defensive engagement (one trained operator).**

1. Profile = `Defensive Engagement`. Operator opens **Engage** (hotkey `4`).
2. Engage workspace shows the same canvas + a serial-actuator slew indicator + safety-interlock checklist (camera live? tracker healthy? operator authenticated? actuator armed?). All 4 must be green.
3. Operator clicks a person in the canvas → target-lock confirmation dialog (Skydio Pause-slider style; anti-fat-finger) → confirms.
4. Pan/tilt head slews to that track's projected image-plane coordinates; the slew indicator overlays the canvas in real time.
5. `Space` to pause / `Esc` to release lock.

All four workflows live inside 4 first-class destinations + 1 left-rail Models drawer + 1 left-rail Settings drawer. None requires a 5th tab.

---

## 8. Risk Analysis

| # | Risk | Severity | Mitigation |
|---|---|---|---|
| R1 | **Operator confusion** if existing locked labels (Observer/Spatial/Kinematic/Hardware) are kept but workflow expectations diverge from the survey's task-named patterns. | **High** — directly maps onto Rec 1 disagreement. | Re-label inside the locked 4-count; document mapping in `dashboard/architecture_plan.md`. Parent synthesises wording. |
| R2 | **Screen real-estate exhaustion** if every future feature gets a panel slot. With 11 stated features on a 1920×1080 display, even at 320 px/panel = 6 panels max per row. | **Medium** | Strict P10 two-tier preset compartmentalisation; presets *hide* features that aren't relevant to the current Scenario. |
| R3 | **Hot-path regression** if recommendations inadvertently couple `aiming_config` or `hardware_config` to the `trackPool` / RAF loop. Locked rule violation. | **High** — pre-tripwired by `.cursor/rules/200-dashboard-telemetry.mdc`. | All new panels follow the existing `aiming_config` / `hardware_config` precedent. Composited video layer is a `<video>` element under the SVG, **not** another reactive store reading from `trackPool` — the canvas RAF loop is unchanged. |
| R4 | **Locked-4 rule breach** if the parent agent reads Rec 1+4 as "left rail → 5+ destinations". | **High** — explicit constraint. | Rec 4 keeps exactly 4 first-class destinations on the rail; Models + Settings are *drawers*, not destinations. Workspace count unchanged; layout chrome only. |
| R5 | **Composited-canvas video sync drift** — `<video>` element and SVG overlay can fall out of frame-perfect sync (web video frame-rate is a stream of `requestVideoFrameCallback` events, not RAF-synchronised in general). | **Medium** | Use `HTMLVideoElement.requestVideoFrameCallback()` to drive an "EKF interpolation request" event with the exact presentation timestamp; SVG layer renders at that timestamp using cubic-Hermite interpolation from `phase_5_telemetry_ui.md` §7. Already the architecture's design — no new work, just be explicit. |
| R6 | **Replay-mode actuator misfire** — operator forgets they're in REPLAY and commands a slew. | **Critical** (defensive-engagement use case). | Hard-disable all `Engage` workspace controls when masthead badge shows REPLAY (Rec 7). Bonus: log every command issued while in LIVE for audit. |
| R7 | **Scenario / Profile sprawl** — users author 30 scenarios over 6 months, none of them documented; "operational tribal knowledge in JSON". | **Medium** | Bake the 6 default scenarios into the build (Surveillance Shift / Calibration / Forensic / Defensive / Behavioural / Bring-Up). User-authored scenarios always tagged with `author` + `created_at` + `description`. Foxglove organization-layouts pattern. |
| R8 | **Hotkey conflict with browser shortcuts** (e.g., `Ctrl-R` reloads page). | **Low** | Avoid `Ctrl`/`Cmd` combos that browsers reserve; default to bare letters with the focused-input guard (Rec 6). Document deviations from common conventions in the help overlay. |
| R9 | **Left-rail nav added without keeping the locked rule's "4-domain" structural mandate intact.** | **High** | Left rail = 4 destination icons + 2 drawer icons (visually grouped, separator between). The `CONTROLLER_DOMAINS` array stays length-4 in the code; only the rendering chrome changes. |
| R10 | **Bot-blocked Anduril primary source** during this research means my Lattice analysis is one degree more inferential than Foxglove / RViz. | **Low** (the recommendations don't hinge on Lattice-specific patterns) | The two Lattice take-aways (single-screen multi-asset, node-portable) are both confirmed in the [EX2 long-form article](https://www.ex2.com.au/uncategorized/in-detail-anduril-launches-extended-lattice-os/) and the Anduril public videos cited in §11. |
| R11 | **Composited `<video>` element introduces a second media stream** — increases bandwidth, decode CPU/GPU cost, and a sync requirement against the EKF telemetry stream. | **Medium** | Acceptable on the locked single-machine deployment (no network constraint); decode goes through the OS hardware-decoder; sync handled via `requestVideoFrameCallback` per R5. Document the GPU-memory budget in the implementation PR. |
| R12 | **Drawer over-use** — if Models and Settings drawers grow to host more features, the IA implicitly creeps from 4 destinations + 2 drawers to 4 + N. | **Medium** | Treat drawers as cross-cutting *selectors*, not destinations. The acid test: if a workflow requires staying in the drawer for >5 seconds, it should be a destination instead. Models browser fails this test sometimes (filtering 113 ONNX files takes time); reserve the right to promote Models to a 5th destination ONLY by amending `200-dashboard-telemetry.mdc`, not silently. |

---

## 9. Where the locked 4-domain matrix is genuinely wrong (saying so loudly per the brief)

The current 4 are **not** wrong in *cardinality*; they are wrong in *labelling*. Specifically:

- **"Observer Matrix"** sounds like a control-theory concept (`y = Cx`), not an operator destination. Operators in every comparable system call this surface either *Inspect*, *Diagnostics*, or *Metrics*. The current name will not survive contact with a real operator's mental model.
- **"Spatial Projection Canvas"** is technically what the panel does (image-plane re-projection of world-frame EKF state) but operators call this surface *Live*, *Operate*, or *Watch*. Naming it after the math is a tell that the design hasn't yet been used in a shift.
- **"Kinematic Trajectory Controller"** is a misnomer — the panel today contains `aiming_config` smoothing / deadband / profile, which is *tuning* / *calibration*. There is no live trajectory control in the panel today.
- **"Hardware Actuation Interface"** is closest to right — but operators call this *Engage*, *Drive*, or *Command*.

**Verdict:** the matrix is structurally fine; the *labels* are mathematician-named when they should be operator-named. The parent agent should choose final labels — my recommended set (Operate / Inspect / Calibrate / Engage) is one defensible synthesis, not the only one.

Cardinality of 4 holds. Don't move to 5+.

---

## 10. Addendum — "Should live capture be its own tab, or composited into the Spatial Projection Canvas?"

**Recommendation: composited into the same canvas. Not its own destination.**

Three primary-source citations:

1. **Foxglove `Image` panel.** [Foxglove docs](https://docs.foxglove.dev/docs/visualization/panels) lists the `Image` panel as the single live-camera panel; image annotations (boxes, lines, text from `foxglove_msgs/ImageAnnotations`) are rendered *inside the same panel* on top of the image. There is no separate "live tab" — Image annotations are composited at the panel level. This is the architectural precedent closest to ROC's situation (single camera, single overlay schema, single full-screen destination).

2. **RViz2 `Camera` display.** The [ROS 2 Jazzy RViz user guide](https://docs.ros.org/en/jazzy/Tutorials/Intermediate/RViz/RViz-User-Guide/RViz-User-Guide.html#built-in-display-types) describes the `Camera` display verbatim as: *"Creates a new rendering window from the perspective of a camera, and overlays the image on top of it"*. That single display is both the live-image viewer **and** the overlay destination. RViz deliberately gives it its own panel within the 3D viewport, not its own tab.

3. **NVIDIA DeepStream `Gst-nvdsosd` plugin.** The [DeepStream Reference Application doc](https://docs.nvidia.com/metropolis/deepstream/dev-guide/text/DS_ref_app_deepstream.html#application-architecture) shows the OSD plugin sits *inside the pipeline*, drawing "shaded boxes, rectangles and text on the composited frame using the generated metadata". The expected output screenshots in the same doc show the tiled video and the bounding-box overlay as a single image — they are not separate UI surfaces, they are not separate tabs, they are one composited frame.

**Why this is the right call for ROC specifically:**

- The defensive-engagement use case demands that *target click* happen on the same coordinates the camera image occupies. Splitting video into another tab makes click-to-lock physically impossible without re-implementing pixel-coordinate mapping.
- The forensic-playback use case ("is the system tracking smoothly or snapping between sub-features") needs the operator to see the EKF overlay *registered against the recorded video* — splitting them across tabs means the operator alt-tabs to compare, which is precisely the failure mode the question is trying to avoid.
- The hot-path render budget (`phase_5_telemetry_ui.md` §3 + `research_dashboard_tech_2026.md` §2) is already an SVG overlay that knows how to re-project world coords onto the image plane via the `K, R_CW, t_CW` triple. Adding a `<video>` element under that SVG is **zero extra render cost** — the browser composites them on the GPU. Splitting them across two tabs *adds* cost (two render contexts, twice the texture upload, sync overhead).

**Counter-argument considered (and rejected):** "But operators want a *clean* view sometimes — just video, no overlay, to verify ground truth." Acknowledged. Survey shows the right answer is a **layer-visibility toggle** (OBS eye icon, RViz display checkbox, Foxglove panel settings — all P9). Implement hotkey `v` to hide the video layer (overlay-only debug view) and hotkey `o` to hide the overlay (raw-camera-only view). Neither requires a separate tab; both are O(1) state flips. This addresses the Tesla failure-mode (no raw-video ground truth on the operator display) directly.

**Implementation contract (Phase A, parent-synthesis-friendly).** The current `Spatial Projection Canvas` panel becomes the composited Operate surface:

```html
<!-- conceptual layering inside Operate -->
<div class="operate-canvas">
  <video class="layer-0 video" autoplay muted /> <!-- bottom; toggleable by `v` -->
  <canvas class="layer-1 trajectories" /> <!-- Phase B promotion of trajectory paths -->
  <svg class="layer-2-3 overlay" /> <!-- bounding boxes + EKF ellipses + hierarchy edges -->
  <div class="layer-4 reticle" /> <!-- target-lock crosshair, slew indicator -->
</div>
```

The SVG overlay's existing world-to-image reprojection (`phase_5_telemetry_ui.md` §7.4) is unchanged; only Layer 0 (`<video>`) is added, and it is bound to a separate MediaSource stream that does *not* go through the binary WebSocket frame format (video is its own dedicated transport — e.g., WebRTC or HLS — orthogonal to the EKF telemetry stream). This means no change to `decode.ts`, no change to the `trackPool` schema, no change to `200-dashboard-telemetry.mdc`. The composited surface is therefore a *pure additive* change to the existing locked architecture.

---

## 11. References

All URLs accessed 2026-05-17 unless noted otherwise. Cached copies under `.firecrawl/dashboard_ux/` and `.firecrawl/console-research/`.

1. **Foxglove Studio — Panels.** Foxglove Technologies. `https://docs.foxglove.dev/docs/visualization/panels`. Cached: `.firecrawl/console-research/foxglove-panels-index.md`. Accessed 2026-05-17.
2. **Foxglove Studio — Layouts.** Foxglove Technologies. `https://docs.foxglove.dev/docs/visualization/layouts`. Cached: `.firecrawl/dashboard_ux/foxglove-layouts.md`. Accessed 2026-05-17.
3. **RViz User Guide (ROS 2 Jazzy).** Open Source Robotics Foundation. `https://docs.ros.org/en/jazzy/Tutorials/Intermediate/RViz/RViz-User-Guide/RViz-User-Guide.html`. Cached: `.firecrawl/console-research/rviz2-user-guide.md`. Accessed 2026-05-17.
4. **DeepStream 9.0 Reference Application — deepstream-app.** NVIDIA Corporation. `https://docs.nvidia.com/metropolis/deepstream/dev-guide/text/DS_ref_app_deepstream.html`. Cached: `.firecrawl/dashboard_ux/deepstream-ref-app.md`. Accessed 2026-05-17.
5. **Mission Autonomy — Anduril.** Anduril Industries. `https://www.anduril.com/lattice/mission-autonomy`. *(Primary URL bot-blocked during scrape; meta/description captured in `.firecrawl/console-research/05-anduril-search.json`.)*
6. **Ferguson, Gregor. "IN DETAIL: Anduril launches extended Lattice OS."** EX2 Defence Analysis (Australia). 25 May 2023. `https://www.ex2.com.au/uncategorized/in-detail-anduril-launches-extended-lattice-os/`. Cached: `.firecrawl/dashboard_ux/anduril-ex2.md`. Accessed 2026-05-17.
7. **Anduril. "Lattice for Mission Autonomy: An Unfair Advantage for Defense."** Video. `https://www.youtube.com/watch?v=RpFFScTovII`. Accessed 2026-05-17.
8. **Skydio Remote Ops.** Skydio Inc. `https://www.skydio.com/software/remote-ops`. Cached: `.firecrawl/dashboard_ux/skydio-remote-ops.md`. Accessed 2026-05-17.
9. **QGroundControl Fly View (master).** QGroundControl Project. `https://docs.qgroundcontrol.com/master/en/qgc-user-guide/fly_view/fly_view.html`. Cached: `.firecrawl/dashboard_ux/qgc-fly-view.md`. Accessed 2026-05-17.
10. **OBS Studio — Sources Guide.** Open Broadcaster Software. `https://obsproject.com/kb/sources-guide`. Cached: `.firecrawl/dashboard_ux/obs-sources.md`. Accessed 2026-05-17.
11. **OBS Studio — Scene Collections.** Open Broadcaster Software. `https://obsproject.com/kb/scene-collections`. Cached: `.firecrawl/dashboard_ux/obs-scene-collections.md`. Accessed 2026-05-17.
12. **OBS Studio — Profiles.** Open Broadcaster Software. `https://obsproject.com/kb/profiles`. Accessed 2026-05-17 (via 10 + 11 cross-references).
13. **ReShade — Setup Guide.** FRAMED Screenshot Community / TheGordinho. `https://framedsc.com/ReshadeGuides/setupreshade.htm`. Cached: `.firecrawl/dashboard_ux/reshade-setup.md`. Accessed 2026-05-17.
14. **Grafana — Build dashboards.** Grafana Labs. `https://grafana.com/docs/grafana/latest/visualizations/dashboards/build-dashboards/`. Cached: `.firecrawl/dashboard_ux/grafana-build-dashboards.md`. Accessed 2026-05-17.
15. **Datadog — Dashboards.** Datadog Inc. `https://docs.datadoghq.com/dashboards/`. Cached: `.firecrawl/dashboard_ux/datadog-dashboards.md`. Accessed 2026-05-17.
16. **Bevilacqua, Fernando. "cvui: A GUI lib built on top of OpenCV drawing primitives."** LearnOpenCV / OpenCV.org. 21 June 2017. `https://learnopencv.com/cvui-gui-lib-built-on-top-of-opencv-drawing-primitives/`. Cached: `.firecrawl/dashboard_ux/cvui-overview.md`. Accessed 2026-05-17.
17. **Dovyski. cvui — A (very) simple UI lib built on top of OpenCV drawing primitives.** GitHub repository. `https://github.com/Dovyski/cvui`. Accessed 2026-05-17.
18. **Klender, Joey. "Tesla Autopilot visualization gets big upgrade with tons of new additions."** Teslarati. 13 October 2025. `https://www.teslarati.com/tesla-autopilot-visualization-big-upgrade-new-additions/`. Cached: `.firecrawl/dashboard_ux/tesla-viz-upgrade.md`. Cites @greentheonly Tesla-firmware reverse-engineering. Accessed 2026-05-17.
19. **Klender, Joey. "Tesla just fixed a four-year-old bug with Full Self-Driving visualization."** Teslarati. 9 October 2024. `https://www.teslarati.com/tesla-just-fixed-four-year-old-bug-with-full-self-driving-visualization/`. Accessed 2026-05-17 (linked from 18).
20. **Ultralytics — Model Prediction with Ultralytics YOLO.** Ultralytics. `https://docs.ultralytics.com/modes/predict`. Cached: `.firecrawl/dashboard_ux/ultralytics-predict.md`. Accessed 2026-05-17.
21. **Ultralytics HUB.** Ultralytics. `https://hub.ultralytics.com/`. Accessed 2026-05-17 (via predict-docs cross-reference).
22. **Ultralytics — Inference (Platform).** Ultralytics. `https://docs.ultralytics.com/platform/deploy/inference/`. Accessed 2026-05-17.
23. **Milestone XProtect Smart Client — User interface overview.** Milestone Systems. `https://doc.milestonesys.com/en-US/bundle/doc1055_2025r3/page/content/standard_features/sf_sc/sf_funda/sc_uioverview.htm`. Cached: `.firecrawl/console-research/01-milestone-search.json`. Accessed 2026-05-17.
24. **Milestone XProtect Smart Client — Tabs explained.** Milestone Systems. `https://doc.milestonesys.com/2020R3/en-US/standard_features/sf_sc/sf_common/sc_tabsexplained.htm`. Accessed 2026-05-17.
25. **Genetec Security Center 5.13 — About Security Desk.** Genetec Inc. `https://techdocs.genetec.com/r/en-US/Security-Center-User-Guide-5.13/About-Security-Desk`. Cached: `.firecrawl/console-research/02-genetec-search.json`. Accessed 2026-05-17.
26. **Genetec Security Center 5.13 — Monitoring in Security Desk.** Genetec Inc. `https://techdocs.genetec.com/r/en-US/Security-Desk-Getting-Started-Guide-5.12/Monitoring-in-Security-Desk`. Accessed 2026-05-17.
27. **Palantir — Gotham product page.** Palantir Technologies. `https://www.palantir.com/platforms/gotham/`. Cached: `.firecrawl/console-research/06-palantir-search.json`. Accessed 2026-05-17.
28. **NVIDIA Metropolis platform overview.** Reference: DeepStream is the inference-graph layer of Metropolis; the Metropolis Microservices stack ships separately for the multi-camera dashboard layer. Pattern confirmation via 4 + ([Sample Configurations and Streams](https://docs.nvidia.com/metropolis/deepstream/dev-guide/text/DS_sample_configs_streams.html)). Accessed 2026-05-17.

**Constraint-gate note.** None of the recommendations in §6 violate the constraint-gate from `.cursor/rules/300-research-standards.mdc` (latency <33 ms, Windows x64, zero hot-path heap allocation, double-precision EKF math). The reason is structural: every recommendation operates at the dashboard *chrome* (sidebar, masthead, drawer, hotkey registry, preset persistence) — none touches `TelemetryCanvas`'s RAF loop, the `trackPool`, the binary wire protocol, or the C++ orchestrator. The locked rule `200-dashboard-telemetry.mdc` continues to enforce the hot-path / config-store separation.

---

## 12. Synthesis hand-off summary (for the parent agent)

The single sentence the parent should keep, even if everything else gets compressed:

> ROC's locked 4-domain count is correct; the labels and the live-capture-tab placement are wrong. Re-label to operator-task names (Operate / Inspect / Calibrate / Engage), composite live video underneath the existing EKF overlay in the Operate workspace, add a saved-Scenario × Profile preset layer, and route the remaining 11 future features through 2 left-rail drawers (Models, Settings) + an Inspect panel-grid — not new tabs.

Three patterns the dashboard does not currently have and will hurt without:

1. **Composited live-video layer underneath the EKF overlay** (universal across 10 of 12 surveyed systems — P1).
2. **Saved scenario × profile preset system** (universal across 7 of 12 surveyed systems — P2 + P10; nearest-precedent OBS).
3. **Hotkey table with single-letter destination switching + layer toggles** (universal across operator-grade systems — P5; nearest-precedent RViz).

If the parallel game-CV-overlay research agent's output diverges from these three, the divergence is the synthesis point worth interrogating; everything else here should compose cleanly.

---

*End of research document. Final synthesis with the parallel game-CV-overlay agent's output is the parent's responsibility.*
