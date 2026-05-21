<!--
	roc-ai-vision · dashboard · destinations/BiometricsDestination.svelte

	Mouse Dynamics Capture (MDC) destination — the 5th nav tab per
	dashboard_menu_architecture_2026.md Amendment v6 / UI_SPEC_SHEET §16.

	Layout (7 sections, exact match to reference image):
	  [1] Top bar (collapsible via Shift+F or splitter dblclick)
	  [2] Trace CURRENT (60% width, 50% height)
	  [3] Trace PREVIOUS (60% width, 50% height)
	  [4] Session stats column (12% width, both rows)
	  [5] Report rate + Velocity stacked column (14% width, both rows)
	  [6] Sign flip rate column with RED 1px border (14% width, both rows)

	Data plane: self-contained. ZERO imports from $lib/telemetry/* or
	$lib/stores/*_pool*. All state lives in $lib/biometrics/.
-->
<script lang="ts">
	import { onMount } from 'svelte';
	import TracePanel from '$lib/components/biometrics/TracePanel.svelte';
	import SessionStatsPanel from '$lib/components/biometrics/SessionStatsPanel.svelte';
	import SparklinePanel from '$lib/components/biometrics/SparklinePanel.svelte';
	import { mouseCapture } from '$lib/biometrics/mouse_capture.svelte';
	import { recordingLabel, LABEL_VOCABULARY } from '$lib/biometrics/labels.svelte';
	import {
		reportRateSeries,
		velocitySeries,
		signFlipRateSeries,
		type SeriesStats
	} from '$lib/biometrics/metrics';
	import { exportRowAsPNG, exportSlotJSONL } from '$lib/biometrics/export';

	// =========================================================================
	// Control state (persisted via localStorage)
	// =========================================================================
	const LS_KEY = 'mdc.controls.v1';
	interface PersistedControls {
		ewmaAlpha: number;
		plotWindowSec: number;
		angleThreshDeg: number;
		topBarCollapsed: boolean;
	}
	const DEFAULTS: PersistedControls = {
		ewmaAlpha: 0.20,
		plotWindowSec: 3.0,
		angleThreshDeg: 45,
		topBarCollapsed: false
	};
	let ewmaAlpha = $state(DEFAULTS.ewmaAlpha);
	let plotWindowSec = $state(DEFAULTS.plotWindowSec);
	let angleThreshDeg = $state(DEFAULTS.angleThreshDeg);
	let topBarCollapsed = $state(DEFAULTS.topBarCollapsed);

	function persistControls(): void {
		if (typeof localStorage === 'undefined') return;
		const payload: PersistedControls = {
			ewmaAlpha,
			plotWindowSec,
			angleThreshDeg,
			topBarCollapsed
		};
		localStorage.setItem(LS_KEY, JSON.stringify(payload));
	}

	function hydrateControls(): void {
		if (typeof localStorage === 'undefined') return;
		const raw = localStorage.getItem(LS_KEY);
		if (!raw) return;
		try {
			const p = JSON.parse(raw) as Partial<PersistedControls>;
			if (typeof p.ewmaAlpha === 'number') ewmaAlpha = p.ewmaAlpha;
			if (typeof p.plotWindowSec === 'number') plotWindowSec = p.plotWindowSec;
			if (typeof p.angleThreshDeg === 'number') angleThreshDeg = p.angleThreshDeg;
			if (typeof p.topBarCollapsed === 'boolean') topBarCollapsed = p.topBarCollapsed;
		} catch {
			/* ignore — corrupt persisted state, fall back to defaults */
		}
	}

	// Persist controls on any change (debounced via microtask).
	$effect(() => {
		// eslint-disable-next-line @typescript-eslint/no-unused-expressions
		ewmaAlpha; plotWindowSec; angleThreshDeg; topBarCollapsed;
		queueMicrotask(persistControls);
	});

	// =========================================================================
	// Lifecycle: bind capture engine, hydrate, register Shift+F hotkey
	// =========================================================================
	let captureAreaRef: HTMLElement | undefined = $state();

	onMount(() => {
		hydrateControls();
		recordingLabel.hydrate();
		if (captureAreaRef) mouseCapture.bind(captureAreaRef);

		const onKey = (e: KeyboardEvent): void => {
			// Shift+F toggles top bar collapse. Only when not typing in inputs.
			const tgt = e.target as HTMLElement | null;
			if (tgt && (tgt.tagName === 'INPUT' || tgt.tagName === 'SELECT' || tgt.tagName === 'TEXTAREA')) return;
			if (e.shiftKey && (e.key === 'F' || e.key === 'f')) {
				e.preventDefault();
				topBarCollapsed = !topBarCollapsed;
			}
		};
		window.addEventListener('keydown', onKey);

		return () => {
			window.removeEventListener('keydown', onKey);
			mouseCapture.unbind();
		};
	});

	// =========================================================================
	// Compute callbacks (closed over slot kind + ewmaAlpha + plotWindowSec)
	// =========================================================================
	const BUCKETS = 240; // ~60Hz × 4s, plenty of resolution for the sparkline

	const computeReportRateCurrent = (slot: typeof mouseCapture.current, out: Float32Array): SeriesStats =>
		reportRateSeries(slot, plotWindowSec, out);
	const computeVelocityCurrent = (slot: typeof mouseCapture.current, out: Float32Array): SeriesStats =>
		velocitySeries(slot, plotWindowSec, ewmaAlpha, out);
	const computeSignFlipCurrent = (slot: typeof mouseCapture.current, out: Float32Array): SeriesStats =>
		signFlipRateSeries(slot, plotWindowSec, out);

	const reportFmt = (s: SeriesStats): string =>
		`Avg: ${s.avg.toFixed(1)}   Min: ${s.min.toFixed(1)}   Max: ${s.max.toFixed(1)}`;
	const velocityFmt = (s: SeriesStats): string =>
		`Avg: ${s.avg.toFixed(1)}   Max: ${s.max.toFixed(1)}`;
	const signFlipFmt = (s: SeriesStats): string =>
		`Session: ${s.avg.toFixed(2)} flips/sec`;
	const signFlipSub = (s: SeriesStats): string[] => [
		`Flip rate window: min ${s.min.toFixed(2)}  avg ${s.avg.toFixed(2)}  max ${s.max.toFixed(2)}`
	];

	// =========================================================================
	// DOM refs for per-row PNG export
	// =========================================================================
	let currentRowRef: HTMLElement | undefined = $state();
	let previousRowRef: HTMLElement | undefined = $state();

	async function onExportCurrentPNG(): Promise<void> {
		if (!currentRowRef) return;
		await exportRowAsPNG(currentRowRef, recordingLabel.value, 'current');
	}
	async function onExportPreviousPNG(): Promise<void> {
		if (!previousRowRef) return;
		await exportRowAsPNG(previousRowRef, recordingLabel.value, 'previous');
	}
	function onExportCurrentJSONL(): void {
		exportSlotJSONL(mouseCapture.current, recordingLabel.value, 'current');
	}
	function onExportPreviousJSONL(): void {
		exportSlotJSONL(mouseCapture.previous, recordingLabel.value, 'previous');
	}

	// =========================================================================
	// Render-state helpers
	// =========================================================================
	const stateBadgeText = $derived.by(() => {
		switch (mouseCapture.state) {
			case 'IDLE': return 'click capture area to arm';
			case 'POINTERLOCK_REQUESTED': return 'requesting pointer lock…';
			case 'ARMED': return 'ARMED — hold Mouse4 (or Space) to record';
			case 'RECORDING': return 'RECORDING';
			case 'COMPLETED': return 'session captured';
		}
	});
	const stateBadgeClass = $derived.by(() => {
		switch (mouseCapture.state) {
			case 'RECORDING': return 'badge-rec';
			case 'ARMED': return 'badge-armed';
			case 'COMPLETED': return 'badge-armed';
			default: return 'badge-idle';
		}
	});
