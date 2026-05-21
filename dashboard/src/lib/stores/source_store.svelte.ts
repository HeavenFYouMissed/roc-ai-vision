// =============================================================================
// roc-ai-vision · dashboard · source_store.svelte.ts
//
// Camera / capture-card / file source selection state for the Operate canvas
// Layer 0 <video> element. Mirrors the architecture in UI_SPEC_SHEET.md ss8:
//   - Lives in a dedicated Sources drawer (NOT a destination)
//   - Owns: device list (enumerated), active source, MJPEG URL, resolution
//   - Cross-cutting: used by Operate AND Engage (both render the canvas)
//   - Future: populated by HTTP /sources endpoint on roc_vision.exe
//
// Wire format extension (Phase 6+):
//   OperatorCommandKind.SetCameraSource = 8  (sends {device_index, width, height})
// =============================================================================

import { browser } from '$app/environment';

export type SourceKind = 'mock' | 'webcam' | 'capture_card' | 'file' | 'rtsp' | 'webrtc';

export interface SourceDevice {
	id: string;
	kind: SourceKind;
	label: string;
	deviceIndex?: number;            // for cv::VideoCapture index
	backend?: 'msmf' | 'dshow' | 'v4l2' | 'avfoundation';
	width?: number;
	height?: number;
	fps?: number;
	mjpegUrl?: string;               // when video is available via MJPEG-over-HTTP
	online?: boolean;                // health-check result from /sources endpoint
}

// Built-in starter list — replaced by /sources HTTP endpoint when available.
// Until the C++ side ships the endpoint, these are the canonical options
// the operator can pick from.
const BUILTIN_DEVICES: SourceDevice[] = [
	{
		id: 'mock',
		kind: 'mock',
		label: 'Mock Frame Source (synthetic v1/v2/v3)',
		online: true
	},
	{
		id: 'webcam-0',
		kind: 'webcam',
		label: 'Laptop Webcam (camera index 0)',
		deviceIndex: 0,
		backend: 'msmf',
		width: 640,
		height: 480,
		fps: 30,
		mjpegUrl: 'http://127.0.0.1:8766/mjpeg/0',
		online: false
	},
	{
		id: 'webcam-1',
		kind: 'webcam',
		label: 'External Webcam (camera index 1)',
		deviceIndex: 1,
		backend: 'msmf',
		width: 1280,
		height: 720,
		fps: 30,
		mjpegUrl: 'http://127.0.0.1:8766/mjpeg/1',
		online: false
	},
	{
		id: 'capture-2',
		kind: 'capture_card',
		label: 'Capture Card (camera index 2 · typical HDMI)',
		deviceIndex: 2,
		backend: 'msmf',
		width: 1920,
		height: 1080,
		fps: 60,
		mjpegUrl: 'http://127.0.0.1:8766/mjpeg/2',
		online: false
	},
	{
		id: 'file',
		kind: 'file',
		label: 'File Replay (.mp4 / .mkv)',
		online: false
	}
];

const STORAGE_KEY = 'roc.source.activeId';
const ENUMERATION_ENDPOINT = 'http://127.0.0.1:8766/sources';

class SourceStore {
	devices = $state<SourceDevice[]>([...BUILTIN_DEVICES]);
	activeSourceId = $state<string>('mock');
	enumerating = $state(false);
	lastEnumerationError = $state<string | null>(null);

	// Operator-tweakable per-device overrides (resolution + fps + mjpeg URL)
	// Persisted alongside activeSourceId.
	resolutionOverride = $state<{ width: number; height: number; fps: number } | null>(null);

	private hydrated = false;

	hydrate(): void {
		if (this.hydrated || !browser) return;
		try {
			const saved = localStorage.getItem(STORAGE_KEY);
			if (saved && this.devices.some((d) => d.id === saved)) {
				this.activeSourceId = saved;
			}
		} catch {
			// ignore
		}
		this.hydrated = true;
	}

	get active(): SourceDevice {
		return this.devices.find((d) => d.id === this.activeSourceId) ?? this.devices[0];
	}

	get mjpegUrl(): string | null {
		const a = this.active;
		if (!a) return null;
		if (a.kind === 'mock' || a.kind === 'file') return null;
		return a.mjpegUrl ?? null;
	}

	get sourceLabel(): string {
		const a = this.active;
		if (!a) return 'MOCK';
		if (a.kind === 'mock') return 'MOCK';
		if (a.kind === 'webcam') return `WEBCAM ${a.deviceIndex ?? '?'}`;
		if (a.kind === 'capture_card') return `CAPTURE ${a.deviceIndex ?? '?'}`;
		if (a.kind === 'file') return 'FILE';
		if (a.kind === 'rtsp') return 'RTSP';
		if (a.kind === 'webrtc') return 'WEBRTC';
		return a.label.toUpperCase();
	}

	setSource(id: string): void {
		if (!this.devices.some((d) => d.id === id)) return;
		this.activeSourceId = id;
		if (browser) {
			try {
				localStorage.setItem(STORAGE_KEY, id);
			} catch {
				// ignore
			}
		}
	}

	// Calls /sources HTTP endpoint on the C++ orchestrator to discover real
	// hardware. Merges with the builtin list, replacing any matching id.
	// Falls back silently when endpoint is unreachable (orchestrator offline).
	async enumerate(): Promise<void> {
		if (!browser || this.enumerating) return;
		this.enumerating = true;
		this.lastEnumerationError = null;
		try {
			const ctrl = new AbortController();
			const tid = setTimeout(() => ctrl.abort(), 2000);
			const res = await fetch(ENUMERATION_ENDPOINT, { signal: ctrl.signal });
			clearTimeout(tid);
			if (!res.ok) {
				throw new Error(`HTTP ${res.status}`);
			}
			const data = (await res.json()) as { devices?: SourceDevice[] };
			if (data.devices && Array.isArray(data.devices)) {
				// Merge: keep built-ins, override or add discovered devices
				const map = new Map<string, SourceDevice>();
				for (const d of BUILTIN_DEVICES) map.set(d.id, d);
				for (const d of data.devices) map.set(d.id, { ...d, online: true });
				this.devices = Array.from(map.values());
			}
		} catch (e) {
			this.lastEnumerationError = e instanceof Error ? e.message : String(e);
		} finally {
			this.enumerating = false;
		}
	}

	setResolutionOverride(width: number, height: number, fps: number): void {
		this.resolutionOverride = { width, height, fps };
	}

	clearResolutionOverride(): void {
		this.resolutionOverride = null;
	}
}

export const sourceStore = new SourceStore();
