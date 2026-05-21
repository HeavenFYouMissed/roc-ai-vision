# Phase 5 Research: Robotics + ML Pipeline Introspection Operator Consoles

> **ROC AI Vision** · Lead Author: Lead Robotics & CV Research Scientist
> **Scope:** `dashboard/` menu architecture — vertical = robotics + ML-pipeline tooling
> **Sibling verticals (out of scope):** surveillance/NVR consoles, broadcast monitoring tools
> **Date:** 17 May 2026
> **Focus:** Lock the SvelteKit `NavigationShell` panel hierarchy before adding live camera
> and 113-model collection ad-hoc

---

## 0. Problem Statement

The current `NavigationShell` exposes a **4-Domain Controller Matrix** — Observer Matrix,
Spatial Projection Canvas, Kinematic Trajectory Controller, Hardware Actuation Interface.
That naming is *controller-biased*: 3 of 4 nodes are labelled as controls. Inspection of
nine production-grade robotics / ML-pipeline introspection consoles shows the inverse:
**~75% of operator surface is observation**, ~20% is configuration, and only ~5% is
real-time control. Before adding tabs for the live camera feed and the 113-model ONNX
collection, this report locks the menu architecture against documented patterns from
the canonical reference systems.

The constraint gate from §300-research-standards.mdc still applies: every adopted
pattern must be implementable inside the SvelteKit 5 + WS 8765 frame budget
(<33 ms latency, zero-GC hot path, see `phase_5_telemetry_ui.md` §3) without
re-architecting the binary frame reader (`phase_5_telemetry_ui.md` §1.5).

---

## 1. Per-System Survey

### 1.1 Foxglove Studio (open-source robotics observability)

**Top-level chrome:** single horizontal app bar (Layout / Connection menus); a
left **Sidebar** with collapsible drawers (Topics, Panel settings, Layouts, Variables,
Extensions, Performance, Studio); the rest of the viewport is a tiling
container of user-positioned **Panels**. [Foxglove docs · "Panels"][fox-panels].

**Panel catalogue (22 panels, scraped 17 May 2026):** Plot, Raw Messages, Publish,
Diagnostics, Table, State Transitions, Indicator, Parameters, Map, Log, **3D**,
Variable Slider, Tab, **Image**, Gauge, Markdown, Service Call, Topic Graph,
TF Tree, Teleop, Audio, Data Source Info. [Foxglove sitemap map][fox-map].

| Panel | Shows (data type) | Controls (passive/interactive) | Rationale |
|---|---|---|---|
| **3D** | TF-rooted 3D scene: meshes, URDF, point clouds, occupancy grids, markers, scene entities, depth-as-pointcloud | Camera orbit/select; topic-visibility cog; per-topic colour / opacity / frame lock | Single fused scene avoids N camera views; TF unifies frames over time [fox-3d] |
| **Image** | Raw + compressed image (jpg, png, webp, avif) + compressed video (h264, h265, vp9, av1) with 2D ImageAnnotations overlay | Source dropdown; calibration topic selector; superimpose SceneEntity from 3D space | Encoding-aware viewer + camera-calibration-aware projection in *one* panel [fox-img] |
| **Plot** | Numeric message paths, x-axis = Timestamp / Message path / Array index | Per-series Y path, sliding/fixed time window, sync zoom with other Plot/State Transitions panels | Generic time series with sub-series; sync enables multi-plot correlation [fox-plot] |
| **Diagnostics – Summary / – Detail** | ROS `diagnostic_msgs/DiagnosticArray` aggregated by hardware ID, OK/Warn/Err/Stale levels | Click row → spawns linked Detail panel; pin row; level dropdown; fuzzy search; stale timeout | Summary→Detail drill-down pattern (recurs in MLflow, W&B) [fox-diag] |
| **Teleop** | None (write-only) | Configurable D-pad publishing `geometry_msgs/Twist`; per-button axis + value; publish rate; "stop on release" | Encapsulates *the* operator-control surface as a single isolated panel [fox-tele] |
| **Topic Graph** | Publisher/subscriber graph from `rosgraph` | Pan/zoom; node-name filter | Audit "who is producing what" without dropping to a terminal |
| **Parameters** | Live ROS parameter tree | Inline edit + commit | Configuration as a panel, not as a modal |
| **Publish** | None | Compose any ROS message and one-shot publish | Surgical message injection during debugging |
| **Service Call** | Service response | Service name + JSON request input → click Call | Same drill-down model as ROS services |
| **State Transitions** | Boolean / enum value over time | Sync zoom with Plot | Step-pattern visualization of mode changes |
| **Indicator / Gauge** | Single scalar | None | Numeric "vital sign" tile |
| **Map** | Lat/lon overlay + path | Map-tile zoom/pan | GPS-bound scene without TF overhead |
| **TF Tree** | Visual `tf2` tree | Tree expand/collapse | Frame-graph debug |
| **Tab** | Container for other panels | Add/rename tab | Layout-within-layout |

**Layout / Blueprint behaviour.** Each panel stores its config as **a JSON object**
exposed through "Import/Export Settings"; the full multi-panel arrangement is a
named **Layout**, also JSON, also import/exportable as a file [fox-panels]. Drag-and-drop
from the Topics drawer auto-routes a topic to the *right* panel type (image → Image
panel; numeric path → Plot/State Transitions; any topic → Raw Messages/Table) — a
schema-driven affordance, not user training.

**Shortcuts (verbatim):** `,` opens panel settings sidebar; `Cmd+A` selects all
panels; `Cmd+B` toggles sidebar; hover + backtick shows per-panel shortcuts.

---

### 1.2 Rerun (multimodal data viewer)

**Top-level chrome:** single window; **Blueprint Panel** (left), **Selection Panel**
(right), **Time Panel** (bottom), and the central **Viewport** of user-defined
**Views** arranged in **Containers** [rerun-bp].

**View catalogue (10 view archetypes, scraped 17 May 2026):**

