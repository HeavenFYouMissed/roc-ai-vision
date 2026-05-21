/**
 * roc-ai-vision · dashboard · biometrics/metrics.ts
 *
 * Pure compute functions for the Biometrics destination (UI_SPEC_SHEET §16.4).
 * Each function reads from a CaptureSlot ring buffer and either returns
 * scalar stats or writes a time series into a caller-supplied scratch
 * Float32Array. No allocations in steady state.
 *
 * The four metrics correspond directly to the reference image's four
 * panels:
 *   - sessionStats        -> Session text panel
 *   - reportRateSeries    -> Report rate (Hz) sparkline
 *   - velocitySeries      -> Velocity (counts/sec) sparkline
 *   - signFlipRateSeries  -> Sign flip rate (flips/sec) sparkline (RED border)
 *
 * The sign-flip rate is the AC-critical metric. Humans produce ~30 flips/sec
 * during aim. Bots / aimbots produce 100-200+ because they over-correct each
 * tick. See dashboard_menu_architecture_2026.md §14.4 and the empirical
 * evidence in the reference image (current row 31 flips/sec vs previous row
 * 197 flips/sec).
 */

import type { CaptureSlot } from './mouse_capture.svelte';
import { RING_CAPACITY } from './mouse_capture.svelte';

// =============================================================================
// SessionStats
// =============================================================================

export interface SessionStats {
	durationS: number;
	pathPoints: number;
	reports: number;
	dirChanges: number;
	signFlipsX: number;
	signFlipsY: number;
	flipsPerSec: number;
}

const EMPTY_SESSION_STATS: SessionStats = {
	durationS: 0,
	pathPoints: 0,
	reports: 0,
	dirChanges: 0,
	signFlipsX: 0,
	signFlipsY: 0,
	flipsPerSec: 0
};

/**
 * Compute one-shot session statistics for a slot.
 *
 * @param slot               CaptureSlot to analyse.
 * @param angleThreshRad     Angle threshold (radians) for "direction change"
 *                           detection. The reference UI exposes a slider
 *                           in degrees; convert before calling.
 * @returns                  Stats object (NEW allocation — callers should
 *                           throttle calls, not invoke per-frame at 60Hz).
 */
export function sessionStats(slot: CaptureSlot, angleThreshRad: number): SessionStats {
	if (slot.fill < 2) {
		return { ...EMPTY_SESSION_STATS, pathPoints: slot.fill, reports: slot.fill };
	}

	const dx = slot.dx;
	const dy = slot.dy;
	const N = slot.fill;
	const cap = RING_CAPACITY;
	const start = (slot.head - N + cap) % cap;

	let dirChanges = 0;
	let signFlipsX = 0;
	let signFlipsY = 0;
	let prevDx = dx[start];
	let prevDy = dy[start];

	for (let k = 1; k < N; k++) {
		const idx = (start + k) % cap;
		const cdx = dx[idx];
		const cdy = dy[idx];

		// Direction change (angle between consecutive velocity vectors).
		const a2 = prevDx * prevDx + prevDy * prevDy;
		const b2 = cdx * cdx + cdy * cdy;
		if (a2 > 0 && b2 > 0) {
			// cos(theta) = (a · b) / (|a||b|). Clamp for fp safety.
			const dot = prevDx * cdx + prevDy * cdy;
			const denom = Math.sqrt(a2 * b2);
			let cosT = dot / denom;
			if (cosT < -1) cosT = -1;
			else if (cosT > 1) cosT = 1;
			const theta = Math.acos(cosT);
			if (theta > angleThreshRad) dirChanges++;
		}

		// Per-axis sign flips. Sign-flip iff both non-zero AND signs differ.
		// We use Math.sign rather than bitwise XOR because XOR coerces
		// Float32 → Int32 (truncating toward 0); fractional sub-pixel deltas
		// from high-DPI / free-scrolling mice would silently truncate to 0
		// and be missed even though `prevDx !== 0 && cdx !== 0` passes on the
		// original floats.
		if (prevDx !== 0 && cdx !== 0 && Math.sign(prevDx) !== Math.sign(cdx)) signFlipsX++;
		if (prevDy !== 0 && cdy !== 0 && Math.sign(prevDy) !== Math.sign(cdy)) signFlipsY++;

		prevDx = cdx;
		prevDy = cdy;
	}

	const durationS = slot.durationMs / 1000;
	const flipsPerSec = durationS > 0 ? (signFlipsX + signFlipsY) / durationS : 0;

	return {
		durationS,
		pathPoints: N,
		reports: N,
		dirChanges,
		signFlipsX,
		signFlipsY,
		flipsPerSec
	};
}

// =============================================================================
// Time-series sparklines (zero-alloc; caller owns the output Float32Array)
// =============================================================================

