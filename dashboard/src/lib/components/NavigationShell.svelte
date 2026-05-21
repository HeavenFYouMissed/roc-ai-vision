<!--
	roc-ai-vision · dashboard · NavigationShell.svelte

	The 3-zone mission-control shell. Top chrome + diag strip pinned across
	the top; left rail + center content + right rail in the body row.

	Per-destination wiring:
		Operate (1): OperateCanvas + LayerToolbar + ModelCard + ObjectCount + RightRail
		Inspect (2): featured biometric panels + companion grid (Phase 3 sessions)
		Calibrate (3): 5-step wizard (Phase 4 sessions)
		Engage (4): shared OperateCanvas + safety interlock + actuator controls (Phase 2 sessions)

	Includes Command Palette (Ctrl+K), Cheatsheet (?), Drawers, and the
	keybinding manager hooks.
-->
<script lang="ts">
	import OperateCanvas from '$lib/components/destinations/OperateCanvas.svelte';
	import EngageDestination from '$lib/components/destinations/EngageDestination.svelte';
	import InspectDestination from '$lib/components/destinations/InspectDestination.svelte';
	import CalibrateDestination from '$lib/components/destinations/CalibrateDestination.svelte';
	import BiometricsDestination from '$lib/components/destinations/BiometricsDestination.svelte';
	import LayerVisibilityToolbar from '$lib/components/overlays/LayerVisibilityToolbar.svelte';
	import ModelCard from '$lib/components/overlays/ModelCard.svelte';
	import ObjectCountBadge from '$lib/components/overlays/ObjectCountBadge.svelte';
	import LeftRail from '$lib/components/shell/LeftRail.svelte';
	import TopChrome from '$lib/components/shell/TopChrome.svelte';
	import DiagnosticsStrip from '$lib/components/shell/DiagnosticsStrip.svelte';
	import RightRail from '$lib/components/shell/RightRail.svelte';
	import CommandPalette from '$lib/components/shell/CommandPalette.svelte';
	import ModelsDrawer from '$lib/components/drawers/ModelsDrawer.svelte';
	import FovDrawer from '$lib/components/drawers/FovDrawer.svelte';
	import SettingsDrawer from '$lib/components/drawers/SettingsDrawer.svelte';
	import SourcesDrawer from '$lib/components/drawers/SourcesDrawer.svelte';
	import { type ControllerDomain } from '$lib/domains/controller_matrix';
	import { scenarioStore } from '$lib/profiles/scenario_store.svelte';
	import { keybindingManager } from '$lib/keybindings/manager.svelte';
	import { renderSettings } from '$lib/stores/render_settings.svelte';
	import { selectionStore } from '$lib/stores/selection_store.svelte';
	import { profileStore } from '$lib/stores/profile_store.svelte';
	import { overlayMode } from '$lib/stores/overlay_mode.svelte';
	import { sourceStore } from '$lib/stores/source_store.svelte';
	import { railState } from '$lib/stores/rail_state.svelte';
	import { focusedPanel } from '$lib/stores/focused_panel.svelte';
	import { telemetrySocket } from '$lib/telemetry/telemetry_socket.svelte';
	import { mockFrameSource } from '$lib/telemetry/mock_frame_source';
	import { decode_frame } from '$lib/telemetry/wire_decoder';

	let activeDomain = $state<ControllerDomain>('operate');
	let activeDrawer = $state<'models' | 'sources' | 'settings' | 'fov' | null>(null);
	let paletteOpen = $state(false);
	let cheatsheetOpen = $state(false);

	$effect(() => {
		// Hydrate persisted state
		renderSettings.hydrate();
		profileStore.hydrate();
		scenarioStore.hydrate();
		overlayMode.hydrate();
		sourceStore.hydrate();
		railState.hydrate();
		focusedPanel.hydrate();

		// --- Telemetry lifecycle owned at shell level, NOT per-destination ---
		// Previously OperateCanvas connected/disconnected the WS on mount/unmount,
		// which caused a 1-2s WS drop every time the user switched destinations.
		// All four destinations need the same track/actuator stream, so we own
		// the socket here and let each destination just read from the pools.
		mockFrameSource.start((buf) => {
			// Mock source feeds when WS is offline. v1 (track) frames are
			// suppressed when WS is connected so the live pipeline wins.
			const version = new Uint8Array(buf, 0, 1)[0];
			if (version === 1 && telemetrySocket.state === 'connected') return;
			decode_frame(buf);
		});
		telemetrySocket.connect();

		// Register all keybindings
		keybindingManager.register('destination:operate', () => handleDomainChange('operate'));
		keybindingManager.register('destination:inspect', () => handleDomainChange('inspect'));
		keybindingManager.register('destination:calibrate', () => handleDomainChange('calibrate'));
		keybindingManager.register('destination:engage', () => handleDomainChange('engage'));
		keybindingManager.register('destination:biometrics', () => handleDomainChange('biometrics'));
		keybindingManager.register('layer:toggle-video', () => {
			renderSettings.showVideo = !renderSettings.showVideo;
			renderSettings.persist();
		});
		keybindingManager.register('layer:toggle-overlay', () => {
			const next = !renderSettings.showDetections;
			renderSettings.showDetections = next;
			renderSettings.showTrajectories = next;
			renderSettings.showHierarchy = next;
			renderSettings.persist();
		});
		keybindingManager.register('global:command-palette', () => {
			paletteOpen = !paletteOpen;
		});
		keybindingManager.register('global:overlay-mode', () => {
			overlayMode.toggle();
		});
		keybindingManager.register('global:cheatsheet', () => {
			cheatsheetOpen = !cheatsheetOpen;
		});
		keybindingManager.register('global:dismiss', () => {
			if (paletteOpen) paletteOpen = false;
			else if (cheatsheetOpen) cheatsheetOpen = false;
			else if (activeDrawer) activeDrawer = null;
			else if (overlayMode.enabled) overlayMode.disable();
			else if (selectionStore.clickToLockMode) selectionStore.disableClickToLock();
			else selectionStore.deselect();
		});
		keybindingManager.register('cycle:next-track', () => selectionStore.cycleNext());
		keybindingManager.register('cycle:prev-track', () => selectionStore.cyclePrev());
		keybindingManager.register('rail:toggle-right', () => {
			railState.toggleRight();
		});
		keybindingManager.register('rail:toggle-left', () => {
			railState.toggleLeft();
		});
		keybindingManager.register('lock:click-to-lock', () => {
			selectionStore.enableClickToLock();
		});

		return () => {
			telemetrySocket.disconnect();
			mockFrameSource.stop();
			keybindingManager.unregister('destination:operate');
			keybindingManager.unregister('destination:inspect');
			keybindingManager.unregister('destination:calibrate');
			keybindingManager.unregister('destination:engage');
			keybindingManager.unregister('destination:biometrics');
			keybindingManager.unregister('layer:toggle-video');
			keybindingManager.unregister('layer:toggle-overlay');
			keybindingManager.unregister('global:command-palette');
			keybindingManager.unregister('global:overlay-mode');
			keybindingManager.unregister('global:cheatsheet');
			keybindingManager.unregister('global:dismiss');
			keybindingManager.unregister('cycle:next-track');
			keybindingManager.unregister('cycle:prev-track');
			keybindingManager.unregister('rail:toggle-right');
			keybindingManager.unregister('rail:toggle-left');
			keybindingManager.unregister('lock:click-to-lock');
		};
	});

	function handleDomainChange(domain: ControllerDomain) {
		activeDomain = domain;
		activeDrawer = null;
	}

	function handleDrawerToggle(drawer: 'models' | 'sources' | 'settings' | 'fov') {
		activeDrawer = activeDrawer === drawer ? null : drawer;
	}

	function handleKeydown(e: KeyboardEvent) {
		keybindingManager.isModalOpen = paletteOpen || cheatsheetOpen;
		keybindingManager.dispatch(e);
	}

	function handleSwitchModel() {
		activeDrawer = 'models';
	}
