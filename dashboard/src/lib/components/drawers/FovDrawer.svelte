<!--
	roc-ai-vision · dashboard · drawers/FovDrawer.svelte

	Operator interface for the Phase 6 FOV crop. Lets you:
	  * Enable / disable the inference-time crop
	  * Drag the centre with sliders (cx / cy) — relative to source frame
	  * Resize the half-extent radius (in source pixels)
	  * Toggle "follow target" (Phase 7) — locks cx/cy to the currently
	    selected track's EKF position so the model only ever sees a window
	    around the active operator-locked subject

	The diagnostic block at the bottom shows the *actual* crop rect the
	server applied last frame, so you can visually confirm that clamping
	to source bounds didn't push the window off-axis.
-->
<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { fovStore } from '$lib/stores/fov_store.svelte';

	interface Props {
		open: boolean;
		onClose: () => void;
	}

	let { open, onClose }: Props = $props();

	// When the drawer is open, poll the server every 500 ms so the diagnostic
	// active-crop block stays live (the inference thread updates it once per
	// frame on the C++ side).
	$effect(() => {
		if (open) {
			fovStore.startPolling();
		} else {
			fovStore.stopPolling();
		}
	});

	onMount(() => {
		void fovStore.refresh();
	});

	onDestroy(() => {
		fovStore.stopPolling();
	});

	// Convert per-frame fraction → percent for human display.
	function pct(v: number): string { return (v * 100).toFixed(1); }

	function onToggleEnabled(): void {
		void fovStore.update({ enabled: !fovStore.settings.enabled });
	}
	function onToggleFollow(): void {
		void fovStore.update({ followTarget: !fovStore.settings.followTarget });
	}
	function onCx(v: number): void {
		void fovStore.update({ cxNorm: v });
	}
	function onCy(v: number): void {
		void fovStore.update({ cyNorm: v });
	}
	function onRadius(v: number): void {
		void fovStore.update({ radiusPx: v });
	}
	function recenter(): void {
		void fovStore.update({ cxNorm: 0.5, cyNorm: 0.5 });
	}
</script>

