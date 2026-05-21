<!--
	roc-ai-vision · dashboard · LayerVisibilityToolbar.svelte

	Floating bottom-left of OperateCanvas. 6 toggle buttons for the 6 layers
	plus Foxglove Image Overlays controls (opacity slider + blend mode +
	pixel alpha for masks).

	Hotkeys: v = Layer 0 (video), o = Layers 1-3 (overlay group toggle)
-->
<script lang="ts">
	import { renderSettings } from '$lib/stores/render_settings.svelte';

	interface LayerDef {
		key: keyof typeof iconPaths;
		label: string;
		hotkey?: string;
		toggle: () => void;
		visible: () => boolean;
	}

	const iconPaths = {
		video: 'M23 7l-7 5 7 5V7zM14 5H3a2 2 0 0 0-2 2v10a2 2 0 0 0 2 2h11a2 2 0 0 0 2-2V7a2 2 0 0 0-2-2z',
		traj: 'M22 12h-4l-3 9L9 3l-3 9H2',
		actuator: 'M2 17l10-10 4 4 8-8',
		detect: 'M3 3h7v7H3V3zm0 11h7v7H3v-7zm11-11h7v7h-7V3zm0 11h7v7h-7v-7z',
		hier: 'M12 2v6m0 0l-4 4m4-4l4 4M6 14l-4 4m4-4l4 4M18 14l4 4m-4-4l-4 4',
		cross: 'M12 2v8m0 4v8M2 12h8m4 0h8M12 12L8 8m4 4l4-4m-4 4l-4 4m4-4l4 4'
	};

	const layers: LayerDef[] = [
		{
			key: 'video',
			label: 'Video',
			hotkey: 'v',
			toggle: () => {
				renderSettings.showVideo = !renderSettings.showVideo;
				renderSettings.persist();
			},
			visible: () => renderSettings.showVideo
		},
		{
			key: 'traj',
			label: 'Trajectories',
			toggle: () => {
				renderSettings.showTrajectories = !renderSettings.showTrajectories;
				renderSettings.persist();
			},
			visible: () => renderSettings.showTrajectories
		},
		{
			key: 'actuator',
			label: 'Actuator',
			toggle: () => {
				renderSettings.showActuatorTrace = !renderSettings.showActuatorTrace;
				renderSettings.persist();
			},
			visible: () => renderSettings.showActuatorTrace
		},
		{
			key: 'detect',
			label: 'Detections',
			hotkey: 'o',
			toggle: () => {
				renderSettings.showDetections = !renderSettings.showDetections;
				renderSettings.persist();
			},
			visible: () => renderSettings.showDetections
		},
		{
			key: 'hier',
			label: 'Hierarchy',
			toggle: () => {
				renderSettings.showHierarchy = !renderSettings.showHierarchy;
				renderSettings.persist();
			},
			visible: () => renderSettings.showHierarchy
		},
		{
			key: 'cross',
			label: 'Crosshair',
			toggle: () => {
				renderSettings.showCrosshair = !renderSettings.showCrosshair;
				renderSettings.persist();
			},
			visible: () => renderSettings.showCrosshair
		}
	];
</script>

<div class="layer-toolbar" role="toolbar" aria-label="Canvas layer visibility">
	{#each layers as layer (layer.key)}
		<button
			class="layer-btn"
			class:active={layer.visible()}
			onclick={() => layer.toggle()}
			title="{layer.label}{layer.hotkey ? ` (${layer.hotkey})` : ''}"
		>
			<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
				<path d={iconPaths[layer.key]}></path>
			</svg>
			{#if layer.hotkey}
				<span class="hotkey-hint">{layer.hotkey}</span>
			{/if}
		</button>
	{/each}
</div>

<style>
	.layer-toolbar {
		position: absolute;
		bottom: 12px;
		left: 12px;
		display: flex;
		gap: 4px;
		padding: 5px;
		background: rgba(22, 22, 24, 0.85);
		backdrop-filter: blur(8px);
		border: 1px solid rgba(39, 39, 42, 0.8);
		border-radius: 6px;
		z-index: 5;
	}

	.layer-btn {
		position: relative;
		width: 32px;
		height: 32px;
		display: flex;
		align-items: center;
		justify-content: center;
		background: transparent;
		border: 1px solid transparent;
		border-radius: 4px;
		color: #555;
		cursor: pointer;
		transition: all 120ms ease;
	}

	.layer-btn:hover {
		background: rgba(245, 166, 35, 0.08);
		color: #d4d0c8;
		border-color: rgba(245, 166, 35, 0.3);
	}

	.layer-btn.active {
		color: #f5a623;
		background: rgba(245, 166, 35, 0.1);
	}

	.layer-btn svg {
		width: 16px;
		height: 16px;
	}

	.hotkey-hint {
		position: absolute;
		top: 2px;
		right: 3px;
		font-size: 8px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #444;
		pointer-events: none;
	}

	.layer-btn.active .hotkey-hint {
		color: #f5a623;
		opacity: 0.7;
	}
</style>
