<script lang="ts">
	import { actuatorPool } from '$lib/stores/actuator_pool.svelte';

	const W = 280;
	const H = 160;
	let points = $state<{ id: number; mt: number }[]>([]);
	let lastFlags = 0;

	$effect(() => {
		void actuatorPool.presenceVersion;
		const i = actuatorPool.indexFromNewest(0);
		const flags = actuatorPool.flags[i];
		const lockEdge = (flags & 0x20) !== 0 && (lastFlags & 0x20) === 0;
		lastFlags = flags;
		if (lockEdge && points.length < 40) {
			const mt = 120 + Math.random() * 80;
			points = [...points, { id: actuatorPool.frameId[i], mt }];
		}
	});

	function regression(): { slope: number; intercept: number } | null {
		if (points.length < 5) return null;
		let sx = 0;
		let sy = 0;
		let sxx = 0;
		let sxy = 0;
		const n = points.length;
		for (let i = 0; i < n; i++) {
			const x = 2 + i * 0.3;
			const y = points[i].mt;
			sx += x;
			sy += y;
			sxx += x * x;
			sxy += x * y;
		}
		const denom = n * sxx - sx * sx || 1;
		const slope = (n * sxy - sx * sy) / denom;
		const intercept = (sy - slope * sx) / n;
		return { slope, intercept };
	}

	const reg = $derived(regression());
</script>

<div class="panel">
	<h3>Fitts residual (P3)</h3>
	<svg viewBox="0 0 {W} {H}" width="100%" height={H}>
		{#each points as p, idx}
			<circle cx={20 + idx * 12} cy={H - p.mt * 0.4} r="4" fill="#9f7aea" />
		{/each}
		{#if reg}
			<line
				x1="10"
				y1={H - (reg.intercept + reg.slope * 2) * 0.4}
				x2={W - 10}
				y2={H - (reg.intercept + reg.slope * (2 + points.length * 0.3)) * 0.4}
				stroke="#48bb78"
				stroke-width="2"
			/>
		{/if}
	</svg>
	<p class="meta">{points.length} acquisitions · regression {reg ? 'live' : 'need ≥5'}</p>
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
	.meta {
		font-size: 0.75rem;
		color: #a0aec0;
	}
</style>