| View archetype | Shows | Controls | Rationale |
|---|---|---|---|
| `Spatial3DView` | 3D entities, transforms, lines, point clouds, meshes | Orbit camera; per-entity visualisers | Equivalent to RViz 3D and Foxglove 3D |
| `Spatial2DView` | 2D images with overlaid 2D geometry | Pan/zoom; per-entity visualisers | Mirrors Foxglove Image panel |
| `TimeSeriesView` | Scalars logged via `archetypes.Scalars` | Timestamp scrub; per-series toggle | Plot |
| `BarChartView` | Categorical histograms | None | Distribution shape |
| `DataframeView` | Tabular Arrow data | Sort, filter | Inspect raw rows |
| `GraphView` | Time-varying directed/undirected graphs | Node selection | Topology over time |
| `MapView` | Geospatial primitives | Pan/zoom | GPS overlay |
| `StateTimelineView` | Categorical state-change events | Scrub | Mode transition viewer |
| `TensorView` | N-D tensor (any rank) — image-style heatmaps, slices | Slice axis selectors | Inspect inference inputs/outputs |
| `TextDocumentView` / `TextLogView` | Text doc / log stream | Search, level filter | Log inspection |

**Containers:** `Grid`, `Horizontal`, `Vertical`, `Tabs` — layout is data, not
chrome [rerun-bp].

**Blueprints (the killer pattern).** A *Blueprint* is the full visual config:
panel visibility, layout, view types, per-view overrides. Blueprints are bound to
data via **Application ID** — every recording with that ID gets the same default
blueprint. Three create paths: **(1) interactive** (drag/drop, +button); **(2)
file** (`.rbl` save/load — version-controllable); **(3) programmatic** (Python
`rr.send_blueprint(rrb.Grid(rrb.Spatial3DView(...), rrb.TextLogView(...), …))`).
Reset to **heuristic** = auto-generate from data; reset to **default** = restore
saved/programmatic blueprint [rerun-bp].

**Architectural insight:** "If you can modify how something looks, you are
modifying the blueprint" — meaning the blueprint *is* the entire UI state, stored
in the same Entity-Component-System as the recording, on a separate blueprint
timeline. The viewer is a deterministic function of `(blueprint, recording)`
[rerun-bp].

---

### 1.3 ROS 2 RViz2 (canonical robotics visualisation)

**Top-level chrome:** Menu bar (File/Panels/Help) + horizontal **Tools toolbar**
+ left **Displays** list + central **3D view** + right **Views** panel + bottom
**Time** panel + status bar [rviz-ug].

**Tools (toolbar, hotkeys verbatim):** `i` Interact, `m` Move Camera (default),
`s` Select, `c` Focus Camera, `n` Measure, `p` 2D Pose Estimate, `g` 2D Nav Goal,
`u` Publish Point [rviz-ug §Tools]. Every interactive tool is **opt-in** — the
default cursor is passive (Move Camera).

**Displays (built-in display types, scraped from Jazzy user guide):**

| Display | ROS Message | What it shows |
|---|---|---|
| Axes | — | A set of axes (no message needed) |
| Camera | `sensor_msgs/Image` + `CameraInfo` | Camera view with 3D scene overlaid via calibration |
| Effort | `sensor_msgs/JointState` | Per-joint torque |
| Grid | — | 2D/3D ground grid |
| Grid Cells | `nav_msgs/GridCells` | Costmap obstacle cells |
| Image | `sensor_msgs/Image` | Image *without* camera-info projection (cheaper than Camera) |
| InteractiveMarker | `visualization_msgs/InteractiveMarker` | Click/drag scene objects |
| Laser Scan | `sensor_msgs/LaserScan` | 2D lidar return points |
| Map | `nav_msgs/OccupancyGrid` | Ground-plane map |
| Markers / MarkerArray | `visualization_msgs/Marker*` | Programmer-controlled primitives |
| Path | `nav_msgs/Path` | Navigation path polyline |
| Point | `geometry_msgs/PointStamped` | Single sphere |
| Pose | `geometry_msgs/PoseStamped` | Arrow or axes for a single pose |
| Pose Array | `geometry_msgs/PoseArray` | Pose cloud |
| Point Cloud(2) | `sensor_msgs/PointCloud(2)` | 3D point cloud |
| Polygon | `geometry_msgs/Polygon` | Polygon outline |
| Odometry | `nav_msgs/Odometry` | Accumulated odometry trail |
| Range | `sensor_msgs/Range` | Sonar/IR cone |
| RobotModel | — (TF + URDF) | Articulated robot in current pose |
| TF | — | tf2 hierarchy |
| Wrench / Twist | `geometry_msgs/Wrench/TwistStamped` | Force / velocity arrows |

**View controllers:** Orbital (default), FPS, Top-down Orthographic, XY Orbit,
Third Person Follower. Each defined by `(controller type, target frame, camera
pose)` and **saved per user** (not in config file) [rviz-ug §Views].

**Frame discipline.** Two coordinate frames are first-class menu citizens:
**Fixed Frame** (world reference; if mis-set "all objects ever seen appear in
front of the robot") and **Target Frame** (the camera-look reference) [rviz-ug
§Coordinate Frames]. Misuse is the #1 RViz support question; surfacing them in
the chrome is a deliberate UX choice.

**Configurations file.** Display list, tool properties, viewpoint dumped to YAML
and reloadable — same pattern as Foxglove Layouts and Rerun Blueprints.

---

### 1.4 NVIDIA DeepStream Graph Composer (pipeline graph editor)

**Top-level chrome (Composer, DeepStream 8.0):** menu bar — File / Edit /
**Graph** / **Registry** / Tools / Window / Help — plus toolbar shelf, central
**Graph Canvas**, right-side **Property Window**, left-side stack of
**Extension Grouping** + **Extension Search Bar** + **Component List**, and bottom
**Console** [ds-gc].

| UI surface | Shows | Controls | Rationale |
|---|---|---|---|
| Graph Canvas | Directed graph of GStreamer/GXF nodes with typed input/output ports | Drag-drop components from list; wire ports; right-click context menu | The pipeline *is* the document |
| Property Window | Selected node's property schema (type-aware fields) | Inline editor | One inspector window, multiplexed by selection |
| Component List | Hierarchical (by extension or by workflow category) catalogue of all available `INvDsElement`/GXF components fetched from registry on `127.0.0.1` | Drag-drop, double-click to drop into canvas | Catalogue-as-palette pattern |
| Toolbar / Graph menu | — | Run Graph / Stop Graph / **Debug View** / Focus Selection / Layout All / Expand All / Minimize All / Close All | Pipeline lifecycle as toolbar buttons |
| Registry menu | — | Sync Repo / Clear Cache | Decouples palette content from binary |
| Console | Build + runtime output | None | Always-visible log strip |

**Hotkeys (verbatim, DeepStream 8.0):** `Ctrl+N/O/S/W/Q` (File), `Ctrl+C/V/Z/Y`
+ `Del` (Edit). Run/Stop graph live in the Graph menu without a default hotkey —
a deliberate guard against accidental run.