export interface SeriesStats {
	count: number;     // number of valid samples written to outF32 (0..outF32.length)
	avg: number;
	min: number;
	max: number;
	last: number;
}

const EMPTY_SERIES_STATS: SeriesStats = { count: 0, avg: 0, min: 0, max: 0, last: 0 };

/**
 * Per-bucket report rate (Hz): for each bucket of `windowSec / outF32.length`
 * width across the slot's session duration, count samples and divide by
 * bucket width. Writes one value per bucket into `outF32`.
 *
 * The series is computed left-to-right over [t_start, t_start + windowSec)
 * relative to the most-recent sample, so the right edge of the chart is
 * "now" and the left edge is `windowSec` ago.
 */
export function reportRateSeries(
	slot: CaptureSlot,
	windowSec: number,
	outF32: Float32Array
): SeriesStats {
	const buckets = outF32.length;
	if (buckets === 0 || slot.fill < 2) {
		outF32.fill(0);
		return { ...EMPTY_SERIES_STATS };
	}

	const t = slot.t;
	const cap = RING_CAPACITY;
	const N = slot.fill;
	const start = (slot.head - N + cap) % cap;

	// Right-anchor the window on the most-recent sample's t.
	const tEnd = t[(start + N - 1) % cap];
	const tStart = tEnd - windowSec * 1000; // ms
	const bucketMs = (windowSec * 1000) / buckets;

	outF32.fill(0);
	for (let k = 0; k < N; k++) {
		const tk = t[(start + k) % cap];
		if (tk < tStart || tk >= tEnd) continue;
		const b = Math.floor((tk - tStart) / bucketMs);
		if (b >= 0 && b < buckets) outF32[b]++;
	}
	const perSec = 1000 / bucketMs;
	let sum = 0, min = Infinity, max = 0;
	for (let i = 0; i < buckets; i++) {
		outF32[i] *= perSec; // -> Hz
		const v = outF32[i];
		sum += v;
		if (v < min) min = v;
		if (v > max) max = v;
	}
	const stats: SeriesStats = {
		count: buckets,
		avg: sum / buckets,
		min: min === Infinity ? 0 : min,
		max,
		last: outF32[buckets - 1]
	};
	return stats;
}

/**
 * Per-bucket EWMA-smoothed velocity (mouse counts per second). For each
 * bucket, take the mean instantaneous velocity (= |delta| / dt) of samples
 * landing in that bucket, then EWMA-smooth across buckets.
 *
 * Reference UI's "Hz smoothing (EWMA alpha)" slider feeds `ewmaAlpha`.
 * `alpha=1` -> no smoothing. `alpha=0.0` -> heavy smoothing (output ~= 0).
 */
export function velocitySeries(
	slot: CaptureSlot,
	windowSec: number,
	ewmaAlpha: number,
	outF32: Float32Array
): SeriesStats {
	const buckets = outF32.length;
	if (buckets === 0 || slot.fill < 2) {
		outF32.fill(0);
		return { ...EMPTY_SERIES_STATS };
	}

	const dx = slot.dx;
	const dy = slot.dy;
	const t = slot.t;
	const cap = RING_CAPACITY;
	const N = slot.fill;
	const start = (slot.head - N + cap) % cap;

	const tEnd = t[(start + N - 1) % cap];
	const tStart = tEnd - windowSec * 1000;
	const bucketMs = (windowSec * 1000) / buckets;

	// Two scratch loops: accumulate sum + count per bucket, then divide.
	outF32.fill(0);
	// Reuse the upper half of outF32 to store counts? No — outF32 is the
	// caller's display buffer. We allocate a tiny scratch once per call.
	// (The caller can amortise this by passing a single ScratchPair; for v1
	// keep the API simple.)
	const counts = new Uint16Array(buckets); // small alloc; <= 4096 buckets typical
	let prevT = t[start];
	for (let k = 1; k < N; k++) {
		const idx = (start + k) % cap;
		const tk = t[idx];
		const dt = tk - prevT;
		prevT = tk;
		if (tk < tStart || tk >= tEnd) continue;
		const b = Math.floor((tk - tStart) / bucketMs);
		if (b < 0 || b >= buckets) continue;
		if (dt > 0) {
			const vi = Math.sqrt(dx[idx] * dx[idx] + dy[idx] * dy[idx]) / (dt / 1000);
			outF32[b] += vi;
			counts[b]++;
		}
	}

	// Mean per bucket
	for (let i = 0; i < buckets; i++) {
		if (counts[i] > 0) outF32[i] /= counts[i];
	}

	// EWMA pass (forward)
	if (ewmaAlpha > 0 && ewmaAlpha < 1) {
		let acc = outF32[0];
		for (let i = 1; i < buckets; i++) {
			if (outF32[i] > 0) {
				acc = ewmaAlpha * outF32[i] + (1 - ewmaAlpha) * acc;
				outF32[i] = acc;
			} else {
				acc = (1 - ewmaAlpha) * acc;
				outF32[i] = acc;
			}
		}
	}

	let sum = 0, max = 0;
	for (let i = 0; i < buckets; i++) {
		const v = outF32[i];
		sum += v;
		if (v > max) max = v;
	}
	return {
		count: buckets,
		avg: sum / buckets,
		min: 0,
		max,
		last: outF32[buckets - 1]
	};
}

