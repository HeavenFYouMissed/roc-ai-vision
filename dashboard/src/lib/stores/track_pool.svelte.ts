// =============================================================================
// roc-ai-vision · dashboard · track_pool.svelte.ts
//
// Zero-GC, Structure-of-Arrays (SoA) track pool that mirrors the binary
// payload from `core/kinematics_engine/include/roc/kinematics/wire_format.hpp`
// (12 B header + 86 B/track, little-endian).
//
// All hot-path columns are flat typed arrays.  Reads happen INSIDE the
// TelemetryCanvas RAF tick — no Svelte reactivity touches the typed-array
// storage.  Reactivity is intentionally limited to four small $state runes
// that summarise the latest frame; non-canvas consumers (e.g. the Observer
// Matrix panel) subscribe to those without burning the canvas allocator
// budget.
//
// Capacity:  64 slots  (matches §7 traffic budget at the design ceiling)
// Trajectory ring: 16 entries / slot
// Total SoA backing-store: ~14 KB  (fits inside an L1d cache)
// =============================================================================

export const MAX_SLOTS = 64;
export const TRAJECTORY_RING = 16;

// `$state` runes used inside class fields are compiled by Svelte when the
// file carries the `.svelte.ts` extension.  No explicit `$state.raw` is
// needed for the typed arrays because we read them imperatively from the
// canvas RAF; Svelte never sees them as reactive dependencies.
class TrackPoolStore {
	// -------------------------------------------------------------------------
	// SoA columns mirroring the §7.2 wire payload.  The (track_id, parent_id)
	// u64 fields are split into hi/lo 32-bit halves to avoid BigInt allocation
	// on every decode — `Uint32Array` is plain ArrayBuffer-backed storage.
	// -------------------------------------------------------------------------
	readonly trackIdHi = new Uint32Array(MAX_SLOTS);
	readonly trackIdLo = new Uint32Array(MAX_SLOTS);
	readonly parentIdHi = new Uint32Array(MAX_SLOTS);
	readonly parentIdLo = new Uint32Array(MAX_SLOTS);

	readonly label = new Uint8Array(MAX_SLOTS);
	readonly statusFlags = new Uint8Array(MAX_SLOTS);
	readonly confidence = new Float32Array(MAX_SLOTS);

	readonly pWorld = new Float32Array(MAX_SLOTS * 3);
	readonly vWorld = new Float32Array(MAX_SLOTS * 3);
	readonly aWorld = new Float32Array(MAX_SLOTS * 3);
	readonly pPosDiag = new Float32Array(MAX_SLOTS * 3);
	readonly bboxOrig = new Float32Array(MAX_SLOTS * 4);

	// Client-side trajectory ring — populated by the decoder from each
	// frame's bbox centroid.  16 entries × 4 B × 2 = 128 B / slot.
	readonly trajU = new Float32Array(MAX_SLOTS * TRAJECTORY_RING);
	readonly trajV = new Float32Array(MAX_SLOTS * TRAJECTORY_RING);
	readonly trajHead = new Uint8Array(MAX_SLOTS); // next write index in the ring
	readonly trajFill = new Uint8Array(MAX_SLOTS); // valid entry count (saturates at RING)

	readonly lastUpdatedFrame = new Uint32Array(MAX_SLOTS);

	// Byte-per-slot occupancy flag (vs. a packed bitmap — the per-byte form
	// is cheaper in the canvas hot loop than the bit-test arithmetic and only
	// costs 56 bytes of memory).
	readonly presenceBitmap = new Uint8Array(MAX_SLOTS);

	// Pre-allocated string cache for track-id rendering.  Only mutated when a
	// slot is reassigned to a new track id (rare).  The TelemetryCanvas
	// reads `idStrings[slot]` per frame; no per-frame allocation.
	readonly idStrings: string[] = new Array<string>(MAX_SLOTS).fill('');

	// -------------------------------------------------------------------------
	// Small reactive surface for non-canvas consumers (Observer Matrix etc.).
	// `presenceVersion` bumps once per decoded frame and acts as the
	// "something happened" tick for runed subscribers that don't want to
	// poll typed arrays directly.
	// -------------------------------------------------------------------------
	presenceVersion = $state(0);
	frameSeq = $state(0);
	frameFlags = $state(0);
	activeCount = $state(0);

	// -------------------------------------------------------------------------
	// Slot management — all O(N) linear scans over MAX_SLOTS=64; trivial cost.
	// Maintaining a JS Map<id, slot> would be faster asymptotically but would
	// allocate on every insert/delete.
	// -------------------------------------------------------------------------
	findSlotByTrackId(lo: number, hi: number): number {
		for (let s = 0; s < MAX_SLOTS; s++) {
			if (
				this.presenceBitmap[s] === 1 &&
				this.trackIdLo[s] === lo &&
				this.trackIdHi[s] === hi
			) {
				return s;
			}
		}
		return -1;
	}

	getAvailableSlot(): number {
		for (let s = 0; s < MAX_SLOTS; s++) {
			if (this.presenceBitmap[s] === 0) return s;
		}
		return -1;
	}

	reclaimStaleSlots(currentFrameId: number, maxAgeFrames: number): void {
		for (let s = 0; s < MAX_SLOTS; s++) {
			if (this.presenceBitmap[s] !== 1) continue;
			// Unsigned wrap is handled correctly here because all values fit
			// in 32-bit non-negative ints and we never expect age > 2^31.
			if (currentFrameId - this.lastUpdatedFrame[s] > maxAgeFrames) {
				this.presenceBitmap[s] = 0;
				this.trackIdLo[s] = 0;
				this.trackIdHi[s] = 0;
				this.parentIdLo[s] = 0;
				this.parentIdHi[s] = 0;
				this.trajFill[s] = 0;
				this.trajHead[s] = 0;
				this.idStrings[s] = '';
			}
		}
	}

	// Full reset — used by tests and by the SvelteKit page lifecycle when the
	// dashboard tab is closed.  Does NOT reallocate the typed arrays.
	reset(): void {
		this.presenceBitmap.fill(0);
		this.trajFill.fill(0);
		this.trajHead.fill(0);
		this.lastUpdatedFrame.fill(0);
		this.trackIdLo.fill(0);
		this.trackIdHi.fill(0);
		this.parentIdLo.fill(0);
		this.parentIdHi.fill(0);
		for (let i = 0; i < MAX_SLOTS; i++) this.idStrings[i] = '';
		this.presenceVersion = 0;
		this.frameSeq = 0;
		this.frameFlags = 0;
		this.activeCount = 0;
	}
}

// Singleton instance — matches the `aimingConfig` / `hardwareConfig` pattern
// used elsewhere in the dashboard.
export const trackPool = new TrackPoolStore();
export type TrackPool = TrackPoolStore;
