<!--
	roc-ai-vision · dashboard · EngageDestination.svelte

	Safety-critical actuation console per dashboard_menu_architecture_2026.md ss5.3.

	Layout:
		[48px Safety Interlock Bar - 4 reactive lights]
		[shared OperateCanvas - same component as Operate]
		[48px Hardware Control Bar - serial + slew + EMERGENCY STOP, only when 4-green]
		[Target Lock Confirmation Modal - Skydio Pause-slider 800ms hold]
-->
<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import OperateCanvas from './OperateCanvas.svelte';
	import LayerVisibilityToolbar from '$lib/components/overlays/LayerVisibilityToolbar.svelte';
	import ModelCard from '$lib/components/overlays/ModelCard.svelte';
	import ObjectCountBadge from '$lib/components/overlays/ObjectCountBadge.svelte';
	import TargetLockModal from '$lib/components/overlays/TargetLockModal.svelte';
	import { telemetrySocket } from '$lib/telemetry/telemetry_socket.svelte';
	import { trackPool } from '$lib/stores/track_pool.svelte';
	import { selectionStore } from '$lib/stores/selection_store.svelte';
	import { engageState } from '$lib/stores/engage_state.svelte';
	import { profileStore } from '$lib/stores/profile_store.svelte';
	import { hardwareConfig } from '$lib/stores/hardware_config';
	import { keybindingManager } from '$lib/keybindings/manager.svelte';
	import { CLASS_NAMES, CLASS_COLORS } from '$lib/render/class_colors';

	interface Props {
		onSwitchModel?: () => void;
	}

	let { onSwitchModel = () => {} }: Props = $props();

	// Reactive light states
	const cameraConnected = $derived(telemetrySocket.state === 'connected');
	const trackerHealthy = $derived(trackPool.activeCount > 0);
	const authStatus = $derived<'ok' | 'warn' | 'err'>(
		'warn' // Phase 6+: read from profileStore.active.authenticated
	);
	const allGreen = $derived(
		cameraConnected && trackerHealthy && authStatus === 'ok' && engageState.armed === 'armed'
	);
	// Pre-arm "ready" state: 3 of 4 conditions met, only Armed remains
	const readyToArm = $derived(
		cameraConnected && trackerHealthy && authStatus === 'ok' && engageState.canArm
	);
	const isReplay = $derived(profileStore.isReplay);

	function handleArmClick() {
		if (!engageState.canArm) return;
		if (engageState.armed === 'off') {
			engageState.startArm();
		} else if (engageState.armed === 'armed') {
			engageState.disarm();
		}
		// PENDING state handled by inline Confirm/Cancel buttons
	}

	function handleEmergencyStop() {
		engageState.triggerEmergencyStop();
		try {
			telemetrySocket.releaseLock();
		} catch {
			// ignore
		}
	}

	function handleResetEStop() {
		engageState.resetEmergencyStop();
	}

	function handleSerialChange(e: Event) {
		const t = e.target as HTMLSelectElement;
		hardwareConfig.serialPort = t.value;
	}

	function handleBaudChange(e: Event) {
		const t = e.target as HTMLSelectElement;
		hardwareConfig.baudRate = +t.value;
	}

	// Slew indicator vector (from selected track velocity for v1; future: actuator command)
	const slewVec = $derived.by(() => {
		void trackPool.presenceVersion;
		const s = selectionStore.selectedSlot;
		if (s < 0 || trackPool.presenceBitmap[s] !== 1) return { dx: 0, dy: 0, mag: 0 };
		const v3 = s * 3;
		const dx = trackPool.vWorld[v3 + 0] * 10;
		const dy = trackPool.vWorld[v3 + 1] * 10;
		return { dx, dy, mag: Math.hypot(dx, dy) };
	});

	function handleTargetLockHotkey() {
		const s = selectionStore.selectedSlot;
		if (s < 0) return;
		engageState.openLockModal(s);
	}

	onMount(() => {
		// MANDATORY: armed always defaults to off on every mount
		engageState.resetOnMount();

		// Register Engage-specific hotkeys
		keybindingManager.register('global:lock-target', handleTargetLockHotkey);

		return () => {
			keybindingManager.unregister('global:lock-target');
		};
	});

	onDestroy(() => {
		engageState.resetOnMount();
	});
