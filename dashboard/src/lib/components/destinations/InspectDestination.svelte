<!--
	roc-ai-vision · dashboard · InspectDestination.svelte

	Foxglove-grade debug workspace:
		[HierarchyExplorerDrawer (240px left, collapsible)]
		[FEATURED MouseDynamicsInspector + 4 derived feature mini-plots]
		[2-col grid: SpectatorTelemetryMatrix + PipelineGraph + ModelInspector + TopicGraph]
		[EventLogDock bottom (collapsible)]
-->
<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import HierarchyExplorerDrawer from '$lib/components/panels/inspect/HierarchyExplorerDrawer.svelte';
	import PipelineGraphPanel from '$lib/components/panels/inspect/PipelineGraphPanel.svelte';
	import ModelInspectorPanel from '$lib/components/panels/inspect/ModelInspectorPanel.svelte';
	import TopicGraphPanel from '$lib/components/panels/inspect/TopicGraphPanel.svelte';
	import EventLogDock from '$lib/components/panels/inspect/EventLogDock.svelte';
	import MouseDynamicsInspector from '$lib/components/panels/MouseDynamicsInspector.svelte';
	import SpectatorTelemetryMatrix from '$lib/components/panels/SpectatorTelemetryMatrix.svelte';
	import VelocityProfilePanel from '$lib/components/panels/VelocityProfilePanel.svelte';
	import FittsResidualPanel from '$lib/components/panels/FittsResidualPanel.svelte';
	import SubPixelPanel from '$lib/components/panels/SubPixelPanel.svelte';
	import { scenarioStore } from '$lib/profiles/scenario_store.svelte';
	import { eventLog } from '$lib/stores/event_log.svelte';
	import { keybindingManager } from '$lib/keybindings/manager.svelte';
	import { focusedPanel, type InspectPanelId } from '$lib/stores/focused_panel.svelte';

	function focus(id: InspectPanelId) {
		focusedPanel.focus(id);
	}

	let hierCollapsed = $state(false);

	function handleToggleEventLog() {
		eventLog.toggleCollapse();
	}

	onMount(() => {
		keybindingManager.register('global:perf-hud', handleToggleEventLog);
		return () => {
			keybindingManager.unregister('global:perf-hud');
		};
	});

	// Bind ` to toggle event log
	function handleKeydown(e: KeyboardEvent) {
		const target = e.target as HTMLElement | null;
		const inInput = target && ['INPUT', 'TEXTAREA', 'SELECT'].includes(target.tagName);
		if (inInput) return;
		if (e.key === '`') {
			e.preventDefault();
			eventLog.toggleCollapse();
		}
	}
</script>

<svelte:window onkeydown={handleKeydown} />

