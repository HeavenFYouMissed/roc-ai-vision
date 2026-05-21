// =============================================================================
// roc-ai-vision · dashboard · mock_frame_source.ts
//
// Stand-in for the production C++ tracker's WebSocket egress: synthesises
// wire-format-conformant ArrayBuffers across all three frame versions so the
// dashboard can be developed and visually validated without live hardware.
//
// Three independent emitters share the same `onFrame` callback contract; the
// consumer (`wire_decoder.decode_frame()`) dispatches on byte 0:
//
//   v1 — Track frames    at  30 Hz (12 B header + N × 86 B/track)
//   v2 — Actuator tel.   at  60 Hz (Phase 5b placeholder; 500 Hz in 5c)  — §12.4
//   v3 — Bg-odometry     at  30 Hz (32 B/frame)                          — §13.4
//
// Steady-state heap allocation per frame: 0 bytes. Each emitter has its own
// pre-allocated buffer reused on every tick. Consumers MUST consume the
// bytes synchronously inside `onFrame` — the buffers are overwritten on
// the next tick of that emitter.
//
// Scene (v1):
//   * Person A — wide orbit (radius 520 px about screen centre) that grazes
//     the frame edges, intentionally tripping §3.10 boundary_probation when
//     either bbox edge crosses the εb = 4 px guard.
//   * Person B — moderate orbit (radius 280 px), periodically pinned to a
//     fixed point to fire the §2.6 stationary regime detector visualisation.
//   * Head + UpperTorso for each Person, anatomically offset above the
//     parent bbox centroid (image-plane analogue of the §3.11.1 priors).
//   * One head occasionally fires a 3-frame hierarchy-violation flash.
//
// Scene (v2):
//   * Actuator tracks Person A's head with the EKF+Ruckig "clean" pipeline.
//   * `raw_u/v` injects ±1.5 px white-noise jitter on top of the clean signal
//     to simulate HID polling noise — this is the "dirty" trace the
//     MouseDynamicsInspector will visualise as sharp polyline segments.
//   * `applied_alpha` slowly oscillates between 0.15 and 0.45 to exercise the
//     config-change-marker rendering path in MouseDynamicsInspector per §12.3.
//
// Scene (v3):
//   * Background optical-flow synthesised from a slow camera-yaw rotation
//     (1 rad/sec sinusoidal pan) — Δθ tracks the derivative of that pan,
//     Δx/Δy track the projected screen-space pixel motion.
//   * `feature_count` and `inlier_count` synthesised in the 150-200 range
//     with occasional drops to 30-50 to simulate degraded-low-features
//     events (the SpectatorTelemetryMatrix should grey-out those windows).
//   * `confidence` derived from `inlier_count / feature_count` ratio.
//
// =============================================================================

const HEADER_SIZE = 12;
const TRACK_SIZE = 86;
const MAX_CAPACITY = 64;
const BUFFER_BYTES = HEADER_SIZE + MAX_CAPACITY * TRACK_SIZE;

const VIEW_W = 1920;
const VIEW_H = 1080;
const CX = VIEW_W * 0.5;
const CY = VIEW_H * 0.5;

// Pseudo-metres scale for the p_world field.  The C++ contract expects metres
// in world frame; here we synthesise plausible scale by dividing pixels by
// 100 — so a (960, 540) px centroid maps to a (9.60, 5.40) m world position.
// This keeps any future Observer-Matrix panel that displays raw p_world
// reading reasonable units rather than viewport pixels.
const PX_TO_M = 1.0 / 100.0;

// Status-flag bit constants (mirror wire_format.hpp wire_status_bits).
const FLAG_CONFIRMED = 0x01;
const FLAG_BOUNDARY = 0x02;
const FLAG_STATIONARY = 0x04;
const FLAG_HIERARCHY_VIOLATION = 0x08;
// Frame-level flag bits (mirror wire_format.hpp wire_frame_flag_bits).
const FLAG_NIS_DEGRADED = 0x01;

// Label enum (mirror types.hpp Label).
const LABEL_PERSON = 1;
const LABEL_HEAD = 2;
const LABEL_UPPER_TORSO = 3;

