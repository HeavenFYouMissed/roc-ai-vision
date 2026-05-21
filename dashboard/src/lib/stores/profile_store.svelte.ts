// =============================================================================
// roc-ai-vision · dashboard · profile_store.svelte.ts
//
// Runtime Profiles per dashboard_menu_architecture_2026.md §5.6 + §6.5.
// Profiles describe "what the C++ engine is running" — EP order, recording
// mode, actuator safety caps. Orthogonal to Scenarios (which describe
// "what panels the operator is looking at").
//
// Profiles are static defs for now; future Phase 6+ adds user-defined
// profiles persisted under `dashboard/profiles/*.json`.
// =============================================================================

import { browser } from '$app/environment';

export type EPName = 'CUDA' | 'TensorRT' | 'DML' | 'OpenVINO' | 'CPU';
export type RecordingMode = 'off' | 'jsonl' | 'jsonl_mp4' | 'playback_only';
export type ActuatorCapsMode = 'hard_disabled' | 'armed_and_configured';

export interface ProfileDef {
	id: string;
	label: string;
	epOrder: EPName[];
	recording: RecordingMode;
	actuatorCaps: ActuatorCapsMode;
	notes: string;
}

export const PROFILES: readonly ProfileDef[] = [
	{
		id: 'default',
		label: 'Default (dev)',
		epOrder: ['CPU', 'DML'],
		recording: 'off',
		actuatorCaps: 'hard_disabled',
		notes: 'Local development, no hardware risks'
	},
	{
		id: 'prod-nvidia',
		label: 'Prod-NVIDIA',
		epOrder: ['CUDA', 'TensorRT', 'CPU'],
		recording: 'jsonl_mp4',
		actuatorCaps: 'armed_and_configured',
		notes: 'Full hardware acceleration, writes local telemetry logs'
	},
	{
		id: 'prod-amd-intel',
		label: 'Prod-AMD/Intel',
		epOrder: ['DML', 'OpenVINO', 'CPU'],
		recording: 'jsonl_mp4',
		actuatorCaps: 'armed_and_configured',
		notes: 'Cross-vendor hardware acceleration'
	},
	{
		id: 'replay',
		label: 'Replay/Forensic',
		epOrder: [],
		recording: 'playback_only',
		actuatorCaps: 'hard_disabled',
		notes: 'Forces dashboard into strict playback mode'
	}
] as const;

const STORAGE_KEY = 'roc.profile.activeId';

class ProfileStore {
	activeProfileId = $state<string>(PROFILES[0].id);

	private hydrated = false;

	hydrate(): void {
		if (this.hydrated || !browser) return;
		try {
			const saved = localStorage.getItem(STORAGE_KEY);
			if (saved && PROFILES.some((p) => p.id === saved)) {
				this.activeProfileId = saved;
			}
		} catch {
			// localStorage unavailable - keep default
		}
		this.hydrated = true;
	}

	get active(): ProfileDef {
		return PROFILES.find((p) => p.id === this.activeProfileId) ?? PROFILES[0];
	}

	setProfile(id: string): void {
		if (!PROFILES.some((p) => p.id === id)) return;
		this.activeProfileId = id;
		if (browser) {
			try {
				localStorage.setItem(STORAGE_KEY, id);
			} catch {
				// silently ignore
			}
		}
	}

	get isReplay(): boolean {
		return this.active.id === 'replay';
	}

	get recordingEnabled(): boolean {
		return this.active.recording !== 'off' && this.active.recording !== 'playback_only';
	}

	get actuatorAllowed(): boolean {
		return this.active.actuatorCaps === 'armed_and_configured' && !this.isReplay;
	}
}

export const profileStore = new ProfileStore();
