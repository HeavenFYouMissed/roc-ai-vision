<!--
	roc-ai-vision · dashboard · OperateCanvas.svelte

	The Roboflow-grade composite Operate canvas. 6-layer SVG z-stack:
		Layer 0  · <video> placeholder + animated grid baseline
		Layer 1  · per-class colored trajectory tails
		Layer 1.5· actuator path history (raw polyline + smoothed Bezier)
		Layer 2  · per-class colored detection bboxes with Roboflow-style
		           confidence labels + class+id badges + FOV ring + ellipses
		Layer 3  · hierarchy edges (parent_id graph)
		Layer 4  · selection outline + anchor crosshair + velocity arrow +
		           target-lock crosshair + slew indicator

	Reads:
		· trackPool (presence + bbox + label + confidence + flags + traj + pPosDiag + parent)
		· actuatorPool (raw + clean u/v for layer 1.5)
		· renderSettings (visibility + thresholds + label mode + per-element toggles)
		· selectionStore (selected + hovered + locks + click-to-lock mode)
		· aimingConfig (FOV radius)
		· profileStore (REPLAY mode disables crosshair tint to red)

	Performance contract: zero per-frame heap allocation. All SVG nodes are
	pre-allocated once at mount, mutated in place via setAttribute inside the
	RAF tick. Pool size 64 slots × ~12 SVG elements/slot = 768 nodes max.
