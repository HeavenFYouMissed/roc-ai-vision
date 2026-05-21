// =============================================================================
// roc-ai-vision · dashboard · selection_store.svelte.ts
//
// Shared selection / hover / lock state between:
//   · OperateCanvas (highlights selected slot, hover bbox glow, target-lock crosshair)
//   · RightRail Detection JSON Feed (selected card)
//   · RightRail Selected Track Inspector (fills with selected track data)
//   · HierarchyExplorerDrawer (selected row sync)
//   · EngageDestination (active lock list)
//
// `selectedSlot` and `hoveredSlot` are slot indices into trackPool (0..63);
// -1 means "no selection". Locks store the full track identity (id hi/lo)
// so the lock survives the slot getting reassigned to a different track.
// =============================================================================

import { trackPool, MAX_SLOTS } from '$lib/stores/track_pool.svelte';

export interface ActiveLock {
	trackIdLo: number;
	trackIdHi: number;
	label: number;
	lockedAtMs: number;
	idString: string;
}

class SelectionStore {
	selectedSlot = $state<number>(-1);
	hoveredSlot = $state<number>(-1);
	locks = $state<ActiveLock[]>([]);
	clickToLockMode = $state<boolean>(false);

	select(slot: number): void {
		if (slot < -1 || slot >= MAX_SLOTS) return;
		if (slot !== -1 && trackPool.presenceBitmap[slot] !== 1) return;
		this.selectedSlot = slot;
	}

	deselect(): void {
		this.selectedSlot = -1;
	}

	setHover(slot: number): void {
		if (slot === this.hoveredSlot) return;
		this.hoveredSlot = slot;
	}

	clearHover(): void {
		this.hoveredSlot = -1;
	}

	cycleNext(): void {
		const start = this.selectedSlot >= 0 ? this.selectedSlot + 1 : 0;
		for (let i = 0; i < MAX_SLOTS; i++) {
			const s = (start + i) % MAX_SLOTS;
			if (trackPool.presenceBitmap[s] === 1) {
				this.selectedSlot = s;
				return;
			}
		}
		this.selectedSlot = -1;
	}

	cyclePrev(): void {
		const start = this.selectedSlot >= 0 ? this.selectedSlot - 1 : MAX_SLOTS - 1;
		for (let i = 0; i < MAX_SLOTS; i++) {
			const s = (start - i + MAX_SLOTS) % MAX_SLOTS;
			if (trackPool.presenceBitmap[s] === 1) {
				this.selectedSlot = s;
				return;
			}
		}
		this.selectedSlot = -1;
	}

	// Find the slot currently displaying the locked track id (or -1 if it's
	// been reassigned / dropped). Used by canvas to draw the lock crosshair.
	slotForLock(lock: ActiveLock): number {
		return trackPool.findSlotByTrackId(lock.trackIdLo, lock.trackIdHi);
	}

	addLock(slot: number): boolean {
		if (slot < 0 || slot >= MAX_SLOTS) return false;
		if (trackPool.presenceBitmap[slot] !== 1) return false;
		const lo = trackPool.trackIdLo[slot];
		const hi = trackPool.trackIdHi[slot];
		// dedup
		if (this.locks.some((l) => l.trackIdLo === lo && l.trackIdHi === hi)) return false;
		this.locks = [
			...this.locks,
			{
				trackIdLo: lo,
				trackIdHi: hi,
				label: trackPool.label[slot],
				lockedAtMs: performance.now(),
				idString: trackPool.idStrings[slot] || `0x${lo.toString(16)}`
			}
		];
		return true;
	}

	removeLock(idx: number): void {
		if (idx < 0 || idx >= this.locks.length) return;
		this.locks = this.locks.filter((_, i) => i !== idx);
	}

	clearAllLocks(): void {
		this.locks = [];
	}

	enableClickToLock(): void {
		this.clickToLockMode = true;
	}

	disableClickToLock(): void {
		this.clickToLockMode = false;
	}
}

export const selectionStore = new SelectionStore();