</script>

<div class="biometrics-destination" class:topbar-collapsed={topBarCollapsed}>
	<!-- ========================= [1] TOP BAR ========================= -->
	<header class="bio-topbar">
		{#if !topBarCollapsed}
			<div class="bio-topbar-row prompt-row">
				<span class="bio-prompt">Hold Mouse4 to record. Top = current/last, Bottom = previous.</span>
				<span class="bio-state-badge {stateBadgeClass}">{stateBadgeText}</span>
				{#if mouseCapture.lastError}
					<span class="bio-error">{mouseCapture.lastError}</span>
				{/if}
			</div>

			<div class="bio-topbar-row slider-row">
				<label class="bio-slider">
					<span class="bio-slider-label">Hz smoothing (EWMA alpha)</span>
					<input
						type="range"
						min="0"
						max="1"
						step="0.01"
						bind:value={ewmaAlpha}
					/>
					<span class="bio-slider-value">{ewmaAlpha.toFixed(2)}</span>
				</label>
				<label class="bio-slider">
					<span class="bio-slider-label">Plot window (sec)</span>
					<input
						type="range"
						min="0.5"
						max="10"
						step="0.1"
						bind:value={plotWindowSec}
					/>
					<span class="bio-slider-value">{plotWindowSec.toFixed(1)}</span>
				</label>
				<label class="bio-slider">
					<span class="bio-slider-label">Direction change angle (deg)</span>
					<input
						type="range"
						min="5"
						max="90"
						step="1"
						bind:value={angleThreshDeg}
					/>
					<span class="bio-slider-value">{angleThreshDeg}</span>
				</label>
			</div>

			<div class="bio-topbar-row action-row">
				<button
					type="button"
					class="bio-clear-btn"
					onclick={() => mouseCapture.clearHistory()}
				>Clear history</button>
				<label class="bio-label-select">
					<span>label</span>
					<select
						value={recordingLabel.value}
						onchange={(e) => recordingLabel.setLabel((e.currentTarget as HTMLSelectElement).value)}
					>
						{#each LABEL_VOCABULARY as opt}
							<option value={opt}>{opt}</option>
						{/each}
					</select>
				</label>
				<button
					type="button"
					class="bio-collapse-btn"
					onclick={() => (topBarCollapsed = true)}
					title="Collapse top bar (Shift+F)"
				>↑ collapse</button>
			</div>
		{:else}
			<div class="bio-topbar-row collapsed-row">
				<span class="bio-mini-stat">α {ewmaAlpha.toFixed(2)}</span>
				<span class="bio-mini-stat">window {plotWindowSec.toFixed(1)}s</span>
				<span class="bio-mini-stat">θ {angleThreshDeg}°</span>
				<span class="bio-state-badge {stateBadgeClass}">{stateBadgeText}</span>
				<button
					type="button"
					class="bio-collapse-btn"
					onclick={() => (topBarCollapsed = false)}
					title="Expand top bar (Shift+F)"
				>↓ expand</button>
			</div>
		{/if}
		<!-- Splitter handle: dblclick to toggle collapse -->
		<div
			class="bio-splitter"
			role="separator"
			aria-orientation="horizontal"
			ondblclick={() => (topBarCollapsed = !topBarCollapsed)}
			title="Double-click to toggle (Shift+F)"
		></div>
	</header>

	<!-- Capture area: ALL mouse events route here. Single click arms pointer
	     lock. While locked, deltas land in mouseCapture.current via Mouse4/Space. -->
	<div
		class="bio-capture-area"
		role="application"
		aria-label="Mouse capture surface — click to arm pointer lock, hold Mouse4 or Space to record"
		bind:this={captureAreaRef}
	>
		<!-- ====================== [2] CURRENT row ====================== -->
		<div class="bio-row-label">CURRENT / LAST</div>
		<div class="bio-row" bind:this={currentRowRef}>
			<div class="bio-cell bio-cell-trace" data-export-panel>
				<TracePanel
					slot={mouseCapture.current}
					version={mouseCapture.currentVersion}
					live={true}
					label="Trace"
				/>
				<div class="bio-row-actions">
					<button type="button" onclick={onExportCurrentPNG}>Export PNG</button>
					<button type="button" onclick={onExportCurrentJSONL}>Export JSONL</button>
				</div>
			</div>
			<div class="bio-cell bio-cell-session" data-export-panel>
				<SessionStatsPanel
					slot={mouseCapture.current}
					version={mouseCapture.currentVersion}
					{angleThreshDeg}
				/>
			</div>
			<div class="bio-cell bio-cell-charts">
				<div class="bio-chart" data-export-panel>
					<SparklinePanel
						slot={mouseCapture.current}
						version={mouseCapture.currentVersion}
						title="Report rate (Hz)"
						buckets={BUCKETS}
						compute={computeReportRateCurrent}
						headerFormat={reportFmt}
					/>
				</div>
				<div class="bio-chart" data-export-panel>
					<SparklinePanel
						slot={mouseCapture.current}
						version={mouseCapture.currentVersion}
						title="Velocity (counts/sec)"
						buckets={BUCKETS}
						compute={computeVelocityCurrent}
						headerFormat={velocityFmt}
					/>
				</div>
			</div>
			<div class="bio-cell bio-cell-signflip" data-export-panel>
				<SparklinePanel
					slot={mouseCapture.current}
					version={mouseCapture.currentVersion}
					title="Sign flip rate (flips/sec)"
					buckets={BUCKETS}
					compute={computeSignFlipCurrent}
					headerFormat={signFlipFmt}
					subHeaderLines={signFlipSub}
					borderAccent="rgba(255, 68, 68, 0.6)"
				/>
			</div>
		</div>

		<!-- ====================== [3] PREVIOUS row ====================== -->
		<div class="bio-row-label">PREVIOUS</div>
		<div class="bio-row" class:row-empty={!mouseCapture.hasPrevious} bind:this={previousRowRef}>
			<div class="bio-cell bio-cell-trace" data-export-panel>
				<TracePanel
					slot={mouseCapture.previous}
					version={mouseCapture.previousVersion}
					live={false}
					label="Trace"
				/>
				<div class="bio-row-actions">
					<button
						type="button"
						onclick={onExportPreviousPNG}
						disabled={!mouseCapture.hasPrevious}
					>Export PNG</button>
					<button
						type="button"
						onclick={onExportPreviousJSONL}
						disabled={!mouseCapture.hasPrevious}
					>Export JSONL</button>
				</div>
			</div>
			<div class="bio-cell bio-cell-session" data-export-panel>
				<SessionStatsPanel
					slot={mouseCapture.previous}
					version={mouseCapture.previousVersion}
					{angleThreshDeg}
				/>
			</div>
			<div class="bio-cell bio-cell-charts">
				<div class="bio-chart" data-export-panel>
					<SparklinePanel
						slot={mouseCapture.previous}
						version={mouseCapture.previousVersion}
						title="Report rate (Hz)"
						buckets={BUCKETS}
						compute={(s, o) => reportRateSeries(s, plotWindowSec, o)}
						headerFormat={reportFmt}
					/>
				</div>
				<div class="bio-chart" data-export-panel>
					<SparklinePanel
						slot={mouseCapture.previous}
						version={mouseCapture.previousVersion}
						title="Velocity (counts/sec)"
						buckets={BUCKETS}
						compute={(s, o) => velocitySeries(s, plotWindowSec, ewmaAlpha, o)}
						headerFormat={velocityFmt}
					/>
				</div>
			</div>
			<div class="bio-cell bio-cell-signflip" data-export-panel>
				<SparklinePanel
					slot={mouseCapture.previous}
					version={mouseCapture.previousVersion}
					title="Sign flip rate (flips/sec)"
					buckets={BUCKETS}
					compute={(s, o) => signFlipRateSeries(s, plotWindowSec, o)}
					headerFormat={signFlipFmt}
					subHeaderLines={signFlipSub}
					borderAccent="rgba(255, 68, 68, 0.6)"
				/>
			</div>
		</div>
	</div>
</div>

<style>
	.biometrics-destination {
		display: grid;
		grid-template-rows: auto 1fr;
		flex: 1;
		min-height: 0;
		min-width: 0;
		background: #0e0e10;
		color: #d4d0c8;
		font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
	}

	/* ============================== Top bar ============================== */
	.bio-topbar {
		display: flex;
		flex-direction: column;
		background: #111113;
		border-bottom: 1px solid #1d1d20;
		position: relative;
	}

	.bio-topbar-row {
		display: flex;
		align-items: center;
		gap: 14px;
		padding: 6px 14px;
		flex-wrap: wrap;
	}

	.bio-topbar-row.prompt-row {
		padding-top: 8px;
	}

	.bio-topbar-row.action-row {
		padding-bottom: 8px;
	}

	.bio-prompt {
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 11px;
		color: #d4d0c8;
	}

	.bio-state-badge {
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 10px;
		padding: 2px 8px;
		border-radius: 3px;
		text-transform: uppercase;
		letter-spacing: 0.04em;
	}

	.bio-state-badge.badge-idle {
		background: #1a1a1e;
		color: #878787;
		border: 1px solid #27272a;
	}
	.bio-state-badge.badge-armed {
		background: rgba(245, 166, 35, 0.12);
		color: #f5a623;
		border: 1px solid rgba(245, 166, 35, 0.4);
	}
	.bio-state-badge.badge-rec {
		background: rgba(239, 68, 68, 0.15);
		color: #ef4444;
		border: 1px solid rgba(239, 68, 68, 0.5);
		animation: rec-pulse 1.2s ease-in-out infinite;
	}

	@keyframes rec-pulse {
		0%, 100% { opacity: 1; }
		50% { opacity: 0.55; }
	}

	.bio-error {
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 10px;
		color: #ef4444;
	}

	.bio-slider {
		display: flex;
		align-items: center;
		gap: 10px;
		flex: 1;
		min-width: 280px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 11px;
		color: #878787;
	}

	.bio-slider-label {
		flex-shrink: 0;
		min-width: 200px;
		color: #878787;
	}

	.bio-slider input[type="range"] {
		flex: 1;
		min-width: 0;
		height: 6px;
		appearance: none;
		background: linear-gradient(
			to right,
			#3a6fa0 0%,
			#3a6fa0 var(--p, 50%),
			#1d1d20 var(--p, 50%),
			#1d1d20 100%
		);
		border-radius: 3px;
		outline: none;
	}
	.bio-slider input[type="range"]::-webkit-slider-thumb {
		appearance: none;
		width: 12px;
		height: 12px;
		border-radius: 50%;
		background: #d4d0c8;
		cursor: pointer;
	}
	.bio-slider input[type="range"]::-moz-range-thumb {
		width: 12px;
		height: 12px;
		border-radius: 50%;
		background: #d4d0c8;
		border: none;
		cursor: pointer;
	}

	.bio-slider-value {
		min-width: 56px;
		text-align: right;
		color: #d4d0c8;
		font-variant-numeric: tabular-nums;
	}

	.bio-clear-btn {
		background: #2c5d8c;
		color: #f0f0f0;
		border: 1px solid #3a6fa0;
		padding: 4px 12px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 11px;
		border-radius: 3px;
		cursor: pointer;
	}
	.bio-clear-btn:hover {
		background: #3a6fa0;
	}

	.bio-label-select {
		display: flex;
		align-items: center;
		gap: 6px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 11px;
		color: #878787;
	}
	.bio-label-select select {
		background: #1a1a1e;
		color: #d4d0c8;
		border: 1px solid #333;
		font-family: inherit;
		font-size: 11px;
		padding: 2px 6px;
		border-radius: 3px;
	}

	.bio-collapse-btn {
		margin-left: auto;
		background: transparent;
		color: #555;
		border: 1px solid #27272a;
		padding: 2px 8px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 10px;
		border-radius: 3px;
		cursor: pointer;
	}
	.bio-collapse-btn:hover {
		color: #d4d0c8;
		border-color: #555;
	}

	.collapsed-row {
		padding: 4px 14px;
	}
	.bio-mini-stat {
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 11px;
		color: #878787;
	}

	.bio-splitter {
		height: 4px;
		background: transparent;
		cursor: ns-resize;
	}
	.bio-splitter:hover {
		background: rgba(245, 166, 35, 0.3);
	}

	/* ============================ Capture area ============================ */
	.bio-capture-area {
		display: grid;
		grid-template-rows: auto 1fr auto 1fr;
		min-height: 0;
		min-width: 0;
		padding: 8px 14px 14px;
		gap: 6px;
	}

	.bio-row-label {
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 10px;
		color: #878787;
		text-transform: uppercase;
		letter-spacing: 0.06em;
		padding: 4px 2px 2px;
	}

	.bio-row {
		display: grid;
		grid-template-columns: 60fr 12fr 14fr 14fr;
		gap: 8px;
		min-height: 0;
		min-width: 0;
	}

	.bio-row.row-empty .bio-cell { opacity: 0.45; }

	.bio-cell {
		display: flex;
		flex-direction: column;
		min-height: 0;
		min-width: 0;
		gap: 4px;
	}

	.bio-cell-trace {
		min-width: 0;
	}

	.bio-cell-trace :global(.trace-panel) {
		flex: 1;
	}

	.bio-cell-charts {
		display: grid;
		grid-template-rows: 1fr 1fr;
		gap: 4px;
		min-height: 0;
		min-width: 0;
	}

	.bio-chart {
		display: flex;
		min-height: 0;
		min-width: 0;
	}

	.bio-chart :global(.spark-panel) {
		flex: 1;
	}

	/* Sign-flip panel must fill its full-row-height column to match the
	   stacked Report-Rate + Velocity column to its left. Without this rule
	   the SparklinePanel defaults to flex:0 and collapses to its header's
	   intrinsic height, leaving dead space below — visually unbalanced
	   against the reference layout. */
	.bio-cell-signflip :global(.spark-panel) {
		flex: 1;
	}

	.bio-row-actions {
		display: flex;
		gap: 6px;
		justify-content: flex-end;
		flex-shrink: 0;
	}

	.bio-row-actions button {
		background: #161618;
		color: #878787;
		border: 1px solid #27272a;
		padding: 3px 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 10px;
		border-radius: 3px;
		cursor: pointer;
	}
	.bio-row-actions button:hover:not(:disabled) {
		color: #f5a623;
		border-color: #f5a623;
	}
	.bio-row-actions button:disabled {
		opacity: 0.4;
		cursor: not-allowed;
	}
</style>
