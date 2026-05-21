// =============================================================================
// roc-ai-vision · dashboard · overlay_mode.svelte.ts
//
// Alt+Z Operator Overlay mode per dashboard_menu_architecture_2026.md §2.2.
// Collapses left rail to icons-only, hides right rail, shrinks top chrome
// to brand + badges, expands canvas to full viewport. Persists to localStorage.
// =============================================================================

import { browser } from '$app/environment';

const STORAGE_KEY = 'roc.overlay_mode.enabled';

class OverlayModeStore {
	enabled = $state(false);
	private hydrated = false;

	hydrate(): void {
		if (this.hydrated || !browser) return;
		try {
			const saved = localStorage.getItem(STORAGE_KEY);
			if (saved === 'true') this.enabled = true;
		} catch {
			// ignore
		}
		this.hydrated = true;
	}

	toggle(): void {
		this.enabled = !this.enabled;
		if (browser) {
			try {
				localStorage.setItem(STORAGE_KEY, String(this.enabled));
			} catch {
				// ignore
			}
		}
	}

	disable(): void {
		this.enabled = false;
		if (browser) {
			try {
				localStorage.setItem(STORAGE_KEY, 'false');
			} catch {
				// ignore
			}
		}
	}
}

export const overlayMode = new OverlayModeStore();
