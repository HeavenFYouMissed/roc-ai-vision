<!--
	roc-ai-vision · dashboard · biometrics/TracePanel.svelte

	SVG trace renderer for the Biometrics destination. Reads a CaptureSlot,
	integrates per-sample mouse deltas into a 2D path (anchored at origin),
	auto-fits the SVG viewBox to the path's bounding box (so short flicks
	and long swipes both fill the panel), and renders:
	  - cyan polyline path (#00f0ff, vector-effect non-scaling-stroke)
	  - green dot at start sample
	  - orange dot at current head sample

	Reads from slot.dx/dy (Float32Array ring). Uses pre-allocated scratch
	arrays for the polyline points string so per-frame allocation is zero
	in steady state.
-->
<script lang="ts">
	import type { CaptureSlot } from '$lib/biometrics/mouse_capture.svelte';
	import { RING_CAPACITY } from '$lib/biometrics/mouse_capture.svelte';

	interface Props {
		slot: CaptureSlot;
		/** Bumps when slot.fill/head/version changes — drives redraw. */
		version: number;
		/** Whether this is the live (current) slot or the frozen (previous) slot. */
		live: boolean;
		label?: string;
	}

	let { slot, version, live, label = 'Trace' }: Props = $props();

	let polylineRef: SVGPolylineElement | undefined = $state();
	let startDotRef: SVGCircleElement | undefined = $state();
	let endDotRef: SVGCircleElement | undefined = $state();
	let svgRef: SVGSVGElement | undefined = $state();

	const pointsScratch: string[] = [];
	const VIEWBOX_PAD = 12;

	$effect(() => {
		// eslint-disable-next-line @typescript-eslint/no-unused-expressions
		version;
		render();
	});

	function render(): void {
		if (!polylineRef || !startDotRef || !endDotRef || !svgRef) return;
		const N = slot.fill;
		if (N < 2) {
			polylineRef.setAttribute('points', '');
			startDotRef.setAttribute('display', 'none');
			endDotRef.setAttribute('display', 'none');
			svgRef.setAttribute('viewBox', `0 0 200 100`);
			return;
		}
		const dx = slot.dx;
		const dy = slot.dy;
		const cap = RING_CAPACITY;
		const start = (slot.head - N + cap) % cap;

		// Integrate deltas into cumulative path; compute bbox in same pass.
		let cx = 0, cy = 0;
		let minX = 0, maxX = 0, minY = 0, maxY = 0;
		pointsScratch.length = 0;
		pointsScratch.push('0,0');
		let firstX = 0, firstY = 0, lastX = 0, lastY = 0;
		for (let k = 0; k < N; k++) {
			const idx = (start + k) % cap;
			cx += dx[idx];
			cy += dy[idx];
			pointsScratch.push(`${cx},${cy}`);
			if (cx < minX) minX = cx;
			else if (cx > maxX) maxX = cx;
			if (cy < minY) minY = cy;
			else if (cy > maxY) maxY = cy;
			if (k === 0) { firstX = cx; firstY = cy; }
			lastX = cx;
			lastY = cy;
		}

		// Auto-fit viewBox with padding. Floor of 100 in each dimension so
		// degenerate paths (e.g. perfectly straight horizontal flick) still
		// have visible vertical headroom.
		const w = Math.max(maxX - minX, 100);
		const h = Math.max(maxY - minY, 100);
		const vbX = minX - VIEWBOX_PAD;
		const vbY = minY - VIEWBOX_PAD;
		const vbW = w + VIEWBOX_PAD * 2;
		const vbH = h + VIEWBOX_PAD * 2;
		svgRef.setAttribute('viewBox', `${vbX} ${vbY} ${vbW} ${vbH}`);

		polylineRef.setAttribute('points', pointsScratch.join(' '));

		// Scale dot radii inversely with viewBox so they render as a constant
		// CSS-pixel size regardless of zoom. Using min(vbW, vbH) keeps them
		// proportional to the smaller axis (so a thin horizontal flick still
		// gets readable dot sizes).
		const dotR = Math.min(vbW, vbH) * 0.015;
		startDotRef.setAttribute('cx', String(firstX));
		startDotRef.setAttribute('cy', String(firstY));
		startDotRef.setAttribute('r', String(dotR));
		startDotRef.setAttribute('display', 'inline');
		endDotRef.setAttribute('cx', String(lastX));
		endDotRef.setAttribute('cy', String(lastY));
		endDotRef.setAttribute('r', String(dotR));
		endDotRef.setAttribute('display', 'inline');
	}
</script>

<div class="trace-panel" class:trace-live={live}>
	<div class="trace-header">
		<span class="trace-label">{label}</span>
		{#if live && slot.fill === 0}
			<span class="trace-hint">hold Mouse4 (or Space) to record</span>
		{/if}
	</div>
	<div class="trace-canvas-wrap">
		<svg
			bind:this={svgRef}
			preserveAspectRatio="xMidYMid meet"
			class="trace-svg"
		>
			<polyline
				bind:this={polylineRef}
				fill="none"
				stroke="#00f0ff"
				stroke-width="1.5"
				stroke-opacity="0.85"
				stroke-linejoin="round"
				stroke-linecap="round"
				vector-effect="non-scaling-stroke"
			/>
			<circle
				bind:this={startDotRef}
				fill="#34d399"
				stroke="none"
				display="none"
			/>
			<circle
				bind:this={endDotRef}
				fill="#ff8a4c"
				stroke="none"
				display="none"
			/>
		</svg>
	</div>
</div>

<style>
	.trace-panel {
		display: flex;
		flex-direction: column;
		background: #0a0e14;
		border: 1px solid #27272a;
		border-radius: 4px;
		overflow: hidden;
		min-height: 0;
		min-width: 0;
	}

	.trace-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding: 4px 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 11px;
		color: #878787;
		background: #111113;
		border-bottom: 1px solid #1d1d20;
		flex-shrink: 0;
	}

	.trace-label {
		text-transform: lowercase;
		letter-spacing: 0.02em;
	}

	.trace-hint {
		font-size: 10px;
		color: #555;
		font-style: italic;
	}

	.trace-canvas-wrap {
		flex: 1;
		min-height: 0;
		min-width: 0;
		position: relative;
		background: #161618;
	}

	.trace-svg {
		position: absolute;
		inset: 0;
		width: 100%;
		height: 100%;
		display: block;
	}
</style>
