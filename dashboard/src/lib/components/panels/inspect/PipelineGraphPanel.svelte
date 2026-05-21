<!--
	roc-ai-vision · dashboard · PipelineGraphPanel.svelte

	Directed graph: Camera → Inference → Tracker → Output Sink.
	Per-node status dot + latency tooltip; per-edge FPS label;
	Run/Stop/Pause node-state badges (Phase 6+ control).
-->
<script lang="ts">
	import { trackPool } from '$lib/stores/track_pool.svelte';
	import { telemetrySocket } from '$lib/telemetry/telemetry_socket.svelte';
	import { scenarioStore } from '$lib/profiles/scenario_store.svelte';
	import { SINK_LABELS } from '$lib/render/class_colors';

	let fps = $state(0);
	let infMs = $state(0);

	$effect(() => {
		let rafId: number;
		let count = 0;
		let last = performance.now();
		let lastSeq = -1;
		let lastFrameT = 0;

		function tick() {
			count++;
			const now = performance.now();
			if (now - last >= 500) {
				fps = Math.round((count * 1000) / (now - last));
				count = 0;
				last = now;
			}
			const seq = trackPool.frameSeq;
			if (seq !== lastSeq && lastFrameT > 0) {
				infMs = now - lastFrameT;
				lastFrameT = now;
				lastSeq = seq;
			} else if (seq !== lastSeq) {
				lastFrameT = now;
				lastSeq = seq;
			}
			rafId = requestAnimationFrame(tick);
		}
		rafId = requestAnimationFrame(tick);
		return () => cancelAnimationFrame(rafId);
	});

	const nodes = $derived([
		{
			id: 'camera',
			label: 'Camera',
			lat: '---',
			status: telemetrySocket.state === 'connected' ? 'ok' : 'stale',
			fps: 'mock'
		},
		{
			id: 'inference',
			label: 'Inference',
			lat: infMs > 0 ? `${infMs.toFixed(1)}ms` : '---',
			status: infMs > 0 ? (infMs > 50 ? 'err' : infMs > 30 ? 'warn' : 'ok') : 'stale',
			fps: fps > 0 ? `${fps}` : '---'
		},
		{
			id: 'tracker',
			label: 'Tracker',
			lat: '0.8ms',
			status: trackPool.activeCount > 0 ? 'ok' : 'stale',
			fps: fps > 0 ? `${fps}` : '---'
		},
		{
			id: 'sink',
			label: SINK_LABELS[scenarioStore.active.sinkKind] || 'NO-OP',
			lat: '---',
			status: 'stale',
			fps: '500'
		}
	]);

	const STATUS_COLORS: Record<string, string> = {
		ok: '#34d399',
		warn: '#f5a623',
		err: '#ef4444',
		stale: '#444'
	};
</script>

<div class="pgraph">
	<div class="pgraph-row">
		{#each nodes as node, i (node.id)}
			<div class="pgraph-node" title="{node.label} · latency {node.lat}">
				<div class="node-header">
					<span class="node-dot" style="background: {STATUS_COLORS[node.status]}"></span>
					<span class="node-name">{node.label}</span>
				</div>
				<div class="node-lat">{node.lat}</div>
				<div class="node-state">{node.status === 'stale' ? 'STOP' : 'RUN'}</div>
			</div>
			{#if i < nodes.length - 1}
				<div class="pgraph-edge">
					<span class="edge-fps">{nodes[i + 1].fps}fps</span>
					<svg viewBox="0 0 40 12" preserveAspectRatio="none">
						<line x1="0" y1="6" x2="34" y2="6" stroke="#444" stroke-width="1.5"></line>
						<polygon points="34,3 40,6 34,9" fill="#444"></polygon>
					</svg>
				</div>
			{/if}
		{/each}
	</div>
</div>

<style>
	.pgraph {
		padding: 8px 4px;
	}

	.pgraph-row {
		display: flex;
		align-items: center;
		gap: 4px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
	}

	.pgraph-node {
		flex: 1;
		min-width: 0;
		background: #111113;
		border: 1px solid #2a2a2e;
		border-radius: 4px;
		padding: 6px 8px;
		display: flex;
		flex-direction: column;
		gap: 3px;
	}

	.node-header {
		display: flex;
		align-items: center;
		gap: 5px;
	}

	.node-dot {
		width: 7px;
		height: 7px;
		border-radius: 50%;
		box-shadow: 0 0 3px currentColor;
	}

	.node-name {
		font-size: 10px;
		font-weight: 600;
		color: #d4d0c8;
		overflow: hidden;
		text-overflow: ellipsis;
		white-space: nowrap;
	}

	.node-lat {
		font-size: 10px;
		color: #f5a623;
	}

	.node-state {
		font-size: 8px;
		font-weight: 700;
		color: #555;
		letter-spacing: 0.06em;
	}

	.pgraph-edge {
		display: flex;
		flex-direction: column;
		align-items: center;
		gap: 1px;
		flex-shrink: 0;
		min-width: 40px;
	}

	.edge-fps {
		font-size: 9px;
		color: #878787;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
	}

	.pgraph-edge svg {
		width: 40px;
		height: 10px;
	}
</style>
