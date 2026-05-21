<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { actuatorPool, ACTUATOR_RING } from '$lib/stores/actuator_pool.svelte';

	const W = 400;
	const H = 100;
	const HISTORY_SEC = 8;
	const RATE = 60;
	const CAP = HISTORY_SEC * RATE;
	let canvasEl: HTMLCanvasElement | undefined;
	let rafId = 0;

	function draw() {
		void actuatorPool.presenceVersion;
		if (!canvasEl) return;
		const ctx = canvasEl.getContext('2d');
		if (!ctx) return;
		ctx.fillStyle = '#11141a';
		ctx.fillRect(0, 0, W, H);
		const n = Math.min(CAP, actuatorPool.fill);
		if (n < 2) {
			rafId = requestAnimationFrame(draw);
			return;
		}
		const mags = new Float32Array(n);
		let maxM = 1e-6;
		for (let k = 0; k < n; k++) {
			const i = actuatorPool.indexFromNewest(k);
			const ip = actuatorPool.indexFromNewest(Math.min(k + 1, actuatorPool.fill - 1));
			const du = actuatorPool.rawU[i] - actuatorPool.rawU[ip];
			const dv = actuatorPool.rawV[i] - actuatorPool.rawV[ip];
			const m = Math.sqrt(du * du + dv * dv);
			mags[n - 1 - k] = m;
			maxM = Math.max(maxM, m);
		}
		ctx.strokeStyle = '#63b3ed';
		ctx.beginPath();
		for (let i = 0; i < n; i++) {
			const x = (i / (n - 1)) * (W - 1);
			const y = H - (mags[i] / maxM) * (H - 4) - 2;
			if (i === 0) ctx.moveTo(x, y);
			else ctx.lineTo(x, y);
		}
		ctx.stroke();
		rafId = requestAnimationFrame(draw);
	}

	onMount(() => {
		rafId = requestAnimationFrame(draw);
	});
	onDestroy(() => cancelAnimationFrame(rafId));
</script>

<div class="panel">
	<h3>Velocity profile (P1)</h3>
	<canvas bind:this={canvasEl} width={W} height={H}></canvas>
</div>

<style>
	.panel {
		padding: 12px;
		color: #e2e8f0;
	}
	h3 {
		margin: 0 0 8px;
		font-size: 0.85rem;
	}
	canvas {
		width: 100%;
		border: 1px solid #2d3748;
	}
</style>
