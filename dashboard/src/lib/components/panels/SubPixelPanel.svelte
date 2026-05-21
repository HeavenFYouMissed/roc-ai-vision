<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { actuatorPool } from '$lib/stores/actuator_pool.svelte';

	const W = 200;
	const H = 40;
	let ratio = $state(0);
	let canvasEl: HTMLCanvasElement | undefined;
	const spark = new Float32Array(64);
	let sparkHead = 0;
	let rafId = 0;

	function computeRatio(windowSize: number): number {
		const n = Math.min(windowSize, actuatorPool.fill);
		if (n < 2) return 0;
		let sub = 0;
		let total = 0;
		for (let k = 0; k < n - 1; k++) {
			const i0 = actuatorPool.indexFromNewest(k);
			const i1 = actuatorPool.indexFromNewest(k + 1);
			const du = actuatorPool.rawU[i0] - actuatorPool.rawU[i1];
			const dv = actuatorPool.rawV[i0] - actuatorPool.rawV[i1];
			const d = Math.sqrt(du * du + dv * dv);
			if (!Number.isFinite(d)) continue;
			total++;
			if (d < 1) sub++;
		}
		return total > 0 ? sub / total : 0;
	}

	function drawSpark() {
		if (!canvasEl) return;
		const ctx = canvasEl.getContext('2d');
		if (!ctx) return;
		ctx.fillStyle = '#11141a';
		ctx.fillRect(0, 0, W, H);
		ctx.fillStyle = 'rgba(72,187,120,0.15)';
		ctx.fillRect(0, H * 0.15, W, H * 0.65);
		ctx.fillStyle = 'rgba(252,129,129,0.15)';
		ctx.fillRect(0, H * 0.85, W, H * 0.1);
		ctx.strokeStyle = '#63b3ed';
		ctx.beginPath();
		for (let i = 0; i < spark.length; i++) {
			const v = spark[i];
			const x = (i / (spark.length - 1)) * (W - 1);
			const y = H - v * (H - 4) - 2;
			if (i === 0) ctx.moveTo(x, y);
			else ctx.lineTo(x, y);
		}
		ctx.stroke();
	}

	function tick() {
		void actuatorPool.presenceVersion;
		const r = computeRatio(500);
		ratio = ratio * 0.9 + r * 0.1;
		spark[sparkHead] = ratio;
		sparkHead = (sparkHead + 1) % spark.length;
		drawSpark();
		rafId = requestAnimationFrame(tick);
	}

	onMount(() => {
		rafId = requestAnimationFrame(tick);
	});
	onDestroy(() => cancelAnimationFrame(rafId));
</script>

<div class="panel">
	<h3>Sub-pixel ratio (P7)</h3>
	<p class="big">{ratio.toFixed(3)}</p>
	<canvas bind:this={canvasEl} width={W} height={H}></canvas>
	<p class="bands">Human 0.70–0.95 · Bot 0.00–0.10</p>
</div>

<style>
	.panel {
		padding: 12px;
		color: #e2e8f0;
	}
	.big {
		font-size: 2.5rem;
		font-family: ui-monospace, monospace;
		margin: 0;
		color: #63b3ed;
	}
	.bands {
		font-size: 0.7rem;
		color: #718096;
	}
	canvas {
		width: 100%;
		margin: 8px 0;
	}
</style>