-->
<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { trackPool, MAX_SLOTS, TRAJECTORY_RING } from '$lib/stores/track_pool.svelte';
	import { actuatorPool } from '$lib/stores/actuator_pool.svelte';
	import { renderSettings } from '$lib/stores/render_settings.svelte';
	import { selectionStore } from '$lib/stores/selection_store.svelte';
	import { aimingConfig } from '$lib/stores/aiming_config';
	import { profileStore } from '$lib/stores/profile_store.svelte';
	import { sourceStore } from '$lib/stores/source_store.svelte';
	import { fovStore } from '$lib/stores/fov_store.svelte';
	import { telemetrySocket } from '$lib/telemetry/telemetry_socket.svelte';
	import {
		CLASS_COLORS,
		CLASS_LABEL_TEXT,
		CLASS_SHORT_NAMES,
		STATUS_FLAG_BITS
	} from '$lib/render/class_colors';

	// Wire status flag bits (mirror class_colors export for performance)
	const FLAG_BOUNDARY = STATUS_FLAG_BITS.BOUNDARY;
	const FLAG_STATIONARY = STATUS_FLAG_BITS.STATIONARY;
	const FLAG_HIERARCHY_VIOLATION = STATUS_FLAG_BITS.HIERARCHY_VIOLATION;

	// Image-plane canvas dimensions — pulled live from the active source
	// (e.g., 640x480 for a USB webcam, 1920x1080 for HDMI capture). The C++
	// orchestrator emits bbox_orig in source-pixel space, so the SVG viewBox
	// MUST match the source frame size or every detection lands in the wrong
	// place relative to the MJPEG video underneath. Falls back to 1920x1080
	// when the source isn't enumerated yet (mock mode or pre-handshake).
	const VIEW_W = $derived(sourceStore.active?.width  || 1920);
	const VIEW_H = $derived(sourceStore.active?.height || 1080);

	// Tracks whether the MJPEG <img> has received its first JPEG frame.
	// Flipped by the onload/onerror handlers so the "CONNECTING…" chip hides
	// the moment the stream is live and reappears if it drops.
	let mjpegLoaded = $state(false);

	// Reset the loaded flag whenever the source URL flips (user picks a
	// different camera in the Sources drawer) so the chip reappears during
	// the new handshake.
	$effect(() => {
		// eslint-disable-next-line @typescript-eslint/no-unused-expressions
		sourceStore.mjpegUrl;
		mjpegLoaded = false;
	});

	// Velocity arrow forward-prediction horizon (in pool ticks ~ 33ms each)
	const VEL_ARROW_HORIZON_S = 0.1; // 100ms ahead

	// 3-frame red-flash decay for hierarchy violations
	const violationDecay = new Uint8Array(MAX_SLOTS);

	// Reused scratch arrays - zero per-frame allocation
	const trajScratch: string[] = [];
	const actuatorRawScratch: string[] = [];
	const actuatorCleanScratch: string[] = [];

	// =========================================================================
	// JS Ruckig-lite virtual cursor controller (Phase 6-part-A stand-in).
	//
	// The C++ actuator_telemetry_emit_thread currently emits raw_u/v as the
	// target's ABSOLUTE pixel position (selector_cx + dx_px), not as the
	// per-tick mouse delta a real aim-assist would inject. Per binding spec
	// dashboard_menu_architecture_2026.md §12.4 line 863, the actuator thread
	// is supposed to interpolate between tracker updates with jerk-limited
	// kinematics — that piece is Phase 6 part B and not built yet.
	//
	// Until that lands, we synthesize the AC-relevant trail HERE on the
	// dashboard: take the C++ stream's newest target position, run a
	// dashboard-side jerk-limited approach controller, and plot THAT cursor
	// path. The result is the actual "mouse drawing" a kernel input logger
	// would see — the data point the anticheat research mission cares about.
	//
	// State is module-local (per OperateCanvas instance). Zero allocation in
	// steady state. Replace with direct reads of `actuatorPool.cleanU/V` once
	// Phase 6 part B lands the controller in C++.
	// =========================================================================
	const CURSOR_TRAIL_CAP = 240;       // ~4 s of trail at 60 Hz RAF
	const CURSOR_MAX_V_PX_S = 1500;     // upper bound of pro FPS aim velocity
	const CURSOR_MAX_A_PX_S2 = 12000;   // jerk-limited approach acceleration
	const CURSOR_APPROACH_GAIN = 8;     // higher = snappier; tune for human feel
	const cursorTrailX = new Float32Array(CURSOR_TRAIL_CAP);
	const cursorTrailY = new Float32Array(CURSOR_TRAIL_CAP);
	let cursorTrailHead = 0;
	let cursorTrailFill = 0;
	let cursorX = 0;
	let cursorY = 0;
	let cursorVX = 0;
	let cursorVY = 0;
	let cursorInit = false;
	let cursorLastTickMs = 0;

	function resetCursorController(): void {
		cursorInit = false;
		cursorVX = 0;
		cursorVY = 0;
		cursorTrailHead = 0;
		cursorTrailFill = 0;
	}

	// Advance the virtual cursor toward (targetX, targetY) using jerk-limited
	// motion. targetX/Y may be NaN — in that case the cursor decelerates to
	// rest at its current position. Caller passes performance.now() so dt is
	// based on RAF cadence not real-time, keeping the trail deterministic
	// regardless of frame jitter.
	function tickCursorController(targetX: number, targetY: number, nowMs: number): void {
		if (!cursorInit) {
			cursorX = VIEW_W * 0.5;
			cursorY = VIEW_H * 0.5;
			cursorInit = true;
			cursorLastTickMs = nowMs;
			return;
		}
		// Clamp dt so a tab switch / GC pause doesn't teleport the cursor.
		const dt = Math.max(0.001, Math.min(0.05, (nowMs - cursorLastTickMs) / 1000));
		cursorLastTickMs = nowMs;

		const hasTarget = Number.isFinite(targetX) && Number.isFinite(targetY);
		if (!hasTarget) {
			// No target: decelerate toward rest. Magnitude-preserving.
			const v_mag = Math.hypot(cursorVX, cursorVY);
			if (v_mag > 1) {
				const next_v = Math.max(0, v_mag - CURSOR_MAX_A_PX_S2 * dt);
				const factor = next_v / v_mag;
				cursorVX *= factor;
				cursorVY *= factor;
			} else {
				cursorVX = 0;
				cursorVY = 0;
			}
		} else {
			// Desired velocity: gain × distance, capped at max velocity. This
			// produces a critically-damped-ish approach — fast far from target,
			// slow at lockon. Direction is unit vector toward target.
			const ex = targetX - cursorX;
			const ey = targetY - cursorY;
			const dist = Math.hypot(ex, ey);
			const v_desired_mag = Math.min(CURSOR_MAX_V_PX_S, dist * CURSOR_APPROACH_GAIN);
			const desired_vx = dist > 0.5 ? (ex / dist) * v_desired_mag : 0;
			const desired_vy = dist > 0.5 ? (ey / dist) * v_desired_mag : 0;

			// Limit acceleration toward desired velocity — this is the jerk
			// limit that produces the swooping arcs from the reference image.
			const dvx = desired_vx - cursorVX;
			const dvy = desired_vy - cursorVY;
			const dv_mag = Math.hypot(dvx, dvy);
			const dv_max = CURSOR_MAX_A_PX_S2 * dt;
			if (dv_mag > dv_max && dv_mag > 0) {
				cursorVX += (dvx / dv_mag) * dv_max;
				cursorVY += (dvy / dv_mag) * dv_max;
			} else {
				cursorVX = desired_vx;
				cursorVY = desired_vy;
			}
		}

		cursorX += cursorVX * dt;
		cursorY += cursorVY * dt;
		// Clamp to viewBox so cursor stays inside the visible video bounds.
		if (cursorX < 0) { cursorX = 0; cursorVX = 0; }
		else if (cursorX > VIEW_W) { cursorX = VIEW_W; cursorVX = 0; }
		if (cursorY < 0) { cursorY = 0; cursorVY = 0; }
		else if (cursorY > VIEW_H) { cursorY = VIEW_H; cursorVY = 0; }

		cursorTrailX[cursorTrailHead] = cursorX;
		cursorTrailY[cursorTrailHead] = cursorY;
		cursorTrailHead = (cursorTrailHead + 1) % CURSOR_TRAIL_CAP;
		if (cursorTrailFill < CURSOR_TRAIL_CAP) cursorTrailFill++;
	}

	// Reset the controller when the active source changes (different camera =
	// different image plane, so any old trail samples are meaningless).
	$effect(() => {
		// eslint-disable-next-line @typescript-eslint/no-unused-expressions
		sourceStore.activeSourceId;
		resetCursorController();
	});

	// Per-frame label placement scratch — used by collision detection so labels
	// don't pile up when multiple bboxes share the same y range. Pre-allocated
	// once at module load; mutated in place every RAF tick.
	interface PlacedLabel {
		slot: number;
		x: number;
		y: number;       // top of label pill
		w: number;       // width of label pill
		h: number;       // height of label pill (= 16)
	}
	const placedLabels: PlacedLabel[] = [];
	const LABEL_HEIGHT = 16;
	const LABEL_VERT_GAP = 2;

	// Pre-allocated SVG node refs - one per slot per layer element.
	// MUST be $state so bind:this works in Svelte 5.
	let bboxHaloRefs = $state<(SVGRectElement | undefined)[]>(new Array(MAX_SLOTS));
	let bboxRefs = $state<(SVGRectElement | undefined)[]>(new Array(MAX_SLOTS));
	let labelBgRefs = $state<(SVGRectElement | undefined)[]>(new Array(MAX_SLOTS));
	let labelTextRefs = $state<(SVGTextElement | undefined)[]>(new Array(MAX_SLOTS));
	let idBadgeRefs = $state<(SVGTextElement | undefined)[]>(new Array(MAX_SLOTS));
	let trajRefs = $state<(SVGPathElement | undefined)[]>(new Array(MAX_SLOTS));
	let anchorRefs = $state<(SVGGElement | undefined)[]>(new Array(MAX_SLOTS));
	let anchorCenterRefs = $state<(SVGRectElement | undefined)[]>(new Array(MAX_SLOTS));
	let velArrowRefs = $state<(SVGLineElement | undefined)[]>(new Array(MAX_SLOTS));
	let ellipseRefs = $state<(SVGEllipseElement | undefined)[]>(new Array(MAX_SLOTS));
	let selectionRingRefs = $state<(SVGRectElement | undefined)[]>(new Array(MAX_SLOTS));
	let hierarchyEdgeRefs = $state<(SVGLineElement | undefined)[]>(new Array(MAX_SLOTS));
	let hoverRingRefs = $state<(SVGRectElement | undefined)[]>(new Array(MAX_SLOTS));

	// Layer 1.5 actuator trace - 2 single paths (one raw=dirty, one smoothed=clean)
	// + an origin crosshair anchor + a live cursor marker showing where the
	// virtual cursor is RIGHT NOW.
	let actuatorRawPathRef: SVGPolylineElement | undefined = $state();
	let actuatorCleanPathRef: SVGPathElement | undefined = $state();
	let cursorOriginRef: SVGGElement | undefined = $state();
	let cursorMarkerRef: SVGGElement | undefined = $state();
	let cursorMarkerDotRef: SVGCircleElement | undefined = $state();

	// FOV ring - single SVG circle, centered on canvas
	let fovRingRef: SVGCircleElement | undefined = $state();
	// Phase 6 FOV inference-crop rectangle (orange dashed). Bound to the
	// orchestrator's /api/fov diagnostics by `applyFovCropOverlay()` below.
	let fovCropRectRef: SVGRectElement | undefined = $state();

	// Lock crosshair group (renders per-lock)
	let lockCrosshairGroupRef: SVGGElement | undefined = $state();

	// Camera placeholder visibility
	let cameraPlaceholderVisible = $state(true);

	let rafId = 0;

	// Reactive: hydrate persisted settings on mount
	$effect(() => {
		renderSettings.hydrate();
		profileStore.hydrate();
		sourceStore.hydrate();
	});

	// Layer 0 video binding — live MJPEG when active source has a URL,
	// placeholder grid otherwise.
	const videoSrcUrl = $derived(sourceStore.mjpegUrl);
	const sourceLabel = $derived(sourceStore.sourceLabel);

	// Resolve anchor offset per ss11.3 anchor type from active profile.
	// Default to bbox_center for all classes for now (full anchor config
	// will land in profile schema extension in Session 5).
	function anchorOffset(label: number, h: number): { du: number; dv: number } {
		// In v1: simple per-class default. Future: read from profileStore.active.actuation.targeting_anchors
		switch (label) {
			case 2: // Head
				return { du: 0, dv: -h * 0.4 }; // bbox_top_center
			case 4: // AccessoryHat
				return { du: 0, dv: -h * 0.5 }; // bbox_top
			case 5: // AccessoryBackpack
				return { du: 0, dv: h * 0.2 }; // bbox_center_lower
			default:
				return { du: 0, dv: 0 }; // bbox_center
		}
	}

	function tick(): void {
		const pool = trackPool;
		const settings = renderSettings;
		const sel = selectionStore;
		const isReplay = profileStore.isReplay;

		// Reset label placement registry each frame
		placedLabels.length = 0;

		// Layer 0 visibility
		cameraPlaceholderVisible = settings.showVideo;

		// FOV ring - bound to aimingConfig (Phase 5 maps via aimingConfig.smoothingCoefficient
		// for slider feedback demo; future: profile.actuation.targeting_fov_radius_px)
		if (fovRingRef) {
			if (settings.showDetections && settings.showFovRing) {
				fovRingRef.setAttribute('display', 'inline');
				fovRingRef.setAttribute('cx', String(VIEW_W * 0.5));
				fovRingRef.setAttribute('cy', String(VIEW_H * 0.5));
				// Belt-and-suspenders cap so the ring can never exceed the visible
				// source rect; primary guard is the stage's overflow: hidden.
				// TODO(phase-6-fov-system): replace with profile.actuation.targeting_fov_radius_px once profile schema lands
				const fovRadius = Math.min(
					200 + aimingConfig.smoothingCoefficient * 520,
					Math.min(VIEW_W, VIEW_H) * 0.48
				);
				fovRingRef.setAttribute('r', String(fovRadius));
			} else {
				fovRingRef.setAttribute('display', 'none');
			}
		}

		// Phase 6 — inference-crop rectangle. Drawn whenever the orchestrator
		// reports an active crop (i.e. server-side FOV is enabled AND it
		// successfully computed activeCropW>0 from the latest frame). The
		// source-pixel rect from the server is translated into SVG view space
		// by the (VIEW / source) ratio; if the SVG viewBox already matches
		// source dims (the common case) the ratio is 1.0 and these are NOPs.
		if (fovCropRectRef) {
			const fov = fovStore.settings;
			const srcW = fov.sourceW;
			const srcH = fov.sourceH;
			const shouldDraw =
				fov.enabled &&
				settings.showDetections &&
				srcW > 0 && srcH > 0 &&
				fov.activeCropW > 0 && fov.activeCropH > 0;
			if (shouldDraw) {
				const sx = VIEW_W / srcW;
				const sy = VIEW_H / srcH;
				fovCropRectRef.setAttribute('display', 'inline');
				fovCropRectRef.setAttribute('x',      String(fov.activeCropX * sx));
				fovCropRectRef.setAttribute('y',      String(fov.activeCropY * sy));
				fovCropRectRef.setAttribute('width',  String(fov.activeCropW * sx));
				fovCropRectRef.setAttribute('height', String(fov.activeCropH * sy));
			} else {
				fovCropRectRef.setAttribute('display', 'none');
			}
		}

		// --- Layer 1.5 actuator paths: virtual cursor controller (Phase 6-part-A) ---
		// The dashboard runs its own jerk-limited approach controller against
		// the C++ stream's newest target sample. The CLEAN trail is the
		// controller's smoothed cursor path (what a humanised aim-assist
		// produces — the green sketch from the reference image). The DIRTY
		// trail is the raw target-snap signature (what an unhumanised bot
		// produces — the red sketch). Both anchored at canvas center; an
		// origin crosshair and a live cursor marker visualise the current
		// cursor state. See the controller block at the top of <script> for
		// the rationale (Phase 6-part-B C++ controller pending).
		if (actuatorRawPathRef && actuatorCleanPathRef) {
			const showActuator = settings.showActuatorTrace;

			if (!showActuator) {
				actuatorRawPathRef.setAttribute('display', 'none');
				actuatorCleanPathRef.setAttribute('display', 'none');
				if (cursorOriginRef) cursorOriginRef.setAttribute('display', 'none');
				if (cursorMarkerRef) cursorMarkerRef.setAttribute('display', 'none');
			} else {
				// Pull newest target sample from the C++ actuator stream. The
				// stream sends raw_u/v = absolute target pixel on the image;
				// that's our controller's setpoint.
				let targetU = Number.NaN;
				let targetV = Number.NaN;
				if (actuatorPool.fill > 0) {
					const newestIdx = (actuatorPool.head - 1 + 4096) % 4096;
					const ru = actuatorPool.rawU[newestIdx];
					const rv = actuatorPool.rawV[newestIdx];
					if (Number.isFinite(ru) && Number.isFinite(rv)) {
						targetU = ru;
						targetV = rv;
					}
				}

				tickCursorController(targetU, targetV, performance.now());

				// Build the CLEAN trail path from the controller's history ring.
				actuatorCleanScratch.length = 0;
				const fill = cursorTrailFill;
				if (fill >= 2) {
					for (let k = 0; k < fill; k++) {
						const idx = (cursorTrailHead - fill + k + CURSOR_TRAIL_CAP) % CURSOR_TRAIL_CAP;
						const x = cursorTrailX[idx];
						const y = cursorTrailY[idx];
						actuatorCleanScratch.push(k === 0 ? `M ${x} ${y}` : `L ${x} ${y}`);
					}
					actuatorCleanPathRef.setAttribute('d', actuatorCleanScratch.join(' '));
					actuatorCleanPathRef.setAttribute('display', 'inline');
				} else {
					actuatorCleanPathRef.setAttribute('display', 'none');
				}

				// Build the DIRTY trail from the most-recent 60 target samples
				// (~2 s at 30 Hz). Anchored at canvas center via an opening
				// move-to so the comparison is fair vs the clean trail.
				actuatorRawScratch.length = 0;
				const dirtySamples = Math.min(actuatorPool.fill, 60);
				if (dirtySamples >= 2) {
					actuatorRawScratch.push(`${VIEW_W * 0.5},${VIEW_H * 0.5}`);
					for (let k = 0; k < dirtySamples; k++) {
						const idx = (actuatorPool.head - dirtySamples + k + 4096) % 4096;
						const ru = actuatorPool.rawU[idx];
						const rv = actuatorPool.rawV[idx];
						if (Number.isFinite(ru) && Number.isFinite(rv)) {
							actuatorRawScratch.push(`${ru},${rv}`);
						}
					}
					if (actuatorRawScratch.length >= 2) {
						actuatorRawPathRef.setAttribute('points', actuatorRawScratch.join(' '));
						actuatorRawPathRef.setAttribute('display', 'inline');
					} else {
						actuatorRawPathRef.setAttribute('display', 'none');
					}
				} else {
					actuatorRawPathRef.setAttribute('display', 'none');
				}

				// Origin crosshair: small fixed cross at canvas center showing
				// where the virtual cursor was born (= where the player's
				// in-game crosshair would always be).
				if (cursorOriginRef) {
					cursorOriginRef.setAttribute(
						'transform',
						`translate(${VIEW_W * 0.5} ${VIEW_H * 0.5})`
					);
					cursorOriginRef.setAttribute('display', 'inline');
				}

				// Live cursor marker: filled circle at the controller's current
				// (cursorX, cursorY). Shows where the virtual mouse "is" right
				// now per the synthesised actuator state.
				if (cursorMarkerRef && cursorInit) {
					cursorMarkerRef.setAttribute(
						'transform',
						`translate(${cursorX} ${cursorY})`
					);
					cursorMarkerRef.setAttribute('display', 'inline');
				}
			}
		}

		// --- Per-slot rendering ---
		const showDetections = settings.showDetections;
		const showTrajectories = settings.showTrajectories;
		const showHierarchy = settings.showHierarchy;
		const showAnchor = settings.showAnchor;
		const showVel = settings.showVelocityArrow;
		const showEllipse = settings.showUncertaintyEllipse;
		const showTrackIdBadge = settings.showTrackId;
		const labelMode = settings.labelMode;
		const bboxStyle = settings.bboxStyle;
		const confThreshold = settings.confidenceThreshold;
		const visibility = settings.classVisibility;
		const selSlot = sel.selectedSlot;
		const hovSlot = sel.hoveredSlot;

		for (let s = 0; s < MAX_SLOTS; s++) {
			if (violationDecay[s] > 0) violationDecay[s]--;

			const halo = bboxHaloRefs[s];
			const bbox = bboxRefs[s];
			const labelBg = labelBgRefs[s];
			const labelText = labelTextRefs[s];
			const idBadge = idBadgeRefs[s];
			const traj = trajRefs[s];
			const anchor = anchorRefs[s];
			const anchorCenter = anchorCenterRefs[s];
			const velArrow = velArrowRefs[s];
			const ellipse = ellipseRefs[s];
			const selRing = selectionRingRefs[s];
			const hoverRing = hoverRingRefs[s];
			const hEdge = hierarchyEdgeRefs[s];

			const present = pool.presenceBitmap[s] === 1;
			const label = pool.label[s];
			const confidence = pool.confidence[s];
			const shouldRender =
				present && showDetections && confidence >= confThreshold && visibility[label] === true;

			if (!present || !shouldRender) {
				halo?.setAttribute('display', 'none');
				bbox?.setAttribute('display', 'none');
				labelBg?.setAttribute('display', 'none');
				labelText?.setAttribute('display', 'none');
				idBadge?.setAttribute('display', 'none');
				traj?.setAttribute('display', 'none');
				anchor?.setAttribute('display', 'none');
				velArrow?.setAttribute('display', 'none');
				ellipse?.setAttribute('display', 'none');
				selRing?.setAttribute('display', 'none');
				hoverRing?.setAttribute('display', 'none');
				hEdge?.setAttribute('display', 'none');
				continue;
			}

			const b4 = s * 4;
			const u = pool.bboxOrig[b4 + 0];
			const v = pool.bboxOrig[b4 + 1];
			const w = pool.bboxOrig[b4 + 2];
			const h = pool.bboxOrig[b4 + 3];
			const x = u - w * 0.5;
			const y = v - h * 0.5;

			const flags = pool.statusFlags[s];
			if (flags & FLAG_HIERARCHY_VIOLATION) violationDecay[s] = 3;

			const classColor = CLASS_COLORS[label] ?? CLASS_COLORS[0];
			const labelTextColor = CLASS_LABEL_TEXT[label] ?? CLASS_LABEL_TEXT[0];
			const shortName = CLASS_SHORT_NAMES[label] ?? 'Unknown';

			// --- Layer 2: bbox halo (dark 3px stroke under colored 2px for MJPEG legibility) ---
			if (halo) {
				halo.setAttribute('x', String(x));
				halo.setAttribute('y', String(y));
				halo.setAttribute('width', String(w));
				halo.setAttribute('height', String(h));
				halo.setAttribute('display', 'inline');
			}

			// --- Layer 2: bbox colored stroke ---
			if (bbox) {
				bbox.setAttribute('x', String(x));
				bbox.setAttribute('y', String(y));
				bbox.setAttribute('width', String(w));
				bbox.setAttribute('height', String(h));
				bbox.setAttribute('display', 'inline');

				if (violationDecay[s] > 0) {
					bbox.setAttribute('stroke', '#ef4444');
					bbox.setAttribute('stroke-width', '3');
					bbox.removeAttribute('stroke-dasharray');
				} else if (flags & FLAG_BOUNDARY) {
					bbox.setAttribute('stroke', '#f5a623');
					bbox.setAttribute('stroke-width', '2');
					bbox.setAttribute('stroke-dasharray', '8 4');
				} else {
					bbox.setAttribute('stroke', classColor);
					bbox.setAttribute('stroke-width', '2');
					if (bboxStyle === 'dashed') {
						bbox.setAttribute('stroke-dasharray', '6 3');
					} else {
						bbox.removeAttribute('stroke-dasharray');
					}
				}
				// Fill: subtle tint when stationary, none otherwise
				if (flags & FLAG_STATIONARY) {
					bbox.setAttribute('fill', classColor);
					bbox.setAttribute('fill-opacity', '0.06');
				} else {
					bbox.setAttribute('fill', 'none');
				}
			}

			// --- Layer 2: Roboflow-style label pill (above bbox, collision-aware) ---
			const showLabel = labelMode !== 'none';
			if (labelBg && labelText) {
				if (!showLabel) {
					labelBg.setAttribute('display', 'none');
					labelText.setAttribute('display', 'none');
				} else {
					const pct = (confidence * 100).toFixed(1);
					let labelStr = '';
					if (labelMode === 'confidence') labelStr = `${shortName} ${pct}%`;
					else if (labelMode === 'labels') labelStr = shortName;
					else labelStr = `${shortName} ${pct}%`;

					// Approximate text width (mono 11px ~= 7px/char)
					const textWidth = labelStr.length * 7 + 8;

					// Initial placement: above bbox if room, below if near top
					const labelAbove = y > 20;
					let pillX = x;
					let pillY = labelAbove ? y - LABEL_HEIGHT - 2 : y + h + 2;

					// COLLISION DETECTION (Claude polish #2 option a):
					// Walk already-placed labels; if AABB overlaps, shift this
					// label horizontally by previous label's right edge + small
					// gap. If that pushes us off-canvas, stack vertically up
					// (above) or down (below).
					for (let pl = 0; pl < placedLabels.length; pl++) {
						const prev = placedLabels[pl];
						const aabbOverlap =
							pillX < prev.x + prev.w + 4 &&
							pillX + textWidth + 4 > prev.x &&
							pillY < prev.y + prev.h + LABEL_VERT_GAP &&
							pillY + LABEL_HEIGHT + LABEL_VERT_GAP > prev.y;
						if (!aabbOverlap) continue;
						// Try horizontal shift first
						const shiftedX = prev.x + prev.w + 6;
						if (shiftedX + textWidth <= VIEW_W) {
							pillX = shiftedX;
							pl = -1; // restart collision check vs all placed
							continue;
						}
						// Out of horizontal room: stack vertically by another row
						pillY = labelAbove
							? pillY - (LABEL_HEIGHT + LABEL_VERT_GAP)
							: pillY + (LABEL_HEIGHT + LABEL_VERT_GAP);
						pillX = x;
						pl = -1;
					}

					// Final clamp to canvas top edge
					if (pillY < 0) {
						pillY = 0;
					}

					placedLabels.push({ slot: s, x: pillX, y: pillY, w: textWidth, h: LABEL_HEIGHT });

					labelBg.setAttribute('x', String(pillX));
					labelBg.setAttribute('y', String(pillY));
					labelBg.setAttribute('width', String(textWidth));
					labelBg.setAttribute('height', String(LABEL_HEIGHT));
					labelBg.setAttribute('fill', classColor);
					labelBg.setAttribute('rx', '2');
					labelBg.setAttribute('display', 'inline');

					labelText.setAttribute('x', String(pillX + 4));
					labelText.setAttribute('y', String(pillY + 12)); // baseline 12px below pill top
					labelText.setAttribute('fill', labelTextColor);
					labelText.textContent = labelStr;
					labelText.setAttribute('display', 'inline');
				}
			}

			// --- Layer 2: track ID corner badge ---
			if (idBadge) {
				if (showTrackIdBadge && w > 60) {
					idBadge.setAttribute('x', String(x + w - 4));
					idBadge.setAttribute('y', String(y + 12));
					idBadge.setAttribute('fill', classColor);
					idBadge.setAttribute('fill-opacity', '0.7');
					idBadge.textContent = pool.idStrings[s] || '';
					idBadge.setAttribute('display', 'inline');
				} else {
					idBadge.setAttribute('display', 'none');
				}
			}

			// --- Layer 1: trajectory tail ---
			if (traj) {
				const fill = pool.trajFill[s];
				if (showTrajectories && fill >= 2) {
					const ringBase = s * TRAJECTORY_RING;
					const head = pool.trajHead[s];
					const start = (head - fill + TRAJECTORY_RING) % TRAJECTORY_RING;
					trajScratch.length = 0;
					let lastU = 0,
						lastV = 0,
						prevU = 0,
						prevV = 0;
					for (let k = 0; k < fill; k++) {
						const idx = ringBase + ((start + k) % TRAJECTORY_RING);
						const tu = pool.trajU[idx];
						const tv = pool.trajV[idx];
						trajScratch.push(k === 0 ? `M ${tu} ${tv}` : `L ${tu} ${tv}`);
						if (k === fill - 2) {
							prevU = tu;
							prevV = tv;
						}
						if (k === fill - 1) {
							lastU = tu;
							lastV = tv;
						}
					}
					// 3-frame forward extrapolation
					trajScratch.push(`L ${lastU + (lastU - prevU) * 3} ${lastV + (lastV - prevV) * 3}`);
					traj.setAttribute('d', trajScratch.join(' '));
					traj.setAttribute('stroke', classColor);
					traj.setAttribute('display', 'inline');
				} else {
					traj.setAttribute('display', 'none');
				}
			}

			// --- Layer 4: anchor crosshair (resolved per class) ---
			if (anchor && anchorCenter) {
				if (showAnchor) {
					const { du, dv } = anchorOffset(label, h);
					const au = u + du;
					const av = v + dv;
					anchor.setAttribute('transform', `translate(${au} ${av})`);
					anchor.setAttribute('display', 'inline');
					anchorCenter.setAttribute('fill', '#f5a623');
				} else {
					anchor.setAttribute('display', 'none');
				}
			}

			// --- Layer 4: velocity arrow (100ms forward prediction in image plane) ---
			if (velArrow) {
				if (showVel) {
					// Use trajectory delta as image-plane velocity approximation
					// (vWorld is in world units; conversion requires camera intrinsics)
					const fill = pool.trajFill[s];
					if (fill >= 2) {
						const ringBase = s * TRAJECTORY_RING;
						const head = pool.trajHead[s];
						const lastIdx = ringBase + ((head - 1 + TRAJECTORY_RING) % TRAJECTORY_RING);
						const prevIdx = ringBase + ((head - 2 + TRAJECTORY_RING) % TRAJECTORY_RING);
						const dvu = pool.trajU[lastIdx] - pool.trajU[prevIdx];
						const dvv = pool.trajV[lastIdx] - pool.trajV[prevIdx];
						const mag = Math.hypot(dvu, dvv);
						if (mag > 1.5) {
							// 100ms forward at ~33ms/frame = ~3 frames
							const px = u + dvu * 3;
							const py = v + dvv * 3;
							velArrow.setAttribute('x1', String(u));
							velArrow.setAttribute('y1', String(v));
							velArrow.setAttribute('x2', String(px));
							velArrow.setAttribute('y2', String(py));
							velArrow.setAttribute('stroke', '#00f0ff');
							velArrow.setAttribute('display', 'inline');
						} else {
							velArrow.setAttribute('display', 'none');
						}
					} else {
						velArrow.setAttribute('display', 'none');
					}
				} else {
					velArrow.setAttribute('display', 'none');
				}
			}

			// --- Layer 2: uncertainty ellipse from pPosDiag ---
			if (ellipse) {
				if (showEllipse) {
					const di = s * 3;
					// pPosDiag is variance in world units; scale to px (placeholder until intrinsics)
					const rx = Math.max(8, Math.sqrt(Math.abs(pool.pPosDiag[di + 0])) * 80);
					const ry = Math.max(8, Math.sqrt(Math.abs(pool.pPosDiag[di + 1])) * 80);
					ellipse.setAttribute('cx', String(u));
					ellipse.setAttribute('cy', String(v));
					ellipse.setAttribute('rx', String(rx));
					ellipse.setAttribute('ry', String(ry));
					ellipse.setAttribute('stroke', classColor);
					ellipse.setAttribute('display', 'inline');
				} else {
					ellipse.setAttribute('display', 'none');
				}
			}

			// --- Layer 3: hierarchy edge to parent ---
			if (hEdge) {
				if (showHierarchy && (pool.parentIdLo[s] !== 0 || pool.parentIdHi[s] !== 0)) {
					const parentSlot = pool.findSlotByTrackId(pool.parentIdLo[s], pool.parentIdHi[s]);
					if (parentSlot >= 0 && pool.presenceBitmap[parentSlot] === 1) {
						const pb4 = parentSlot * 4;
						const pu = pool.bboxOrig[pb4 + 0];
						const pv = pool.bboxOrig[pb4 + 1];
						hEdge.setAttribute('x1', String(u));
						hEdge.setAttribute('y1', String(v));
						hEdge.setAttribute('x2', String(pu));
						hEdge.setAttribute('y2', String(pv));
						hEdge.setAttribute('display', 'inline');
					} else {
						hEdge.setAttribute('display', 'none');
					}
				} else {
					hEdge.setAttribute('display', 'none');
				}
			}

			// --- Layer 4: selection outline ---
			if (selRing) {
				if (s === selSlot) {
					selRing.setAttribute('x', String(x - 4));
					selRing.setAttribute('y', String(y - 4));
					selRing.setAttribute('width', String(w + 8));
					selRing.setAttribute('height', String(h + 8));
					selRing.setAttribute('display', 'inline');
				} else {
					selRing.setAttribute('display', 'none');
				}
			}

			// --- Hover ring (from RightRail card hover) ---
			if (hoverRing) {
				if (s === hovSlot && s !== selSlot) {
					hoverRing.setAttribute('x', String(x - 2));
					hoverRing.setAttribute('y', String(y - 2));
					hoverRing.setAttribute('width', String(w + 4));
					hoverRing.setAttribute('height', String(h + 4));
					hoverRing.setAttribute('display', 'inline');
				} else {
					hoverRing.setAttribute('display', 'none');
				}
			}
		}

		// --- Layer 4: target-lock crosshair (one per active lock) ---
		// Done via {#each} render of locks - svelte handles diffing.
		// REPLAY mode tints crosshair color via CSS variable (set on root g)

		rafId = requestAnimationFrame(tick);
	}

	function handleBboxClick(e: MouseEvent, slot: number) {
		e.stopPropagation();
		selectionStore.select(slot);
		if (selectionStore.clickToLockMode) {
			selectionStore.addLock(slot);
			selectionStore.disableClickToLock();
		}
	}

	function handleCanvasClick(e: MouseEvent) {
		// Click on empty area deselects (unless click-to-lock is mid-press)
		if (e.target === e.currentTarget) {
			selectionStore.deselect();
		}
	}

	// NOTE: telemetrySocket + mockFrameSource lifecycle is OWNED by
	// NavigationShell so the WS doesn't drop on every destination switch.
	// This component only owns its render loop.
	onMount(() => {
		rafId = requestAnimationFrame(tick);
		// Start polling the orchestrator's /api/fov so the dashed inference-
		// crop rectangle stays in sync even when the FOV drawer is closed.
		// fovStore.startPolling() is idempotent, so opening the FOV drawer
		// later (which also starts polling) is harmless.
		fovStore.startPolling();
	});

	onDestroy(() => {
		cancelAnimationFrame(rafId);
		fovStore.stopPolling();
	});

	// Derived for template - current locks & their slot lookups
	const lockSlots = $derived(
		selectionStore.locks.map((lock) => ({
			lock,
			slot: selectionStore.slotForLock(lock)
		}))
	);

	const isReplayMode = $derived(profileStore.isReplay);
