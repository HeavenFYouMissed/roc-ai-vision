<!--
	roc-ai-vision · dashboard · TopicGraphPanel.svelte

	Foxglove-style pub-sub map of WS bridge channels. Shows the C++ orchestrator
	publishing v1/v2/v3 frames and the dashboard subscribers consuming them.
-->
<script lang="ts">
	import { telemetrySocket } from '$lib/telemetry/telemetry_socket.svelte';

	const topics = $derived([
		{
			id: 'v1-tracks',
			version: 'v1',
			label: 'Track Frames',
			rate: telemetrySocket.framesReceived > 0 ? 30 : 30,
			subscriber: 'trackPool + OperateCanvas',
			color: '#00f0ff'
		},
		{
			id: 'v2-actuator',
			version: 'v2',
			label: 'Actuator Telemetry',
			rate: 500,
			subscriber: 'actuatorPool + MouseDynamicsInspector',
			color: '#ff8a4c'
		},
		{
			id: 'v3-bg-odom',
			version: 'v3',
			label: 'BG Optical-Flow',
			rate: 30,
			subscriber: 'bgOdometryPool + SpectatorTelemetryMatrix',
			color: '#a78bfa'
		}
	]);
</script>

<div class="tg">
	<div class="tg-source">
		<div class="src-node">
			<div class="src-dot" class:src-live={telemetrySocket.state === 'connected'}></div>
			<div class="src-info">
				<div class="src-title">C++ Orchestrator</div>
				<div class="src-uri">{telemetrySocket.url}</div>
			</div>
			<div class="src-state">
				{telemetrySocket.state === 'connected' ? 'PUB' : 'OFF'}
			</div>
		</div>
	</div>

	<div class="tg-topics">
		{#each topics as t}
			<div class="topic-row" style="--topic-color: {t.color}">
				<span class="topic-version">{t.version}</span>
				<div class="topic-mid">
					<div class="topic-label">{t.label}</div>
					<div class="topic-sub">→ {t.subscriber}</div>
				</div>
				<span class="topic-rate">{t.rate}Hz</span>
			</div>
		{/each}
	</div>
</div>

<style>
	.tg {
		display: flex;
		flex-direction: column;
		gap: 8px;
		padding: 8px 4px;
		font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
	}

	.src-node {
		display: flex;
		align-items: center;
		gap: 8px;
		padding: 8px 10px;
		background: #111113;
		border: 1px solid #2a2a2e;
		border-radius: 4px;
	}

	.src-dot {
		width: 10px;
		height: 10px;
		border-radius: 50%;
		background: #444;
	}

	.src-dot.src-live {
		background: #34d399;
		box-shadow: 0 0 4px rgba(52, 211, 153, 0.5);
	}

	.src-info {
		flex: 1;
		display: flex;
		flex-direction: column;
	}

	.src-title {
		font-size: 11px;
		font-weight: 600;
		color: #d4d0c8;
	}

	.src-uri {
		font-size: 9px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #555;
	}

	.src-state {
		font-size: 10px;
		font-weight: 700;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #f5a623;
		letter-spacing: 0.06em;
	}

	.tg-topics {
		display: flex;
		flex-direction: column;
		gap: 4px;
	}

	.topic-row {
		display: flex;
		align-items: center;
		gap: 10px;
		padding: 6px 10px;
		background: #161618;
		border: 1px solid #1d1d20;
		border-left: 3px solid var(--topic-color, #2a2a2e);
		border-radius: 4px;
	}

	.topic-version {
		font-size: 10px;
		font-weight: 700;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: var(--topic-color, #555);
		letter-spacing: 0.06em;
	}

	.topic-mid {
		flex: 1;
		display: flex;
		flex-direction: column;
		min-width: 0;
	}

	.topic-label {
		font-size: 11px;
		color: #d4d0c8;
	}

	.topic-sub {
		font-size: 9px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #555;
		overflow: hidden;
		text-overflow: ellipsis;
		white-space: nowrap;
	}

	.topic-rate {
		font-size: 11px;
		font-weight: 600;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #f5a623;
	}
</style>