/**
 * Per-bucket sliding-window sign flip rate (flips per second).
 *
 * THE AC-critical metric (UI_SPEC_SHEET §16.3 red-border panel).
 *
 * Algorithm (two-pass over `outF32`):
 *   1. Count sign reversals per bucket (X axis + Y axis independently) into
 *      a tiny prefix-sum scratch.
 *   2. Convolve with a rolling window of width `WINDOW_FRAC * buckets` and
 *      divide by the window's duration in seconds to emit flips/sec at each
 *      output bucket.
 *
 * Why a sliding window instead of raw per-bucket counts: with `windowSec=3`
 * and `buckets=240` we get 12.5 ms per bucket. A perfectly normal human
 * tremor rate of ~30 flips/sec yields ≤1 flip per bucket on average, so a
 * raw per-bucket sparkline is a sparse 0/spike binary plot — visually
 * useless. A sliding window of ~25% of the plot width turns the noisy
 * counts into a smooth curve whose *shape* is the discriminator: humans
 * plateau around 30 flips/sec, bots saturate at 150–200 (see
 * dashboard_menu_architecture_2026.md §14.4 empirical evidence).
 *
 * Sign-flip detection uses `Math.sign` rather than bitwise XOR because XOR
 * coerces Float32 → Int32 (truncating toward zero); fractional sub-pixel
 * mouse deltas would otherwise be silently missed.
 */
const SIGN_FLIP_WINDOW_FRAC = 0.25; // 25% of plot window = smoothing kernel width

export function signFlipRateSeries(
	slot: CaptureSlot,
	windowSec: number,
	outF32: Float32Array
): SeriesStats {
	const buckets = outF32.length;
	if (buckets === 0 || slot.fill < 2) {
		outF32.fill(0);
		return { ...EMPTY_SERIES_STATS };
	}

	const dx = slot.dx;
	const dy = slot.dy;
	const t = slot.t;
	const cap = RING_CAPACITY;
	const N = slot.fill;
	const start = (slot.head - N + cap) % cap;

	const tEnd = t[(start + N - 1) % cap];
	const tStart = tEnd - windowSec * 1000;
	const bucketMs = (windowSec * 1000) / buckets;

	// Pass 1: per-bucket raw flip counts into outF32.
	outF32.fill(0);
	let prevDx = dx[start];
	let prevDy = dy[start];
	for (let k = 1; k < N; k++) {
		const idx = (start + k) % cap;
		const cdx = dx[idx];
		const cdy = dy[idx];
		const tk = t[idx];
		if (tk >= tStart && tk < tEnd) {
			const b = Math.floor((tk - tStart) / bucketMs);
			if (b >= 0 && b < buckets) {
				if (prevDx !== 0 && cdx !== 0 && Math.sign(prevDx) !== Math.sign(cdx)) outF32[b]++;
				if (prevDy !== 0 && cdy !== 0 && Math.sign(prevDy) !== Math.sign(cdy)) outF32[b]++;
			}
		}
		prevDx = cdx;
		prevDy = cdy;
	}

	// Pass 2: sliding-window sum. We need original counts while overwriting,
	// so copy to a scratch first. Float32Array(buckets) is a single small
	// allocation per call (~1 KB for buckets=240); acceptable at the chart's
	// 10–30 Hz update rate.
	const raw = new Float32Array(outF32);
	const winBuckets = Math.max(1, Math.floor(buckets * SIGN_FLIP_WINDOW_FRAC));
	const winSec = (winBuckets * bucketMs) / 1000;

	let runSum = 0;
	let leftEdge = 0;
	for (let i = 0; i < buckets; i++) {
		runSum += raw[i];
		while (leftEdge < i - winBuckets + 1) {
			runSum -= raw[leftEdge];
			leftEdge++;
		}
		outF32[i] = runSum / winSec; // flips/sec averaged over the rolling window
	}

	let sum = 0, min = Infinity, max = 0;
	for (let i = 0; i < buckets; i++) {
		const v = outF32[i];
		sum += v;
		if (v < min) min = v;
		if (v > max) max = v;
	}
	return {
		count: buckets,
		avg: sum / buckets,
		min: min === Infinity ? 0 : min,
		max,
		last: outF32[buckets - 1]
	};
}

// =============================================================================
// Helpers
// =============================================================================

export function degToRad(deg: number): number {
	return (deg * Math.PI) / 180;
}
