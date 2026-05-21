// =============================================================================
// roc-ai-vision · dashboard · wire_decoder.ts
//
// Zero-allocation top-level decoder for the C++ engine's binary egress frames.
// Dispatches on byte 0 (version) per the locked schema in
// `dashboard_menu_architecture_2026.md` §13.4 + `200-dashboard-telemetry.mdc`:
//
//   version 0x01 → v1 track frame (12 B header + N × 86 B/track)       → trackPool
//   version 0x02 → v2 actuator-telemetry frame (36 B)                   → actuatorPool
//   version 0x03 → v3 bg-odometry frame (32 B)                          → bgOdometryPool
//   anything else → dropped silently (one dev-mode warning per session)
//
// All numeric DataView accessors below pass `true` for `littleEndian` — a
// silent omission is a silent corruption. The decoder NEVER allocates on the
// hot path; DataView reuse pattern matches the original v1-only implementation.
// =============================================================================

import { trackPool, TRAJECTORY_RING, type TrackPool } from '../stores/track_pool.svelte';
import { actuatorPool, type ActuatorPool } from '../stores/actuator_pool.svelte';
import { bgOdometryPool, type BgOdometryPool } from '../stores/bg_odometry_pool.svelte';

// -----------------------------------------------------------------------------
// Wire-format constants (mirror C++ side; see §200-dashboard-telemetry.mdc).
// -----------------------------------------------------------------------------
export const WIRE_HEADER_SIZE = 12;                  // v1 track-frame header
export const WIRE_TRACK_SIZE = 86;                   // v1 per-track payload
export const WIRE_V2_FRAME_SIZE = 36;                // v2 actuator-telemetry
export const WIRE_V3_FRAME_SIZE = 32;                // v3 bg-odometry

export const WIRE_PROTOCOL_VERSION_V1 = 1;
export const WIRE_PROTOCOL_VERSION_V2 = 2;
export const WIRE_PROTOCOL_VERSION_V3 = 3;

export const STALE_RECLAIM_FRAMES = 60; // ~2 s @ 30 Hz

// -----------------------------------------------------------------------------
// Reused DataView. Re-bound only when the incoming ArrayBuffer pointer differs
// from the cached one — in the mock-source path the same buffer is recycled
// every frame so we pay zero DataView construction cost in steady state.
// -----------------------------------------------------------------------------
let _view: DataView | null = null;

// Track unknown-version warnings — one per (session, version-byte) so the dev
// console doesn't get spammed during version transitions.
const _unknownVersionWarned = new Set<number>();

// -----------------------------------------------------------------------------
// Top-level dispatcher. Returns true iff a known version byte was recognised
// AND the frame decoded successfully. Callers should treat a `false` return
// as a "frame dropped" event (typically harmless — backpressure / version
// transition / partial buffer).
// -----------------------------------------------------------------------------
export function decode_frame(buf: ArrayBuffer): boolean {
	if (!buf || buf.byteLength < 1) return false;

	if (_view === null || _view.buffer !== buf) {
		_view = new DataView(buf);
	}
	const view = _view;

	const version = view.getUint8(0);
	switch (version) {
		case WIRE_PROTOCOL_VERSION_V1:
			return decode_v1_track_frame(view, buf.byteLength, trackPool);
		case WIRE_PROTOCOL_VERSION_V2:
			return decode_v2_actuator_telemetry(view, buf.byteLength, actuatorPool);
		case WIRE_PROTOCOL_VERSION_V3:
			return decode_v3_bg_odometry(view, buf.byteLength, bgOdometryPool);
		default:
			if (import.meta.env.DEV && !_unknownVersionWarned.has(version)) {
				_unknownVersionWarned.add(version);
				console.warn(
					`[wire_decoder] dropping frame with unknown version byte 0x${version
						.toString(16)
						.padStart(2, '0')} (this warning is shown once per version per session)`
				);
			}
			return false;
	}
}

// =============================================================================
// V1 — Track frame (12 B header + N × 86 B/track) — UNCHANGED from the
// production-locked layout pinned in `core/kinematics_engine/include/roc/
// kinematics/wire_format.hpp`.
// =============================================================================