<div class="inspect-dest">
	<div class="inspect-main">
		<HierarchyExplorerDrawer />

		<div class="inspect-content">
			<header class="inspect-header">
				<div class="header-left">
					<h2 class="dest-title">Inspect</h2>
					<p class="dest-subtitle">
						Behavioural telemetry · {scenarioStore.active.label}
					</p>
				</div>
				<button class="evt-toggle-btn" onclick={handleToggleEventLog}>
					{eventLog.collapsed ? 'Show' : 'Hide'} Event Log
					<span class="evt-toggle-hint">`</span>
				</button>
			</header>

			<div class="featured-panel">
				<button class="card card-clickable" class:focused={focusedPanel.focusedId === 'mouse-dynamics-inspector'} onclick={() => focus('mouse-dynamics-inspector')}>
					<div class="card-header">
						<span class="card-title">MOUSE DYNAMICS INSPECTOR</span>
						<span class="card-tag">FEATURED · ss12.1</span>
					</div>
					<div class="card-body featured-body">
						<MouseDynamicsInspector />
					</div>
				</button>
			</div>

			<div class="derived-strip">
				<button class="card mini card-clickable" class:focused={focusedPanel.focusedId === 'velocity-profile'} onclick={() => focus('velocity-profile')}>
					<div class="card-header">
						<span class="card-title">VELOCITY PROFILE</span>
						<span class="card-tag">P1 · F01</span>
					</div>
					<div class="card-body">
						<VelocityProfilePanel />
					</div>
				</button>
				<button class="card mini card-clickable" class:focused={focusedPanel.focusedId === 'fitts-residual'} onclick={() => focus('fitts-residual')}>
					<div class="card-header">
						<span class="card-title">FITTS RESIDUAL</span>
						<span class="card-tag">P3 · F21</span>
					</div>
					<div class="card-body">
						<FittsResidualPanel />
					</div>
				</button>
				<button class="card mini card-clickable" class:focused={focusedPanel.focusedId === 'subpixel'} onclick={() => focus('subpixel')}>
					<div class="card-header">
						<span class="card-title">SUB-PIXEL RATIO</span>
						<span class="card-tag">P7 · F16</span>
					</div>
					<div class="card-body">
						<SubPixelPanel />
					</div>
				</button>
				<button class="card mini card-clickable" class:focused={focusedPanel.focusedId === 'jerk-spectrum'} onclick={() => focus('jerk-spectrum')}>
					<div class="card-header">
						<span class="card-title">JERK + TREMOR</span>
						<span class="card-tag">P2 · F03/F09</span>
					</div>
					<div class="card-body placeholder-body">
						<div class="placeholder-msg">
							<span class="placeholder-title">8-12 Hz tremor band</span>
							<span class="placeholder-sub">FFT spectrogram · Phase 6+</span>
							<span class="placeholder-data">peak: 4218 · band: 0.42 · ✓ HUMAN</span>
						</div>
					</div>
				</button>
			</div>

			<div class="companion-grid">
				<button class="card card-clickable" class:focused={focusedPanel.focusedId === 'spectator-telemetry-matrix'} onclick={() => focus('spectator-telemetry-matrix')}>
					<div class="card-header">
						<span class="card-title">SPECTATOR TELEMETRY MATRIX</span>
						<span class="card-tag">ss13.5 · bg-odometry corr</span>
					</div>
					<div class="card-body">
						<SpectatorTelemetryMatrix />
					</div>
				</button>
				<button class="card card-clickable" class:focused={focusedPanel.focusedId === 'pipeline-graph'} onclick={() => focus('pipeline-graph')}>
					<div class="card-header">
						<span class="card-title">PIPELINE GRAPH</span>
						<span class="card-tag">DeepStream + Hailo</span>
					</div>
					<div class="card-body">
						<PipelineGraphPanel />
					</div>
				</button>
				<button class="card card-clickable" class:focused={focusedPanel.focusedId === 'model-inspector'} onclick={() => focus('model-inspector')}>
					<div class="card-header">
						<span class="card-title">MODEL INSPECTOR</span>
						<span class="card-tag">ORT per-layer profile</span>
					</div>
					<div class="card-body">
						<ModelInspectorPanel />
					</div>
				</button>
				<button class="card card-clickable" class:focused={focusedPanel.focusedId === 'topic-graph'} onclick={() => focus('topic-graph')}>
					<div class="card-header">
						<span class="card-title">WS TOPIC GRAPH</span>
						<span class="card-tag">Foxglove pub-sub map</span>
					</div>
					<div class="card-body">
						<TopicGraphPanel />
					</div>
				</button>
			</div>
		</div>
	</div>

	<EventLogDock />
</div>

<style>
	.inspect-dest {
		flex: 1;
		display: flex;
		flex-direction: column;
		min-height: 0;
		background: #0e0e10;
	}

	.inspect-main {
		flex: 1;
		display: flex;
		min-height: 0;
		overflow: hidden;
	}

	.inspect-content {
		flex: 1;
		overflow-y: auto;
		padding: 16px;
		display: flex;
		flex-direction: column;
		gap: 12px;
		min-width: 0;
	}

	.inspect-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		gap: 12px;
	}

	.header-left {
		display: flex;
		align-items: baseline;
		gap: 12px;
	}

	.dest-title {
		margin: 0;
		font-size: 16px;
		font-weight: 600;
		color: #d4d0c8;
	}

	.dest-subtitle {
		margin: 0;
		font-size: 11px;
		color: #666;
	}

	.evt-toggle-btn {
		display: flex;
		align-items: center;
		gap: 6px;
		padding: 5px 12px;
		background: #161618;
		color: #878787;
		border: 1px solid #2a2a2e;
		border-radius: 4px;
		font-size: 11px;
		font-family: inherit;
		cursor: pointer;
	}

	.evt-toggle-btn:hover {
		color: #f5a623;
		border-color: rgba(245, 166, 35, 0.4);
	}

	.evt-toggle-hint {
		font-size: 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		padding: 1px 5px;
		background: #222;
		border: 1px solid #333;
		border-radius: 2px;
		color: #555;
	}

	.featured-panel {
		display: flex;
	}

	.featured-panel .card {
		width: 100%;
		min-height: 320px;
	}

	.derived-strip {
		display: grid;
		grid-template-columns: repeat(4, 1fr);
		gap: 10px;
	}

	.companion-grid {
		display: grid;
		grid-template-columns: 1fr 1fr;
		gap: 10px;
	}

	.card {
		display: flex;
		flex-direction: column;
		background: #161618;
		border: 1px solid #27272a;
		border-radius: 6px;
		overflow: hidden;
		transition: border-color 150ms ease;
		text-align: left;
		color: inherit;
		font-family: inherit;
		padding: 0;
	}

	.card-clickable {
		cursor: pointer;
	}

	.card:hover {
		border-color: #3a3a3e;
	}

	.card.focused {
		border-color: rgba(245, 166, 35, 0.5);
		box-shadow: 0 0 0 1px rgba(245, 166, 35, 0.2);
	}

	.card.mini {
		min-height: 200px;
	}

	.card-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding: 8px 12px;
		background: #1a1a1e;
		border-bottom: 1px solid #1d1d20;
		flex-shrink: 0;
	}

	.card-title {
		font-size: 10px;
		font-weight: 700;
		color: #d4d0c8;
		letter-spacing: 0.06em;
		text-transform: uppercase;
	}

	.card-tag {
		font-size: 9px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #555;
		letter-spacing: 0.04em;
	}

	.card-body {
		flex: 1;
		padding: 10px 12px;
		overflow: hidden;
		min-height: 0;
	}

	.featured-body {
		padding: 8px;
		overflow: auto;
	}

	.placeholder-body {
		display: flex;
		align-items: center;
		justify-content: center;
	}

	.placeholder-msg {
		display: flex;
		flex-direction: column;
		align-items: center;
		gap: 4px;
		text-align: center;
	}

	.placeholder-title {
		font-size: 11px;
		font-weight: 600;
		color: #00f0ff;
	}

	.placeholder-sub {
		font-size: 9px;
		color: #555;
		font-style: italic;
	}

	.placeholder-data {
		margin-top: 8px;
		font-size: 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #d4d0c8;
		padding: 4px 8px;
		background: rgba(0, 240, 255, 0.05);
		border-radius: 3px;
	}
</style>