**Design choice worth quoting:** node-expansion has three states (`Expand`,
`Minimize` = ports only, `Close` = hide all ports) so a 50-node graph stays
legible. Auto-`Layout All` is one click.

---

### 1.5 NVIDIA Isaac Sim / Isaac ROS (simulation + perception panels)

**Top-level chrome (Isaac Sim 4.5):** Menu bar — Create / Window / Tools /
Utilities / **Layout** — plus a **Main Toolbar** (Select, Move Global/Local,
Rotate Global/Local, Scale, Snap, Select Mode, **Play / Stop**) and a default
docked tab arrangement: **Viewport** (centre), **Stage** (USD scene hierarchy,
right), **Property Panel** (right under Stage), **Browsers** (asset & example,
left) [isaacsim-uiref].

| Panel | Shows | Controls |
|---|---|---|
| Viewport | 3D RTX-rendered USD scene | Camera nav; Play/Stop sim |
| Stage | USD prim tree | Show/hide; select; reorder |
| Property Panel | Selected prim's USD attributes | Inline edit any attribute |
| Browsers | Asset library + Example library (drag-drop) | Drag asset into Viewport |
| Layout menu | — | Switch between predefined layout templates (per workflow) |
| Tools menu | — | Animation, Physics, Replicator (synthetic data), Robotics, USD |
| Utilities menu | — | Debugging, extension templates |

**Workflow modes (orthogonal to panels):** GUI / **Extensions** (Python hot
reload) / **Standalone Python** (headless or scripted) — the same scene works
in all three [isaacsim-wf]. The Extensions model is what allows Isaac to ship
new panels without rebuilding the app.

**Layout templates.** The Layout menu maps a *use case* (e.g. assemble robot,
attach sensors, ROS bridge) to a panel arrangement — the same pattern as Rerun
Blueprints and Foxglove Layouts, but pre-curated by NVIDIA. Tabs are
draggable and can be **detached to a separate OS window** for multi-monitor use.

---

### 1.6 Intel OpenVINO DL Workbench (now deprecated, archived 2022)

Web GUI at `http://127.0.0.1:5665/`. Five conceptual sections, navigated as a
wizard-style top-level workflow rather than a free-tile dashboard
[openvino-dlwb]:

| Section | Shows | Controls |
|---|---|---|
| **Model Evaluator** | Imported model's framework (TF / ONNX / Caffe / MXNet / Kaldi) + format (FP32, IR, INT8) | Choose model file or pick from "model downloader" |
| **Target Environment** | CPU / iGPU / VPU / remote target | Click target chip |
| **Dataset Selector** | MS COCO / Pascal VOC / ImageNet / CSS / CSR / generated random | Click dataset |
| **Optimizer / Quantizer** | Layer-wise + kernel-level performance bars (FP32 vs INT8) | DefaultQuantization / AccuracyAwareQuantization toggle; calibration set size |
| **Accuracy Checker** | mAP / top-1 / IoU vs FP32 baseline | Run-on-dataset button |
| **Deployment Package Manager** | Bundle contents (model + cfg + run results) | One-click export to zip |

**Pattern adopted by ROC scope:** the **per-layer, per-kernel performance
breakdown** is the only UI that lets a model author trace a single slow inference
back to the responsible op. ONNX Runtime's profiler (`OrtTuningPerf`) exposes the
same data but without the GUI — a recurrent ML-pipeline-introspection need
(see §1.9 Hailo).

**Status note.** DL Workbench was retired in OpenVINO 2022.3. Pattern is still
canonical for the model-introspection vertical; references are kept as
deprecated-but-instructive.

---

### 1.7 Roboflow Workspace UI (dataset + model + deployment dashboard)

**Hierarchy (verbatim from docs, accessed 17 May 2026):** `Workspace → Projects
→ Dataset Versions → Models → Workflows → Deployments` [robo-keyconcepts]. This
is a **strict 6-level breadcrumb**, not a free dashboard.

| Level | Shows | Controls | Why |
|---|---|---|---|
| Workspace | Team, billing, all projects | New project; invite teammates | Tenancy boundary |
| Projects | Images, annotations, history, per-project type (Object Detection / Classification / Instance Segmentation / Keypoint Detection / Semantic Segmentation / Multimodal) | Upload, label, version | Task type pins available architectures |
| Dataset Versions | Snapshot (frozen, immutable) | Generate version, augment, split | Reproducibility for training |
| Models | Trained model per (Dataset Version, Architecture) tuple; perf metrics | Train, upload weights, deploy | Each model permanently bound to a version |
| Workflows | Pipeline of (model → post-processing → outputs) | Wire steps | Composable inference |
| Deployments | Deployed inference endpoints | Roll versions, monitor latency | Production fan-out |

**Adopted pattern.** Roboflow's 113-model-equivalent inventory problem is solved
via *Projects* as the partitioning axis, **never** via tabs. Each project shows
its own model list; the workspace does not flatten 113 entries into one
navigator.

---

### 1.8 MLflow + Weights & Biases (model tracking, ML A/B)

**MLflow** [mlflow-track][mlflow-vis]:

| Surface | Shows | Controls |
|---|---|---|
| Experiments list (left nav) | Tree of experiments | New / archive / search |
| Runs table (per experiment) | Rows = runs; cols = parameters, metrics, tags, model | Select rows → Compare/Delete/AddTags |
| Run detail page | Parameters, metrics, artifacts, model | Re-open, register model |
| **Chart View** | Up to 10 runs charted simultaneously | Add chart dropdown (Line / Bar / Scatter / **Parallel coordinates** / Contour / Box / Image) ; Group-by parameter; Sort-by metric |
| Compare Runs page | Side-by-side parameter + metric table | Diff selected runs |
| Model Registry | Registered models, stage (None/Staging/Production/Archived) | Promote, transition stage |

**Weights & Biases** [wandb-panels][wandb-ws]:

| Surface | Shows | Controls |
|---|---|---|
| Project navigation: Overview / Workspace / Runs / Sweeps / Reports / Artifacts / Automations / Jobs | — | Switch context |
| **Workspace** (per project) — modes: **Automated** (auto-generates panel per logged key) or **Manual** (start blank, add panels intentionally) | Sections → Panels | Drag-rearrange; reset workspace |
| Sections (within workspace) | Grouped panels by namespace | Collapse, layout per section |
| Panel types | Line Plots, Bar Plots, Scatter, **Parallel Coordinates**, Media Panels (image/video/audio), Code Diff, **Parameter Importance**, **Run Comparer**, Query Panels, Custom Charts | Per-panel edit |
| Runs Table | Rows = runs, cols = logged keys | Filter, sort, group, group-aggregate |
| **System** section (auto) | GPU util, GPU memory, CPU, RAM | None |
| Reports | Authorable narrative with embedded panels | Add text/panel blocks |

