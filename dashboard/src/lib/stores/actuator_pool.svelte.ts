// =============================================================================
// roc-ai-vision · dashboard · actuator_pool.svelte.ts
//
// Zero-GC global ring buffer for v2 actuator-telemetry frames (high-rate
// pointer-command stream from the C++ orchestrator's actuator-telemetry-emit
// thread). Mirrors the wire schema pinned in
// `dashboard_menu_architecture_2026.md` §11.1 (introduction) + §12.4 (final
// 36-byte layout):
//
//   offset  size   field
//        0     1   u8    version              (= 2 — dispatch byte, not stored)
//        1     1   u8    flags                (bit 0 = smoothed-only,
//                                              bit 1 = raw-only,
//                                              bit 2 = click_down,
//                                              bit 3 = click_up,
//                                              bit 4 = config_change_marker)
//        2     4   u32   frame_id             (monotonic; shared with v1)
//        6     8   u64   ts_ns                (capture timestamp, ns)
//       14     4   f32   raw_u_px             (NaN when bit 1 clear)
//       18     4   f32   raw_v_px
//       22     4   f32   clean_u_px           (NaN when bit 0 clear)
//       26     4   f32   clean_v_px
//       30     2   u16   sink_kind            (OutputSinkKind enum per v3 §11.4)
//       32     4   f32   applied_alpha        (slider feedback per v4 §12.3)
//       36     ─   end
//
// Capacity: 4096 samples (per v5 §13.8 update — was 256/track in v3 §11.1;
// corrected to GLOBAL non-per-track in v5). At 500 Hz that is ~8.2 s of
// history; at 60 Hz Phase 5b placeholder that is ~68 s.
//
// Memory:
//   ring* typed arrays: 4096 × (8 + 4 + 4 + 4 + 4 + 4 + 4 + 2 + 1) B = ~144 KB
//   (fits comfortably in L2 on every supported host).
//
// The ring is GLOBAL — not per-track. There is exactly one actuator at any
// given moment (one HID mouse, or one Dynamixel chain, or one logfile sink),
// so a single global ring captures the entire actuator history.
// =============================================================================

export const ACTUATOR_RING = 4096;

class ActuatorPoolStore {
	// -------------------------------------------------------------------------
	// SoA columns mirroring the §12.4 v2 wire payload.
	// `ts_ns` is split into hi/lo 32-bit halves to avoid BigInt allocation on
	// every decode (BigInt boxing would burn the entire GC budget at 500 Hz).
	// -------------------------------------------------------------------------
	readonly tsLo = new Uint32Array(ACTUATOR_RING);
	readonly tsHi = new Uint32Array(ACTUATOR_RING);
	readonly frameId = new Uint32Array(ACTUATOR_RING);
	readonly rawU = new Float32Array(ACTUATOR_RING);
	readonly rawV = new Float32Array(ACTUATOR_RING);
	readonly cleanU = new Float32Array(ACTUATOR_RING);
	readonly cleanV = new Float32Array(ACTUATOR_RING);
	readonly appliedAlpha = new Float32Array(ACTUATOR_RING);
	readonly sinkKind = new Uint16Array(ACTUATOR_RING);
	readonly flags = new Uint8Array(ACTUATOR_RING);

	// Ring-buffer cursors. `head` = next-write index; `fill` = valid-entry count
	// (saturates at ACTUATOR_RING once the ring wraps for the first time).
	head = 0;
	fill = 0;

	// -------------------------------------------------------------------------
	// Reactive surface for non-canvas consumers (MouseDynamicsInspector
	// control strip, SpectatorTelemetryMatrix KPI strip). The canvas RAF
	// deliberately reads typed arrays directly without subscribing to these.
	// -------------------------------------------------------------------------
	presenceVersion = $state(0);
	framesReceived = $state(0);
	lastFrameId = $state(0);
	lastTsLo = $state(0);
	lastTsHi = $state(0);
	lastSinkKind = $state(0);
	// Last `applied_alpha` value observed — drives the config-change marker
	// rendering in MouseDynamicsInspector per v4 §12.3.
	lastAppliedAlpha = $state(0);