// Track ids (low 32 bits; high 32 bits are always 0 in the mock).
const ID_PERSON_A = 100;
const ID_HEAD_A = 101;
const ID_TORSO_A = 102;
const ID_PERSON_B = 200;
const ID_HEAD_B = 201;
const ID_TORSO_B = 202;

const TRACK_COUNT = 6;

const buffer = new ArrayBuffer(BUFFER_BYTES);
const view = new DataView(buffer);

// Reusable parent-position table (two persons × {u, v, isStationary}).  Cached
// in module scope so the second pass over heads/torsos reads from a single
// computed parent state without re-running the orbit math three times.  No
// allocation per tick.
const parentU = new Float32Array(2);
const parentV = new Float32Array(2);
const parentStationary = new Uint8Array(2);
const parentBoundary = new Uint8Array(2);

let intervalId: ReturnType<typeof setInterval> | null = null;
let frameCounter = 0;
let startEpochMs = 0;

function compute_parents(elapsedMs: number): void {
	const t = elapsedMs * 0.001;

	// Person A — wide orbit, intentionally grazes the frame edges.
	const angleA = t * 0.6 + 0.0;
	const uA = CX + Math.cos(angleA) * 520;
	const vA = CY + Math.sin(angleA) * 280;
	parentU[0] = uA;
	parentV[0] = vA;
	parentStationary[0] = 0;
	// Person bbox is 120×240 — boundary fires when an edge is within εb=4 px
	// of the viewport limit.  Half-width = 60, half-height = 120.
	const aLeft = uA - 60;
	const aRight = uA + 60;
	const aTop = vA - 120;
	const aBot = vA + 120;
	parentBoundary[0] =
		aLeft <= 4 || aRight >= VIEW_W - 4 || aTop <= 4 || aBot >= VIEW_H - 4 ? 1 : 0;

	// Person B — moderate orbit, periodically pinned to fire the §2.6
	// stationary-regime detector.  Pinned for the first 2.5 s of every 6 s.
	const phase = elapsedMs % 6000;
	const isStationary = phase < 2500;
	parentStationary[1] = isStationary ? 1 : 0;
	if (isStationary) {
		parentU[1] = CX - 300;
		parentV[1] = CY + 100;
	} else {
		const angleB = t * -0.4 + Math.PI;
		parentU[1] = CX + Math.cos(angleB) * 280;
		parentV[1] = CY + Math.sin(angleB) * 160;
	}
	const bLeft = parentU[1] - 60;
	const bRight = parentU[1] + 60;
	const bTop = parentV[1] - 120;
	const bBot = parentV[1] + 120;
	parentBoundary[1] =
		bLeft <= 4 || bRight >= VIEW_W - 4 || bTop <= 4 || bBot >= VIEW_H - 4 ? 1 : 0;
}

// Writes one 86-byte payload at offset `base`.  All numeric writes are
// explicit little-endian to match wire_format.hpp.
function write_track(
	base: number,
	idLo: number,
	parentLo: number,
	label: number,
	statusFlags: number,
	uPx: number,
	vPx: number,
	wPx: number,
	hPx: number,
	confidence: number
): void {
	// track_id u64 — split lo/hi
	view.setUint32(base + 0, idLo, true);
	view.setUint32(base + 4, 0, true);
	// parent_id u64
	view.setUint32(base + 8, parentLo, true);
	view.setUint32(base + 12, 0, true);
	view.setUint8(base + 16, label);
	view.setUint8(base + 17, statusFlags);
	view.setFloat32(base + 18, confidence, true);

	// p_world in fake metres so downstream consumers reading the world-frame
	// vector see sensible units.  v_world / a_world are zeroed — the C++
	// tracker emits them too, but the dashboard only renders bbox today.
	view.setFloat32(base + 22, uPx * PX_TO_M, true);
	view.setFloat32(base + 26, vPx * PX_TO_M, true);
	view.setFloat32(base + 30, 4.0, true);

	view.setFloat32(base + 34, 0.0, true);
	view.setFloat32(base + 38, 0.0, true);
	view.setFloat32(base + 42, 0.0, true);

	view.setFloat32(base + 46, 0.0, true);
	view.setFloat32(base + 50, 0.0, true);
	view.setFloat32(base + 54, 0.0, true);

	// P_pos_diag (3 floats) — small diagonal so future ellipse rendering
	// doesn't look degenerate.
	view.setFloat32(base + 58, 0.05, true);
	view.setFloat32(base + 62, 0.05, true);
	view.setFloat32(base + 66, 0.20, true);

	// bbox_orig (u, v, w_px, h_px) — capture-frame pixels, centroid form
	view.setFloat32(base + 70, uPx, true);
	view.setFloat32(base + 74, vPx, true);
	view.setFloat32(base + 78, wPx, true);
	view.setFloat32(base + 82, hPx, true);
}

