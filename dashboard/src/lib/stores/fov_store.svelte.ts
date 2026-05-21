// =============================================================================
// roc-ai-vision · dashboard · fov_store.svelte.ts
//
// Reactive client for the orchestrator's Phase 6 FOV crop API:
//   GET  /api/fov  -> FovSettings
//   POST /api/fov  -> FovSettings (echoed after merge)
//
// The FOV crop is the operator-driven inference window inside the source
// frame. When enabled, the InferenceEngine sees only a square crop centred
// on (cxNorm * W, cyNorm * H) with half-extent radius_px. Detections are
// translated back to source-frame coordinates by the C++ side, so the
// dashboard's overlay math is unaffected.
//
// Why two cx/cy plus crop xywh? cx/cy + radius are the OPERATOR-EDITABLE
// fields (what the slider/drag publishes). active_crop_* are READ-ONLY
// diagnostics published by the inference thread each frame, used by the
// dashboard's overlay ring to draw EXACTLY the rect the model saw. This
// avoids subtle off-by-one disagreements between "what was requested" and
// "what was clamped to source bounds".
// =============================================================================

import { browser } from '$app/environment';

export interface FovSettings {
	enabled: boolean;
	cxNorm: number;          // [0, 1] horizontal centre of crop, fraction of source W
	cyNorm: number;          // [0, 1] vertical   centre of crop, fraction of source H
	radiusPx: number;        // half-extent of the SQUARE crop in source pixels
	followTarget: boolean;   // Phase 7 — when true, server overrides cx/cy from EKF
	// Diagnostics — written by the inference thread, read-only here.
	sourceW: number;
	sourceH: number;
	activeCropX: number;
	activeCropY: number;
	activeCropW: number;
	activeCropH: number;
}

const FOV_ENDPOINT     = 'http://127.0.0.1:8766/api/fov';
const POLL_INTERVAL_MS = 500;     // re-sync diagnostics every 500 ms while drawer open
const FETCH_TIMEOUT_MS = 2000;

async function fetchJson<T>(url: string, init: RequestInit = {}, timeout = FETCH_TIMEOUT_MS): Promise<T> {
	const ctrl = new AbortController();
	const tid  = setTimeout(() => ctrl.abort(), timeout);
	try {
		const res = await fetch(url, { ...init, signal: ctrl.signal });
		if (!res.ok) throw new Error(`HTTP ${res.status} ${res.statusText}`);
		return (await res.json()) as T;
	} finally {
		clearTimeout(tid);
	}
}

const DEFAULTS: FovSettings = {
	enabled: false,
	cxNorm: 0.5,
	cyNorm: 0.5,
	radiusPx: 360,
	followTarget: false,
	sourceW: 0,
	sourceH: 0,
	activeCropX: 0,
	activeCropY: 0,
	activeCropW: 0,
	activeCropH: 0
};

class FovStore {
	settings = $state<FovSettings>({ ...DEFAULTS });
	loading  = $state(false);
	lastError = $state<string | null>(null);

	private pollHandle: ReturnType<typeof setInterval> | null = null;

	async refresh(): Promise<void> {
		if (!browser) return;
		this.loading = true;
		try {
			const data = await fetchJson<FovSettings>(FOV_ENDPOINT);
			this.settings = { ...DEFAULTS, ...data };
			this.lastError = null;
		} catch (e) {
			this.lastError = e instanceof Error ? e.message : String(e);
		} finally {
			this.loading = false;
		}
	}

	// Push a partial update. Server merges with existing settings and echoes
	// the full struct back. We immediately update the local state with the
	// echoed values so the UI mirrors the clamping the server applied.
	async update(patch: Partial<FovSettings>): Promise<void> {
		if (!browser) return;
		// Optimistic local update for snappy UX while POST is in flight.
		this.settings = { ...this.settings, ...patch };
		try {
			const echoed = await fetchJson<FovSettings>(FOV_ENDPOINT, {
				method: 'POST',
				headers: { 'Content-Type': 'application/json' },
				body: JSON.stringify(patch)
			});
			this.settings = { ...DEFAULTS, ...echoed };
			this.lastError = null;
		} catch (e) {
			this.lastError = e instanceof Error ? e.message : String(e);
		}
	}

	// Start polling — used by the FOV drawer when it's open so the diagnostic
	// activeCrop* fields stay live. Idempotent.
	startPolling(): void {
		if (!browser || this.pollHandle) return;
		void this.refresh();
		this.pollHandle = setInterval(() => {
			void this.refresh();
		}, POLL_INTERVAL_MS);
	}

	stopPolling(): void {
		if (this.pollHandle) {
			clearInterval(this.pollHandle);
			this.pollHandle = null;
		}
	}
}

export const fovStore = new FovStore();
