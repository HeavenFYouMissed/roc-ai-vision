<!--
	roc-ai-vision · dashboard · biometrics/SparklinePanel.svelte

	Canvas-based sparkline. Used 6× across the destination — 3 metric types
	(Report rate / Velocity / Sign flip rate) × 2 rows (current / previous).

	Canvas was chosen over SVG because 6 sparklines × 240 samples × 60Hz
	would generate ~86k SVG path commands per second, which causes
	GC pressure and layout thrash. Canvas drawImage / lineTo is O(1) per
	point with no DOM mutation.

	Owns its own scratch Float32Array; caller passes a `compute` callback
	that fills the buffer each tick.

	The header line matches the reference image format:
	  "Avg: 163.1   Min: 58.3   Max: 1102.4"  (top row)
	  "Session: 31.56 flips/sec"               (session line, optional)
	  "Flip rate window: min 0.00 avg 27.96 max 40.0"  (window line, optional)
-->
<script lang="ts">
	import type { CaptureSlot } from '$lib/biometrics/mouse_capture.svelte';
	import type { SeriesStats } from '$lib/biometrics/metrics';

	interface Props {
		slot: CaptureSlot;
		version: number;
		/** Display title — "Report rate (Hz)", "Velocity (counts/sec)", etc. */
		title: string;
		/** Bucket count for the rolling window. */
		buckets: number;
		/** Compute callback — fills `out` with `buckets` values, returns stats. */
		compute: (slot: CaptureSlot, out: Float32Array) => SeriesStats;
		/** Format header text from stats. */
		headerFormat: (stats: SeriesStats) => string;
		/** Optional second/third header lines (for sign-flip panel). */
		subHeaderLines?: (stats: SeriesStats) => string[];
		/** Optional 1px border accent (used for the sign-flip "alarm" panel). */
		borderAccent?: string;
	}

	let {
		slot,
		version,
		title,
		buckets,
		compute,
		headerFormat,
		subHeaderLines,
		borderAccent
	}: Props = $props();

	let canvasRef: HTMLCanvasElement | undefined = $state();
	let wrapRef: HTMLDivElement | undefined = $state();
	let stats = $state<SeriesStats>({ count: 0, avg: 0, min: 0, max: 0, last: 0 });
	const scratch = new Float32Array(buckets);

	let dpr = 1;
	let cssW = 1;
	let cssH = 1;
	let ro: ResizeObserver | undefined;

	// Last drawn stats — kept as a plain (non-reactive) variable so `draw()`
	// doesn't track the `stats` $state when reading the y-axis max. Reading
	// the $state from inside the effect would cause an
	// `effect_update_depth_exceeded` loop because the same effect also writes
	// to `stats`.
	let lastStats: SeriesStats = { count: 0, avg: 0, min: 0, max: 0, last: 0 };

	$effect(() => {
		if (!canvasRef || !wrapRef) return;
		dpr = window.devicePixelRatio || 1;
		const resize = (): void => {
			const r = wrapRef!.getBoundingClientRect();
			cssW = Math.max(1, Math.floor(r.width));
			cssH = Math.max(1, Math.floor(r.height));
			canvasRef!.width = Math.floor(cssW * dpr);
			canvasRef!.height = Math.floor(cssH * dpr);
			canvasRef!.style.width = cssW + 'px';
			canvasRef!.style.height = cssH + 'px';
			draw(lastStats);
		};
		resize();
		ro = new ResizeObserver(resize);
		ro.observe(wrapRef);
		return () => ro?.disconnect();
	});

	$effect(() => {
		// eslint-disable-next-line @typescript-eslint/no-unused-expressions
		version;
		const next = compute(slot, scratch);
		lastStats = next;
		stats = next;
		draw(next);
	});

	function draw(s: SeriesStats): void {
		if (!canvasRef) return;
		const ctx = canvasRef.getContext('2d');
		if (!ctx) return;
		const W = canvasRef.width;
		const H = canvasRef.height;
		ctx.clearRect(0, 0, W, H);

		// Pad inside DPR-scaled coords so axes / strokes stay sharp.
		const padX = 4 * dpr;
		const padY = 4 * dpr;
		const plotW = W - padX * 2;
		const plotH = H - padY * 2;
		if (plotW <= 0 || plotH <= 0) return;

		const n = scratch.length;
		if (n < 2) return;

		// Y-scale: peg max to the rolling max (with a tiny headroom).
		const yMax = Math.max(s.max * 1.05, 1e-6);

		// Draw filled area under curve (subtle cyan).
		ctx.beginPath();
		ctx.moveTo(padX, padY + plotH);
		for (let i = 0; i < n; i++) {
			const x = padX + (i / (n - 1)) * plotW;
			const y = padY + plotH - (scratch[i] / yMax) * plotH;
			ctx.lineTo(x, y);
		}
		ctx.lineTo(padX + plotW, padY + plotH);
		ctx.closePath();
		ctx.fillStyle = 'rgba(0, 240, 255, 0.08)';
		ctx.fill();

		// Draw stroke.
		ctx.beginPath();
		for (let i = 0; i < n; i++) {
			const x = padX + (i / (n - 1)) * plotW;
			const y = padY + plotH - (scratch[i] / yMax) * plotH;
			if (i === 0) ctx.moveTo(x, y);
			else ctx.lineTo(x, y);
		}
		ctx.lineWidth = 1.2 * dpr;
		ctx.lineJoin = 'round';
		ctx.strokeStyle = '#00f0ff';
		ctx.globalAlpha = 0.9;
		ctx.stroke();
		ctx.globalAlpha = 1;
	}
</script>

<div
	class="spark-panel"
	style:border-color={borderAccent ?? '#27272a'}
>
	<div class="spark-header">
		<div class="spark-title">{title}</div>
		<div class="spark-stats-line">{headerFormat(stats)}</div>
		{#if subHeaderLines}
			{#each subHeaderLines(stats) as line}
				<div class="spark-sub-line">{line}</div>
			{/each}
		{/if}
	</div>
	<div class="spark-canvas-wrap" bind:this={wrapRef}>
		<canvas bind:this={canvasRef}></canvas>
	</div>
</div>

<style>
	.spark-panel {
		display: flex;
		flex-direction: column;
		background: #161618;
		border: 1px solid #27272a;
		border-radius: 4px;
		overflow: hidden;
		min-height: 0;
		min-width: 0;
	}

	.spark-header {
		padding: 4px 10px 6px;
		background: #111113;
		border-bottom: 1px solid #1d1d20;
		flex-shrink: 0;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
	}

	.spark-title {
		font-size: 11px;
		color: #878787;
		text-transform: lowercase;
		letter-spacing: 0.02em;
	}

	.spark-stats-line {
		font-size: 11px;
		color: #d4d0c8;
		margin-top: 2px;
		font-variant-numeric: tabular-nums;
	}

	.spark-sub-line {
		font-size: 10px;
		color: #878787;
		margin-top: 1px;
		font-variant-numeric: tabular-nums;
	}

	.spark-canvas-wrap {
		flex: 1;
		min-height: 0;
		min-width: 0;
		position: relative;
		background: #0e0e10;
	}

	canvas {
		display: block;
		position: absolute;
		inset: 0;
	}
</style>