{#if open}
	<div class="drawer-backdrop" onclick={onClose} role="presentation">
		<aside class="drawer" onclick={(e) => e.stopPropagation()} role="dialog" aria-label="FOV">
			<div class="drawer-header">
				<div>
					<h3 class="drawer-title">FOV — Inference Crop</h3>
					<div class="drawer-sub">
						{fovStore.settings.enabled ? 'ENABLED' : 'DISABLED — model sees full frame'}
					</div>
				</div>
				<button class="drawer-close" onclick={onClose} aria-label="Close">
					<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M18 6L6 18M6 6l12 12" /></svg>
				</button>
			</div>

			{#if fovStore.lastError}
				<div class="banner-error"><strong>Orchestrator offline:</strong> {fovStore.lastError}</div>
			{/if}

			<div class="drawer-body">
				<div class="fov-row">
					<label class="checkbox-row">
						<input type="checkbox" checked={fovStore.settings.enabled} onchange={onToggleEnabled} />
						<span>Enable inference crop</span>
					</label>
					<p class="hint">
						When enabled, only the centred square crop below is forwarded
						to the InferenceEngine. Detections are translated back to
						source-frame coordinates so the dashboard overlays don't change.
					</p>
				</div>

				<div class="fov-row" class:fov-row-disabled={!fovStore.settings.enabled}>
					<label class="checkbox-row">
						<input
							type="checkbox"
							checked={fovStore.settings.followTarget}
							disabled={!fovStore.settings.enabled}
							onchange={onToggleFollow}
						/>
						<span>Follow locked target (Phase 7)</span>
					</label>
					<p class="hint">
						When a track is locked, the orchestrator overrides cx/cy each
						frame with the target's EKF position (low-pass filtered).
						Operator cx/cy below are ignored while this is on.
					</p>
				</div>

				<div class="fov-row" class:fov-row-disabled={!fovStore.settings.enabled}>
					<div class="slider-block">
						<div class="slider-label">
							Centre X
							<span class="slider-value">{pct(fovStore.settings.cxNorm)}%</span>
						</div>
						<input
							type="range" min="0" max="1" step="0.005"
							value={fovStore.settings.cxNorm}
							disabled={!fovStore.settings.enabled || fovStore.settings.followTarget}
							oninput={(e) => onCx(parseFloat((e.currentTarget as HTMLInputElement).value))}
						/>
					</div>
					<div class="slider-block">
						<div class="slider-label">
							Centre Y
							<span class="slider-value">{pct(fovStore.settings.cyNorm)}%</span>
						</div>
						<input
							type="range" min="0" max="1" step="0.005"
							value={fovStore.settings.cyNorm}
							disabled={!fovStore.settings.enabled || fovStore.settings.followTarget}
							oninput={(e) => onCy(parseFloat((e.currentTarget as HTMLInputElement).value))}
						/>
					</div>
					<div class="slider-block">
						<div class="slider-label">
							Radius
							<span class="slider-value">{fovStore.settings.radiusPx.toFixed(0)} px</span>
						</div>
						<input
							type="range" min="32" max="1080" step="4"
							value={fovStore.settings.radiusPx}
							disabled={!fovStore.settings.enabled}
							oninput={(e) => onRadius(parseFloat((e.currentTarget as HTMLInputElement).value))}
						/>
					</div>

					<div class="action-row">
						<button
							class="action-btn"
							disabled={!fovStore.settings.enabled || fovStore.settings.followTarget}
							onclick={recenter}
						>
							Re-centre (50%, 50%)
						</button>
					</div>
				</div>

				<div class="diag">
					<div class="diag-title">DIAGNOSTICS — live from server</div>
					<dl class="diag-grid">
						<dt>Source frame</dt>
						<dd>{fovStore.settings.sourceW || '—'} × {fovStore.settings.sourceH || '—'}</dd>
						<dt>Active crop</dt>
						<dd>
							{fovStore.settings.activeCropW || '—'} × {fovStore.settings.activeCropH || '—'}
							{#if fovStore.settings.activeCropW > 0}
								@ ({fovStore.settings.activeCropX}, {fovStore.settings.activeCropY})
							{/if}
						</dd>
						<dt>Crop ratio</dt>
						<dd>
							{#if fovStore.settings.sourceW > 0 && fovStore.settings.activeCropW > 0}
								{((fovStore.settings.activeCropW * fovStore.settings.activeCropH) /
									(fovStore.settings.sourceW * fovStore.settings.sourceH) * 100).toFixed(1)}% of pixels
							{:else}—{/if}
						</dd>
					</dl>
					<p class="hint">
						Crop ratio &lt; 100% means the model sees less of the frame
						per inference step — directly reduces preprocess + inference
						compute by the same factor (typical 4–10× speedup at 1080p).
					</p>
				</div>
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

	.banner-error {
		padding: 10px 16px;
		font-size: 11px;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		background: rgba(239, 68, 68, 0.12);
		color: #ef4444;
		border-left: 3px solid #ef4444;
		border-bottom: 1px solid #222;
	}

	.drawer-body {
		flex: 1;
		overflow-y: auto;
		padding: 12px;
		display: flex;
		flex-direction: column;
		gap: 18px;
	}

	.fov-row {
		display: flex;
		flex-direction: column;
		gap: 10px;
		padding: 14px;
		background: #1a1a1e;
		border: 1px solid #222;
		border-radius: 6px;
		transition: opacity 120ms ease;
	}
	.fov-row-disabled {
		opacity: 0.5;
		pointer-events: none;
	}

	.checkbox-row {
		display: flex;
		align-items: center;
		gap: 10px;
		font-size: 12px;
		color: #d4d0c8;
		cursor: pointer;
	}

	.checkbox-row input[type='checkbox'] {
		accent-color: #00f0ff;
		width: 16px;
		height: 16px;
	}

	.hint {
		margin: 0;
		font-size: 10.5px;
		color: #777;
		line-height: 1.45;
	}

	.slider-block {
		display: flex;
		flex-direction: column;
		gap: 4px;
	}

	.slider-label {
		display: flex;
		justify-content: space-between;
		align-items: baseline;
		font-size: 11px;
		color: #d4d0c8;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
	}

	.slider-value {
		font-size: 10px;
		color: #00f0ff;
		font-variant-numeric: tabular-nums;
	}

	input[type='range'] {
		width: 100%;
		accent-color: #00f0ff;
	}

	.action-row {
		display: flex;
		gap: 8px;
		margin-top: 6px;
	}

	.action-btn {
		flex: 1;
		padding: 6px 10px;
		background: transparent;
		color: #d4d0c8;
		border: 1px solid #444;
		border-radius: 4px;
		font-size: 11px;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		cursor: pointer;
	}
	.action-btn:hover:not(:disabled) {
		border-color: #00f0ff;
		color: #00f0ff;
	}
	.action-btn:disabled {
		opacity: 0.4;
		cursor: not-allowed;
	}

	.diag {
		padding: 14px;
		background: #111113;
		border: 1px solid #222;
		border-radius: 6px;
	}

	.diag-title {
		font-size: 9px;
		font-weight: 600;
		color: #6b6b6b;
		letter-spacing: 0.06em;
		text-transform: uppercase;
		margin-bottom: 10px;
	}

	.diag-grid {
		display: grid;
		grid-template-columns: max-content 1fr;
		gap: 4px 10px;
		margin: 0 0 8px 0;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		font-size: 11px;
	}
	.diag-grid dt { color: #6b6b6b; }
	.diag-grid dd { margin: 0; color: #00f0ff; font-variant-numeric: tabular-nums; }
</style>