function decode_v1_track_frame(view: DataView, byteLength: number, pool: TrackPool): boolean {
	if (byteLength < WIRE_HEADER_SIZE) return false;

	const flags = view.getUint8(1);
	const frameId = view.getUint32(2, true);
	const numTracks = view.getUint16(6, true);
	// const tCaptureMsOffset = view.getInt32(8, true);  // unused on the dashboard side today

	const required = WIRE_HEADER_SIZE + numTracks * WIRE_TRACK_SIZE;
	if (byteLength < required) return false;

	let activeCount = 0;
	for (let i = 0; i < numTracks; i++) {
		const base = WIRE_HEADER_SIZE + i * WIRE_TRACK_SIZE;

		const idLo = view.getUint32(base + 0, true);
		const idHi = view.getUint32(base + 4, true);
		const parentLo = view.getUint32(base + 8, true);
		const parentHi = view.getUint32(base + 12, true);
		const label = view.getUint8(base + 16);
		const statusFlags = view.getUint8(base + 17);
		const confidence = view.getFloat32(base + 18, true);

		let slot = pool.findSlotByTrackId(idLo, idHi);
		if (slot < 0) {
			slot = pool.getAvailableSlot();
			if (slot < 0) continue; // pool full — drop gracefully

			pool.presenceBitmap[slot] = 1;
			pool.trackIdLo[slot] = idLo;
			pool.trackIdHi[slot] = idHi;
			pool.trajHead[slot] = 0;
			pool.trajFill[slot] = 0;

			const hiHex = idHi.toString(16);
			const loHex = idLo.toString(16).padStart(8, '0');
			pool.idStrings[slot] = '0x' + hiHex + loHex;
		}

		pool.parentIdLo[slot] = parentLo;
		pool.parentIdHi[slot] = parentHi;
		pool.label[slot] = label;
		pool.statusFlags[slot] = statusFlags;
		pool.confidence[slot] = confidence;

		const v3 = slot * 3;
		pool.pWorld[v3 + 0] = view.getFloat32(base + 22, true);
		pool.pWorld[v3 + 1] = view.getFloat32(base + 26, true);
		pool.pWorld[v3 + 2] = view.getFloat32(base + 30, true);

		pool.vWorld[v3 + 0] = view.getFloat32(base + 34, true);
		pool.vWorld[v3 + 1] = view.getFloat32(base + 38, true);
		pool.vWorld[v3 + 2] = view.getFloat32(base + 42, true);

		pool.aWorld[v3 + 0] = view.getFloat32(base + 46, true);
		pool.aWorld[v3 + 1] = view.getFloat32(base + 50, true);
		pool.aWorld[v3 + 2] = view.getFloat32(base + 54, true);

		pool.pPosDiag[v3 + 0] = view.getFloat32(base + 58, true);
		pool.pPosDiag[v3 + 1] = view.getFloat32(base + 62, true);
		pool.pPosDiag[v3 + 2] = view.getFloat32(base + 66, true);

		const b4 = slot * 4;
		const u = view.getFloat32(base + 70, true);
		const v_ = view.getFloat32(base + 74, true);
		pool.bboxOrig[b4 + 0] = u;
		pool.bboxOrig[b4 + 1] = v_;
		pool.bboxOrig[b4 + 2] = view.getFloat32(base + 78, true);
		pool.bboxOrig[b4 + 3] = view.getFloat32(base + 82, true);

		const head = pool.trajHead[slot];
		const ringIdx = slot * TRAJECTORY_RING + head;
		pool.trajU[ringIdx] = u;
		pool.trajV[ringIdx] = v_;
		pool.trajHead[slot] = (head + 1) % TRAJECTORY_RING;
		if (pool.trajFill[slot] < TRAJECTORY_RING) pool.trajFill[slot]++;

		pool.lastUpdatedFrame[slot] = frameId;
		activeCount++;
	}

	pool.reclaimStaleSlots(frameId, STALE_RECLAIM_FRAMES);

	pool.frameSeq = frameId;
	pool.frameFlags = flags;
	pool.activeCount = activeCount;
	pool.presenceVersion++;
	return true;
}

// =============================================================================
// V2 — Actuator-telemetry frame (36 B total). Per `dashboard_menu_
// architecture_2026.md` §12.4 (final layout supersedes the §11.1 introduction).
//
//   offset  size   field
//        0     1   u8    version              (= 2; consumed by dispatcher)
//        1     1   u8    flags                (bit 0=smoothed-only, bit 1=raw-only,
//                                              bit 2=click_down, bit 3=click_up,
//                                              bit 4=config_change_marker)
//        2     4   u32   frame_id
//        6     8   u64   ts_ns                (lo @6, hi @10)
//       14     4   f32   raw_u_px             (NaN when flags bit 1 clear)
//       18     4   f32   raw_v_px
//       22     4   f32   clean_u_px           (NaN when flags bit 0 clear)
//       26     4   f32   clean_v_px
//       30     2   u16   sink_kind
//       32     4   f32   applied_alpha
//       36     ─   end
// =============================================================================

function decode_v2_actuator_telemetry(
	view: DataView,
	byteLength: number,
	pool: ActuatorPool
): boolean {
	if (byteLength < WIRE_V2_FRAME_SIZE) return false;

	const flags = view.getUint8(1);
	const frameId = view.getUint32(2, true);
	const tsLo = view.getUint32(6, true);
	const tsHi = view.getUint32(10, true);
	const rawU = view.getFloat32(14, true);
	const rawV = view.getFloat32(18, true);
	const cleanU = view.getFloat32(22, true);
	const cleanV = view.getFloat32(26, true);
	const sinkKind = view.getUint16(30, true);
	const appliedAlpha = view.getFloat32(32, true);

	pool.push(tsLo, tsHi, frameId, rawU, rawV, cleanU, cleanV, appliedAlpha, sinkKind, flags);
	return true;
}

