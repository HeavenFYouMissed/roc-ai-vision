export interface KeyBinding {
	combo: string;
	action: string;
	label: string;
	category: 'destination' | 'layer' | 'global' | 'panel' | 'selection';
	requiresModifier: boolean;
}

export const DEFAULT_BINDINGS: KeyBinding[] = [
	// Destinations
	{ combo: '1', action: 'destination:operate', label: 'Switch to Operate', category: 'destination', requiresModifier: false },
	{ combo: '2', action: 'destination:inspect', label: 'Switch to Inspect', category: 'destination', requiresModifier: false },
	{ combo: '3', action: 'destination:calibrate', label: 'Switch to Calibrate', category: 'destination', requiresModifier: false },
	{ combo: '4', action: 'destination:engage', label: 'Switch to Engage', category: 'destination', requiresModifier: false },
	{ combo: '5', action: 'destination:biometrics', label: 'Switch to Biometrics', category: 'destination', requiresModifier: false },

	// Layer toggles
	{ combo: 'v', action: 'layer:toggle-video', label: 'Toggle video layer', category: 'layer', requiresModifier: false },
	{ combo: 'o', action: 'layer:toggle-overlay', label: 'Toggle overlay layers', category: 'layer', requiresModifier: false },

	// Selection cycling
	{ combo: 'Tab', action: 'cycle:next-track', label: 'Next track', category: 'selection', requiresModifier: false },
	{ combo: 'Shift+Tab', action: 'cycle:prev-track', label: 'Previous track', category: 'selection', requiresModifier: true },
	{ combo: 'g', action: 'lock:click-to-lock', label: 'Click-to-lock cursor', category: 'selection', requiresModifier: false },

	// Rail toggles
	{ combo: '[', action: 'rail:toggle-left', label: 'Toggle left rail', category: 'global', requiresModifier: false },
	{ combo: ']', action: 'rail:toggle-right', label: 'Toggle right rail', category: 'global', requiresModifier: false },

	// Global
	{ combo: 'Ctrl+k', action: 'global:command-palette', label: 'Command Palette', category: 'global', requiresModifier: true },
	{ combo: 'Alt+z', action: 'global:overlay-mode', label: 'Toggle Overlay Mode', category: 'global', requiresModifier: true },
	{ combo: 'Alt+l', action: 'global:lock-target', label: 'Toggle target lock', category: 'global', requiresModifier: true },
	{ combo: 'Alt+r', action: 'global:perf-hud', label: 'Expand performance HUD', category: 'global', requiresModifier: true },
	{ combo: '?', action: 'global:cheatsheet', label: 'Hotkey Cheatsheet', category: 'global', requiresModifier: false },
	{ combo: 'Escape', action: 'global:dismiss', label: 'Dismiss / Cancel', category: 'global', requiresModifier: false },

	// Panel-focused
	{ combo: 'r', action: 'panel:record', label: 'Start Recording', category: 'panel', requiresModifier: false },
	{ combo: 'x', action: 'panel:stop', label: 'Stop Recording', category: 'panel', requiresModifier: false },
	{ combo: 'p', action: 'panel:pause', label: 'Pause / Resume Recording', category: 'panel', requiresModifier: false },
	{ combo: 'c', action: 'panel:mode-clean', label: 'Clean trace mode', category: 'panel', requiresModifier: false },
	{ combo: 'd', action: 'panel:mode-dirty', label: 'Dirty trace mode', category: 'panel', requiresModifier: false },
	{ combo: 't', action: 'panel:mode-both', label: 'Both traces overlap', category: 'panel', requiresModifier: false },
	{ combo: 's', action: 'panel:mode-split', label: 'Side-by-side traces', category: 'panel', requiresModifier: false }
];

export function comboMatchesEvent(combo: string, e: KeyboardEvent): boolean {
	const parts = combo.toLowerCase().split('+');
	const key = parts[parts.length - 1];
	const needCtrl = parts.includes('ctrl');
	const needAlt = parts.includes('alt');
	const needShift = parts.includes('shift');

	// Use e.key for non-modifier matching; handle 'tab', 'escape' as keys
	const evKey = e.key.toLowerCase();
	return (
		evKey === key &&
		e.ctrlKey === needCtrl &&
		e.altKey === needAlt &&
		e.shiftKey === needShift
	);
}