</script>

<svelte:window onkeydown={handleKeydown} />

<div class="shell" class:overlay-mode={overlayMode.enabled}>
	<TopChrome sinkKind={scenarioStore.active.sinkKind} />
	<DiagnosticsStrip />

	<div class="shell-body" class:left-collapsed={railState.leftCollapsed}>
		{#if !railState.leftCollapsed}
			<LeftRail
				{activeDomain}
				onDomainChange={handleDomainChange}
				onDrawerToggle={handleDrawerToggle}
			/>
		{:else}
			<button class="left-collapsed-tab" onclick={() => railState.toggleLeft()} title="Expand left rail ([)">
				<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
					<polyline points="9 18 15 12 9 6"></polyline>
				</svg>
			</button>
		{/if}

		<main class="content-viewport">
			{#if activeDomain === 'operate'}
				<div class="destination-operate">
					<OperateCanvas />
					<LayerVisibilityToolbar />
					<ModelCard onSwitchModel={handleSwitchModel} />
					<ObjectCountBadge />
					<div class="bottom-time-cursor">
						<span class="tc-label">LIVE</span>
						<div class="tc-track">
							<div class="tc-handle"></div>
						</div>
						<span class="tc-time">00:00:00 / --:--:--</span>
						<span class="tc-note">scrubber available in Phase 6+</span>
					</div>
				</div>
			{:else if activeDomain === 'inspect'}
				<InspectDestination />
			{:else if activeDomain === 'calibrate'}
				<CalibrateDestination />
			{:else if activeDomain === 'biometrics'}
				<BiometricsDestination />
			{:else}
				<EngageDestination onSwitchModel={handleSwitchModel} />
			{/if}
		</main>

		{#if activeDomain !== 'biometrics'}
			<RightRail
				{activeDomain}
				collapsed={railState.rightCollapsed}
				onToggleCollapse={() => railState.expandRight()}
			/>
		{/if}
	</div>
</div>

<SourcesDrawer open={activeDrawer === 'sources'} onClose={() => (activeDrawer = null)} />
<ModelsDrawer open={activeDrawer === 'models'} onClose={() => (activeDrawer = null)} />
<FovDrawer open={activeDrawer === 'fov'} onClose={() => (activeDrawer = null)} />
<SettingsDrawer open={activeDrawer === 'settings'} onClose={() => (activeDrawer = null)} />

<CommandPalette
	open={paletteOpen}
	onClose={() => (paletteOpen = false)}
	onSelectDestination={(d) => {
		handleDomainChange(d);
		paletteOpen = false;
	}}
	onSelectScenario={(id) => {
		scenarioStore.setScenario(id);
		paletteOpen = false;
	}}
/>

{#if cheatsheetOpen}
	<div
		class="cheatsheet-backdrop"
		onclick={() => (cheatsheetOpen = false)}
		role="presentation"
	>
		<div
			class="cheatsheet"
			onclick={(e) => e.stopPropagation()}
			role="dialog"
			aria-label="Keyboard shortcuts"
		>
			<h3 class="cheatsheet-title">Keyboard Shortcuts</h3>
			<div class="cheatsheet-grid">
				{#each keybindingManager.getBindings() as binding}
					<div class="cs-row">
						<kbd class="cs-key">{binding.combo}</kbd>
						<span class="cs-label">{binding.label}</span>
						<span
							class="cs-cat"
							style="color: {binding.category === 'destination'
								? '#f5a623'
								: binding.category === 'global'
									? '#a78bfa'
									: binding.category === 'layer'
										? '#60a5fa'
										: '#555'}"
						>{binding.category}</span>
					</div>
				{/each}
			</div>
		</div>
	</div>
{/if}

<style>
	:global(body) {
		margin: 0;
		padding: 0;
		background: #0e0e10;
		color: #d4d0c8;
		font-family:
			-apple-system,
			BlinkMacSystemFont,
			'Segoe UI',
			Roboto,
			'Helvetica Neue',
			sans-serif;
		-webkit-font-smoothing: antialiased;
	}

	.shell {
		display: flex;
		flex-direction: column;
		width: 100vw;
		height: 100vh;
		overflow: hidden;
		background: #0e0e10;
		transition: opacity 120ms ease;
	}

	/* Overlay Mode (Alt+Z) collapses non-canvas chrome to give the
	   composite canvas maximum viewport. Chrome stays at 75% opacity
	   so peripheral motion in the feed stays visible. */
	.shell.overlay-mode :global(.left-rail) {
		opacity: 0.75;
	}

	/* Left rail collapsed tab (Foxglove [ hotkey) */
	.left-collapsed-tab {
		width: 14px;
		flex-shrink: 0;
		background: #111113;
		border-right: 1px solid #27272a;
		display: flex;
		align-items: center;
		justify-content: center;
		color: #555;
		border: none;
		cursor: pointer;
		transition: color 100ms ease;
	}

	.left-collapsed-tab:hover {
		color: #f5a623;
	}

	.left-collapsed-tab svg {
		width: 12px;
		height: 12px;
	}

	.shell.overlay-mode :global(.right-rail) {
		display: none;
	}

	.shell.overlay-mode :global(.top-chrome) {
		opacity: 0.75;
	}

	.shell.overlay-mode :global(.top-chrome .selector-group) {
		display: none;
	}

	.shell-body {
		display: flex;
		flex: 1;
		min-height: 0;
	}

	.content-viewport {
		flex: 1;
		min-width: 0;
		overflow: hidden;
		display: flex;
		flex-direction: column;
		position: relative;
	}

	/* --- Operate --- */
	.destination-operate {
		flex: 1;
		position: relative;
		background: #06080c;
		display: flex;
		flex-direction: column;
	}

	.bottom-time-cursor {
		position: absolute;
		bottom: 0;
		left: 0;
		right: 0;
		height: 30px;
		background: rgba(17, 17, 19, 0.85);
		backdrop-filter: blur(6px);
		border-top: 1px solid #1d1d20;
		display: flex;
		align-items: center;
		gap: 12px;
		padding: 0 14px;
		z-index: 4;
		opacity: 0.6;
	}

	.tc-label {
		font-size: 10px;
		font-weight: 700;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #34d399;
		letter-spacing: 0.06em;
	}

	.tc-track {
		flex: 1;
		height: 4px;
		background: #1d1d20;
		border-radius: 2px;
		position: relative;
	}

	.tc-handle {
		position: absolute;
		right: 0;
		top: -3px;
		width: 10px;
		height: 10px;
		border-radius: 50%;
		background: #f5a623;
		box-shadow: 0 0 4px rgba(245, 166, 35, 0.6);
	}

	.tc-time {
		font-size: 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #d4d0c8;
	}

	.tc-note {
		font-size: 9px;
		font-style: italic;
		color: #555;
	}

	/* --- Inspect --- */
	.destination-inspect {
		flex: 1;
		overflow-y: auto;
		padding: 20px 24px;
		background: #0e0e10;
	}

	.inspect-header {
		margin-bottom: 20px;
	}

	.dest-title {
		margin: 0;
		font-size: 18px;
		font-weight: 600;
		color: #d4d0c8;
	}

	.dest-subtitle {
		margin: 4px 0 0;
		font-size: 12px;
		color: #666;
	}

	.inspect-grid {
		display: grid;
		grid-template-columns: repeat(auto-fit, minmax(400px, 1fr));
		gap: 16px;
	}

	.panel-card {
		background: #161618;
		border: 1px solid #27272a;
		border-radius: 8px;
		overflow: hidden;
		transition: border-color 200ms ease;
	}

	.panel-card:hover {
		border-color: #3a3a3e;
	}

	.panel-card-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding: 10px 14px;
		background: #1a1a1e;
		border-bottom: 1px solid #222;
	}

	.panel-card-title {
		font-size: 11px;
		font-weight: 600;
		color: #878787;
		text-transform: uppercase;
		letter-spacing: 0.04em;
	}

	.panel-card-dot {
		width: 6px;
		height: 6px;
		border-radius: 50%;
		background: #f5a623;
		opacity: 0.6;
	}

	.panel-card-body {
		padding: 12px 14px;
		min-height: 180px;
	}

	/* --- Calibrate Wizard --- */
	.destination-calibrate {
		flex: 1;
		overflow: hidden;
		display: flex;
	}

	.wizard-layout {
		display: flex;
		flex: 1;
		min-height: 0;
	}

	.wizard-steps {
		width: 200px;
		flex-shrink: 0;
		background: #141416;
		border-right: 1px solid #222;
		padding: 20px 0;
		display: flex;
		flex-direction: column;
		gap: 2px;
	}

	.step {
		display: flex;
		align-items: center;
		gap: 10px;
		padding: 10px 16px;
		cursor: pointer;
		transition: background 150ms ease;
	}

	.step:hover {
		background: #1e1e22;
	}

	.selected-step {
		background: #1e1e22;
		border-left: 3px solid #f5a623;
	}

	.step-num {
		width: 22px;
		height: 22px;
		display: flex;
		align-items: center;
		justify-content: center;
		border-radius: 50%;
		font-size: 11px;
		font-weight: 600;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		background: #222;
		color: #666;
	}

	.selected-step .step-num {
		background: #f5a623;
		color: #111;
	}

	.step-label {
		font-size: 12px;
		color: #878787;
	}

	.selected-step .step-label {
		color: #d4d0c8;
	}

	.step-dot {
		width: 8px;
		height: 8px;
		border-radius: 50%;
		margin-left: auto;
	}

	.step-dot.ok {
		background: #34d399;
	}
	.step-dot.pending {
		background: #ef4444;
	}

	.wizard-content {
		flex: 1;
		overflow-y: auto;
		padding: 20px 24px;
	}

	/* --- Engage --- */
	.destination-engage {
		flex: 1;
		overflow: hidden;
		display: grid;
		grid-template-rows: auto 1fr auto;
		background: #06080c;
	}

	.safety-interlock {
		background: #161618;
		border-bottom: 1px solid #27272a;
		padding: 12px 20px;
	}

	.interlock-title {
		margin: 0 0 10px;
		font-size: 11px;
		font-weight: 700;
		color: #878787;
		text-transform: uppercase;
		letter-spacing: 0.08em;
	}

	.interlock-grid {
		display: flex;
		gap: 12px;
		flex-wrap: wrap;
	}

	.interlock-light {
		display: flex;
		align-items: center;
		gap: 8px;
		padding: 6px 14px;
		background: #111113;
		border-radius: 5px;
		border: 1px solid #222;
	}

	.light-dot {
		width: 10px;
		height: 10px;
		border-radius: 50%;
	}

	.interlock-light.ok .light-dot {
		background: #34d399;
		box-shadow: 0 0 6px rgba(52, 211, 153, 0.4);
	}
	.interlock-light.warn .light-dot {
		background: #f5a623;
		box-shadow: 0 0 6px rgba(245, 166, 35, 0.3);
	}
	.interlock-light.err .light-dot {
		background: #ef4444;
		box-shadow: 0 0 6px rgba(239, 68, 68, 0.3);
	}
	.interlock-light.stale .light-dot {
		background: #444;
	}

	.light-label {
		font-size: 12px;
		font-weight: 500;
		color: #d4d0c8;
	}

	.interlock-status {
		margin: 10px 0 0;
		font-size: 11px;
		color: #ef4444;
		font-style: italic;
	}

	.engage-canvas-wrap {
		position: relative;
		min-height: 0;
	}

	.engage-panel-wrap {
		background: #161618;
		border-top: 1px solid #27272a;
		padding: 12px 16px;
		max-height: 240px;
		overflow-y: auto;
	}

	.disabled-panel {
		opacity: 0.4;
		pointer-events: none;
		filter: grayscale(0.5);
	}

	/* Cheatsheet */
	.cheatsheet-backdrop {
		position: fixed;
		inset: 0;
		background: rgba(0, 0, 0, 0.6);
		display: flex;
		justify-content: center;
		padding-top: 12vh;
		z-index: 100;
		backdrop-filter: blur(2px);
	}

	.cheatsheet {
		width: 480px;
		max-height: 480px;
		background: #1a1a1e;
		border: 1px solid #333;
		border-radius: 10px;
		box-shadow: 0 16px 48px rgba(0, 0, 0, 0.5);
		overflow: hidden;
		display: flex;
		flex-direction: column;
	}

	.cheatsheet-title {
		margin: 0;
		padding: 14px 18px;
		font-size: 14px;
		font-weight: 600;
		color: #d4d0c8;
		border-bottom: 1px solid #27272a;
	}

	.cheatsheet-grid {
		overflow-y: auto;
		padding: 8px 12px;
	}

	.cs-row {
		display: flex;
		align-items: center;
		gap: 12px;
		padding: 6px 8px;
		border-radius: 4px;
	}

	.cs-row:hover {
		background: #222;
	}

	.cs-key {
		min-width: 72px;
		font-size: 11px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		padding: 2px 8px;
		background: #222;
		color: #d4d0c8;
		border-radius: 3px;
		border: 1px solid #333;
		text-align: center;
	}

	.cs-label {
		flex: 1;
		font-size: 12px;
		color: #878787;
	}

	.cs-cat {
		font-size: 9px;
		font-weight: 600;
		text-transform: uppercase;
		letter-spacing: 0.04em;
	}
</style>
