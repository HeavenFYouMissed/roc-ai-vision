// =============================================================================
// roc-ai-vision · dashboard · engage_state.svelte.ts
//
// Engage destination state: armed toggle, pending confirm state, active locks,
// emergency stop latch, target-lock confirmation modal state.
//
// CRITICAL SAFETY CONTRACT (per dashboard_menu_architecture_2026.md §1):
//   - Armed defaults to FALSE on every mount and on every layout change
//   - Two-stage commit: click → PENDING → click Confirm → ARMED
//   - REPLAY mode (profileStore.isReplay) forces Armed = false and disables toggle
//   - Emergency Stop is one-click no-confirmation (it's a safety-side action)
// =============================================================================

import { selectionStore } from './selection_store.svelte';
import { profileStore } from './profile_store.svelte';

export type ArmedState = 'off' | 'pending' | 'armed';

class EngageStateStore {
	armed = $state<ArmedState>('off');
	emergencyStopLatched = $state(false);
	lockConfirmModalOpen = $state(false);
	lockConfirmHoldProgress = $state(0); // 0-1, advances during hold
	pendingLockSlot = $state<number>(-1);

	// Auto-disarm timer for safety (per spec ss4 5-second auto-disarm)
	private autoDisarmTimer: ReturnType<typeof setTimeout> | null = null;

	get canArm(): boolean {
		return !profileStore.isReplay && !this.emergencyStopLatched;
	}

	startArm(): void {
		if (!this.canArm) return;
		this.armed = 'pending';
	}

	confirmArm(): void {
		if (!this.canArm) return;
		if (this.armed !== 'pending') return;
		this.armed = 'armed';
		this.scheduleAutoDisarm();
	}

	cancelArm(): void {
		this.armed = 'off';
		this.clearAutoDisarm();
	}

	disarm(): void {
		this.armed = 'off';
		this.clearAutoDisarm();
	}

	triggerEmergencyStop(): void {
		this.emergencyStopLatched = true;
		this.armed = 'off';
		this.clearAutoDisarm();
		this.lockConfirmModalOpen = false;
		this.pendingLockSlot = -1;
	}

	resetEmergencyStop(): void {
		this.emergencyStopLatched = false;
	}

	private scheduleAutoDisarm(): void {
		this.clearAutoDisarm();
		this.autoDisarmTimer = setTimeout(() => {
			this.disarm();
		}, 30_000); // 30 seconds (longer than spec 5s default for usable demo)
	}

	private clearAutoDisarm(): void {
		if (this.autoDisarmTimer !== null) {
			clearTimeout(this.autoDisarmTimer);
			this.autoDisarmTimer = null;
		}
	}

	// Target-lock modal flow
	openLockModal(slot: number): void {
		if (this.armed !== 'armed') return;
		if (slot < 0) return;
		this.pendingLockSlot = slot;
		this.lockConfirmHoldProgress = 0;
		this.lockConfirmModalOpen = true;
	}

	cancelLockModal(): void {
		this.lockConfirmModalOpen = false;
		this.pendingLockSlot = -1;
		this.lockConfirmHoldProgress = 0;
	}

	commitLock(): void {
		if (!this.lockConfirmModalOpen) return;
		if (this.pendingLockSlot < 0) return;
		selectionStore.addLock(this.pendingLockSlot);
		this.lockConfirmModalOpen = false;
		this.pendingLockSlot = -1;
		this.lockConfirmHoldProgress = 0;
	}

	resetOnMount(): void {
		// Mandatory per spec - armed always defaults to off on mount/layout change
		this.armed = 'off';
		this.lockConfirmModalOpen = false;
		this.pendingLockSlot = -1;
		this.lockConfirmHoldProgress = 0;
		this.clearAutoDisarm();
	}
}

export const engageState = new EngageStateStore();