</script>

<div class="operate-canvas-root" role="presentation">
	<!--
		Canvas stage: aspect-ratio-locked box that hosts the raster (img / grid)
		AND the SVG overlay as siblings. Stage width/height collapse to the
		largest box that fits inside .operate-canvas-root while preserving the
		source AR (VIEW_W : VIEW_H). overflow: hidden hard-clips overlay
		geometry to the visible video rect — see
		workspace_blueprint/research_notes/research_svg_overlay_alignment_2026.md
		and .cursor/rules/220-operate-canvas-stage.mdc.

		Status bar, CTL banner, and Replay banner live OUTSIDE the stage so
		they remain pinned to the panel chrome and may legitimately overlap
		the dark gutter area.
	-->
	<div class="canvas-stage" style="aspect-ratio: {VIEW_W} / {VIEW_H};">
		<!-- Layer 0: live MJPEG video OR placeholder grid (Sources drawer controls) -->
		{#if cameraPlaceholderVisible}
			{#if videoSrcUrl}
				<!-- Fallback grid ALWAYS rendered first so it shows through during the
				     few ms before the first MJPEG frame arrives. -->
				<div class="grid-bg layer-0-fallback"></div>

				<!-- MJPEG-over-HTTP: browsers natively render multipart/x-mixed-replace
				     into <img>. Initial display: none so the grid is visible until the
				     first JPEG frame fires onload; that flips display: block + sets
				     mjpegLoaded so the "connecting" chip hides. object-fit: fill is
				     safe because the stage already enforces source AR; this removes
				     any sub-pixel rounding seam vs the SVG overlay. -->
				<!-- svelte-ignore a11y_missing_attribute -->
				<img
					class="layer-0-video"
					src={videoSrcUrl}
					style="display: none;"
					onerror={(e) => {
						(e.currentTarget as HTMLImageElement).style.display = 'none';
						mjpegLoaded = false;
					}}
					onload={(e) => {
						(e.currentTarget as HTMLImageElement).style.display = 'block';
						mjpegLoaded = true;
					}}
				/>

				{#if !mjpegLoaded}
					<div class="awaiting-chip">
						<span class="chip-dot"></span>
						<span>CONNECTING TO MJPEG</span>
						<span class="chip-sep">·</span>
						<span class="chip-src">SRC: {sourceLabel}</span>
					</div>
				{/if}
			{:else}
				<div class="grid-bg"></div>
				<!-- Claude polish #3: small chip at bottom-center so bbox geometry stays unobstructed -->
				<div class="awaiting-chip">
					<span class="chip-dot"></span>
					<span>AWAITING VIDEO FEED</span>
					<span class="chip-sep">·</span>
					<span class="chip-src">SRC: {sourceLabel}</span>
				</div>
			{/if}
		{/if}

		<!-- SVG composite overlay (Layers 1, 1.5, 2, 3, 4) -->
	<svg
		class="layer-svg"
		viewBox="0 0 {VIEW_W} {VIEW_H}"
		preserveAspectRatio="xMidYMid meet"
		xmlns="http://www.w3.org/2000/svg"
		onclick={handleCanvasClick}
		role="img"
		aria-label="Operate composite spatial canvas"
	>
		<!-- Layer 1: trajectories -->
		<g class="layer-1-trajectories">
			{#each Array(MAX_SLOTS) as _, i (i)}
				<path
					bind:this={trajRefs[i]}
					fill="none"
					stroke="#00f0ff"
					stroke-width="1.5"
					stroke-dasharray="2 2"
					stroke-linecap="round"
					stroke-linejoin="round"
					opacity="0.6"
					vector-effect="non-scaling-stroke"
					display="none"
					pointer-events="none"
				></path>
			{/each}
		</g>

		<!-- Layer 1.5: actuator path history (raw=dirty polyline, clean=smoothed
		     path from JS controller) + origin crosshair + live cursor marker.
		     See the JS controller block at the top of <script> for the data
		     model (Phase 6-part-A stand-in until C++ Ruckig lands). -->
		<g class="layer-15-actuator">
			<!-- DIRTY trail: the raw target-snap signature (red in the user's
			     reference image) — what an unhumanised bot produces. -->
			<polyline
				bind:this={actuatorRawPathRef}
				fill="none"
				stroke="#ff8a4c"
				stroke-width="1.5"
				stroke-opacity="0.5"
				stroke-linejoin="round"
				stroke-linecap="round"
				vector-effect="non-scaling-stroke"
				display="none"
				pointer-events="none"
			></polyline>
			<!-- CLEAN trail: jerk-limited approach from the JS controller
			     (green in the user's reference image) — what a humanised aim
			     assist produces. Indistinguishable from real human aim. -->
			<path
				bind:this={actuatorCleanPathRef}
				fill="none"
				stroke="#00f0ff"
				stroke-width="2"
				stroke-opacity="0.8"
				stroke-linecap="round"
				stroke-linejoin="round"
				vector-effect="non-scaling-stroke"
				display="none"
				pointer-events="none"
			></path>

			<!-- Origin crosshair: marks (VIEW_W/2, VIEW_H/2) — the virtual
			     cursor's birthplace = the in-game crosshair anchor. Small,
			     dim, always-on while actuator overlay is visible. -->
			<g
				bind:this={cursorOriginRef}
				display="none"
				pointer-events="none"
				class="cursor-origin"
			>
				<line
					x1="-10" y1="0" x2="10" y2="0"
					stroke="#7a8a9a" stroke-width="1" stroke-opacity="0.45"
					vector-effect="non-scaling-stroke"
				></line>
				<line
					x1="0" y1="-10" x2="0" y2="10"
					stroke="#7a8a9a" stroke-width="1" stroke-opacity="0.45"
					vector-effect="non-scaling-stroke"
				></line>
				<circle
					cx="0" cy="0" r="2.5"
					fill="none" stroke="#7a8a9a" stroke-width="1" stroke-opacity="0.45"
					vector-effect="non-scaling-stroke"
				></circle>
			</g>

			<!-- Live cursor marker: shows where the JS controller's virtual
			     cursor is RIGHT NOW. This is the head of the clean trail. -->
			<g
				bind:this={cursorMarkerRef}
				display="none"
				pointer-events="none"
				class="cursor-marker"
			>
				<circle
					cx="0" cy="0" r="6"
					fill="none" stroke="#00f0ff" stroke-width="1.5" stroke-opacity="0.85"
					vector-effect="non-scaling-stroke"
				></circle>
				<circle
					bind:this={cursorMarkerDotRef}
					cx="0" cy="0" r="2"
					fill="#00f0ff" fill-opacity="0.9"
				></circle>
			</g>
		</g>

		<!-- Layer 2: detection boxes + FOV ring + ellipses -->
		<g class="layer-2-detections">
			<!-- Phase 6 FOV inference-crop rectangle (dashed orange).
			     Drawn ONLY when the orchestrator reports an active crop
			     (i.e. fovStore.settings.enabled && activeCropW > 0).
			     This is the EXACT rect the InferenceEngine saw last frame,
			     so the operator can visually verify what the model is and
			     isn't being shown. -->
			<rect
				bind:this={fovCropRectRef}
				fill="none"
				stroke="#ff8a4c"
				stroke-width="2"
				stroke-opacity="0.6"
				stroke-dasharray="6 4"
				vector-effect="non-scaling-stroke"
				display="none"
				pointer-events="none"
			></rect>

			<!-- FOV targeting ring -->
			<circle
				bind:this={fovRingRef}
				fill="none"
				stroke="#ffbb00"
				stroke-width="2"
				stroke-opacity="0.30"
				stroke-dasharray="8 4"
				vector-effect="non-scaling-stroke"
				display="none"
				pointer-events="none"
			></circle>

			<!-- Uncertainty ellipses (one per slot) -->
			{#each Array(MAX_SLOTS) as _, i (i)}
				<ellipse
					bind:this={ellipseRefs[i]}
					fill="none"
					stroke="#00f0ff"
					stroke-width="1"
					stroke-opacity="0.35"
					stroke-dasharray="4 2"
					vector-effect="non-scaling-stroke"
					display="none"
					pointer-events="none"
				></ellipse>
			{/each}

			<!-- Bounding box halos — dark 3px stroke under the colored 2px for legibility on real video pixels (Claude bigger thought #1) -->
			{#each Array(MAX_SLOTS) as _, i (i)}
				<rect
					bind:this={bboxHaloRefs[i]}
					fill="none"
					stroke="#000000"
					stroke-opacity="0.45"
					stroke-width="4"
					vector-effect="non-scaling-stroke"
					display="none"
					pointer-events="none"
				></rect>
			{/each}

			<!-- Bounding boxes (interactive, colored stroke) -->
			{#each Array(MAX_SLOTS) as _, i (i)}
				<rect
					bind:this={bboxRefs[i]}
					fill="none"
					stroke="#00f0ff"
					stroke-width="2"
					vector-effect="non-scaling-stroke"
					display="none"
					class="bbox-interactive"
					onclick={(e) => handleBboxClick(e, i)}
					onmouseenter={() => selectionStore.setHover(i)}
					onmouseleave={() => selectionStore.clearHover()}
					role="button"
					tabindex="-1"
					aria-label="Track {i}"
				></rect>
			{/each}

			<!-- Label pills (background rects) -->
			{#each Array(MAX_SLOTS) as _, i (i)}
				<rect
					bind:this={labelBgRefs[i]}
					fill="#00f0ff"
					stroke="none"
					rx="2"
					display="none"
					pointer-events="none"
				></rect>
			{/each}

			<!-- Label text -->
			{#each Array(MAX_SLOTS) as _, i (i)}
				<text
					bind:this={labelTextRefs[i]}
					fill="#0a1014"
					font-family="ui-monospace, 'SF Mono', Consolas, monospace"
					font-size="11"
					font-weight="600"
					display="none"
					pointer-events="none"
				></text>
			{/each}

			<!-- Track ID badge in bbox corner -->
			{#each Array(MAX_SLOTS) as _, i (i)}
				<text
					bind:this={idBadgeRefs[i]}
					fill="#00f0ff"
					font-family="ui-monospace, 'SF Mono', Consolas, monospace"
					font-size="9"
					text-anchor="end"
					fill-opacity="0.7"
					display="none"
					pointer-events="none"
				></text>
			{/each}
		</g>

		<!-- Layer 3: hierarchy edges -->
		<g class="layer-3-hierarchy">
			{#each Array(MAX_SLOTS) as _, i (i)}
				<line
					bind:this={hierarchyEdgeRefs[i]}
					stroke="#555"
					stroke-width="1"
					stroke-dasharray="6 3"
					stroke-opacity="0.6"
					vector-effect="non-scaling-stroke"
					display="none"
					pointer-events="none"
				></line>
			{/each}
		</g>

		<!-- Layer 4: anchor crosshairs + velocity arrows + selection rings + locks -->
		<g class="layer-4-overlay">
			<!-- Anchor crosshair per slot (g containing diamond + cross arms) -->
			{#each Array(MAX_SLOTS) as _, i (i)}
				<g bind:this={anchorRefs[i]} display="none" pointer-events="none">
					<!-- white outline halo for visibility on any background -->
					<line x1="-8" y1="0" x2="8" y2="0" stroke="#ffffff" stroke-width="3" stroke-opacity="0.4" vector-effect="non-scaling-stroke"></line>
					<line x1="0" y1="-8" x2="0" y2="8" stroke="#ffffff" stroke-width="3" stroke-opacity="0.4" vector-effect="non-scaling-stroke"></line>
					<!-- orange crosshair -->
					<line x1="-8" y1="0" x2="8" y2="0" stroke="#f5a623" stroke-width="1.5" vector-effect="non-scaling-stroke"></line>
					<line x1="0" y1="-8" x2="0" y2="8" stroke="#f5a623" stroke-width="1.5" vector-effect="non-scaling-stroke"></line>
					<!-- center diamond -->
					<rect
						bind:this={anchorCenterRefs[i]}
						x="-3"
						y="-3"
						width="6"
						height="6"
						fill="#f5a623"
						transform="rotate(45)"
					></rect>
				</g>
			{/each}

			<!-- Velocity arrows -->
			{#each Array(MAX_SLOTS) as _, i (i)}
				<line
					bind:this={velArrowRefs[i]}
					stroke="#00f0ff"
					stroke-width="2"
					stroke-opacity="0.8"
					marker-end="url(#arrowhead-cyan)"
					vector-effect="non-scaling-stroke"
					display="none"
					pointer-events="none"
				></line>
			{/each}

			<!-- Hover rings (transient, from RightRail) -->
			{#each Array(MAX_SLOTS) as _, i (i)}
				<rect
					bind:this={hoverRingRefs[i]}
					fill="none"
					stroke="#f5a623"
					stroke-width="1.5"
					stroke-opacity="0.6"
					rx="2"
					vector-effect="non-scaling-stroke"
					display="none"
					pointer-events="none"
				></rect>
			{/each}

			<!-- Selection rings (persistent until deselect) -->
			{#each Array(MAX_SLOTS) as _, i (i)}
				<rect
					bind:this={selectionRingRefs[i]}
					fill="none"
					stroke="#f5a623"
					stroke-width="3"
					rx="3"
					vector-effect="non-scaling-stroke"
					display="none"
					pointer-events="none"
				></rect>
			{/each}

			<!-- Target-lock crosshairs (one per active lock, REPLAY mode = red) -->
			<g bind:this={lockCrosshairGroupRef} pointer-events="none">
				{#each lockSlots as { slot } (slot)}
					{#if slot >= 0 && trackPool.presenceBitmap[slot] === 1}
						{@const u = trackPool.bboxOrig[slot * 4 + 0]}
						{@const v = trackPool.bboxOrig[slot * 4 + 1]}
						{@const lockColor = isReplayMode ? '#ef4444' : '#ef4444'}
						<g transform="translate({u} {v})" class="lock-crosshair">
							<circle r="20" fill="none" stroke={lockColor} stroke-width="2" opacity="0.6" vector-effect="non-scaling-stroke"></circle>
							<line x1="-32" y1="0" x2="-12" y2="0" stroke={lockColor} stroke-width="2" vector-effect="non-scaling-stroke"></line>
							<line x1="12" y1="0" x2="32" y2="0" stroke={lockColor} stroke-width="2" vector-effect="non-scaling-stroke"></line>
							<line x1="0" y1="-32" x2="0" y2="-12" stroke={lockColor} stroke-width="2" vector-effect="non-scaling-stroke"></line>
							<line x1="0" y1="12" x2="0" y2="32" stroke={lockColor} stroke-width="2" vector-effect="non-scaling-stroke"></line>
							<circle r="3" fill={lockColor}></circle>
						</g>
					{/if}
				{/each}
			</g>
		</g>

		<!-- Arrow marker definitions -->
		<defs>
			<marker
				id="arrowhead-cyan"
				viewBox="0 0 10 10"
				refX="9"
				refY="5"
				markerWidth="6"
				markerHeight="6"
				orient="auto-start-reverse"
			>
				<path d="M 0 0 L 10 5 L 0 10 z" fill="#00f0ff" fill-opacity="0.8"></path>
			</marker>
		</defs>
	</svg>
	</div>

	<!-- Status bar (top-left, semi-transparent dark pill) -->
	<div class="canvas-status-bar">
		<span class="status-src" class:src-live={telemetrySocket.state === 'connected' || sourceStore.mjpegUrl !== null}>
			SRC: {sourceLabel}
		</span>
		<span class="status-sep">·</span>
		<span class="status-mono">SEQ: {trackPool.frameSeq}</span>
		<span class="status-sep">·</span>
		<span class="status-mono">ACT: {trackPool.activeCount}</span>
		<span class="status-sep">·</span>
		<span
			class="status-mono"
			class:status-warn={trackPool.frameFlags & 0x01}
		>NIS: {(trackPool.frameFlags & 0x01) ? 'DEGRADED' : 'OK'}</span>
		<span class="status-sep">·</span>
		<span
			class="status-mono"
			class:status-warn={trackPool.frameFlags & 0x02}
		>BND: {(trackPool.frameFlags & 0x02) ? 'PRESENT' : 'OK'}</span>
		<span class="status-sep">·</span>
		<span class="status-mono">WS: {telemetrySocket.framesReceived}/{telemetrySocket.reconnectAttempt}</span>
	</div>

	<!-- Click-to-lock cursor mode indicator -->
	{#if selectionStore.clickToLockMode}
		<div class="ctl-mode-banner">
			CLICK-TO-LOCK MODE · Press Esc to cancel
		</div>
	{/if}

	<!-- REPLAY mode banner -->
	{#if isReplayMode}
		<div class="replay-mode-banner">
			REPLAY MODE · Actuator commands hard-disabled
		</div>
	{/if}
</div>

<style>
	.operate-canvas-root {
		position: relative;
		width: 100%;
		height: 100%;
		min-width: 0;
		min-height: 0;
		background: #06080c;
		overflow: hidden;
		/* Grid centering picks the largest aspect-locked stage that fits — the
		   stage's max-width / max-height + aspect-ratio do the rest.
		   Explicit 1fr/1fr template forces the grid cell to fill the root so
		   place-items:center has a full-size cell to center the stage inside.
		   Without explicit tracks the cell collapses to stage size and the
		   centering becomes a no-op. */
		display: grid;
		grid-template-columns: minmax(0, 1fr);
		grid-template-rows: minmax(0, 1fr);
		place-items: center;
	}

	/* Aspect-ratio-locked stage that hosts the raster (img / grid) AND the SVG
	   overlay as absolutely-positioned siblings. Stage carries the source AR
	   (set inline via style="aspect-ratio: {VIEW_W} / {VIEW_H};"), so the SVG
	   viewBox maps 1:1 to image pixels and overflow: hidden hard-clips any
	   geometry (FOV ring, bbox stroke, etc.) to the visible video rect — no
	   gutter spillover possible. See
	   workspace_blueprint/research_notes/research_svg_overlay_alignment_2026.md
	   and .cursor/rules/220-operate-canvas-stage.mdc. */
	.canvas-stage {
		position: relative;
		/* All children are position:absolute so the stage has no intrinsic
		   content size — we must explicitly claim space. The pattern:
		     width: 100%   → fill parent inline axis
		     max-height: 100% → never exceed parent block axis
		     aspect-ratio (inline) → derives height from width
		   When parent's AR matches source AR, width fills exactly and height
		   = width * H/W lands inside max-height. When parent is wider than
		   source AR, derived height > max-height; CSS Sizing 4 clamps height
		   to 100% and re-derives width via aspect-ratio so AR is preserved.
		   Setting BOTH width:100% AND height:100% would override aspect-ratio
		   entirely (spec: explicit values win), defeating the AR lock. */
		width: 100%;
		max-width: 100%;
		max-height: 100%;
		overflow: hidden;
		/* aspect-ratio is set inline from VIEW_W / VIEW_H (reactive to source). */
		/* place-items: center on the parent .operate-canvas-root then visually
		   centers this stage when the parent has a different AR. */
	}

	/* Layer 0 live video — fills the stage exactly. object-fit: fill is safe
	   here (and preferable) because the stage already enforces source AR; this
	   removes any sub-pixel rounding seam between the image and the SVG
	   overlay above. Transparent background so the fallback grid underneath
	   stays visible until the first MJPEG frame arrives (then onload flips
	   display: block and the video covers the grid naturally). */
	.layer-0-video {
		position: absolute;
		inset: 0;
		width: 100%;
		height: 100%;
		object-fit: fill;
		background: transparent;
		display: block;
		z-index: 1;
	}

	.layer-0-fallback {
		z-index: 0;
	}

	.grid-bg {
		position: absolute;
		inset: 0;
		background-image:
			linear-gradient(to right, rgba(245, 166, 35, 0.06) 1px, transparent 1px),
			linear-gradient(to bottom, rgba(245, 166, 35, 0.06) 1px, transparent 1px),
			radial-gradient(circle at center, rgba(20, 20, 24, 0) 0%, rgba(6, 8, 12, 0.8) 100%);
		background-size:
			calc(100% / 16) 100%,
			100% calc(100% / 9),
			100% 100%;
	}

	/* Bottom-center chip — keeps bbox geometry unobstructed (Claude polish #3) */
	.awaiting-chip {
		position: absolute;
		bottom: 56px;
		left: 50%;
		transform: translateX(-50%);
		display: flex;
		align-items: center;
		gap: 8px;
		padding: 5px 12px;
		background: rgba(17, 17, 19, 0.85);
		backdrop-filter: blur(6px);
		border: 1px solid rgba(245, 166, 35, 0.2);
		border-radius: 14px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 10px;
		color: rgba(212, 208, 200, 0.7);
		letter-spacing: 0.06em;
		user-select: none;
		pointer-events: auto;
	}

	.chip-dot {
		width: 6px;
		height: 6px;
		border-radius: 50%;
		background: #f5a623;
		opacity: 0.6;
		animation: chip-pulse 1.6s ease-in-out infinite;
	}

	@keyframes chip-pulse {
		0%, 100% { opacity: 0.4; }
		50% { opacity: 0.9; }
	}

	.chip-sep {
		color: #2a2a2e;
	}

	.chip-src {
		color: #f5a623;
		font-weight: 600;
	}

	/* SVG layer */
	/* SVG composite layer sits ABOVE the MJPEG <img> (z-index: 1) and the
	   fallback grid (z-index: 0) so bbox/trajectory/hierarchy overlays
	   render on top of the camera feed. Without an explicit z-index here
	   the SVG defaults to `auto` and the image (with z:1) stacks above. */
	.layer-svg {
		position: absolute;
		inset: 0;
		width: 100%;
		height: 100%;
		display: block;
		cursor: crosshair;
		z-index: 2;
	}

	.bbox-interactive {
		cursor: pointer;
		transition: stroke-width 80ms ease;
	}

	.bbox-interactive:hover {
		stroke-width: 3;
	}

	.lock-crosshair {
		animation: lock-pulse 1.2s ease-in-out infinite;
	}

	@keyframes lock-pulse {
		0%,
		100% {
			opacity: 1;
		}
		50% {
			opacity: 0.5;
		}
	}

	/* Status bar */
	.canvas-status-bar {
		position: absolute;
		top: 10px;
		left: 10px;
		background: rgba(6, 8, 12, 0.78);
		backdrop-filter: blur(6px);
		border: 1px solid rgba(39, 39, 42, 0.8);
		border-radius: 5px;
		padding: 5px 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 10px;
		color: #d4d0c8;
		letter-spacing: 0.04em;
		pointer-events: none;
		z-index: 5;
	}

	.status-src {
		font-weight: 700;
		color: #878787;
	}

	.status-src.src-live {
		color: #34d399;
		text-shadow: 0 0 6px rgba(52, 211, 153, 0.4);
	}

	.status-mono {
		color: #d4d0c8;
	}

	.status-warn {
		color: #ef4444 !important;
	}

	.status-sep {
		color: #444;
		margin: 0 3px;
	}

	.ctl-mode-banner,
	.replay-mode-banner {
		position: absolute;
		top: 10px;
		left: 50%;
		transform: translateX(-50%);
		background: rgba(239, 68, 68, 0.15);
		color: #ef4444;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 11px;
		font-weight: 700;
		letter-spacing: 0.08em;
		padding: 6px 14px;
		border: 1px solid rgba(239, 68, 68, 0.5);
		border-radius: 4px;
		z-index: 5;
		pointer-events: none;
		animation: pulse-banner 1.2s ease-in-out infinite;
	}

	.replay-mode-banner {
		top: 50px;
		background: rgba(167, 139, 250, 0.15);
		color: #a78bfa;
		border-color: rgba(167, 139, 250, 0.5);
	}

	@keyframes pulse-banner {
		0%,
		100% {
			opacity: 1;
		}
		50% {
			opacity: 0.65;
		}
	}
</style>