function emit_frame(onFrame: (buf: ArrayBuffer) => void): void {
	const elapsedMs = Date.now() - startEpochMs;
	frameCounter++;

	compute_parents(elapsedMs);

	// ── Header ───────────────────────────────────────────────────────────────
	let headerFlags = 0;
	// Burst the NIS-degraded flag for 1.5 s out of every 6 s so the Observer
	// Matrix panel has something to react to.
	if (elapsedMs % 6000 > 4500) headerFlags |= FLAG_NIS_DEGRADED;
	if (parentBoundary[0] || parentBoundary[1]) headerFlags |= 0x02; // boundary_present

	view.setUint8(0, 1); // version
	view.setUint8(1, headerFlags);
	view.setUint32(2, frameCounter, true);
	view.setUint16(6, TRACK_COUNT, true);
	view.setInt32(8, elapsedMs, true);

	// ── Tracks ───────────────────────────────────────────────────────────────
	// Slot mapping in the wire buffer is just the emission order; the decoder
	// re-maps to internal pool slots by track_id, so order here is irrelevant.

	// --- Person A (parent #0) ---
	{
		let statusA = FLAG_CONFIRMED;
		if (parentStationary[0]) statusA |= FLAG_STATIONARY;
		if (parentBoundary[0]) statusA |= FLAG_BOUNDARY;
		write_track(
			HEADER_SIZE + 0 * TRACK_SIZE,
			ID_PERSON_A, 0,
			LABEL_PERSON, statusA,
			parentU[0], parentV[0], 120, 240, 0.92
		);
	}

	// --- Head A (child of Person A) ---
	{
		const uHA = parentU[0] + Math.sin(elapsedMs * 0.005) * 6; // small head wobble
		const vHA = parentV[0] - 100; // 100 px above torso centre
		let statusHA = FLAG_CONFIRMED;
		// Burst a 3-frame hierarchy-violation flash on Head A every 10 s.
		if (elapsedMs % 10000 < 100) statusHA |= FLAG_HIERARCHY_VIOLATION;
		// Heads inherit the parent's boundary flag — a head on a person who is
		// half off-screen will itself be on the boundary.
		if (parentBoundary[0]) statusHA |= FLAG_BOUNDARY;
		write_track(
			HEADER_SIZE + 1 * TRACK_SIZE,
			ID_HEAD_A, ID_PERSON_A,
			LABEL_HEAD, statusHA,
			uHA, vHA, 50, 55, 0.88
		);
	}

	// --- Torso A (child of Person A) ---
	{
		const uTA = parentU[0];
		const vTA = parentV[0] - 20;
		let statusTA = FLAG_CONFIRMED;
		if (parentBoundary[0]) statusTA |= FLAG_BOUNDARY;
		// Torso is the "stable centre" — confidence stays high.
		write_track(
			HEADER_SIZE + 2 * TRACK_SIZE,
			ID_TORSO_A, ID_PERSON_A,
			LABEL_UPPER_TORSO, statusTA,
			uTA, vTA, 90, 100, 0.95
		);
	}

	// --- Person B (parent #1) ---
	{
		let statusB = FLAG_CONFIRMED;
		if (parentStationary[1]) statusB |= FLAG_STATIONARY;
		if (parentBoundary[1]) statusB |= FLAG_BOUNDARY;
		write_track(
			HEADER_SIZE + 3 * TRACK_SIZE,
			ID_PERSON_B, 0,
			LABEL_PERSON, statusB,
			parentU[1], parentV[1], 120, 240, 0.86
		);
	}

	// --- Head B (child of Person B) ---
	{
		const uHB = parentU[1] + Math.sin(elapsedMs * 0.004) * 4;
		const vHB = parentV[1] - 100;
		let statusHB = FLAG_CONFIRMED;
		if (parentStationary[1]) statusHB |= FLAG_STATIONARY;
		if (parentBoundary[1]) statusHB |= FLAG_BOUNDARY;
		write_track(
			HEADER_SIZE + 4 * TRACK_SIZE,
			ID_HEAD_B, ID_PERSON_B,
			LABEL_HEAD, statusHB,
			uHB, vHB, 50, 55, 0.82
		);
	}

	// --- Torso B (child of Person B) ---
	{
		const uTB = parentU[1];
		const vTB = parentV[1] - 20;
		let statusTB = FLAG_CONFIRMED;
		if (parentStationary[1]) statusTB |= FLAG_STATIONARY;
		if (parentBoundary[1]) statusTB |= FLAG_BOUNDARY;
		write_track(
			HEADER_SIZE + 5 * TRACK_SIZE,
			ID_TORSO_B, ID_PERSON_B,
			LABEL_UPPER_TORSO, statusTB,
			uTB, vTB, 90, 100, 0.93
		);
	}

	onFrame(buffer);
}

