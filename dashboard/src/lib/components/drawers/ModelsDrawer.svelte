<!--
	roc-ai-vision · dashboard · drawers/ModelsDrawer.svelte

	Real model browser backed by the orchestrator's Phase 4 REST API. The
	hardcoded mock list that originally lived here has been removed — the
	drawer now reflects whatever .yaml manifests exist in the orchestrator's
	models_dir. Clicking "Switch" actually swaps the running InferenceEngine.

	UX notes:
	  * If the orchestrator is offline (fetch fails) we show a banner with
	    the underlying network error AND fall back to displaying nothing
	    rather than confusing the operator with stale data.
	  * The active model is highlighted with the same cyan/orange tokens
	    used by the rest of the dashboard.
	  * Models flagged loadable=false (missing .onnx alongside the .yaml, or
	    manifest parse error) are shown but the "Switch" button is disabled,
	    with the status_note printed beneath the card. This is the path the
	    operator uses to debug their models/ folder.
	  * While a swap is in-flight the targeted card shows a spinner and the
	    drawer becomes input-disabled (one swap at a time per spec).
-->
<script lang="ts">
	import { onMount } from 'svelte';
	import { modelStore, type ModelInfo } from '$lib/stores/model_store.svelte';

	interface Props {
		open: boolean;
		onClose: () => void;
	}

	let { open, onClose }: Props = $props();

	let search = $state('');

	const filtered = $derived(
		search.length === 0
			? modelStore.models
			: modelStore.models.filter((m) =>
				m.name.toLowerCase().includes(search.toLowerCase()) ||
				m.purpose.toLowerCase().includes(search.toLowerCase()) ||
				m.head.toLowerCase().includes(search.toLowerCase())
			)
	);

	// Refresh whenever the drawer opens — keeps the list in sync with disk
	// changes (operator dropped new .onnx files in while the dashboard was
	// already running).
	$effect(() => {
		if (open) {
			void modelStore.refresh();
		}
	});

	onMount(() => {
		void modelStore.refresh();
	});

	async function onSwitch(m: ModelInfo): Promise<void> {
		if (!m.loadable) return;
		if (modelStore.switching) return;
		if (m.active) return;
		await modelStore.switchTo(m.name);
	}
</script>