**Cross-system pattern.** Both MLflow and W&B use the **table-of-runs +
overlay-of-charts** dual surface: the runs table is the index, the chart view is
the visualisation. Compare/parallel-coordinates is the *only* way to find a
hyper-parameter / architecture cluster that beats a baseline.

---

### 1.9 Hailo TAPPAS + Hailo Dataflow Compiler Studio (embedded vision dev console)

**TAPPAS** is a GStreamer plugin library — not a GUI in itself. Visualisation of
the pipeline is provided through `gst-shark` which emits a **pipeline graph
showing connected elements and negotiated caps** [hailo-tappas]. The Hailo
GStreamer elements (relevant to ROC's pipeline view) [hailo-tappas]:

| Element | What it does |
|---|---|
| `HailoNet` | Send/receive frames to/from Hailo-8/Hailo-10H |
| `HailoFilter` | Apply post-process or drawing op to frame + tensors |
| `HailoPython` | Same as HailoFilter but via Python |
| `HailoMuxer` | Multi-device multiplexing |
| `HailoDeviceStats` | Sample power + temperature |
| `HailoAggregator` | 2-sink-1-source for cascading networks |
| `HailoCropper` | 1-sink-2-source crop for cascading |
| `HailoTileAggregator` / `HailoTileCropper` | Tile-based cascading |
| `HailoTracker` | JDE + Kalman tracker (analogous to ROC's EKF/DeepSORT) |
| `HailoRoundRobin` / `HailoStreamRouter` | Mux/demux for multi-stream |

**Hailo Dataflow Compiler (DFC) Studio** (preview, parsing stage only)
[hailo-dfc-rr]:

| Stage | UI surface | Shows |
|---|---|---|
| 1. Translate (TF/ONNX → HAR) | Parser GUI | Side-by-side: original ONNX graph vs Hailo parsed graph |
| 2. **Profiler** | Per-layer report | Expected FPS, hardware-resource utilisation, **per-layer breakdown** |
| 3. Emulator | Inference simulation | Native (FP32) vs Quantized mode output |
| 4. Optimize / Quantize | Calibration UI | FP32→INT8 with accuracy delta |
| 5. Compile to HEF | Build output | Binary + microcode |
| 6. DFC Studio (preview) | ONNX/TFLite graph viewer | User adjusts start/end nodes; side-by-side parsed graph |

**Pattern.** DFC Studio's "side-by-side original vs transformed graph" is the
canonical pattern for *trust* in any model-compilation tool, including the
ONNX-Runtime EP graph optimisations that ROC depends on.

---

## 2. Synthesis — Cross-System Patterns

### 2.1 Universal Patterns (≥3 systems — adopt by default)

| Pattern | Evidence | Adopt for ROC |
|---|---|---|
| **3-zone shell**: catalogue (left) + canvas/viewport (centre) + selection-inspector (right) | RViz (Displays / 3D / Views), Isaac Sim (Browsers / Viewport / Property), DeepStream Composer (Components / Graph Canvas / Property Window), Foxglove (Topics drawer / Tiles / Panel settings drawer) | Yes — current `NavigationShell` has only centre+top-bar |
| **Layout / Blueprint persistence as JSON** | Foxglove Layouts (JSON), Rerun Blueprints (`.rbl` + programmatic), RViz Configurations (YAML), Isaac Sim Layouts (preset templates), W&B Workspaces (REST-ref-able) | Yes — `dashboard/layouts/*.json` with import/export |
| **Drag-and-drop topic/source → panel** with schema-aware default panel | Foxglove (image topic → Image panel, scalar → Plot), DeepStream Composer (component → canvas), Isaac Sim (asset → Viewport), W&B (logged key → Workspace auto-panel) | Yes — match `cls` (class enum) → Frame Inspector; numeric → Plot |
| **Per-panel settings cog**, single shared sidebar slot, never modals | Foxglove (`,` shortcut), RViz (Display Properties), Isaac Sim (Property Panel), DeepStream (Property Window), W&B (panel edit) | Yes — replace ROC settings modals with a right-rail sidebar |
| **Time scrubber / playback panel** along the bottom edge | Foxglove (Playback), Rerun (Time Panel), RViz (Time panel for sim time), MLflow (timeline within Run page) | Yes — needed for behavioural-analysis-lab use case |
| **Coordinate-frame selector as first-class chrome** | RViz (Fixed Frame, Target Frame in top-left dropdown), Foxglove 3D (Display frame + Follow mode), Isaac Sim (Stage USD root reference) | Yes — current ROC has implicit camera frame; needs world↔camera↔actuator selector |
| **Drill-down: summary panel → detail panel spawn** | Foxglove Diagnostics (Summary → Detail), MLflow (Runs table → Compare → Run Detail), W&B (Workspace → Section → Panel → Run Detail), Roboflow (Workspace → Project → Version → Model) | Yes — replace flat 4-tab matrix with breadcrumb + drill |
| **Pre-allocated panel slots, never add/remove DOM**; visibility toggles only | Foxglove (panel container is fixed once placed, only its content swaps), `phase_5_telemetry_ui.md` §3.4 already documents this for ROC SVG nodes | Yes — already in `phase_5_telemetry_ui.md` §3.4; extend to panel containers |
| **Run/Stop/Status indicator pinned to chrome** (never in a menu) | DeepStream (toolbar Run/Stop), Isaac Sim (Play/Stop in main toolbar), Foxglove (Connection state in top bar), MLflow/W&B (Run status badge) | Yes — chrome must show pipeline state without click |
| **Catalogue of types with type-routed defaults** instead of one mega-component | Foxglove (22 panel types), Rerun (10 view archetypes), RViz (~25 display types), DeepStream (component palette) | Yes — Panel/View registry pattern beats one-size-fits-all dashboards |

### 2.2 ML / Inference-Workflow Patterns

| Pattern | Evidence | Why it matters for ROC |
|---|---|---|
| **Per-layer / per-kernel latency breakdown** for one model | OpenVINO DL Workbench (layer-wise), Hailo DFC Profiler (per-layer FPS) | Required before any of the 113 ONNX models can be chosen rationally |
| **Side-by-side original vs transformed model graph** | Hailo DFC Studio | Needed when ROC adopts ORT EP optimisation (see `phase_1_onnx_gpu_core.md`) |
| **Parallel-coordinates over (hyperparam, metric) tuples** | MLflow Chart View, W&B Parallel Coordinates | The model-selection visualisation for 113-model A/B; nothing else scales |
| **Run comparison table** with diff + group-by | MLflow Compare Runs, W&B Run Comparer | Compare 2-N ONNX runs on the same video segment |
| **Dataset Version pinning** — model is *immutably* bound to one frozen dataset version | Roboflow (Dataset Version → Model binding) | Each ONNX → fixed eval dataset; otherwise FPS/mAP numbers drift silently |
| **Per-key Auto-panel** for any newly-logged scalar | W&B Automated Workspace | New EKF residual logged → panel appears automatically without UI work |
| **Quantization toggle / accuracy delta surface** | OpenVINO Workbench, Hailo DFC | FP32 vs FP16 vs INT8 for each of 113 models |

### 2.3 Live-Camera Workflow Patterns

| Pattern | Evidence | Why it matters for ROC |
|---|---|---|
| **Encoding-aware source dropdown**, not one-tab-per-camera | Foxglove Image panel (raw + jpg + h264 + h265 + vp9 + av1 with one selector) | Avoid Camera-1/Camera-2/Camera-3 tab proliferation |
| **Camera-calibration awareness** with linked 2D↔3D projection | Foxglove (Image panel reads `CameraInfo`; 3D panel projects image as plane); RViz (Camera display vs Image display split) | Mandatory for humanoid surveillance: bbox in 2D image must back-project to world coords for the actuator |
| **Image-Annotations overlay schema** with per-annotation metadata + hover tooltip | Foxglove `ImageAnnotations` (Circle / Points / Text) | Already partially in ROC; standardise on annotation message with `id`, `cls`, `cx`, `cy`, optional `confidence` |
| **Distortion-model dropdown** (`plumb_bob`, `rational_polynomial`, `kannala_brandt`, `fisheye62`, `equidistant`) | Foxglove [fox-img] | Capture-card hardware may vary; surface the model |
| **Single 3D scene fuses N camera feeds + tracks**, not N separate views | RViz 3D, Foxglove 3D | Operator cognition: one scene, multiple cameras as projection sources |
| **Frame-rate vs display-rate decoupling** | Foxglove 3D (60-fps render bound regardless of message rate); ROC frame_version pattern in `phase_5_telemetry_ui.md` §3.5 | Already in ROC RAF loop; preserve when adding live camera |

### 2.4 Anti-Patterns (≥2 systems explicitly avoid)

| Anti-pattern | Where avoided | Mitigation |
|---|---|---|
| **One tab per camera / model / sensor** | Roboflow (Projects, not tabs); Foxglove (Image panel + source dropdown); RViz (one Camera display, source-frame switched); W&B (Workspaces+Sections, not per-run tabs) | Catalogue-and-filter, never tab-and-multiply |
| **Modal dialogs for live ops** | Foxglove (all settings in sidebar drawer); RViz (Display Properties expand in tree); Isaac Sim (Property Panel docked) | Settings live in a side drawer, never block the live frame |
| **Hidden global state with no inspector** | Rerun (explicit "blueprint is data"); Foxglove (panel state as JSON); RViz (config as YAML) | Every piece of UI state must be JSON-dumpable |
| **Inline DOM add/remove for high-frequency entities** | Foxglove 3D (pre-creates entity slots), ROC SVG slots `phase_5_telemetry_ui.md` §3.4 | Pre-allocate slot pool, toggle visibility |
| **One menu bar per panel** | Foxglove (top bar per panel = menu cog + settings cog only, no full menu); RViz (one app-level menu) | Single app menu bar; per-panel toolbar limited to 2 affordances |
| **Wizard-only workflow that forbids panel composition** | Most systems offer both wizard AND free-form (Roboflow has wizard for "Generate Version" but workspace is free); OpenVINO DL Workbench is *the cautionary tale* — wizard-only is partly why it was deprecated | Provide wizards (e.g. "Add new ONNX model") but never force them |
| **Implicit time = wall-clock everywhere** | RViz (Wall Clock vs ROS Time panel), Foxglove (Log time vs Header stamp vs Publish time selector), MLflow (Step vs Time axis switch) | Provide a time-source selector; never assume wall-clock is right |
| **Auto-publish without confirmation for actuator commands** | Foxglove Teleop ("stop on release" guard); RViz interactive tools opt-in (Move Camera is default) | Hardware Actuation must require an "Arm" gate before any publish |

---

## 3. Benchmark Table — Console Capability Comparison

> Numbers are derived from public documentation / GitHub repos (accessed 17 May
> 2026). Where a metric is not documented, the cell holds `n/d` (not documented).
> "Panels" counts named user-facing panel types in the latest docs.

| System | Panels | Layout persistence | Programmatic blueprint | 3-zone shell | Per-layer model profiling | Multi-stream image grid | License | Hot-path budget context |
|---|---|---|---|---|---|---|---|---|
| Foxglove Studio | 22 | JSON (file) | Foxglove SDK + extensions | ✅ | ❌ (out of scope) | ✅ (Tab+Image panel grid) | MPL-2.0 | Browser; ~16 ms frame budget |
| Rerun | 10 | `.rbl` + Python `rrb.*` | ✅ Native | ✅ | ❌ | ✅ (Container Grid) | Apache-2.0 | Native + WASM viewer |
| RViz2 | ~25 displays + ~8 tools | YAML | rviz_common C++ API | ✅ | ❌ | ⚠️ (multiple Camera displays, one per view controller) | BSD-3-Clause | Native Qt |
| DeepStream Composer | n/a (graph editor) | `.yaml` graph file | gxf graph composition API | ✅ | ⚠️ (via `nvinfer` profile flag, log-only) | ✅ (via streammux) | Proprietary | Native Omniverse |
| Isaac Sim | n/a (extension-based) | USD layer + Kit layout preset | Python `omni.ui` | ✅ | ⚠️ (PerfStudio extension) | ✅ (multi-viewport extension) | Proprietary | Native Omniverse |
| OpenVINO DL Workbench | 5 wizard stages | n/a (per-project DB) | REST API | ❌ (wizard) | ✅ | ❌ | Deprecated 2022.3 | Web (browser) |
| Roboflow | n/a (hierarchical CRUD) | Per-project DB | REST + Python SDK | ❌ (CRUD) | ⚠️ (per-model perf page) | ❌ | Proprietary SaaS | Web (browser) |
| MLflow | n/a (Runs / Models / Compare) | Per-experiment | REST API | ❌ (CRUD) | ⚠️ (custom plots only) | ❌ | Apache-2.0 | Web (browser) |
| W&B | 10 panel types | Workspace + section JSON, REST | `wandb_workspaces` Python SDK | ✅ (workspace + sections) | ⚠️ (System section + custom plots) | ✅ (media panel grid) | Proprietary SaaS | Web (browser) |
| Hailo TAPPAS | n/a (gst-shark graph PNG) | `.dot` graph dump | GStreamer pipeline string | ❌ | ✅ (DFC Profiler) | ⚠️ (gstreamer videomixer) | Proprietary | Native GStreamer |
| **ROC current** | 4 (Observer / Spatial / Kinematic / Hardware) | ❌ none | ❌ none | ❌ (no left/right rails) | ❌ | ❌ | This project | 144 Hz, <6.94 ms frame |

---

## 4. Risk Analysis

| Risk | Severity | Mitigation |
|---|---|---|
| **Adopting a 22-panel registry blows past `phase_5_telemetry_ui.md` 144 Hz budget** because pre-allocating DOM slots per panel multiplies the existing pool by panel count | High | Only the *active* panels mount DOM. Use Svelte 5 `{#if mounted}` keyed on the layout JSON; inactive panels live as serialized config only. Each panel type owns its own pre-alloc pool sized to its workload. |
| **Layout-as-JSON + programmatic blueprints invite drift between disk schema and runtime state** | Medium | Single source of truth: a Zod-typed `Blueprint` schema in `src/lib/blueprint/schema.ts`. Runtime state derives from `Blueprint`; any UI mutation goes through `update_blueprint()`; no direct state mutation. Mirrors Rerun's "blueprint is data" mandate. |
| **Drag-and-drop topic routing requires a topic registry** which we do not yet have on the WS bridge (binary frame, no metadata channel) | High | Extend the binary frame with a separate `MetaFrame` opcode (magic `0x5252` = "RR") sent on connect and on schema change. Contains topic list + schemas. Reuses the same WS, zero-copy. |
| **Per-layer ONNX profiling adoption (OpenVINO/Hailo pattern) requires ORT profile JSON ingestion** which exists but is heavy (~MB per session) | Medium | Profile JSON is *not* on the hot path. Dropdown: "Profile current model" → triggers off-hot-path ORT session with `enable_profiling=true` for 200 frames → publishes a one-shot `ProfileReport` message → renders to a Model Inspector panel. Never live. |
| **A free-form blueprint layout can produce a config that overflows the 32-active-target SVG slot pool** if user opens multiple Frame Inspector panels | Medium | Each panel type advertises a `slot_quota` (Frame Inspector: 64; Plot: 8 series; Pipeline Graph: 1 instance). Blueprint validator rejects layouts that exceed total quotas at load time. |
| **Hardware Actuation Interface inheriting Foxglove Teleop pattern without an Arm gate** = catastrophic for defensive robotics use case | Critical | Hard rule: actuator-publishing panels require explicit `armed=true` toggle, defaults to false on every connect + on every layout load. Foxglove "stop on release" is the *baseline* — ROC adds "armed-on-mousedown only" plus a 5-second auto-disarm. |
| **Coordinate-frame selector implicit-default (current ROC) hides RViz-class "robot stuck in front of robot" bug** | Medium | Force the chrome to display the active Fixed Frame (e.g. `world` / `camera_optical` / `actuator_base`); status bar warns when no transform exists between selected frame and detected target. |
| **Adding 113 model entries to a flat navigator** breaks every system surveyed | High | Apply Roboflow hierarchy: ROC has *one* Models view inside a Model Inspector panel, with filter / sort / group-by. Never 113 tabs. |
| **W&B-style auto-panel-per-logged-key** could spam the UI when each new ONNX adds metric keys | Medium | Default to **Manual workspace** (W&B precedent); Automated mode opt-in. Auto-generated panels go to a dedicated `_auto_*` section that can be collapsed. |
| **Detached OS-window pattern (Isaac Sim, Foxglove)** is unavailable in browser-only SvelteKit | Low | Use `window.open()` with a synced state channel (BroadcastChannel) for the secondary monitor case. Phase 6 work. |

---

## 5. Recommendations for ROC

### 5.1 Five-to-Eight Specific Menu / Panel Additions (Impact-to-Effort Ordered)

> Each recommendation cites: source system(s) → ROC use case → effort tier.
> "Effort tier" maps to `phase_5_telemetry_ui.md` slot-pool + RAF-loop budget:
> S = ≤1 day, M = ≤1 week, L = ≤2 weeks.

| # | Addition | Source pattern | ROC justification | Effort |
|---|---|---|---|---|
| 1 | **Frame Inspector panel** — live image + bbox/track-id/class overlay + EKF predicted-next-position vector, source dropdown (cam-0, cam-1, …, file replay), distortion-model dropdown | Foxglove Image panel + ImageAnnotations [fox-img]; RViz Camera display [rviz-ug] | Mandatory for humanoid-surveillance: operator must see the live frame with detection + tracking overlaid on it, in one panel with one source switch, *not* one tab per camera | M |
| 2 | **Pipeline Graph panel** — directed graph `Capture → ONNX → EKF/DeepSORT → Serial` with per-edge FPS + per-node latency tooltip + Run/Stop/Pause node-state badge | DeepStream Composer Graph Canvas [ds-gc]; Hailo TAPPAS gst-shark graph [hailo-tappas] | Makes the 4-Domain controller matrix *concrete* as a connected pipeline; operator sees in 1 second which stage is the bottleneck. For defence-audit trail, this *is* the system diagram. | M |
| 3 | **Model Inspector panel** — for a selected ONNX, show per-layer latency bar chart (ORT profile JSON ingestion), input/output tensor shapes, EP assignment (CUDA/TensorRT/CPU), file size, mAP/FPS on calibration set | OpenVINO DL Workbench layer-wise view [openvino-dlwb]; Hailo DFC Profiler [hailo-dfc-rr] | With 113 ONNX models in the personal collection, A/B selection without per-layer latency is gambling, not engineering. Required before model-swap UX is meaningful. | L |
| 4 | **Model Compare panel** — parallel-coordinates over (model_id × evaluation_video × EKF_config) tuples; axes user-selected from {mAP, FPS, false_positive_rate, average_track_lifetime, EKF_residual_norm, …}; brushable to filter | MLflow Chart View parallel-coordinates [mlflow-vis]; W&B Parallel Coordinates panel [wandb-panels] | The only proven visualisation for high-dimensional model selection at the 100-model scale. Behavioural-analysis lab will run dozens of these comparisons. | L |
| 5 | **Layout / Blueprint system** — workspace state as JSON file; named layouts in `dashboard/layouts/*.json`; layouts shippable per-customer-scenario ("EKF Tuning", "Calibrate New Camera", "Demo @ Lab", "Production Surveillance") | Rerun Blueprints [rerun-bp]; Foxglove Layouts [fox-panels]; RViz Configurations [rviz-ug] | Locks the menu architecture *as data* — the very purpose of this report. Stops feature creep: new requests become new layouts, not new tabs. | M |
| 6 | **Diagnostics / Health Strip** — fixed top-bar strip showing: capture FPS, GPU util %, ONNX inference latency, EKF residual norm, serial actuator queue depth, WS bridge connection state. Each tile colour-coded OK/Warn/Err/Stale with hover detail | Foxglove Diagnostics – Summary [fox-diag]; W&B System section [wandb-panels]; Isaac Sim toolbar status | One-glance ops health; matches "Run/Stop/Status pinned to chrome" universal pattern. Also surfaces stale-timeout (Foxglove pattern) which exposes broken sources before silent failure. | S |
| 7 | **Topic / Stream Graph panel** — publisher↔subscriber map of the WS bridge channels (which producer emits which opcode; which subscriber consumes it). Visualises the metadata frame from Risk #3. | Foxglove Topic Graph [fox-map] | Defensive-robotics audit requirement: every byte on the bridge must be traceable to a producer. Also catches duplicate-producer bugs during channel evolution. | S |
| 8 | **Frame Timeline / Scrubber** — ring-buffered last-N-seconds of binary frames (in-memory), bottom-strip scrubber with playhead and time-source dropdown (Log time / Frame seq / Wall clock) | Foxglove Playback [fox-plot]; Rerun Time Panel [rerun-bp]; RViz Time panel [rviz-ug §Time] | Behavioural-analysis lab use case: a contact / classification event happens in 200 ms; operator must scrub back through that window without losing the live tail. N=10 s × 144 Hz × 8.7 KB/frame ≈ 12.5 MB ring buffer — fits in browser memory. | M |

### 5.2 Keep From the Existing 4-Domain Controller Matrix

| Existing tab | Verdict | Mapping to surveyed systems | Action |
|---|---|---|---|
| **Spatial Projection Canvas** | **Keep, expand** | = Rerun `Spatial2DView` ∪ Foxglove 3D ∪ RViz 3D | Keep the 2D-projection role; let it host an *embeddable* 3D mode later (Phase 6) via a render-mode toggle (Foxglove 3D pattern). |
| **Kinematic Trajectory Controller** | **Keep, rename, demote control** | = RViz Pose Array + Path displays + Plot panel | Rename to **Kinematic Trajectory Inspector** (default: passive observation, like RViz Move Camera tool). Real control surface gates behind explicit Arm. |
| **Hardware Actuation Interface** | **Keep, harden** | = Foxglove Teleop + RViz Interactive Markers | Add the **Arm gate** from Risk #6 (defensive-robotics requirement). Apply the Foxglove "stop on release" pattern as the bare minimum safety. |
| **Observer Matrix** | **Split — too vague** | Currently does too many things | Decompose into: **Frame Inspector** (Recommendation 1) + **Diagnostics Strip** (Rec. 6) + **Pipeline Graph** (Rec. 2). "Observer Matrix" as a single tab is the *anti-pattern* of one-mega-component (§2.4). |

### 5.3 What to Reorganise

- **Invert the control:observer ratio.** Current matrix is 3 controllers : 1 observer.
  Every surveyed system runs ~75% observation surface : ~5% real-time control. Target
  ROC ratio after Phase 5: **5 observers (Frame Inspector, Pipeline Graph, Model
  Inspector, Compare, Diagnostics)** : **2 inspectors (Spatial Projection,
  Kinematic Trajectory)** : **1 controller (Hardware Actuation, gated)**.
- **Move from tabs to Blueprint.** The 4 current tabs become 4 *default panels in
  the default Blueprint*. Replace `NavigationShell` tab strip with: top
  Diagnostics Strip + left Catalogue rail (panel types) + central tiled
  Viewport + right Selection-Inspector rail.
- **Adopt the 3-zone shell (§2.1)** — current shell is 1-zone (centre only). Left
  rail surfaces topic/source catalogue (the MetaFrame from Risk #3); right rail
  surfaces panel settings (Foxglove cog pattern).
- **Demote "controller" naming from the chrome.** RViz / Isaac Sim / Foxglove
  never name a panel as a controller in their menu — they name it for the *data
  type* it inspects. Match: "Frame Inspector" not "Visual Controller";
  "Pipeline Graph" not "Pipeline Controller"; "Hardware Actuation" stays only
  because it really *is* a control surface.

### 5.4 What NOT to Add (Scope Discipline)

| Don't add | Reason | Reference |
|---|---|---|
| One tab per camera (Camera 1 / Camera 2 / Camera 3) | Anti-pattern in §2.4; every reference system uses source dropdown | Foxglove Image panel |
| One tab per ONNX model | Roboflow ships hierarchy, not flat tabs, for >100-model inventory | Roboflow Projects [robo-keyconcepts] |
| A "Settings" top-nav tab | Settings live per-panel in the right rail (Foxglove cog convention). Global app settings live in a single Studio drawer. | Foxglove `,` shortcut |
| A "Chat / Comments / Reports" tab in live ops | MLflow/W&B Reports exist as a *separate product surface*, not in live ops. Mixing it with live frame budget breaks the 6.94 ms RAF window. | MLflow / W&B separation |
| A "Raw Messages" / "Raw WS Frames" panel on the default layout | Foxglove ships Raw Messages but explicitly as a debug panel, off the default layout. Operators don't need it. | Foxglove Panels intro |
| A free-form drag-resize canvas for the 3D scene | Rerun & Foxglove 3D both use a fixed `viewBox` per panel — Free-form scaling defeats coordinate-frame discipline and breaks the `phase_5_telemetry_ui.md` §3.4 pre-allocated SVG slot pool | Foxglove 3D + ROC §3.4 |
| Per-channel WS subscription UI (one toggle per opcode) | The MetaFrame + Topic Graph panel (Rec. 7) replaces N toggles with one panel | Foxglove Topic Graph |
| A wizard-only "Onboarding" mode that hides the dashboard | OpenVINO DL Workbench wizard-only model is cited as the reason it lost ground to W&B. Don't repeat. | OpenVINO DL Workbench deprecation |

---

## 6. Cross-Reference to Existing ROC Blueprints

- **`phase_5_telemetry_ui.md` §1.5** (zero-copy frame reader) is the basis for the
  Frame Inspector hot path; the new MetaFrame opcode (Risk #3) extends §1.3 schema
  with a non-hot-path channel.
- **`phase_5_telemetry_ui.md` §2.2** (static object pool, in-place mutation) must
  be extended *per panel type* — Frame Inspector reuses the existing
  `TargetPool`; Plot panel needs a `ScalarPool`; Pipeline Graph needs a
  `PipelineNodePool`.
- **`phase_5_telemetry_ui.md` §3.4** (pre-created DOM nodes, never add/remove)
  remains the foundation. Each panel type registers its own slot count at mount;
  the Blueprint validator enforces totals.
- **`phase_5_telemetry_ui.md` §4.1** frame budget reserves <0.5 ms for
  `apply_frame` and <2.0 ms for SVG `setAttribute`. With multi-panel layouts the
  budget is still per-frame, not per-panel — only the *active visible* panels run
  RAF logic.
- **`research_ipc_middleware_2026.md`** (WS protocol baseline) needs the
  MetaFrame opcode definition (§1.3 extension), to be drafted in Phase 5
  follow-up.
- **`research_dashboard_tech_2026.md`** (dashboard tech selection) already
  endorses SvelteKit 5 with `$state` runes — the Blueprint system (Recommendation
  5) is naturally expressible as a `$state.raw(blueprint)` tree with derived
  `$derived(panel_list)`.

---

## 7. Final Note on Verticality

This survey deliberately **does not** cover surveillance/NVR consoles (Milestone,
Genetec, Avigilon, Hanwha, Hikvision) nor broadcast monitoring tools (Anduril
Lattice, Palantir Gotham, QGroundControl, BORIS, SOAR) — those vertical surveys
are owned by sibling agents in the `console-research/0X-*.json` files in the
shared cache. The patterns above are exclusively from the **robotics + ML
pipeline introspection** vertical. Where surveillance UX patterns would
contradict (e.g. NVR systems *do* ship one-tile-per-camera grids by default),
the recommendation in §5.4 stands — ROC's primary use case is humanoid tracking
with active targeting, not passive multi-camera observation, so the
robotics-vertical pattern wins.

---

## 8. References

[fox-panels]: Foxglove · "Panels — Introduction" · <https://docs.foxglove.dev/docs/visualization/panels> · accessed 17 May 2026
[fox-3d]: Foxglove · "3D panel" · <https://docs.foxglove.dev/docs/visualization/panels/3d> · accessed 17 May 2026
[fox-img]: Foxglove · "Image panel" · <https://docs.foxglove.dev/docs/visualization/panels/image> · accessed 17 May 2026
[fox-plot]: Foxglove · "Plot panel" · <https://docs.foxglove.dev/docs/visualization/panels/plot> · accessed 17 May 2026
[fox-diag]: Foxglove · "Diagnostics – Summary / – Detail" · <https://docs.foxglove.dev/docs/visualization/panels/diagnostics> · accessed 17 May 2026
[fox-tele]: Foxglove · "Teleop panel" · <https://docs.foxglove.dev/docs/visualization/panels/teleop> · accessed 17 May 2026
[fox-map]: Foxglove sitemap (panel URL enumeration via Firecrawl map) · <https://docs.foxglove.dev> · accessed 17 May 2026
[rerun-bp]: Rerun · "Blueprints (concepts/visualization)" · <https://rerun.io/docs/concepts/visualization/blueprints> · accessed 17 May 2026; Rerun · "Views (reference/types)" · <https://rerun.io/docs/reference/types/views> · accessed 17 May 2026
[rviz-ug]: ROS 2 Jazzy · "RViz User Guide" · <https://docs.ros.org/en/jazzy/Tutorials/Intermediate/RViz/RViz-User-Guide/RViz-User-Guide.html> · accessed 17 May 2026
[ds-gc]: NVIDIA · "Composer — DeepStream 8.0 documentation" · <https://docs.nvidia.com/metropolis/deepstream/8.0/graphtools-docs/docs/text/GraphComposer_Composer.html> · accessed 17 May 2026
[isaacsim-uiref]: NVIDIA Isaac Sim 4.5 · "User Interface Reference" · <https://docs.isaacsim.omniverse.nvidia.com/4.5.0/gui/reference_user_interface.html> · accessed 17 May 2026
[isaacsim-wf]: NVIDIA Isaac Sim 6.0 · "Workflows" · <https://docs.isaacsim.omniverse.nvidia.com/latest/introduction/workflows.html> · accessed 17 May 2026
[openvino-dlwb]: LearnOpenCV (S. Rath, A. Sharma) · "Introduction to OpenVINO Deep Learning Workbench" · <https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/> · published 23 Aug 2021; accessed 17 May 2026 · (DL Workbench retired in OpenVINO 2022.3, original docs preserved at openvinotoolkit/workbench_aux GitHub)
[robo-keyconcepts]: Roboflow Docs · "Workspaces, Projects, and Models" · <https://docs.roboflow.com/workspaces/key-concepts> · accessed 17 May 2026
[mlflow-track]: MLflow · "ML Experiment Tracking" · <https://mlflow.org/docs/latest/ml/tracking/> · accessed 17 May 2026
[mlflow-vis]: Databricks Docs · "Compare MLflow runs and models using graphs and charts" · <https://docs.databricks.com/aws/en/mlflow/visualize-runs> · last updated 25 Feb 2026; accessed 17 May 2026
[wandb-panels]: Weights & Biases · "Panels" · <https://docs.wandb.ai/models/app/features/panels> · accessed 17 May 2026
[wandb-ws]: Weights & Biases · "Workspaces (wandb_workspaces reference)" · <https://docs.wandb.ai/models/ref/wandb_workspaces/workspaces> · accessed 17 May 2026
[hailo-tappas]: Hailo · "TAPPAS Framework architecture" · <https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/TAPPAS_architecture.rst> · accessed 17 May 2026
[hailo-dfc-rr]: RidgeRun Developer Wiki · "Hailo Dataflow Compiler" · <https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-8/AI_Software_and_Tools/Hailo_Dataflow_Compiler> · accessed 17 May 2026
