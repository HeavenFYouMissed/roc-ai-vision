// =============================================================================
// roc-ai-vision · dashboard · bg_odometry_pool.svelte.ts
//
// Zero-GC global ring buffer for v3 bg-odometry frames (background optical-
// flow odometer estimates from the C++ OpticalFlowSampler thread). Mirrors the
// wire schema pinned in `dashboard_menu_architecture_2026.md` §13.4:
//
//   offset  size   field
//        0     1   u8    version              (= 3 — dispatch byte, not stored)
//        1     1   u8    flags                (bit 0 = valid_estimate;
//                                              bit 1 = degraded_low_features;
//                                              bit 2 = center_mask_active;
//                                              bit 3 = scale_estimate_valid;
//                                              bits 4–7 reserved)
//        2     2   u16   feature_count        (LK features tracked successfully)
//        4     2   u16   inlier_count         (RANSAC inliers in the rigid solve)
//        6     4   u32   frame_id             (matches the camera frame_id)
//       10     8   u64   ts_ns                (capture timestamp, ns)
//       18     4   f32   bg_dx_px             (background translation x)
//       22     4   f32   bg_dy_px             (background translation y)
//       26     4   f32   bg_theta_rad         (background rotation, rigid 3-DOF)
//       30     2   u16   confidence_q15       (q15 fixed-point [0.0, 1.0])
//       32     ─   end
//
// Capacity: 1024 samples per v5 §13.8 — at 30 Hz that is ~34 s of history.
// Cross-correlation against the actuator stream operates on an 8-sec window
// (240 samples), so 1024 gives ~4× margin for window slides + history
// inspection.
//
// Memory:
//   ring* typed arrays: 1024 × (8 + 4 + 4 + 4 + 4 + 2 + 2 + 2 + 1) B = ~32 KB
//   (fits inside L1d on every supported host).
//
// Like `actuatorPool`, the ring is GLOBAL — there is exactly one optical-flow
// estimate per camera frame (the WHOLE scene moved by Δ).
// =============================================================================

export const BG_ODOMETRY_RING = 1024;

class BgOdometryPoolStore {
	// -------------------------------------------------------------------------
	// SoA columns mirroring the §13.4 v3 wire payload.
	// -------------------------------------------------------------------------
	readonly tsLo = new Uint32Array(BG_ODOMETRY_RING);
	readonly tsHi = new Uint32Array(BG_ODOMETRY_RING);
	readonly frameId = new Uint32Array(BG_ODOMETRY_RING);
	readonly dx = new Float32Array(BG_ODOMETRY_RING);
	readonly dy = new Float32Array(BG_ODOMETRY_RING);
	readonly theta = new Float32Array(BG_ODOMETRY_RING);
	// Confidence is dequantised from q15 at decode-time → stored as plain f32
	// [0.0, 1.0] for cheap consumption in correlation maths.
	readonly confidence = new Float32Array(BG_ODOMETRY_RING);
	readonly featureCount = new Uint16Array(BG_ODOMETRY_RING);
	readonly inlierCount = new Uint16Array(BG_ODOMETRY_RING);
	readonly flags = new Uint8Array(BG_ODOMETRY_RING);

	head = 0;
	fill = 0;

	// -------------------------------------------------------------------------
	// Reactive surface — drives SpectatorTelemetryMatrix re-renders.
	// -------------------------------------------------------------------------
	presenceVersion = $state(0);
	framesReceived = $state(0);
	lastFrameId = $state(0);
	lastTsLo = $state(0);
	lastTsHi = $state(0);
	// Most-recent valid-estimate flag — drives the "degraded" badge in the
	// SpectatorTelemetryMatrix KPI strip per v5 §13.9 C1.
	lastValidEstimate = $state(false);
	lastFeatureCount = $state(0);
	lastInlierCount = $state(0);
	lastConfidence = $state(0);

	// -------------------------------------------------------------------------
	// Hot-path push — called by `decode_v3_bg_odometry()` once per inbound v3
	// frame. No allocations.
	// -------------------------------------------------------------------------
	push(
		tsLo: number,
		tsHi: number,
		frameId: number,
		dx: number,
		dy: number,
		theta: number,
		confidence: number,
		featureCount: number,
		inlierCount: number,
		flags: number
	): void {
		const i = this.head;
		this.tsLo[i] = tsLo >>> 0;
		this.tsHi[i] = tsHi >>> 0;
		this.frameId[i] = frameId >>> 0;
		this.dx[i] = dx;
		this.dy[i] = dy;
		this.theta[i] = theta;
		this.confidence[i] = confidence;
		this.featureCount[i] = featureCount & 0xffff;
		this.inlierCount[i] = inlierCount & 0xffff;
		this.flags[i] = flags & 0xff;

		this.head = (i + 1) % BG_ODOMETRY_RING;
		if (this.fill < BG_ODOMETRY_RING) this.fill++;

		this.framesReceived++;
		this.lastFrameId = frameId >>> 0;
		this.lastTsLo = tsLo >>> 0;
		this.lastTsHi = tsHi >>> 0;
		this.lastValidEstimate = (flags & 0x01) !== 0;
		this.lastFeatureCount = featureCount & 0xffff;
		this.lastInlierCount = inlierCount & 0xffff;
		this.lastConfidence = confidence;
		this.presenceVersion++;
	}

	// Helpers (mirror actuator_pool.svelte.ts API).
	oldestIndex(): number {
		if (this.fill === 0) return -1;
		return (this.head - this.fill + BG_ODOMETRY_RING) % BG_ODOMETRY_RING;
	}

	indexFromOldest(k: number): number {
		return (this.head - this.fill + k + BG_ODOMETRY_RING) % BG_ODOMETRY_RING;
	}

	indexFromNewest(k: number): number {
		return (this.head - 1 - k + BG_ODOMETRY_RING) % BG_ODOMETRY_RING;
	}

	reset(): void {
		this.head = 0;
		this.fill = 0;
		this.tsLo.fill(0);
		this.tsHi.fill(0);
		this.frameId.fill(0);
		this.dx.fill(0);
		this.dy.fill(0);
		this.theta.fill(0);
		this.confidence.fill(0);
		this.featureCount.fill(0);
		this.inlierCount.fill(0);
		this.flags.fill(0);
		this.presenceVersion = 0;
		this.framesReceived = 0;
		this.lastFrameId = 0;
		this.lastTsLo = 0;
		this.lastTsHi = 0;
		this.lastValidEstimate = false;
		this.lastFeatureCount = 0;
		this.lastInlierCount = 0;
		this.lastConfidence = 0;
	}
}

export const bgOdometryPool = new BgOdometryPoolStore();
export type BgOdometryPool = BgOdometryPoolStore;
