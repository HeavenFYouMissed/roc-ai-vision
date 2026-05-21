<script lang="ts">
	import { CONTROLLER_DOMAINS, type ControllerDomain } from '$lib/domains/controller_matrix';
	import { trackPool, MAX_SLOTS } from '$lib/stores/track_pool.svelte';
	import { telemetrySocket } from '$lib/telemetry/telemetry_socket.svelte';
	import { engageState } from '$lib/stores/engage_state.svelte';
	import { profileStore } from '$lib/stores/profile_store.svelte';
	import { eventLog } from '$lib/stores/event_log.svelte';

	interface Props {
		activeDomain: ControllerDomain;
		onDomainChange: (domain: ControllerDomain) => void;
		onDrawerToggle: (drawer: 'models' | 'sources' | 'settings' | 'fov') => void;
	}

	let { activeDomain, onDomainChange, onDrawerToggle }: Props = $props();

	// Per-destination activity badges (Claude polish #9)
	const cameraOk = $derived(telemetrySocket.state === 'connected');
	const trackerOk = $derived(trackPool.activeCount > 0);

	// Engage activity: pulses when interlocks become ready (3 of 4 green, only ARM missing)
	const engageReady = $derived(
		cameraOk && trackerOk && !profileStore.isReplay && engageState.armed !== 'armed'
	);

	// Inspect activity: any track with hierarchy violation flag (status_flags bit 3 = 0x08)
	const inspectHasViolation = $derived.by(() => {
		void trackPool.presenceVersion;
		for (let s = 0; s < MAX_SLOTS; s++) {
			if (trackPool.presenceBitmap[s] !== 1) continue;
			if (trackPool.statusFlags[s] & 0x08) return true;
		}
		return false;
	});

	// Operate activity: any new event in event log within last 2 seconds (pulses subtly)
	const operateRecentActivity = $derived(eventLog.lastEvent !== null && Date.now() - eventLog.lastEvent.tsMs < 2000);

	function badgeFor(domain: ControllerDomain): { active: boolean; level: 'info' | 'ready' | 'warn'; tooltip: string } | null {
		if (domain === activeDomain) return null; // don't badge the active destination
		switch (domain) {
			case 'engage':
				if (engageReady) return { active: true, level: 'ready', tooltip: 'Ready to arm — 3 of 4 lights green' };
				break;
			case 'inspect':
				if (inspectHasViolation) return { active: true, level: 'warn', tooltip: 'Hierarchy violation detected' };
				break;
			case 'operate':
				if (operateRecentActivity) return { active: true, level: 'info', tooltip: 'Recent telemetry activity' };
				break;
			case 'calibrate':
				// no activity signal yet
				break;
			case 'biometrics':
				// no telemetry-driven activity signal — Biometrics destination
				// has its own self-contained data plane. Could badge on
				// "RECORDING" state in a future polish pass.
				break;
		}
		return null;
	}

	const DOMAIN_ICONS: Record<string, string> = {
		crosshair: 'M12 2v4m0 12v4M2 12h4m12 0h4m-5 0a3 3 0 1 1-6 0 3 3 0 0 1 6 0Z',
		activity: 'M22 12h-4l-3 9L9 3l-3 9H2',
		sliders: 'M4 21v-7m0-4V3m8 18v-9m0-4V3m8 18v-5m0-4V3M1 14h6M9 8h6M17 16h6',
		target: 'M12 2a10 10 0 1 0 0 20 10 10 0 0 0 0-20Zm0 4a6 6 0 1 0 0 12 6 6 0 0 0 0-12Zm0 4a2 2 0 1 0 0 4 2 2 0 0 0 0-4Z',
		// ECG-trace icon for Biometrics destination — visually signals
		// mouse-dynamics/biometric capture distinct from the 4 vision-pipeline
		// verbs. Classic QRS-complex silhouette: flat baseline, small
		// pre-spike, big up/down spike, recovery, flat tail. Starts AND ends
		// on the y=12 baseline so the line reads as a continuous ECG strip.
		pulse: 'M3 12h4l1-3 2 6 2-9 2 9 1-3h6'
	};
</script>

