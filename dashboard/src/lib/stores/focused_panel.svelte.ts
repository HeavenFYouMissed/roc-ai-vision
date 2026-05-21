// =============================================================================
// roc-ai-vision · dashboard · focused_panel.svelte.ts
//
// Tracks which Inspect panel the operator last clicked so the right rail
// can show panel-specific settings (Foxglove `,` cog convention).
//
// Panel IDs match those in panel_registry.ts.
// =============================================================================

import { browser } from '$app/environment';

export type InspectPanelId =
	| 'mouse-dynamics-inspector'
	| 'velocity-profile'
	| 'fitts-residual'
	| 'subpixel'
	| 'jerk-spectrum'
	| 'spectator-telemetry-matrix'
	| 'pipeline-graph'
	| 'model-inspector'
	| 'topic-graph'
	| 'hierarchy-explorer'
	| 'event-log';

const STORAGE_KEY = 'roc.focused_panel.id';

class FocusedPanelStore {
	focusedId = $state<InspectPanelId | null>(null);

	private hydrated = false;

	hydrate(): void {
		if (this.hydrated || !browser) return;
		try {
			const saved = localStorage.getItem(STORAGE_KEY);
			if (saved) this.focusedId = saved as InspectPanelId;
		} catch {
			// ignore
		}
		this.hydrated = true;
	}

	focus(id: InspectPanelId): void {
		this.focusedId = id;
		if (browser) {
			try {
				localStorage.setItem(STORAGE_KEY, id);
			} catch {
				// ignore
			}
		}
	}

	clear(): void {
		this.focusedId = null;
	}
}

export const focusedPanel = new FocusedPanelStore();