// =============================================================================
// V2 — Actuator-telemetry frame synthesis (36 B per §12.4)
// =============================================================================

const V2_FRAME_SIZE = 36;
const v2Buffer = new ArrayBuffer(V2_FRAME_SIZE);
const v2View = new DataView(v2Buffer);

const V2_FLAG_SMOOTHED_ONLY = 0x01;
const V2_FLAG_RAW_ONLY = 0x02;
const V2_FLAG_CLICK_DOWN = 0x04;
const V2_FLAG_CLICK_UP = 0x08;
const V2_FLAG_CONFIG_CHANGE_MARKER = 0x10;

const V2_SINK_HID_MOUSE = 1;

// Simple LCG for cheap, deterministic white-noise injection on the raw trace.
// Module-scope state, no per-tick allocation.
let v2Lcg = 0x12345678 >>> 0;
function lcg_uniform_pm1(): number {
	v2Lcg = (Math.imul(v2Lcg, 1664525) + 1013904223) >>> 0;
	// Map u32 → [-1.0, 1.0] without allocating.
	return v2Lcg / 0x80000000 - 1.0;
}

let v2Counter = 0;
let v2LastAppliedAlpha = 0.35;

function emit_v2_frame(onFrame: (buf: ArrayBuffer) => void): void {
	const elapsedMs = Date.now() - startEpochMs;
	v2Counter++;

	// Derive the clean trace from Person A's head trajectory (computed every
	// v1 tick into the parent* tables). At v2's 60 Hz vs v1's 30 Hz we may
	// emit two v2 samples per v1 update — that's fine, the canvas-trace
	// interpretation is "what the actuator was commanded at this instant".
	const headU = parentU[0] + Math.sin(elapsedMs * 0.005) * 6;
	const headV = parentV[0] - 100;

	const cleanU = headU;
	const cleanV = headV;
	// "Dirty" raw signal: clean ± up to 1.5 px white-noise jitter, simulating
	// HID polling artefacts the EKF+Ruckig pipeline smooths away.
	const rawU = cleanU + lcg_uniform_pm1() * 1.5;
	const rawV = cleanV + lcg_uniform_pm1() * 1.5;

	// Slow alpha oscillation 0.15..0.45 with period 14 s — exercises the
	// config-change-marker visualisation path. Marker flag fires on any
	// noticeable per-tick step (> 0.005).
	const targetAlpha = 0.30 + Math.sin(elapsedMs * 0.000449) * 0.15; // ω ≈ 2π/14000
	let flags = V2_FLAG_SMOOTHED_ONLY | V2_FLAG_RAW_ONLY;
	if (Math.abs(targetAlpha - v2LastAppliedAlpha) > 0.005) {
		flags |= V2_FLAG_CONFIG_CHANGE_MARKER;
	}
	v2LastAppliedAlpha = targetAlpha;

	// Synthesise a click every ~6 s; click_down fires for 1 frame, click_up
	// for the following frame.
	const clickPhase = elapsedMs % 6000;
	if (clickPhase < 16) flags |= V2_FLAG_CLICK_DOWN;
	else if (clickPhase < 32) flags |= V2_FLAG_CLICK_UP;

	// ts_ns: scale elapsedMs → ns; split into hi/lo 32-bit halves to avoid
	// BigInt allocation on every tick.
	const tsNs = elapsedMs * 1_000_000;
	const tsLo = (tsNs >>> 0) >>> 0;
	const tsHi = Math.floor(tsNs / 0x100000000) >>> 0;

	v2View.setUint8(0, 2); // version
	v2View.setUint8(1, flags);
	v2View.setUint32(2, v2Counter, true);
	v2View.setUint32(6, tsLo, true);
	v2View.setUint32(10, tsHi, true);
	v2View.setFloat32(14, rawU, true);
	v2View.setFloat32(18, rawV, true);
	v2View.setFloat32(22, cleanU, true);
	v2View.setFloat32(26, cleanV, true);
	v2View.setUint16(30, V2_SINK_HID_MOUSE, true);
	v2View.setFloat32(32, targetAlpha, true);

	onFrame(v2Buffer);
}

