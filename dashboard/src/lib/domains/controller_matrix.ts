export type ControllerDomain = 'operate' | 'inspect' | 'calibrate' | 'engage' | 'biometrics';

export interface DomainDef {
	id: ControllerDomain;
	label: string;
	hotkey: string;
	icon: string;
}

// Cardinality lock: 5 destinations. Biometrics is the documented exception per
// dashboard_menu_architecture_2026.md Amendment v6 §14.0 — it does NOT consume
// the C++ vision stream (trackPool/actuatorPool/bgOdometryPool); it owns its
// own self-contained data plane (Pointer Lock + mouse_capture.svelte.ts).
export const CONTROLLER_DOMAINS: readonly DomainDef[] = [
	{ id: 'operate', label: 'Operate', hotkey: '1', icon: 'crosshair' },
	{ id: 'inspect', label: 'Inspect', hotkey: '2', icon: 'activity' },
	{ id: 'calibrate', label: 'Calibrate', hotkey: '3', icon: 'sliders' },
	{ id: 'engage', label: 'Engage', hotkey: '4', icon: 'target' },
	{ id: 'biometrics', label: 'Biometrics', hotkey: '5', icon: 'pulse' }
] as const;
