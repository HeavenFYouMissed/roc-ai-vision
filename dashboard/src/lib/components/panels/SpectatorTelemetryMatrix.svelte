<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { actuatorPool } from '$lib/stores/actuator_pool.svelte';
	import { bgOdometryPool } from '$lib/stores/bg_odometry_pool.svelte';

	const W = 640;
	const H = 120;
	const LAGS = 21;
	const WIN = 30;
	let canvasEl: HTMLCanvasElement | undefined;
	let rafId = 0;
	let rho = $state(0);
	let lagMs = $state(0);
	let ampRatio = $state(0);

	function velMag(u0: number, v0: number, u1: number, v1: number): number {
		const du = u1 - u0;
		const dv = v1 - v0;
		return Math.sqrt(du * du + dv * dv);
	}

	function sampleSeries(pool: typeof actuatorPool, useClean: boolean, n: number): Float32Array {
		const out = new Float32Array(n);
		for (let k = 0; k < n; k++) {
			const i = pool.indexFromNewest(k);
			const u = useClean ? pool.cleanU[i] : pool.rawU[i];
			const v = useClean ? pool.cleanV[i] : pool.rawV[i];
			const ip = pool.indexFromNewest(k + 1);
			out[k] = velMag(pool.rawU[ip], pool.rawV[ip], u, v);
		}
		return out;
	}

	function sampleBgVel(n: number): Float32Array {
		const out = new Float32Array(n);
		for (let k = 0; k < n; k++) {
			const i = bgOdometryPool.indexFromNewest(k);
			const dx = bgOdometryPool.dx[i];
			const dy = bgOdometryPool.dy[i];
			out[k] = Math.sqrt(dx * dx + dy * dy);
		}
		return out;
	}

	function crossCorr(a: Float32Array, b: Float32Array): { rho: number; lag: number } {
		let bestR = 0;
		let bestLag = 0;
		const mid = (LAGS - 1) >> 1;
		for (let lag = 0; lag < LAGS; lag++) {
			let sum = 0;
			let na = 0;
			let nb = 0;
			let count = 0;
			for (let t = 0; t < WIN; t++) {
				const tb = t + lag - mid;
				if (tb < 0 || tb >= WIN) continue;
				const va = a[t];
				const vb = b[tb];
				sum += va * vb;
				na += va * va;
				nb += vb * vb;
				count++;
			}
			const denom = Math.sqrt(na * nb) || 1;
			const r = sum / denom;
			if (r > bestR) {
				bestR = r;
				bestLag = lag - mid;
			}
		}
		return { rho: bestR, lag: bestLag };
	}

	function draw() {
		void actuatorPool.presenceVersion;
		void bgOdometryPool.presenceVersion;
		if (!canvasEl) return;
		const ctx = canvasEl.getContext('2d');
		if (!ctx) return;
		ctx.fillStyle = '#0b0d12';
		ctx.fillRect(0, 0, W, H);
		const n = Math.min(WIN, actuatorPool.fill, bgOdometryPool.fill);
		if (n < 4) {
			rafId = requestAnimationFrame(draw);
			return;
		}
		const act = sampleSeries(actuatorPool, true, n);
		const bg = sampleBgVel(n);
		const cc = crossCorr(act, bg);
		rho = rho * 0.95 + cc.rho * 0.05;
		lagMs = lagMs * 0.95 + cc.lag * 16.67 * 0.05;
		let actMax = 1e-6;
		let bgMax = 1e-6;
		for (let i = 0; i < n; i++) {
			actMax = Math.max(actMax, act[i]);
			bgMax = Math.max(bgMax, bg[i]);
		}
		ampRatio = ampRatio * 0.95 + (actMax / bgMax) * 0.05;
		ctx.strokeStyle = '#4fd1c5';
		ctx.beginPath();
		for (let i = 0; i < n; i++) {
			const x = (i / (n - 1)) * (W - 1);
			const y = H - (act[i] / actMax) * (H - 8) - 4;
			if (i === 0) ctx.moveTo(x, y);
			else ctx.lineTo(x, y);
		}
		ctx.stroke();
		ctx.strokeStyle = '#f6ad55';
		ctx.beginPath();
		for (let i = 0; i < n; i++) {
			const x = (i / (n - 1)) * (W - 1);
			const y = H - (bg[i] / bgMax) * (H - 8) - 4;
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

<section class="matrix">
	<header class="kpi">
		<span>ρ {rho.toFixed(3)}</span>
		<span>lag {lagMs.toFixed(1)} ms</span>
		<span>amp {ampRatio.toFixed(2)}</span>
	</header>
	<canvas bind:this={canvasEl} width={W} height={H} aria-label="Actuator vs background velocity"></canvas>
	<p class="legend"><span class="act">|actuator vel|</span> <span class="bg">|bg vel|</span></p>
</section>

<style>
	.matrix {
		padding: 16px;
		color: #e2e8f0;
	}
	.kpi {
		display: flex;
		gap: 16px;
		font-family: ui-monospace, monospace;
		margin-bottom: 8px;
	}
	canvas {
		width: 100%;
		border: 1px solid #2d3748;
		border-radius: 4px;
	}
	.legend {
		font-size: 0.75rem;
		margin-top: 8px;
	}
	.act {
		color: #4fd1c5;
		margin-right: 12px;
	}
	.bg {
		color: #f6ad55;
	}
</style>