// =============================================================================
// V3 — Bg-odometry frame synthesis (32 B per §13.4)
// =============================================================================

const V3_FRAME_SIZE = 32;
const v3Buffer = new ArrayBuffer(V3_FRAME_SIZE);
const v3View = new DataView(v3Buffer);

const V3_FLAG_VALID = 0x01;
const V3_FLAG_DEGRADED_LOW_FEATURES = 0x02;
const V3_FLAG_CENTER_MASK_ACTIVE = 0x04;
const V3_FLAG_SCALE_VALID = 0x08;

let v3Counter = 0;
let v3LastYawRad = 0;

function emit_v3_frame(onFrame: (buf: ArrayBuffer) => void): void {
	const elapsedMs = Date.now() - startEpochMs;
	v3Counter++;

	// Simulate a slow camera-yaw pan: yaw = 0.4 rad amplitude × sin(0.5 rad/s).
	const tSec = elapsedMs * 0.001;
	const yawRad = 0.4 * Math.sin(tSec * 0.5);
	// Per-frame Δθ is the derivative — at our 30 Hz tick, Δθ ≈ (yaw[t] - yaw[t-Δt]).
	const dTheta = yawRad - v3LastYawRad;
	v3LastYawRad = yawRad;

	// Project the yaw into screen-space pixel motion. A camera yaw of dθ rad
	// at a focal length of ~960 px (matches a 1920-wide capture with 90° HFOV)
	// translates the background by ~ -960 × dθ px in the x-axis.
	const dx = -960 * dTheta + lcg_uniform_pm1() * 0.3;
	// Add a tiny vertical drift to simulate slight pitch noise.
	const dy = Math.sin(tSec * 0.7) * 0.4 + lcg_uniform_pm1() * 0.2;

	// Feature count: 180 ± 20 most of the time; periodic 3-sec degradation
	// to 30-50 to exercise the SpectatorTelemetryMatrix grey-out path.
	const degradeWindow = (elapsedMs % 18000) > 15000; // 3 s out of every 18 s
	let featureCount = 180 + Math.round(lcg_uniform_pm1() * 20);
	if (degradeWindow) featureCount = 30 + Math.round(Math.abs(lcg_uniform_pm1()) * 20);
	// Inliers are 85-95% of features under normal conditions, ~50% during degrade.
	const inlierFrac = degradeWindow ? 0.5 + Math.abs(lcg_uniform_pm1()) * 0.1 : 0.85 + Math.abs(lcg_uniform_pm1()) * 0.1;
	const inlierCount = Math.round(featureCount * Math.min(1.0, inlierFrac));

	let flags = V3_FLAG_VALID | V3_FLAG_CENTER_MASK_ACTIVE | V3_FLAG_SCALE_VALID;
	if (degradeWindow) flags |= V3_FLAG_DEGRADED_LOW_FEATURES;

	// Confidence: composite of inlier ratio × feature-count normalisation
	// (cf. v5 §13.3 BgOdometrySample::confidence). Saturates at 1.0.
	const featureNorm = Math.min(1.0, featureCount / 150);
	const inlierRatio = featureCount > 0 ? inlierCount / featureCount : 0.0;
	const confidence = Math.min(1.0, inlierRatio * featureNorm);
	// Quantise to q15 for wire encoding.
	const confidenceQ15 = Math.round(confidence * 32767) & 0xffff;

	const tsNs = elapsedMs * 1_000_000;
	const tsLo = (tsNs >>> 0) >>> 0;
	const tsHi = Math.floor(tsNs / 0x100000000) >>> 0;

	v3View.setUint8(0, 3); // version
	v3View.setUint8(1, flags);
	v3View.setUint16(2, featureCount, true);
	v3View.setUint16(4, inlierCount, true);
	v3View.setUint32(6, v3Counter, true);
	v3View.setUint32(10, tsLo, true);
	v3View.setUint32(14, tsHi, true);
	v3View.setFloat32(18, dx, true);
	v3View.setFloat32(22, dy, true);
	v3View.setFloat32(26, dTheta, true);
	v3View.setUint16(30, confidenceQ15, true);

	onFrame(v3Buffer);
}

