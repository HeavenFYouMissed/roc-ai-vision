<!--
	roc-ai-vision · dashboard · TopChrome.svelte

	42px pinned masthead. Brand left, Scenario+Profile dropdowns center,
	LIVE/REPLAY badge + SINK badge right. Reads from scenarioStore +
	profileStore + active profile to render correctly.
-->
<script lang="ts">
	import { scenarioStore } from '$lib/profiles/scenario_store.svelte';
	import { SCENARIOS } from '$lib/profiles/scenarios';
	import { profileStore, PROFILES } from '$lib/stores/profile_store.svelte';
	import { SINK_LABELS, SINK_COLORS } from '$lib/render/class_colors';

	interface Props {
		sinkKind?: string;
	}

	let { sinkKind = 'noop' }: Props = $props();

	const sinkLabel = $derived(SINK_LABELS[sinkKind] ?? sinkKind.toUpperCase());
	const sinkColor = $derived(SINK_COLORS[sinkKind] ?? '#6b6b6f');
	const liveLabel = $derived(profileStore.isReplay ? 'REPLAY' : 'LIVE');
	const isReplay = $derived(profileStore.isReplay);
</script>

<header class="top-chrome">
	<div class="brand-group">
		<span class="brand-name">ROC</span>
		<span class="brand-product">AI Vision</span>
		<span class="brand-version">v2.0-alpha</span>
	</div>

	<div class="selector-group">
		<label class="chrome-select">
			<span class="select-label">SCENARIO</span>
			<select
				value={scenarioStore.activeScenarioId}
				onchange={(e) => scenarioStore.setScenario(e.currentTarget.value)}
			>
				{#each SCENARIOS as s}
					<option value={s.id}>{s.label}</option>
				{/each}
			</select>
		</label>
		<label class="chrome-select">
			<span class="select-label">PROFILE</span>
			<select
				value={profileStore.activeProfileId}
				onchange={(e) => profileStore.setProfile(e.currentTarget.value)}
				title={profileStore.active.notes}
			>
				{#each PROFILES as p}
					<option value={p.id}>{p.label}</option>
				{/each}
			</select>
		</label>
	</div>

	<div class="badge-group">
		<span class="badge badge-live" class:badge-replay={isReplay}>
			<span class="badge-dot"></span>{liveLabel}
		</span>
		<span class="badge badge-sink" style="--sink-color: {sinkColor}">
			SINK: {sinkLabel}
		</span>
	</div>
</header>

<style>
	.top-chrome {
		display: flex;
		align-items: center;
		justify-content: space-between;
		gap: 16px;
		height: 42px;
		padding: 0 16px;
		background: #18181b;
		border-bottom: 1px solid #2a2a2e;
		z-index: 20;
		flex-shrink: 0;
	}

	.brand-group {
		display: flex;
		align-items: baseline;
		gap: 6px;
		flex-shrink: 0;
	}

	.brand-name {
		font-size: 15px;
		font-weight: 800;
		color: #f5a623;
		letter-spacing: 0.08em;
	}

	.brand-product {
		font-size: 13px;
		font-weight: 500;
		color: #d4d0c8;
	}

	.brand-version {
		font-size: 10px;
		font-weight: 400;
		color: #666;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
	}

	.selector-group {
		display: flex;
		align-items: center;
		gap: 16px;
		flex: 1;
		justify-content: center;
		min-width: 0;
	}

	.chrome-select {
		display: flex;
		align-items: center;
		gap: 6px;
		font-size: 12px;
		color: #878787;
	}

	.select-label {
		font-size: 9px;
		font-weight: 700;
		text-transform: uppercase;
		letter-spacing: 0.08em;
		color: #555;
		flex-shrink: 0;
	}

	.chrome-select select {
		background: #111113;
		color: #d4d0c8;
		border: 1px solid #2a2a2e;
		border-radius: 4px;
		padding: 4px 8px;
		font-size: 11px;
		font-family: inherit;
		max-width: 280px;
		cursor: pointer;
		outline: none;
		transition: border-color 150ms ease;
	}

	.chrome-select select:hover {
		border-color: #3a3a3e;
	}

	.chrome-select select:focus-visible {
		border-color: #f5a623;
	}

	.badge-group {
		display: flex;
		align-items: center;
		gap: 8px;
		flex-shrink: 0;
	}

	.badge {
		display: inline-flex;
		align-items: center;
		gap: 5px;
		padding: 3px 9px;
		font-size: 10px;
		font-weight: 700;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		letter-spacing: 0.06em;
		border-radius: 3px;
		text-transform: uppercase;
	}

	.badge-dot {
		width: 6px;
		height: 6px;
		border-radius: 50%;
		background: currentColor;
		box-shadow: 0 0 4px currentColor;
	}

	.badge-live {
		background: rgba(52, 211, 153, 0.12);
		color: #34d399;
		border: 1px solid rgba(52, 211, 153, 0.3);
	}

	.badge-replay {
		background: rgba(167, 139, 250, 0.12) !important;
		color: #a78bfa !important;
		border-color: rgba(167, 139, 250, 0.3) !important;
		animation: pulse-replay 1.6s ease-in-out infinite;
	}

	@keyframes pulse-replay {
		0%, 100% { opacity: 1; }
		50% { opacity: 0.7; }
	}

	.badge-sink {
		background: color-mix(in srgb, var(--sink-color) 8%, transparent);
		color: var(--sink-color, #6b6b6f);
		border: 1px solid color-mix(in srgb, var(--sink-color) 25%, transparent);
	}
</style>
