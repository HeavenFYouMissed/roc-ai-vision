<!--
	roc-ai-vision · dashboard · ModelCard.svelte

	Floating top-right of OperateCanvas. Roboflow-style model stats card
	showing the active model name, version, mAP/Precision/Recall, and
	live inference latency p50/p99.
-->
<script lang="ts">
	import { trackPool } from '$lib/stores/track_pool.svelte';

	interface Props {
		onSwitchModel?: () => void;
	}

	let { onSwitchModel = () => {} }: Props = $props();

	// v1: hardcoded from manifest YAMLs in models/ (Phase 6+ reads live)
	const ACTIVE_MODEL = {
		name: 'yolo26m-roc-humanoid',
		version: 'v0.1',
		mAP: 84.1,
		precision: 92.7,
		recall: 77.4
	};

	// Live-computed inference latency from inter-frame deltas
	let inferenceP50 = $state(0);
	let inferenceP99 = $state(0);
	let lastSeq = -1;
	let lastTs = 0;
	const samples: number[] = [];
	const SAMPLE_WINDOW = 60;

	$effect(() => {
		// Reactive trigger: any frame seq change
		const seq = trackPool.frameSeq;
		if (seq === lastSeq) return;
		const now = performance.now();
		if (lastTs !== 0 && lastSeq !== -1) {
			const delta = now - lastTs;
			samples.push(delta);
			if (samples.length > SAMPLE_WINDOW) samples.shift();
			// Compute p50 and p99 from sorted copy
			const sorted = [...samples].sort((a, b) => a - b);
			const p50idx = Math.floor(sorted.length * 0.5);
			const p99idx = Math.floor(sorted.length * 0.99);
			inferenceP50 = sorted[p50idx] || 0;
			inferenceP99 = sorted[p99idx] || sorted[sorted.length - 1] || 0;
		}
		lastSeq = seq;
		lastTs = now;
	});
</script>

<div class="model-card" role="status" aria-label="Active model stats">
	<div class="model-header">
		<span class="model-name">{ACTIVE_MODEL.name}</span>
		<span class="model-version">{ACTIVE_MODEL.version}</span>
	</div>
	<div class="model-stats">
		<div class="stat-item">
			<span class="stat-num">{ACTIVE_MODEL.mAP.toFixed(1)}%</span>
			<span class="stat-label">mAP</span>
		</div>
		<div class="stat-divider"></div>
		<div class="stat-item">
			<span class="stat-num">{ACTIVE_MODEL.precision.toFixed(1)}%</span>
			<span class="stat-label">Precision</span>
		</div>
		<div class="stat-divider"></div>
		<div class="stat-item">
			<span class="stat-num">{ACTIVE_MODEL.recall.toFixed(1)}%</span>
			<span class="stat-label">Recall</span>
		</div>
	</div>
	<div class="model-latency">
		<span class="lat-tag">INF</span>
		<span class="lat-val">p50: {inferenceP50.toFixed(1)}ms</span>
		<span class="lat-sep">·</span>
		<span class="lat-val" class:warn={inferenceP99 > 30} class:err={inferenceP99 > 50}>
			p99: {inferenceP99.toFixed(1)}ms
		</span>
	</div>
	<button class="switch-btn" onclick={onSwitchModel}>
		Switch model...
	</button>
</div>

<style>
	.model-card {
		position: absolute;
		top: 12px;
		right: 12px;
		width: 260px;
		background: rgba(22, 22, 24, 0.92);
		backdrop-filter: blur(8px);
		border: 1px solid rgba(39, 39, 42, 0.9);
		border-radius: 6px;
		padding: 10px 12px;
		z-index: 5;
		display: flex;
		flex-direction: column;
		gap: 6px;
		font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
	}

	.model-header {
		display: flex;
		align-items: baseline;
		gap: 6px;
	}

	.model-name {
		font-size: 12px;
		font-weight: 600;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #d4d0c8;
	}

	.model-version {
		font-size: 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #f5a623;
	}

	.model-stats {
		display: flex;
		align-items: center;
		gap: 8px;
		padding: 4px 0;
	}

	.stat-item {
		display: flex;
		flex-direction: column;
		gap: 1px;
		flex: 1;
	}

	.stat-num {
		font-size: 13px;
		font-weight: 600;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #d4d0c8;
	}

	.stat-label {
		font-size: 9px;
		text-transform: uppercase;
		letter-spacing: 0.05em;
		color: #555;
	}

	.stat-divider {
		width: 1px;
		height: 18px;
		background: #2a2a2e;
	}

	.model-latency {
		display: flex;
		align-items: center;
		gap: 6px;
		padding-top: 6px;
		border-top: 1px solid #2a2a2e;
		font-size: 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
	}

	.lat-tag {
		color: #555;
		font-weight: 600;
	}

	.lat-val {
		color: #34d399;
	}

	.lat-val.warn {
		color: #f5a623;
	}

	.lat-val.err {
		color: #ef4444;
	}

	.lat-sep {
		color: #333;
	}

	.switch-btn {
		margin-top: 4px;
		padding: 5px 8px;
		background: rgba(245, 166, 35, 0.08);
		color: #f5a623;
		border: 1px solid rgba(245, 166, 35, 0.3);
		border-radius: 4px;
		font-size: 10px;
		font-weight: 500;
		font-family: inherit;
		cursor: pointer;
		transition: all 120ms ease;
	}

	.switch-btn:hover {
		background: rgba(245, 166, 35, 0.15);
		border-color: rgba(245, 166, 35, 0.5);
	}
</style>
