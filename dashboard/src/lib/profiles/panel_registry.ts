import type { Component } from 'svelte';
import MouseDynamicsInspector from '$lib/components/panels/MouseDynamicsInspector.svelte';
import SpectatorTelemetryMatrix from '$lib/components/panels/SpectatorTelemetryMatrix.svelte';
import VelocityProfilePanel from '$lib/components/panels/VelocityProfilePanel.svelte';
import FittsResidualPanel from '$lib/components/panels/FittsResidualPanel.svelte';
import SubPixelPanel from '$lib/components/panels/SubPixelPanel.svelte';
import ObserverMatrixPanel from '$lib/components/domains/ObserverMatrixPanel.svelte';

export interface PanelEntry {
	id: string;
	label: string;
	component: Component;
	destination: 'inspect' | 'operate' | 'calibrate' | 'engage';
	optional?: boolean;
}

export const PANEL_REGISTRY: PanelEntry[] = [
	{
		id: 'mouse-dynamics-inspector',
		label: 'Mouse Dynamics Inspector',
		component: MouseDynamicsInspector,
		destination: 'inspect'
	},
	{
		id: 'spectator-telemetry-matrix',
		label: 'Spectator Telemetry Matrix',
		component: SpectatorTelemetryMatrix,
		destination: 'inspect'
	},
	{
		id: 'velocity-profile',
		label: 'Velocity Profile',
		component: VelocityProfilePanel,
		destination: 'inspect',
		optional: true
	},
	{
		id: 'fitts-residual',
		label: 'Fitts Residual',
		component: FittsResidualPanel,
		destination: 'inspect',
		optional: true
	},
	{
		id: 'subpixel',
		label: 'Sub-Pixel Sampling',
		component: SubPixelPanel,
		destination: 'inspect',
		optional: true
	},
	{
		id: 'observer-matrix',
		label: 'Observer Matrix',
		component: ObserverMatrixPanel,
		destination: 'inspect',
		optional: true
	}
];

export function panelsForInspect(scenarioPanelIds?: string[]): PanelEntry[] {
	if (!scenarioPanelIds || scenarioPanelIds.length === 0) {
		return PANEL_REGISTRY.filter((p) => p.destination === 'inspect' && !p.optional);
	}
	return scenarioPanelIds
		.map((id) => PANEL_REGISTRY.find((p) => p.id === id))
		.filter((p): p is PanelEntry => p !== undefined);
}