<nav class="left-rail" aria-label="Destination switcher">
	<div class="destinations">
		{#each CONTROLLER_DOMAINS as domain}
			{@const badge = badgeFor(domain.id)}
			<button
				class="rail-btn"
				class:active={activeDomain === domain.id}
				onclick={() => onDomainChange(domain.id)}
				title={badge?.tooltip ?? `${domain.label} (${domain.hotkey})`}
				aria-current={activeDomain === domain.id ? 'page' : undefined}
			>
				<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
					<path d={DOMAIN_ICONS[domain.icon]} />
				</svg>
				<span class="rail-label">{domain.label}</span>
				<span class="hotkey-hint">{domain.hotkey}</span>
				{#if badge}
					<span
						class="activity-badge"
						class:badge-ready={badge.level === 'ready'}
						class:badge-warn={badge.level === 'warn'}
						class:badge-info={badge.level === 'info'}
					></span>
				{/if}
			</button>
		{/each}
	</div>

	<div class="rail-separator"></div>

	<div class="drawers">
		<button class="rail-btn drawer-btn" onclick={() => onDrawerToggle('sources')} title="Sources — Camera / Capture / File / Stream">
			<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
				<path d="M23 7l-7 5 7 5V7zM14 5H3a2 2 0 0 0-2 2v10a2 2 0 0 0 2 2h11a2 2 0 0 0 2-2V7a2 2 0 0 0-2-2z" />
			</svg>
			<span class="rail-label">Sources</span>
		</button>
		<button class="rail-btn drawer-btn" onclick={() => onDrawerToggle('models')} title="Models — ONNX manifest browser">
			<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
				<path d="M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16Z" />
				<polyline points="3.27 6.96 12 12.01 20.73 6.96" />
				<line x1="12" y1="22.08" x2="12" y2="12" />
			</svg>
			<span class="rail-label">Models</span>
		</button>
		<button class="rail-btn drawer-btn" onclick={() => onDrawerToggle('fov')} title="FOV — Inference crop window">
			<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
				<!-- Crosshair-in-square icon: scope reticle inside the source frame. -->
				<rect x="3" y="3" width="18" height="18" rx="2" />
				<line x1="12" y1="7" x2="12" y2="17" />
				<line x1="7" y1="12" x2="17" y2="12" />
				<circle cx="12" cy="12" r="3" />
			</svg>
			<span class="rail-label">FOV</span>
		</button>
		<button class="rail-btn drawer-btn" onclick={() => onDrawerToggle('settings')} title="Settings — Profile / Hotkeys / Preferences">
			<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
				<circle cx="12" cy="12" r="3" />
				<path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83-2.83l.06-.06A1.65 1.65 0 0 0 4.68 15a1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 2.83-2.83l.06.06A1.65 1.65 0 0 0 9 4.68a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 2.83l-.06.06A1.65 1.65 0 0 0 19.4 9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1Z" />
			</svg>
			<span class="rail-label">Settings</span>
		</button>
	</div>
</nav>

<style>
	.left-rail {
		display: flex;
		flex-direction: column;
		width: 56px;
		background: #18181b;
		border-right: 1px solid #2a2a2e;
		padding: 6px 0;
		overflow: hidden;
		transition: width 200ms ease;
		z-index: 10;
	}

	.destinations {
		display: flex;
		flex-direction: column;
		gap: 2px;
		padding: 0 6px;
	}

	.rail-separator {
		margin: 10px 12px;
		height: 1px;
		background: #2a2a2e;
	}

	.drawers {
		display: flex;
		flex-direction: column;
		gap: 2px;
		padding: 0 6px;
		margin-top: auto;
	}

	.rail-btn {
		position: relative;
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
		gap: 3px;
		width: 44px;
		height: 44px;
		padding: 6px 0 2px;
		background: transparent;
		border: none;
		border-radius: 8px;
		color: #878787;
		cursor: pointer;
		transition: all 150ms ease;
	}

	.rail-btn:hover {
		background: #27272a;
		color: #d4d0c8;
	}

	.rail-btn.active {
		background: #27272a;
		color: #f5a623;
		box-shadow: inset 3px 0 0 #f5a623;
	}

	.rail-btn svg {
		width: 20px;
		height: 20px;
		flex-shrink: 0;
	}

	.rail-label {
		font-size: 8px;
		font-weight: 500;
		letter-spacing: 0.02em;
		text-transform: uppercase;
		white-space: nowrap;
		opacity: 0.85;
	}

	.hotkey-hint {
		position: absolute;
		top: 3px;
		right: 5px;
		font-size: 9px;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		color: #555;
		pointer-events: none;
	}

	.rail-btn.active .hotkey-hint {
		color: #f5a623;
		opacity: 0.5;
	}

	/* Activity badge (Claude polish #9) — small pulsing dot top-left of inactive icon */
	.activity-badge {
		position: absolute;
		top: 4px;
		left: 4px;
		width: 7px;
		height: 7px;
		border-radius: 50%;
		pointer-events: none;
		animation: badge-pulse 1.6s ease-in-out infinite;
	}

	.activity-badge.badge-ready {
		background: #34d399;
		box-shadow: 0 0 6px rgba(52, 211, 153, 0.7);
	}

	.activity-badge.badge-warn {
		background: #ef4444;
		box-shadow: 0 0 6px rgba(239, 68, 68, 0.7);
	}

	.activity-badge.badge-info {
		background: #00f0ff;
		box-shadow: 0 0 5px rgba(0, 240, 255, 0.6);
	}

	@keyframes badge-pulse {
		0%, 100% { opacity: 1; transform: scale(1); }
		50% { opacity: 0.45; transform: scale(0.85); }
	}

	.drawer-btn {
		opacity: 0.7;
	}

	.drawer-btn:hover {
		opacity: 1;
	}
</style>