// =============================================================================
// V3 — Bg-odometry frame (32 B total). Per `dashboard_menu_architecture_2026.md`
// §13.4.
//
//   offset  size   field
//        0     1   u8    version              (= 3; consumed by dispatcher)
//        1     1   u8    flags                (bit 0=valid_estimate,
//                                              bit 1=degraded_low_features,
//                                              bit 2=center_mask_active,
//                                              bit 3=scale_estimate_valid)
//        2     2   u16   feature_count
//        4     2   u16   inlier_count
//        6     4   u32   frame_id
//       10     8   u64   ts_ns                (lo @10, hi @14)
//       18     4   f32   bg_dx_px
//       22     4   f32   bg_dy_px
//       26     4   f32   bg_theta_rad
//       30     2   u16   confidence_q15       (q15 fixed-point [0.0, 1.0])
//       32     ─   end
// =============================================================================

function decode_v3_bg_odometry(view: DataView, byteLength: number, pool: BgOdometryPool): boolean {
	if (byteLength < WIRE_V3_FRAME_SIZE) return false;

	const flags = view.getUint8(1);
	const featureCount = view.getUint16(2, true);
	const inlierCount = view.getUint16(4, true);
	const frameId = view.getUint32(6, true);
	const tsLo = view.getUint32(10, true);
	const tsHi = view.getUint32(14, true);
	const dx = view.getFloat32(18, true);
	const dy = view.getFloat32(22, true);
	const theta = view.getFloat32(26, true);
	// q15 dequant: integer / 32767, clamped to [0.0, 1.0].
	const confQ15 = view.getUint16(30, true);
	const confidence = Math.min(1.0, Math.max(0.0, confQ15 / 32767));

	pool.push(tsLo, tsHi, frameId, dx, dy, theta, confidence, featureCount, inlierCount, flags);
	return true;
}

// =============================================================================
// DEV-only layout self-tests. Validate byte-for-byte that this decoder agrees
// with the locked wire schemas. Resets all three pools after running so
// production decodes start from clean state.
// =============================================================================

function verify_v1_layout(): void {
	const testBuf = new ArrayBuffer(WIRE_HEADER_SIZE + WIRE_TRACK_SIZE);
	const tv = new DataView(testBuf);

	tv.setUint8(0, WIRE_PROTOCOL_VERSION_V1);
	tv.setUint8(1, 0x03);
	tv.setUint32(2, 4242, true);
	tv.setUint16(6, 1, true);
	tv.setInt32(8, -987, true);

	const base = WIRE_HEADER_SIZE;
	tv.setUint32(base + 0, 0xdeadbeef, true);
	tv.setUint32(base + 4, 0x00000001, true);
	tv.setUint32(base + 8, 0xcafef00d, true);
	tv.setUint32(base + 12, 0x00000000, true);
	tv.setUint8(base + 16, 2);
	tv.setUint8(base + 17, 0x0d);
	tv.setFloat32(base + 18, 0.875, true);
	tv.setFloat32(base + 22, 1.25, true);
	tv.setFloat32(base + 26, 2.5, true);
	tv.setFloat32(base + 30, 4.0, true);
	tv.setFloat32(base + 70, 500.0, true);
	tv.setFloat32(base + 74, 600.0, true);
	tv.setFloat32(base + 78, 50.0, true);
	tv.setFloat32(base + 82, 70.0, true);

	trackPool.reset();
	_view = null; // force rebind
	const ok = decode_frame(testBuf);
	if (
		!ok ||
		trackPool.frameSeq !== 4242 ||
		trackPool.frameFlags !== 0x03 ||
		trackPool.activeCount !== 1 ||
		trackPool.presenceBitmap[0] !== 1 ||
		trackPool.trackIdLo[0] !== 0xdeadbeef ||
		trackPool.trackIdHi[0] !== 0x00000001 ||
		trackPool.parentIdLo[0] !== 0xcafef00d ||
		trackPool.label[0] !== 2 ||
		trackPool.statusFlags[0] !== 0x0d ||
		Math.abs(trackPool.pWorld[0] - 1.25) > 1e-6 ||
		Math.abs(trackPool.pWorld[2] - 4.0) > 1e-6 ||
		Math.abs(trackPool.bboxOrig[0] - 500.0) > 1e-6 ||
		Math.abs(trackPool.bboxOrig[3] - 70.0) > 1e-6
	) {
		throw new Error(
			'[wire_decoder] verify_v1_layout FAILED — offset table out of sync with wire_format.hpp'
		);
	}
}

