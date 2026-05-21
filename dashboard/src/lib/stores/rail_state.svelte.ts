// =============================================================================
// roc-ai-vision · dashboard · rail_state.svelte.ts
//
// Left + right rail collapse state. Hotkeys [ and ] toggle them.
// Persists to localStorage so the operator's layout survives reloads.
// =============================================================================

import { browser } from '$app/environment';

const KEY = 'roc.rail_state.v1';

interface PersistShape {
	leftCollapsed: boolean;
	rightCollapsed: boolean;
}

const DEFAULTS: PersistShape = { leftCollapsed: false, rightCollapsed: false };

class RailStateStore {
	leftCollapsed = $state(false);
	rightCollapsed = $state(false);
	private hydrated = false;

	hydrate(): void {
		if (this.hydrated || !browser) return;
		try {
			const raw = localStorage.getItem(KEY);
			if (raw) {
				const parsed = JSON.parse(raw) as Partial<PersistShape>;
				this.leftCollapsed = parsed.leftCollapsed ?? DEFAULTS.leftCollapsed;
				this.rightCollapsed = parsed.rightCollapsed ?? DEFAULTS.rightCollapsed;
			}
		} catch {
			// ignore
		}
		this.hydrated = true;
	}

	private persist(): void {
		if (!browser) return;
		try {
			localStorage.setItem(
				KEY,
				JSON.stringify({
					leftCollapsed: this.leftCollapsed,
					rightCollapsed: this.rightCollapsed
				})
			);
		} catch {
			// ignore
		}
	}

	toggleLeft(): void {
		this.leftCollapsed = !this.leftCollapsed;
		this.persist();
	}

	toggleRight(): void {
		this.rightCollapsed = !this.rightCollapsed;
		this.persist();
	}

	expandRight(): void {
		this.rightCollapsed = false;
		this.persist();
	}
}

export const railState = new RailStateStore();
