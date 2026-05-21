// =============================================================================
// roc-ai-vision · dashboard · capture_sources_store.svelte.ts
//
// Reactive client for the orchestrator's Phase 8 desktop-capture enumeration:
//   GET /api/capture-sources -> { monitors: [...], windows: [...] }
//
// Used by the SourcesDrawer to populate the desktop-capture picker with REAL
// monitor + window data instead of a hardcoded list. Refreshes on demand
// (clicking the "scan windows" button) and on drawer open.
//
// NOTE: this is a READ-ONLY enumeration. Switching to a desktop / window /
// region source today happens at orchestrator startup via the CLI flags
// (`--desktop-capture window --capture-window "YouTube"`, etc.). The drawer
// emits the matching command line so the operator can copy-paste it. A live
// source-swap REST endpoint is on the Phase-8.5 wishlist.
// =============================================================================

import { browser } from '$app/environment';

export interface CaptureMonitorInfo {
	index: number;
	x: number;
	y: number;
	w: number;
	h: number;
	isPrimary: boolean;
	deviceName: string;
}

export interface CaptureWindowInfo {
	hwnd: number;
	title: string;
	x: number;
	y: number;
	w: number;
	h: number;
	minimized: boolean;
}

export interface CaptureSourcesSnapshot {
	monitors: CaptureMonitorInfo[];
	windows: CaptureWindowInfo[];
}

const ENDPOINT = 'http://127.0.0.1:8766/api/capture-sources';
const FETCH_TIMEOUT_MS = 3000;

async function fetchJson<T>(url: string): Promise<T> {
	const ctrl = new AbortController();
	const tid = setTimeout(() => ctrl.abort(), FETCH_TIMEOUT_MS);
	try {
		const res = await fetch(url, { signal: ctrl.signal });
		if (!res.ok) throw new Error(`HTTP ${res.status} ${res.statusText}`);
		return (await res.json()) as T;
	} finally {
		clearTimeout(tid);
	}
}

class CaptureSourcesStore {
	monitors = $state<CaptureMonitorInfo[]>([]);
	windows  = $state<CaptureWindowInfo[]>([]);
	loading  = $state(false);
	lastError = $state<string | null>(null);

	get monitorCount(): number { return this.monitors.length; }
	get windowCount(): number  { return this.windows.length; }

	async refresh(): Promise<void> {
		if (!browser) return;
		this.loading = true;
		try {
			const snap = await fetchJson<CaptureSourcesSnapshot>(ENDPOINT);
			this.monitors = snap.monitors ?? [];
			// Filter out tiny / invisible / minimized windows so the UI list
			// doesn't drown in browser background tabs and OS overlays.
			this.windows = (snap.windows ?? []).filter(
				(w) => !w.minimized && w.w >= 200 && w.h >= 150 && w.title.trim().length > 0
			);
			this.lastError = null;
		} catch (e) {
			this.lastError = e instanceof Error ? e.message : String(e);
		} finally {
			this.loading = false;
		}
	}
}

export const captureSourcesStore = new CaptureSourcesStore();

// -----------------------------------------------------------------------------
// CLI-flag synthesizer — small helper so the drawer can show the exact set
// of orchestrator flags needed to reproduce the operator's selection on next
// boot. Kept here (not in the .svelte component) so it's unit-testable.
// -----------------------------------------------------------------------------
export interface DesktopCaptureCliArgs {
	desktopKind: 'primary' | 'monitor' | 'window' | 'region';
	monitorIndex?: number;
	windowTitle?: string;
	regionX?: number;
	regionY?: number;
	regionW?: number;
	regionH?: number;
	fps?: number;
	downscale?: number;
	cursor?: boolean;
}

export function synthesizeCliFlags(args: DesktopCaptureCliArgs): string[] {
	const out: string[] = ['--desktop-capture', args.desktopKind];
	if (args.desktopKind === 'monitor' && args.monitorIndex !== undefined) {
		out.push('--capture-monitor', String(args.monitorIndex));
	}
	if (args.desktopKind === 'window' && args.windowTitle) {
		out.push('--capture-window', JSON.stringify(args.windowTitle));
	}
	if (
		args.desktopKind === 'region' &&
		args.regionX !== undefined && args.regionY !== undefined &&
		args.regionW !== undefined && args.regionH !== undefined
	) {
		out.push('--capture-region', `${args.regionX},${args.regionY},${args.regionW},${args.regionH}`);
	}
	if (args.fps !== undefined)        out.push('--capture-fps',       String(args.fps));
	if (args.downscale !== undefined && args.downscale > 0)
		out.push('--capture-downscale', String(args.downscale));
	if (args.cursor)                   out.push('--capture-cursor');
	return out;
}