function verify_v2_layout(): void {
	const testBuf = new ArrayBuffer(WIRE_V2_FRAME_SIZE);
	const tv = new DataView(testBuf);

	tv.setUint8(0, WIRE_PROTOCOL_VERSION_V2);
	tv.setUint8(1, 0x11); // smoothed-only + config_change_marker
	tv.setUint32(2, 7777, true);
	tv.setUint32(6, 0x12345678, true);  // ts_ns lo
	tv.setUint32(10, 0x9abcdef0, true); // ts_ns hi
	tv.setFloat32(14, Number.NaN, true);
	tv.setFloat32(18, Number.NaN, true);
	tv.setFloat32(22, 960.5, true);
	tv.setFloat32(26, 540.25, true);
	tv.setUint16(30, 1 /* HidMouse */, true);
	tv.setFloat32(32, 0.35, true);

	actuatorPool.reset();
	_view = null;
	const ok = decode_frame(testBuf);
	const head = actuatorPool.head;
	const lastIdx = (head - 1 + 4096) % 4096;
	if (
		!ok ||
		actuatorPool.fill !== 1 ||
		actuatorPool.framesReceived !== 1 ||
		actuatorPool.lastFrameId !== 7777 ||
		actuatorPool.frameId[lastIdx] !== 7777 ||
		actuatorPool.tsLo[lastIdx] !== 0x12345678 ||
		actuatorPool.tsHi[lastIdx] !== 0x9abcdef0 ||
		!Number.isNaN(actuatorPool.rawU[lastIdx]) ||
		!Number.isNaN(actuatorPool.rawV[lastIdx]) ||
		Math.abs(actuatorPool.cleanU[lastIdx] - 960.5) > 1e-6 ||
		Math.abs(actuatorPool.cleanV[lastIdx] - 540.25) > 1e-6 ||
		actuatorPool.sinkKind[lastIdx] !== 1 ||
		Math.abs(actuatorPool.appliedAlpha[lastIdx] - 0.35) > 1e-6 ||
		actuatorPool.flags[lastIdx] !== 0x11
	) {
		throw new Error(
			'[wire_decoder] verify_v2_layout FAILED — v2 offset table out of sync with §12.4'
		);
	}
}

function verify_v3_layout(): void {
	const testBuf = new ArrayBuffer(WIRE_V3_FRAME_SIZE);
	const tv = new DataView(testBuf);

	tv.setUint8(0, WIRE_PROTOCOL_VERSION_V3);
	tv.setUint8(1, 0x0d); // valid + center_mask + scale_valid (bit 0+2+3)
	tv.setUint16(2, 200, true);
	tv.setUint16(4, 175, true);
	tv.setUint32(6, 3300, true);
	tv.setUint32(10, 0xfeedface, true);
	tv.setUint32(14, 0x01234567, true);
	tv.setFloat32(18, 2.5, true);
	tv.setFloat32(22, -1.25, true);
	tv.setFloat32(26, 0.005, true);
	tv.setUint16(30, 0x7fff, true); // q15 = ~1.0

	bgOdometryPool.reset();
	_view = null;
	const ok = decode_frame(testBuf);
	const head = bgOdometryPool.head;
	const lastIdx = (head - 1 + 1024) % 1024;
	if (
		!ok ||
		bgOdometryPool.fill !== 1 ||
		bgOdometryPool.framesReceived !== 1 ||
		bgOdometryPool.lastFrameId !== 3300 ||
		bgOdometryPool.frameId[lastIdx] !== 3300 ||
		bgOdometryPool.tsLo[lastIdx] !== 0xfeedface ||
		bgOdometryPool.tsHi[lastIdx] !== 0x01234567 ||
		bgOdometryPool.featureCount[lastIdx] !== 200 ||
		bgOdometryPool.inlierCount[lastIdx] !== 175 ||
		Math.abs(bgOdometryPool.dx[lastIdx] - 2.5) > 1e-6 ||
		Math.abs(bgOdometryPool.dy[lastIdx] - -1.25) > 1e-6 ||
		Math.abs(bgOdometryPool.theta[lastIdx] - 0.005) > 1e-6 ||
		bgOdometryPool.confidence[lastIdx] < 0.99 ||
		bgOdometryPool.flags[lastIdx] !== 0x0d
	) {
		throw new Error(
			'[wire_decoder] verify_v3_layout FAILED — v3 offset table out of sync with §13.4'
		);
	}
}

if (import.meta.env.DEV) {
	verify_v1_layout();
	verify_v2_layout();
	verify_v3_layout();
	// Reset all three pools and clear DataView cache so production decodes
	// start from clean module state.
	trackPool.reset();
	actuatorPool.reset();
	bgOdometryPool.reset();
	_view = null;
}