{#if open}
	<div class="drawer-backdrop" onclick={onClose} role="presentation">
		<aside class="drawer" onclick={(e) => e.stopPropagation()} role="dialog" aria-label="Models">
			<div class="drawer-header">
				<div>
					<h3 class="drawer-title">Models</h3>
					<div class="drawer-sub">
						{modelStore.readyCount} ready · {modelStore.brokenCount} broken
						{#if modelStore.active}
							· active: <span class="drawer-active-name">{modelStore.active.name}</span>
							<span class="drawer-active-ep">[{modelStore.active.ep}]</span>
						{/if}
					</div>
				</div>
				<button class="drawer-close" onclick={onClose} aria-label="Close">
					<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M18 6L6 18M6 6l12 12" /></svg>
				</button>
			</div>

			<div class="drawer-search">
				<input
					type="text"
					bind:value={search}
					placeholder="Filter models (name / purpose / head)..."
					spellcheck="false"
				/>
				<button
					class="refresh-btn"
					onclick={() => modelStore.refresh()}
					disabled={modelStore.loading}
					title="Re-scan models directory"
				>
					{modelStore.loading ? '⟳ scanning…' : '⟳ refresh'}
				</button>
			</div>

			{#if modelStore.lastError}
				<div class="drawer-banner banner-error">
					<strong>Orchestrator offline:</strong> {modelStore.lastError}
				</div>
			{/if}

			{#if modelStore.lastSwitch && !modelStore.lastSwitch.ok}
				<div class="drawer-banner banner-error">
					<strong>Swap failed:</strong> {modelStore.lastSwitch.error}
				</div>
			{/if}

			<div class="drawer-body">
				{#if modelStore.loading && filtered.length === 0}
					<div class="drawer-empty">Scanning models directory…</div>
				{:else if filtered.length === 0}
					<div class="drawer-empty">
						{search
							? `No models match "${search}".`
							: 'No .yaml manifests found in the models directory.'}
					</div>
				{:else}
					{#each filtered as model (model.name + model.manifestPath)}
						{@const isSwitching = modelStore.switching === model.name}
						<div
							class="model-card"
							class:card-active={model.active}
							class:card-broken={!model.loadable}
						>
							<div class="model-top">
								<span class="model-name">{model.name}</span>
								<span
									class="model-status"
									class:active={model.active}
									class:ready={model.loadable && !model.active}
									class:broken={!model.loadable}
								>
									{#if model.active}
										Active
									{:else if model.loadable}
										Ready
									{:else}
										Broken
									{/if}
								</span>
							</div>
							<div class="model-meta">
								<span>{model.purpose || 'unknown'}</span>
								<span class="meta-sep">·</span>
								<span>{model.head || '—'}</span>
								<span class="meta-sep">·</span>
								<span>{model.inputW}×{model.inputH}</span>
								{#if model.numClasses > 0}
									<span class="meta-sep">·</span>
									<span>{model.numClasses} cls</span>
								{/if}
							</div>
							{#if !model.loadable || isSwitching}
								<div class="model-status-note">
									{isSwitching ? 'Loading + JIT-compiling…' : model.statusNote}
								</div>
							{/if}
							<div class="model-actions">
								<button
									class="switch-btn"
									disabled={!model.loadable || model.active || modelStore.switching !== null}
									onclick={() => onSwitch(model)}
								>
									{#if model.active}
										✓ Loaded
									{:else if isSwitching}
										Switching…
									{:else}
										Switch
									{/if}
								</button>
							</div>
						</div>
					{/each}
				{/if}
			</div>
		</aside>
	</div>
{/if}

<style>
	.drawer-backdrop {
		position: fixed;
		inset: 0;
		background: rgba(0, 0, 0, 0.4);
		z-index: 50;
	}

	.drawer {
		position: fixed;
		top: 0;
		left: 56px;
		width: 380px;
		height: 100vh;
		background: #161618;
		border-right: 1px solid #27272a;
		display: flex;
		flex-direction: column;
		box-shadow: 8px 0 24px rgba(0, 0, 0, 0.3);
		animation: slideIn 150ms ease;
	}

	@keyframes slideIn {
		from { transform: translateX(-20px); opacity: 0.8; }
		to { transform: translateX(0); opacity: 1; }
	}

	.drawer-header {
		display: flex;
		align-items: flex-start;
		justify-content: space-between;
		padding: 14px 16px 10px;
		border-bottom: 1px solid #222;
		gap: 12px;
	}

	.drawer-title {
		margin: 0;
		font-size: 14px;
		font-weight: 600;
		color: #d4d0c8;
	}

	.drawer-sub {
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		font-size: 10px;
		color: #6b6b6b;
		margin-top: 3px;
	}

	.drawer-active-name {
		color: #00f0ff;
	}

	.drawer-active-ep {
		color: #f5a623;
	}

	.drawer-close {
		width: 28px;
		height: 28px;
		display: flex;
		align-items: center;
		justify-content: center;
		background: transparent;
		border: none;
		border-radius: 4px;
		color: #666;
		cursor: pointer;
	}

	.drawer-close:hover { background: #27272a; color: #d4d0c8; }
	.drawer-close svg { width: 16px; height: 16px; }

	.drawer-search {
		display: flex;
		gap: 6px;
		padding: 8px 16px;
		border-bottom: 1px solid #222;
	}

	.drawer-search input {
		flex: 1;
		padding: 7px 10px;
		background: #111113;
		border: 1px solid #333;
		border-radius: 5px;
		color: #d4d0c8;
		font-size: 12px;
		font-family: inherit;
		outline: none;
	}

	.drawer-search input:focus { border-color: #f5a623; }
	.drawer-search input::placeholder { color: #555; }

	.refresh-btn {
		padding: 6px 10px;
		background: #111113;
		color: #878787;
		border: 1px solid #333;
		border-radius: 5px;
		font-size: 10px;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		cursor: pointer;
	}
	.refresh-btn:hover:not(:disabled) {
		color: #00f0ff;
		border-color: #00f0ff;
	}
	.refresh-btn:disabled {
		opacity: 0.5;
		cursor: wait;
	}

	.drawer-banner {
		padding: 10px 16px;
		font-size: 11px;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		border-bottom: 1px solid #222;
	}
	.banner-error {
		background: rgba(239, 68, 68, 0.12);
		color: #ef4444;
		border-left: 3px solid #ef4444;
	}

	.drawer-body {
		flex: 1;
		overflow-y: auto;
		padding: 8px;
		display: flex;
		flex-direction: column;
		gap: 4px;
	}

	.drawer-empty {
		padding: 24px 14px;
		color: #555;
		font-size: 12px;
		text-align: center;
	}

	.model-card {
		display: flex;
		flex-direction: column;
		gap: 6px;
		padding: 12px 14px;
		background: #1a1a1e;
		border: 1px solid #222;
		border-radius: 6px;
		text-align: left;
		font-family: inherit;
		color: #d4d0c8;
		transition: all 150ms ease;
	}

	.model-card.card-active {
		background: rgba(0, 240, 255, 0.05);
		border-color: rgba(0, 240, 255, 0.4);
	}

	.model-card.card-broken {
		opacity: 0.7;
		border-color: rgba(239, 68, 68, 0.25);
	}

	.model-top {
		display: flex;
		align-items: center;
		justify-content: space-between;
	}

	.model-name {
		font-size: 13px;
		font-weight: 500;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		color: #d4d0c8;
	}

	.model-status {
		font-size: 9px;
		font-weight: 600;
		text-transform: uppercase;
		letter-spacing: 0.04em;
		padding: 2px 6px;
		border-radius: 3px;
	}

	.model-status.active { color: #00f0ff; background: rgba(0, 240, 255, 0.12); }
	.model-status.ready  { color: #34d399; background: rgba(52, 211, 153, 0.1); }
	.model-status.broken { color: #ef4444; background: rgba(239, 68, 68, 0.12); }

	.model-meta {
		display: flex;
		align-items: center;
		gap: 4px;
		font-size: 11px;
		color: #6b6b6b;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
	}

	.meta-sep { color: #333; }

	.model-status-note {
		font-size: 10px;
		color: #f5a623;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		font-style: italic;
	}

	.model-actions {
		display: flex;
		justify-content: flex-end;
	}

	.switch-btn {
		padding: 4px 14px;
		background: transparent;
		color: #d4d0c8;
		border: 1px solid #444;
		border-radius: 4px;
		font-size: 11px;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		cursor: pointer;
		transition: all 120ms ease;
	}

	.switch-btn:hover:not(:disabled) {
		border-color: #00f0ff;
		color: #00f0ff;
	}

	.switch-btn:disabled {
		opacity: 0.45;
		cursor: not-allowed;
	}
</style>