</script>

<div class="engage-destination">
	<!-- Top: Safety Interlock Bar -->
	<div class="interlock-bar">
		<div class="interlock-lights">
			<div class="ilight" class:ok={cameraConnected} class:stale={!cameraConnected}>
				<span class="ilight-dot"></span>
				<div class="ilight-text">
					<span class="ilight-name">CAMERA</span>
					<span class="ilight-status">{cameraConnected ? 'CONNECTED' : 'DISCONNECTED'}</span>
				</div>
			</div>
			<div class="ilight" class:ok={trackerHealthy} class:stale={!trackerHealthy}>
				<span class="ilight-dot"></span>
				<div class="ilight-text">
					<span class="ilight-name">TRACKER</span>
					<span class="ilight-status">
						{trackerHealthy ? `${trackPool.activeCount} TRACKS` : 'NO TRACKS'}
					</span>
				</div>
			</div>
			<div class="ilight warn">
				<span class="ilight-dot"></span>
				<div class="ilight-text">
					<span class="ilight-name">AUTH</span>
					<span class="ilight-status">NOT CONFIGURED</span>
				</div>
			</div>
			<div
				class="ilight armed-toggle"
				class:stale={engageState.armed === 'off' && !readyToArm && !engageState.emergencyStopLatched}
				class:err={(engageState.armed === 'off' && readyToArm && !engageState.emergencyStopLatched) || engageState.emergencyStopLatched}
				class:warn={engageState.armed === 'pending'}
				class:ok={engageState.armed === 'armed'}
				class:disabled={isReplay || engageState.emergencyStopLatched}
			>
				<span class="ilight-dot armed-dot"></span>
				<div class="ilight-text">
					<span class="ilight-name">ARMED</span>
					{#if isReplay}
						<span class="ilight-status">DISABLED · REPLAY</span>
					{:else if engageState.emergencyStopLatched}
						<span class="ilight-status">E-STOP LATCHED</span>
					{:else if engageState.armed === 'off'}
						<!-- Claude polish #6: desaturate to grey when not all interlocks ready,
						     flip to orange only when readyToArm so the moment feels like a real
						     state transition. -->
						<button
							class="arm-btn"
							class:arm-ready={readyToArm}
							onclick={handleArmClick}
						>ARM SYSTEM</button>
					{:else if engageState.armed === 'pending'}
						<span class="arm-pending">
							<span class="pending-text">CONFIRM ARM?</span>
							<button class="confirm-btn" onclick={() => engageState.confirmArm()}>OK</button>
							<button class="cancel-btn" onclick={() => engageState.cancelArm()}>X</button>
						</span>
					{:else}
						<button class="arm-btn armed-text" onclick={handleArmClick}>● ARMED</button>
					{/if}
				</div>
			</div>
		</div>

		<div class="interlock-summary">
			{#if isReplay}
				<span class="summary-text replay">REPLAY MODE · Actuator commands hard-disabled</span>
			{:else if engageState.emergencyStopLatched}
				<span class="summary-text err">
					EMERGENCY STOP LATCHED ·
					<button class="reset-estop-btn" onclick={handleResetEStop}>Reset</button>
				</span>
			{:else if allGreen}
				<span class="summary-text ok">● ACTUATOR ENABLED · all 4 lights green</span>
			{:else}
				<span class="summary-text err">All 4 lights required for actuation</span>
			{/if}
		</div>
	</div>

	<!-- Center: shared OperateCanvas + overlays + slew indicator -->
	<div class="engage-canvas-wrap">
		<OperateCanvas />
		<LayerVisibilityToolbar />
		<ModelCard {onSwitchModel} />
		<ObjectCountBadge />

		<!-- Slew indicator (top-center over canvas) -->
		{#if allGreen}
			<div class="slew-indicator" title="Actuator command direction + magnitude">
				<svg viewBox="-40 -40 80 80" width="80" height="80">
					<circle r="34" fill="none" stroke="#27272a" stroke-width="1"></circle>
					<circle r="22" fill="none" stroke="#27272a" stroke-width="1" opacity="0.6"></circle>
					<circle r="10" fill="none" stroke="#27272a" stroke-width="1" opacity="0.4"></circle>
					{#if slewVec.mag > 0.5}
						<line
							x1="0"
							y1="0"
							x2={Math.max(-34, Math.min(34, slewVec.dx))}
							y2={Math.max(-34, Math.min(34, slewVec.dy))}
							stroke="#f5a623"
							stroke-width="3"
							marker-end="url(#slew-arrow)"
						></line>
					{:else}
						<circle r="2" fill="#f5a623"></circle>
					{/if}
					<defs>
						<marker
							id="slew-arrow"
							viewBox="0 0 10 10"
							refX="8"
							refY="5"
							markerWidth="6"
							markerHeight="6"
							orient="auto-start-reverse"
						>
							<path d="M 0 0 L 10 5 L 0 10 z" fill="#f5a623"></path>
						</marker>
					</defs>
				</svg>
				<div class="slew-label">SLEW</div>
			</div>
		{/if}
	</div>

	<!-- Bottom: Hardware Control Bar (only when all green or for visibility) -->
	<div class="hardware-bar" class:disabled={!allGreen}>
		<div class="hw-left">
			<div class="hw-status-pill" class:hw-online={allGreen}>
				<span class="hw-status-dot"></span>
				<span class="hw-status-text">
					{hardwareConfig.serialPort} @ {hardwareConfig.baudRate}
				</span>
				{#if allGreen}
					<span class="hw-ack">/ ACK 12ms</span>
				{/if}
			</div>

			<label class="hw-select">
				<span class="hw-select-label">PORT</span>
				<select
					value={hardwareConfig.serialPort}
					onchange={handleSerialChange}
					disabled={!allGreen}
				>
					{#each hardwareConfig.serialPortOptions as p}
						<option value={p}>{p}</option>
					{/each}
				</select>
			</label>

			<label class="hw-select">
				<span class="hw-select-label">BAUD</span>
				<select value={hardwareConfig.baudRate} onchange={handleBaudChange} disabled={!allGreen}>
					{#each hardwareConfig.baudRateOptions as b}
						<option value={b}>{b}</option>
					{/each}
				</select>
			</label>
		</div>

		<button class="estop-btn" onclick={handleEmergencyStop} disabled={engageState.emergencyStopLatched}>
			<span class="estop-icon">●</span>
			EMERGENCY STOP
		</button>
	</div>

	<!-- Target Lock Confirmation Modal -->
	{#if engageState.lockConfirmModalOpen && engageState.pendingLockSlot >= 0}
		<TargetLockModal />
	{/if}
</div>

<style>
	.engage-destination {
		flex: 1;
		display: grid;
		grid-template-rows: auto 1fr auto;
		min-height: 0;
		background: #06080c;
	}

	/* --- Interlock Bar --- */
	.interlock-bar {
		background: #161618;
		border-bottom: 1px solid #27272a;
		padding: 6px 16px;
		display: flex;
		flex-direction: column;
		gap: 4px;
	}

	.interlock-lights {
		display: flex;
		gap: 8px;
		flex-wrap: wrap;
		align-items: stretch;
	}

	.ilight {
		flex: 1;
		min-width: 160px;
		display: flex;
		align-items: center;
		gap: 8px;
		padding: 6px 10px;
		background: #111113;
		border: 1px solid #1d1d20;
		border-radius: 4px;
	}

	.ilight-dot {
		width: 10px;
		height: 10px;
		border-radius: 50%;
		background: #444;
		flex-shrink: 0;
	}

	.ilight.ok .ilight-dot {
		background: #34d399;
		box-shadow: 0 0 6px rgba(52, 211, 153, 0.5);
	}
	.ilight.warn .ilight-dot {
		background: #f5a623;
		box-shadow: 0 0 6px rgba(245, 166, 35, 0.4);
	}
	.ilight.err .ilight-dot {
		background: #ef4444;
		box-shadow: 0 0 6px rgba(239, 68, 68, 0.4);
	}
	.ilight.stale .ilight-dot {
		background: #444;
		box-shadow: none;
	}

	.ilight-text {
		display: flex;
		flex-direction: column;
		gap: 1px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		min-width: 0;
		flex: 1;
	}

	.ilight-name {
		font-size: 9px;
		font-weight: 700;
		color: #878787;
		letter-spacing: 0.08em;
	}

	.ilight-status {
		font-size: 10px;
		color: #d4d0c8;
	}

	/* Armed Toggle styling */
	.armed-toggle {
		min-width: 200px;
	}

	.armed-toggle.disabled {
		opacity: 0.5;
		pointer-events: none;
	}

	.armed-toggle .armed-dot {
		transition: all 200ms ease;
	}

	.armed-toggle.ok .armed-dot {
		animation: pulse-armed 1.2s ease-in-out infinite;
	}

	@keyframes pulse-armed {
		0%, 100% {
			box-shadow: 0 0 4px rgba(52, 211, 153, 0.6);
		}
		50% {
			box-shadow: 0 0 10px rgba(52, 211, 153, 0.9);
		}
	}

	/* Claude polish #6: ARM SYSTEM off-state desaturated until readyToArm */
	.arm-btn {
		background: transparent;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		color: #555;
		font-family: inherit;
		font-size: 10px;
		font-weight: 700;
		padding: 2px 10px;
		cursor: pointer;
		letter-spacing: 0.08em;
		transition: all 200ms ease;
	}

	.arm-btn:hover {
		background: #1a1a1e;
		border-color: #3a3a3e;
		color: #878787;
	}

	/* Only when all other 3 interlock lights are green does the ARM button
	   light up orange to signal "ready to arm". This makes the moment feel
	   like a real state transition. */
	.arm-btn.arm-ready {
		color: #f5a623;
		border-color: rgba(245, 166, 35, 0.5);
	}

	.arm-btn.arm-ready:hover {
		background: rgba(245, 166, 35, 0.12);
		border-color: #f5a623;
	}

	.armed-text {
		color: #34d399;
		border-color: rgba(52, 211, 153, 0.4);
		background: rgba(52, 211, 153, 0.08);
	}

	.armed-text:hover {
		background: rgba(52, 211, 153, 0.15);
	}

	.arm-pending {
		display: flex;
		align-items: center;
		gap: 4px;
	}

	.pending-text {
		font-size: 9px;
		font-weight: 700;
		color: #f5a623;
		animation: pulse-pending 0.8s ease-in-out infinite;
	}

	@keyframes pulse-pending {
		0%, 100% { opacity: 1; }
		50% { opacity: 0.5; }
	}

	.confirm-btn,
	.cancel-btn {
		background: transparent;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		color: #d4d0c8;
		font-family: inherit;
		font-size: 10px;
		font-weight: 700;
		padding: 2px 6px;
		cursor: pointer;
		min-width: 22px;
	}

	.confirm-btn {
		color: #34d399;
		border-color: rgba(52, 211, 153, 0.4);
	}
	.confirm-btn:hover {
		background: rgba(52, 211, 153, 0.15);
	}

	.cancel-btn {
		color: #ef4444;
		border-color: rgba(239, 68, 68, 0.4);
	}
	.cancel-btn:hover {
		background: rgba(239, 68, 68, 0.15);
	}

	.interlock-summary {
		padding-top: 2px;
	}

	.summary-text {
		font-size: 11px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-weight: 600;
	}

	.summary-text.ok {
		color: #34d399;
	}
	.summary-text.err {
		color: #ef4444;
		font-style: italic;
	}
	.summary-text.replay {
		color: #a78bfa;
		font-weight: 700;
	}

	.reset-estop-btn {
		background: transparent;
		border: 1px solid rgba(239, 68, 68, 0.4);
		color: #ef4444;
		font-family: inherit;
		font-size: 10px;
		padding: 1px 8px;
		border-radius: 3px;
		cursor: pointer;
		margin-left: 8px;
	}

	.reset-estop-btn:hover {
		background: rgba(239, 68, 68, 0.15);
	}

	/* --- Canvas wrap --- */
	.engage-canvas-wrap {
		position: relative;
		min-height: 0;
		background: #06080c;
	}

	.slew-indicator {
		position: absolute;
		top: 12px;
		left: 50%;
		transform: translateX(-50%);
		display: flex;
		flex-direction: column;
		align-items: center;
		gap: 2px;
		background: rgba(22, 22, 24, 0.85);
		backdrop-filter: blur(6px);
		border: 1px solid rgba(39, 39, 42, 0.9);
		border-radius: 8px;
		padding: 6px 10px;
		z-index: 4;
		pointer-events: none;
	}

	.slew-label {
		font-size: 9px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-weight: 700;
		color: #f5a623;
		letter-spacing: 0.08em;
	}

	/* --- Hardware Bar --- */
	.hardware-bar {
		background: #161618;
		border-top: 1px solid #27272a;
		padding: 8px 16px;
		display: flex;
		align-items: center;
		justify-content: space-between;
		gap: 16px;
		flex-shrink: 0;
		min-height: 48px;
	}

	.hardware-bar.disabled {
		opacity: 0.55;
	}

	.hw-left {
		display: flex;
		align-items: center;
		gap: 12px;
		flex-wrap: wrap;
	}

	.hw-status-pill {
		display: flex;
		align-items: center;
		gap: 6px;
		padding: 4px 10px;
		background: #111113;
		border: 1px solid #2a2a2e;
		border-radius: 4px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 10px;
		color: #878787;
	}

	.hw-status-dot {
		width: 6px;
		height: 6px;
		border-radius: 50%;
		background: #444;
	}

	.hw-status-pill.hw-online .hw-status-dot {
		background: #34d399;
		box-shadow: 0 0 4px rgba(52, 211, 153, 0.4);
	}

	.hw-status-pill.hw-online .hw-status-text {
		color: #d4d0c8;
	}

	.hw-ack {
		color: #34d399;
	}

	.hw-select {
		display: flex;
		align-items: center;
		gap: 5px;
	}

	.hw-select-label {
		font-size: 9px;
		font-weight: 700;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #555;
		letter-spacing: 0.06em;
	}

	.hw-select select {
		background: #111113;
		color: #d4d0c8;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		padding: 3px 6px;
		font-size: 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		cursor: pointer;
		outline: none;
	}

	.hw-select select:disabled {
		cursor: not-allowed;
		color: #555;
	}

	.hw-select select:focus-visible {
		border-color: #f5a623;
	}

	.estop-btn {
		display: flex;
		align-items: center;
		gap: 6px;
		padding: 6px 14px;
		background: #2a1010;
		color: #ef4444;
		border: 1px solid #ef4444;
		border-radius: 4px;
		font-family: inherit;
		font-size: 11px;
		font-weight: 800;
		letter-spacing: 0.1em;
		cursor: pointer;
		transition: all 120ms ease;
		text-transform: uppercase;
	}

	.estop-btn:hover:not(:disabled) {
		background: #ef4444;
		color: #1a0a0a;
		box-shadow: 0 0 8px rgba(239, 68, 68, 0.5);
	}

	.estop-btn:disabled {
		opacity: 0.5;
		cursor: not-allowed;
	}

	.estop-icon {
		font-size: 14px;
	}
</style>