// =============================================================================
// Mock-source lifecycle — three independent emitters, single shared
// onFrame callback. Backwards-compatible: legacy callers that called
// `mockFrameSource.start(cb)` get all three streams enabled by default;
// new callers can opt out via the second argument.
// =============================================================================

export type MockFrameOptions = {
	v1Tracks?: boolean;       // default true
	v2Actuator?: boolean;     // default true
	v3BgOdometry?: boolean;   // default true
};

let v2IntervalId: ReturnType<typeof setInterval> | null = null;
let v3IntervalId: ReturnType<typeof setInterval> | null = null;

export const mockFrameSource = {
	start(onFrame: (buf: ArrayBuffer) => void, opts: MockFrameOptions = {}): void {
		if (intervalId !== null || v2IntervalId !== null || v3IntervalId !== null) return;
		const enableV1 = opts.v1Tracks !== false;
		const enableV2 = opts.v2Actuator !== false;
		const enableV3 = opts.v3BgOdometry !== false;
		startEpochMs = Date.now();
		frameCounter = 0;
		v2Counter = 0;
		v3Counter = 0;
		v3LastYawRad = 0;
		v2LastAppliedAlpha = 0.35;

		if (enableV1) intervalId = setInterval(() => emit_frame(onFrame), 1000 / 30);
		// v2 actuator-telemetry at 60 Hz (Phase 5b placeholder; full 500 Hz in
		// Phase 5c when the C++ orchestrator's actuator-thread bumps rate).
		if (enableV2) v2IntervalId = setInterval(() => emit_v2_frame(onFrame), 1000 / 60);
		// v3 bg-odometry at 30 Hz (matches camera frame rate per §13.4).
		if (enableV3) v3IntervalId = setInterval(() => emit_v3_frame(onFrame), 1000 / 30);
	},

	stop(): void {
		if (intervalId !== null) {
			clearInterval(intervalId);
			intervalId = null;
		}
		if (v2IntervalId !== null) {
			clearInterval(v2IntervalId);
			v2IntervalId = null;
		}
		if (v3IntervalId !== null) {
			clearInterval(v3IntervalId);
			v3IntervalId = null;
		}
	},

	isRunning(): boolean {
		return intervalId !== null || v2IntervalId !== null || v3IntervalId !== null;
	}
};
