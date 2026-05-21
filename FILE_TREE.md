# ROC AI Vision — Disk File Tree

> Auto-generated snapshot of the actual files on disk.
> For the curated, living architectural tree see [`workspace_blueprint/project_tree.md`](workspace_blueprint/project_tree.md).

- **Root:** `D:\roc-ai-vision`
- **Total files on disk:** 11,449
- **Total size:** 1.4 GB

**Excluded from tree (noise / build artifacts):**

- `build/` — 9,235 files hidden
- `_deps/` — 8,540 files hidden
- `node_modules/` — 2,000 files hidden
- `CMakeFiles/` — 711 files hidden
- `.git/` — 315 files hidden
- `dist/` — 278 files hidden
- `.firecrawl/` — 187 files hidden
- `.svelte-kit/` — 54 files hidden

---

```
roc-ai-vision/
├── .cursor/
│   ├── plans/
│   │   └── nasa-grade_dashboard_build_manifest_7cb398dd.plan.md  *[81.9 KB]*
│   ├── rules/
│   │   ├── 000-roc-global.mdc
│   │   ├── 100-core-kinematics.mdc
│   │   ├── 200-dashboard-telemetry.mdc
│   │   ├── 250-phase-5c-execution.mdc
│   │   └── 300-research-standards.mdc
│   └── skills/
│       ├── roc-ekf-context/
│       │   └── SKILL.md
│       ├── roc-onnx-inference/
│       │   └── SKILL.md
│       ├── roc-research-methodology/
│       │   └── SKILL.md
│       └── roc-wire-protocol/
│           └── SKILL.md
├── core/
│   ├── cmake/
│   │   ├── RocBundleMingwRuntime.cmake
│   │   └── RocBundleRuntimeDeps.cmake
│   ├── kinematics_engine/
│   │   ├── include/
│   │   │   └── roc/
│   │   │       └── kinematics/
│   │   │           ├── association.hpp
│   │   │           ├── ca_ekf.hpp
│   │   │           ├── hierarchical_constraint.hpp
│   │   │           ├── lap_jv.hpp
│   │   │           ├── object_priors.hpp
│   │   │           ├── pending_spawn_buffer.hpp
│   │   │           ├── pinhole_measurement.hpp
│   │   │           ├── regime_detector.hpp
│   │   │           ├── target_selector.hpp
│   │   │           ├── track.hpp
│   │   │           ├── tracker.hpp
│   │   │           ├── types.hpp
│   │   │           └── wire_format.hpp
│   │   ├── src/
│   │   │   ├── association.cpp
│   │   │   ├── ca_ekf.cpp
│   │   │   ├── hierarchical_constraint.cpp
│   │   │   ├── lap_jv.cpp
│   │   │   ├── pending_spawn_buffer.cpp
│   │   │   ├── pinhole_measurement.cpp
│   │   │   ├── regime_detector.cpp
│   │   │   ├── target_selector.cpp
│   │   │   ├── track.cpp
│   │   │   ├── tracker.cpp
│   │   │   └── wire_format.cpp
│   │   ├── tests/
│   │   │   ├── test_association.cpp
│   │   │   ├── test_ca_ekf.cpp
│   │   │   ├── test_hierarchy.cpp
│   │   │   ├── test_pinhole.cpp
│   │   │   ├── test_regime.cpp
│   │   │   ├── test_tracker.cpp
│   │   │   └── test_wire_format.cpp
│   │   └── CMakeLists.txt
│   ├── transport/
│   │   ├── include/
│   │   │   └── roc/
│   │   │       └── transport/
│   │   │           ├── hardware_sinks.hpp
│   │   │           ├── mjpeg_server.hpp
│   │   │           ├── operator_command.hpp
│   │   │           ├── origin_check.hpp
│   │   │           ├── output_sink.hpp
│   │   │           ├── spsc_links.hpp
│   │   │           ├── wire_format.hpp
│   │   │           └── ws_egress.hpp
│   │   ├── src/
│   │   │   ├── mjpeg_server.cpp
│   │   │   ├── operator_command.cpp
│   │   │   └── ws_egress.cpp
│   │   ├── tests/
│   │   │   ├── test_hardware_sinks.cpp
│   │   │   ├── test_operator_command.cpp
│   │   │   ├── test_orchestrator_smoke.cpp
│   │   │   ├── test_origin_check.cpp
│   │   │   ├── test_spsc_links.cpp
│   │   │   ├── test_wire_format.cpp
│   │   │   └── test_ws_egress.cpp
│   │   └── CMakeLists.txt
│   ├── vision_pipeline/
│   │   ├── include/
│   │   │   └── roc/
│   │   │       └── vision/
│   │   │           ├── postprocess/
│   │   │           │   ├── osnet_reid.hpp
│   │   │           │   ├── yolo26_detect.hpp
│   │   │           │   └── yolov8_detect.hpp
│   │   │           ├── ep_probe.hpp
│   │   │           ├── inference_engine.hpp
│   │   │           ├── letterbox.hpp
│   │   │           ├── manifest.hpp
│   │   │           ├── manifest_loader.hpp
│   │   │           ├── optical_flow_sampler.hpp
│   │   │           ├── postprocessor.hpp
│   │   │           ├── postprocessor_registry.hpp
│   │   │           └── types.hpp
│   │   ├── scripts/
│   │   │   └── fetch_yolo26n.py
│   │   ├── src/
│   │   │   ├── postprocess/
│   │   │   │   ├── osnet_reid.cpp
│   │   │   │   ├── yolo26_detect.cpp
│   │   │   │   └── yolov8_detect.cpp
│   │   │   ├── ep_probe.cpp
│   │   │   ├── inference_engine.cpp
│   │   │   ├── letterbox.cpp
│   │   │   ├── manifest.cpp
│   │   │   ├── manifest_loader.cpp
│   │   │   ├── optical_flow_sampler.cpp
│   │   │   └── postprocessor_registry.cpp
│   │   ├── tests/
│   │   │   ├── test_byte_compat.cpp
│   │   │   ├── test_ep_probe.cpp
│   │   │   ├── test_inference_engine.cpp
│   │   │   ├── test_letterbox.cpp
│   │   │   ├── test_manifest_loader.cpp
│   │   │   ├── test_optical_flow_sampler.cpp
│   │   │   ├── test_osnet_decode.cpp
│   │   │   ├── test_yolo26_decode.cpp
│   │   │   └── test_yolov8_decode.cpp
│   │   └── CMakeLists.txt
│   ├── CMakeLists.txt
│   └── main.cpp
├── dashboard/
│   ├── .vscode/
│   │   └── extensions.json
│   ├── scripts/
│   │   └── ws_sanity.mjs
│   ├── src/
│   │   ├── lib/
│   │   │   ├── assets/
│   │   │   │   └── favicon.svg
│   │   │   ├── components/
│   │   │   │   ├── destinations/
│   │   │   │   │   ├── CalibrateDestination.svelte
│   │   │   │   │   ├── EngageDestination.svelte
│   │   │   │   │   ├── InspectDestination.svelte
│   │   │   │   │   └── OperateCanvas.svelte
│   │   │   │   ├── domains/
│   │   │   │   │   ├── DomainPanelLayout.svelte
│   │   │   │   │   ├── HardwareActuationInterfacePanel.svelte
│   │   │   │   │   ├── KinematicTrajectoryControllerPanel.svelte
│   │   │   │   │   └── ObserverMatrixPanel.svelte
│   │   │   │   ├── drawers/
│   │   │   │   │   ├── ModelsDrawer.svelte
│   │   │   │   │   ├── SettingsDrawer.svelte
│   │   │   │   │   └── SourcesDrawer.svelte
│   │   │   │   ├── overlays/
│   │   │   │   │   ├── LayerVisibilityToolbar.svelte
│   │   │   │   │   ├── ModelCard.svelte
│   │   │   │   │   ├── ObjectCountBadge.svelte
│   │   │   │   │   └── TargetLockModal.svelte
│   │   │   │   ├── panels/
│   │   │   │   │   ├── inspect/
│   │   │   │   │   │   ├── EventLogDock.svelte
│   │   │   │   │   │   ├── HierarchyExplorerDrawer.svelte
│   │   │   │   │   │   ├── ModelInspectorPanel.svelte
│   │   │   │   │   │   ├── PipelineGraphPanel.svelte
│   │   │   │   │   │   └── TopicGraphPanel.svelte
│   │   │   │   │   ├── FittsResidualPanel.svelte
│   │   │   │   │   ├── MouseDynamicsInspector.svelte
│   │   │   │   │   ├── SpectatorTelemetryMatrix.svelte
│   │   │   │   │   ├── SubPixelPanel.svelte
│   │   │   │   │   └── VelocityProfilePanel.svelte
│   │   │   │   ├── shell/
│   │   │   │   │   ├── CommandPalette.svelte
│   │   │   │   │   ├── DiagnosticsStrip.svelte
│   │   │   │   │   ├── LeftRail.svelte
│   │   │   │   │   ├── RightRail.svelte
│   │   │   │   │   └── TopChrome.svelte
│   │   │   │   ├── tabs/
│   │   │   │   ├── NavigationShell.svelte
│   │   │   │   ├── PreRecordModal.svelte
│   │   │   │   └── TelemetryCanvas.svelte
│   │   │   ├── domains/
│   │   │   │   └── controller_matrix.ts
│   │   │   ├── exporter/
│   │   │   │   └── jsonl_writer.svelte.ts
│   │   │   ├── keybindings/
│   │   │   │   ├── defaults.ts
│   │   │   │   └── manager.svelte.ts
│   │   │   ├── profiles/
│   │   │   │   ├── panel_registry.ts
│   │   │   │   ├── scenario_store.svelte.ts
│   │   │   │   ├── scenarios.ts
│   │   │   │   └── schema.ts
│   │   │   ├── render/
│   │   │   │   └── class_colors.ts
│   │   │   ├── stores/
│   │   │   │   ├── actuator_pool.svelte.ts
│   │   │   │   ├── aiming_config.svelte.ts
│   │   │   │   ├── aiming_config.ts
│   │   │   │   ├── bg_odometry_pool.svelte.ts
│   │   │   │   ├── engage_state.svelte.ts
│   │   │   │   ├── event_log.svelte.ts
│   │   │   │   ├── focused_panel.svelte.ts
│   │   │   │   ├── hardware_config.svelte.ts
│   │   │   │   ├── hardware_config.ts
│   │   │   │   ├── overlay_mode.svelte.ts
│   │   │   │   ├── profile_store.svelte.ts
│   │   │   │   ├── rail_state.svelte.ts
│   │   │   │   ├── render_settings.svelte.ts
│   │   │   │   ├── selection_store.svelte.ts
│   │   │   │   ├── source_store.svelte.ts
│   │   │   │   └── track_pool.svelte.ts
│   │   │   ├── telemetry/
│   │   │   │   ├── mock_frame_source.ts
│   │   │   │   ├── telemetry_socket.svelte.ts
│   │   │   │   └── wire_decoder.ts
│   │   │   └── index.ts
│   │   ├── routes/
│   │   │   ├── +layout.svelte
│   │   │   ├── +layout.ts
│   │   │   └── +page.svelte
│   │   ├── app.d.ts
│   │   └── app.html
│   ├── static/
│   │   └── robots.txt
│   ├── .gitignore
│   ├── .gitkeep
│   ├── .npmrc
│   ├── architecture_plan.md
│   ├── fix_tags.py
│   ├── observer_template.txt
│   ├── package-lock.json
│   ├── package.json
│   ├── README.md
│   ├── svelte.config.js
│   ├── tsconfig.json
│   └── vite.config.ts
├── models/
│   ├── .gitkeep
│   ├── coco-yolov8-baseline.yaml
│   ├── osnet-ain-x1-0.yaml
│   ├── person-only-yolov8.yaml
│   ├── yolo26m-roc-humanoid.yaml
│   ├── yolo26n-coco.onnx  *[9.5 MB]*
│   └── yolo26n-coco.yaml
├── ros2_ws/
│   └── src/
│       ├── roc_vision_nodes/
│       │   ├── config/
│       │   │   └── .gitkeep
│       │   ├── launch/
│       │   │   └── .gitkeep
│       │   ├── src/
│       │   │   └── .gitkeep
│       │   ├── CMakeLists.txt
│       │   └── package.xml
│       └── tracker_interfaces/
│           ├── msg/
│           │   └── .gitkeep
│           ├── CMakeLists.txt
│           └── package.xml
├── workspace_blueprint/
│   ├── research_notes/
│   │   ├── anticheat_data_exposure_2026.md  *[97.1 KB]*
│   │   ├── dashboard_game_cv_overlay_ux_2026.md
│   │   ├── dashboard_menu_architecture_2026.md  *[128.4 KB]*
│   │   ├── dashboard_pro_console_ux_2026.md  *[77.1 KB]*
│   │   ├── phase_1_ensemble_architecture.md
│   │   ├── phase_1_inference_and_targeting.md
│   │   ├── phase_1_onnx_gpu_core.md
│   │   ├── phase_1_universal_ep_and_manifest.md
│   │   ├── phase_2_ekf_kinematics.md
│   │   ├── phase_5_console_robotics_ml.md
│   │   ├── phase_5_telemetry_ui.md
│   │   ├── research_dashboard_tech_2026.md
│   │   ├── research_detection_models_2026.md
│   │   ├── research_ipc_middleware_2026.md
│   │   ├── research_mpc_actuator_control_2026.md
│   │   ├── research_reid_embeddings_2026.md
│   │   ├── research_sota_mot_trackers_2026.md
│   │   ├── research_svg_overlay_alignment_2026.md
│   │   └── target_prediction_ai.md
│   ├── architecture.md
│   ├── PHASE_5C_HANDOFF.md
│   ├── PHASE_5C_PLAYBOOK.md
│   ├── project_tree.md
│   ├── roadmap.md  *[95.5 KB]*
│   └── UI_SPEC_SHEET.md
├── _make_tree.py
├── creation-idea.txt
├── cursor.md
├── initial work prompt.md
└── PROJECT_BRIEF.md
```