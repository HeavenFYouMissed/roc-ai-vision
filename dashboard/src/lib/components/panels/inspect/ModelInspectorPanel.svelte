<!--
	roc-ai-vision · dashboard · ModelInspectorPanel.svelte

	Per-layer ORT-profile latency table. v1: schema rows from manifest;
	Phase 6+: real ORT profile JSON streamed from C++.
-->
<script lang="ts">
	interface LayerRow {
		name: string;
		op: string;
		inShape: string;
		outShape: string;
		ep: string;
		latMs: number;
	}

	// Synthetic rows for v1 - representative of yolo26m-roc-humanoid layer stack
	const layers: LayerRow[] = [
		{ name: 'Conv2d_0', op: 'Conv2D', inShape: '3×640×640', outShape: '32×320×320', ep: 'CUDA', latMs: 0.8 },
		{ name: 'BatchNorm_0', op: 'BN', inShape: '32×320×320', outShape: '32×320×320', ep: 'CUDA', latMs: 0.2 },
		{ name: 'SiLU_0', op: 'SiLU', inShape: '32×320×320', outShape: '32×320×320', ep: 'CUDA', latMs: 0.1 },
		{ name: 'Conv2d_1', op: 'Conv2D', inShape: '32×320×320', outShape: '64×160×160', ep: 'CUDA', latMs: 1.4 },
		{ name: 'C3k2_1', op: 'Bottleneck', inShape: '64×160×160', outShape: '64×160×160', ep: 'CUDA', latMs: 3.1 },
		{ name: 'Conv2d_2', op: 'Conv2D', inShape: '64×160×160', outShape: '128×80×80', ep: 'CUDA', latMs: 2.2 },
		{ name: 'C3k2_2', op: 'Bottleneck', inShape: '128×80×80', outShape: '128×80×80', ep: 'CUDA', latMs: 4.8 },
		{ name: 'Conv2d_3', op: 'Conv2D', inShape: '128×80×80', outShape: '256×40×40', ep: 'CUDA', latMs: 1.9 },
		{ name: 'SPPF', op: 'SPPF', inShape: '256×40×40', outShape: '256×40×40', ep: 'CUDA', latMs: 1.6 },
		{ name: 'Detect_p3', op: 'Detect', inShape: '256×80×80', outShape: '5×8400', ep: 'CPU', latMs: 8.4 },
		{ name: 'Detect_p4', op: 'Detect', inShape: '256×40×40', outShape: '5×2100', ep: 'CPU', latMs: 4.1 },
		{ name: 'NMS', op: 'NonMaxSuppression', inShape: '5×8400', outShape: 'variable', ep: 'CPU', latMs: 2.7 }
	];

	const total = layers.reduce((s, l) => s + l.latMs, 0);
	const maxLat = Math.max(...layers.map((l) => l.latMs));

	const EP_COLORS: Record<string, string> = {
		CUDA: '#34d399',
		TensorRT: '#10b981',
		DML: '#60a5fa',
		OpenVINO: '#a78bfa',
		CPU: '#878787'
	};
</script>

<div class="mi">
	<div class="mi-summary">
		<div class="sum-stat">
			<span class="sum-num">{total.toFixed(1)}ms</span>
			<span class="sum-label">Total</span>
		</div>
		<div class="sum-stat">
			<span class="sum-num">{layers.length}</span>
			<span class="sum-label">Layers</span>
		</div>
		<div class="sum-stat">
			<span class="sum-num">37MB</span>
			<span class="sum-label">Memory</span>
		</div>
		<div class="sum-stat">
			<span class="sum-num">11.8M</span>
			<span class="sum-label">Params</span>
		</div>
	</div>

	<div class="mi-table">
		<div class="mi-row mi-head">
			<span class="col-name">Layer</span>
			<span class="col-op">Op</span>
			<span class="col-ep">EP</span>
			<span class="col-lat">ms</span>
		</div>
		{#each layers as l}
			<div class="mi-row" title="{l.name} · in {l.inShape} → out {l.outShape}">
				<span class="col-name">{l.name}</span>
				<span class="col-op">{l.op}</span>
				<span class="col-ep" style="color: {EP_COLORS[l.ep]}">{l.ep}</span>
				<span class="col-lat">
					<span class="lat-bar" style="width: {(l.latMs / maxLat) * 100}%; background: {EP_COLORS[l.ep]}"></span>
					<span class="lat-num">{l.latMs.toFixed(1)}</span>
				</span>
			</div>
		{/each}
	</div>
</div>

<style>
	.mi {
		display: flex;
		flex-direction: column;
		gap: 8px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		height: 100%;
	}

	.mi-summary {
		display: flex;
		gap: 12px;
		padding: 8px 4px;
		border-bottom: 1px solid #1d1d20;
	}

	.sum-stat {
		display: flex;
		flex-direction: column;
		flex: 1;
	}

	.sum-num {
		font-size: 13px;
		font-weight: 700;
		color: #f5a623;
	}

	.sum-label {
		font-size: 8px;
		font-weight: 700;
		text-transform: uppercase;
		letter-spacing: 0.06em;
		color: #555;
	}

	.mi-table {
		flex: 1;
		overflow-y: auto;
		display: flex;
		flex-direction: column;
	}

	.mi-row {
		display: grid;
		grid-template-columns: 1.5fr 0.8fr 0.5fr 1fr;
		gap: 4px;
		padding: 3px 6px;
		font-size: 10px;
		color: #d4d0c8;
		border-bottom: 1px solid #161618;
	}

	.mi-row.mi-head {
		font-size: 8px;
		font-weight: 700;
		color: #555;
		text-transform: uppercase;
		letter-spacing: 0.06em;
		border-bottom: 1px solid #27272a;
	}

	.mi-row:not(.mi-head):hover {
		background: #1d1d20;
	}

	.col-name {
		color: #d4d0c8;
		overflow: hidden;
		text-overflow: ellipsis;
		white-space: nowrap;
	}

	.col-op {
		color: #878787;
	}

	.col-ep {
		font-weight: 600;
	}

	.col-lat {
		position: relative;
		display: flex;
		align-items: center;
		justify-content: flex-end;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
	}

	.lat-bar {
		position: absolute;
		left: 0;
		top: 0;
		bottom: 0;
		opacity: 0.15;
		border-radius: 2px;
	}

	.lat-num {
		position: relative;
		color: #d4d0c8;
		z-index: 1;
	}
</style>