	// -------------------------------------------------------------------------
	// Hot-path push — called by `decode_v2_actuator_telemetry()` exactly once
	// per inbound v2 frame. No allocations.
	// -------------------------------------------------------------------------
	push(
		tsLo: number,
		tsHi: number,
		frameId: number,
		rawU: number,
		rawV: number,
		cleanU: number,
		cleanV: number,
		appliedAlpha: number,
		sinkKind: number,
		flags: number
	): void {
		const i = this.head;
		this.tsLo[i] = tsLo >>> 0;
		this.tsHi[i] = tsHi >>> 0;
		this.frameId[i] = frameId >>> 0;
		this.rawU[i] = rawU;
		this.rawV[i] = rawV;
		this.cleanU[i] = cleanU;
		this.cleanV[i] = cleanV;
		this.appliedAlpha[i] = appliedAlpha;
		this.sinkKind[i] = sinkKind & 0xffff;
		this.flags[i] = flags & 0xff;

		this.head = (i + 1) % ACTUATOR_RING;
		if (this.fill < ACTUATOR_RING) this.fill++;

		this.framesReceived++;
		this.lastFrameId = frameId >>> 0;
		this.lastTsLo = tsLo >>> 0;
		this.lastTsHi = tsHi >>> 0;
		this.lastSinkKind = sinkKind & 0xffff;
		this.lastAppliedAlpha = appliedAlpha;
		// Bump the presence rune at most once per actuator frame so reactive
		// subscribers can drive re-renders without polling typed arrays. At
		// 500 Hz this is 500 rune writes/sec — well inside Svelte's budget.
		this.presenceVersion++;
	}

	// -------------------------------------------------------------------------
	// Helpers for ring traversal — used by MouseDynamicsInspector to walk the
	// last N samples in chronological order. Caller passes the slot index
	// returned by these helpers directly into the typed-array columns.
	// -------------------------------------------------------------------------

	// Oldest-valid-sample index. Returns -1 if the ring is empty.
	oldestIndex(): number {
		if (this.fill === 0) return -1;
		return (this.head - this.fill + ACTUATOR_RING) % ACTUATOR_RING;
	}

	// k-th-from-oldest index (0 = oldest). No bounds check on `k`; caller
	// guarantees `0 <= k < fill`.
	indexFromOldest(k: number): number {
		return (this.head - this.fill + k + ACTUATOR_RING) % ACTUATOR_RING;
	}

	// k-th-from-newest index (0 = newest). No bounds check on `k`; caller
	// guarantees `0 <= k < fill`.
	indexFromNewest(k: number): number {
		return (this.head - 1 - k + ACTUATOR_RING) % ACTUATOR_RING;
	}

	// -------------------------------------------------------------------------
	// Full reset — used by tests and by the SvelteKit lifecycle on Scenario
	// switch. Does NOT reallocate the typed arrays.
	// -------------------------------------------------------------------------
	reset(): void {
		this.head = 0;
		this.fill = 0;
		this.tsLo.fill(0);
		this.tsHi.fill(0);
		this.frameId.fill(0);
		this.rawU.fill(0);
		this.rawV.fill(0);
		this.cleanU.fill(0);
		this.cleanV.fill(0);
		this.appliedAlpha.fill(0);
		this.sinkKind.fill(0);
		this.flags.fill(0);
		this.presenceVersion = 0;
		this.framesReceived = 0;
		this.lastFrameId = 0;
		this.lastTsLo = 0;
		this.lastTsHi = 0;
		this.lastSinkKind = 0;
		this.lastAppliedAlpha = 0;
	}
}

// Singleton instance — matches the `trackPool` / `aimingConfig` /
// `hardwareConfig` pattern used elsewhere in the dashboard.
export const actuatorPool = new ActuatorPoolStore();
export type ActuatorPool = ActuatorPoolStore;
