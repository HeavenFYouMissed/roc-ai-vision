<!--
	roc-ai-vision · dashboard · DiagnosticsStrip.svelte

	26px pinned strip below TopChrome. 6 status tiles with OK/Warn/Err/Stale
	colors. Hover tooltip shows threshold + source + sparkline of last 30
	values. Click cycles text-only / text+graph mode (RTSS pattern).
-->
<script lang="ts">
	import { trackPool } from '$lib/stores/track_pool.svelte';
	import { telemetrySocket } from '$lib/telemetry/telemetry_socket.svelte';

	type TileStatus = 'ok' | 'warn' | 'err' | 'stale';

	interface DiagTile {
		key: string;
		label: string;
		value: string;
		raw: number;
		status: TileStatus;
		unit: string;
		threshold: string;
		source: string;
	}

	// FPS counter (zero-allocation RAF tick)
	let fps = $state(0);
	let fpsHistory: number[] = $state([]);

	// Inference latency (delta between frame seq updates from trackPool)
	let infMs = $state(0);
	let infHistory: number[] = $state([]);

	// WS rate (frames/s derivative)
	let wsRate = $state(0);
	let wsHistory: number[] = $state([]);

	// EKF measurement age (ms since last frame)
	let ekfAgeMs = $state(0);

	// Track count history
	let trkHistory: number[] = $state([]);

	// Per-tile display mode (RTSS pattern)
	type DisplayMode = 'text' | 'text-graph';
	let tileModes = $state<Record<string, DisplayMode>>({
		INF: 'text',
		FPS: 'text',
		TRK: 'text',
		WS: 'text',
		ACT: 'text',
		EKF: 'text'
	});

	let hoveredTile = $state<string | null>(null);

	$effect(() => {
		let rafId: number;
		let frameCount = 0;
		let lastFpsTick = performance.now();
		let lastFrameSeq = -1;
		let lastFrameTime = 0;
		let lastWsCount = 0;
		let lastWsTick = performance.now();

		function tick() {
			frameCount++;
			const now = performance.now();

			// FPS - update every 250ms
			if (now - lastFpsTick >= 250) {
				const newFps = Math.round((frameCount * 1000) / (now - lastFpsTick));
				fps = newFps;
				fpsHistory = [...fpsHistory.slice(-29), newFps];
				frameCount = 0;
				lastFpsTick = now;

				// WS rate (frames/s derivative)
				const wsDelta = telemetrySocket.framesReceived - lastWsCount;
				const wsRateNow = Math.round((wsDelta * 1000) / (now - lastWsTick));
				wsRate = wsRateNow;
				wsHistory = [...wsHistory.slice(-29), wsRateNow];
				lastWsCount = telemetrySocket.framesReceived;
				lastWsTick = now;

				// Track count history
				trkHistory = [...trkHistory.slice(-29), trackPool.activeCount];

				// EKF age (ms since last frame)
				if (lastFrameTime > 0) {
					ekfAgeMs = now - lastFrameTime;
				}
			}

			// Inference latency from inter-frame deltas
			const seq = trackPool.frameSeq;
			if (seq !== lastFrameSeq && lastFrameTime > 0) {
				const delta = now - lastFrameTime;
				infMs = delta;
				infHistory = [...infHistory.slice(-29), delta];
				lastFrameTime = now;
				lastFrameSeq = seq;
			} else if (seq !== lastFrameSeq) {
				lastFrameTime = now;
				lastFrameSeq = seq;
			}

			rafId = requestAnimationFrame(tick);
		}
		rafId = requestAnimationFrame(tick);
		return () => cancelAnimationFrame(rafId);
	});

	const tiles = $derived<DiagTile[]>([
		{
			key: 'INF',
			label: 'INF',
			value: infMs > 0 ? `${infMs.toFixed(1)}` : '---',
			raw: infMs,
			unit: 'ms',
			status:
				infMs === 0 ? 'stale' : infMs > 50 ? 'err' : infMs > 30 ? 'warn' : 'ok',
			threshold: 'p99 > 30ms = Warn, > 50ms = Err',
			source: 'frame seq inter-arrival delta'
		},
		{
			key: 'FPS',
			label: 'FPS',
			value: String(fps),
			raw: fps,
			unit: '',
			status: fps >= 30 ? 'ok' : fps >= 10 ? 'warn' : 'err',
			threshold: '< 30 = Warn, < 10 = Err',
			source: 'requestAnimationFrame counter'
		},
		{
			key: 'TRK',
			label: 'TRK',
			value: String(trackPool.activeCount),
			raw: trackPool.activeCount,
			unit: '',
			status: trackPool.activeCount > 0 ? 'ok' : 'stale',
			threshold: 'informational',
			source: 'trackPool.activeCount'
		},
		{
			key: 'WS',
			label: 'WS',
			value:
				telemetrySocket.state === 'connected'
					? wsRate > 0
						? `${wsRate}/s`
						: 'CONN'
					: 'OFF',
			raw: wsRate,
			unit: '',
			status:
				telemetrySocket.state !== 'connected'
					? 'stale'
					: wsRate >= 20
						? 'ok'
						: wsRate >= 5
							? 'warn'
							: wsRate === 0
								? 'stale'
								: 'err',
			threshold: '< 20 = Warn, < 5 = Err',
			source: 'telemetrySocket.framesReceived/s'
		},
		{
			key: 'ACT',
			label: 'ACT',
			value: '---',
			raw: 0,
			unit: 'ms',
			status: 'stale',
			threshold: '> 25ms = Warn',
			source: 'actuator ack round-trip (Phase 6+)'
		},
		{
			key: 'EKF',
			label: 'EKF',
			value: ekfAgeMs > 0 ? `${ekfAgeMs.toFixed(0)}` : '---',
			raw: ekfAgeMs,
			unit: 'ms',
			status:
				ekfAgeMs === 0
					? 'stale'
					: ekfAgeMs > 200
						? 'warn'
						: 'ok',
			threshold: '> 200ms = Warn',
			source: 'now - lastFrameReceiveTime'
		}
	]);

	const STATUS_COLORS: Record<TileStatus, string> = {
		ok: '#34d399',
		warn: '#f5a623',
		err: '#ef4444',
		stale: '#444'
	};

	function tileHistory(key: string): number[] {
		if (key === 'INF') return infHistory;
		if (key === 'FPS') return fpsHistory;
		if (key === 'WS') return wsHistory;
		if (key === 'TRK') return trkHistory;
		return [];
	}

	function sparklinePath(values: number[], w: number, h: number): string {
		if (values.length < 2) return '';
		const max = Math.max(...values, 1);
		const stepX = w / (values.length - 1);
		const parts: string[] = [];
		for (let i = 0; i < values.length; i++) {
			const x = i * stepX;
			const y = h - (values[i] / max) * h;
			parts.push(i === 0 ? `M ${x} ${y}` : `L ${x} ${y}`);
		}
		return parts.join(' ');
	}

	function cycleMode(key: string) {
		tileModes = {
			...tileModes,
			[key]: tileModes[key] === 'text' ? 'text-graph' : 'text'
		};
	}

	function tileStats(key: string): { min: number; max: number; avg: number } | null {
		const h = tileHistory(key);
		if (h.length === 0) return null;
		let min = Infinity,
			max = -Infinity,
			sum = 0;
		for (const v of h) {
			if (v < min) min = v;
			if (v > max) max = v;
			sum += v;
		}
		return { min, max, avg: sum / h.length };
	}
</script>

<div class="diag-strip" role="status" aria-label="System diagnostics">
	{#each tiles as tile (tile.key)}
		<button
			class="diag-tile"
			class:tile-graph={tileModes[tile.key] === 'text-graph'}
			onclick={() => cycleMode(tile.key)}
			onmouseenter={() => (hoveredTile = tile.key)}
			onmouseleave={() => (hoveredTile = null)}
			title="{tile.label}: {tile.threshold} (source: {tile.source})"
		>
			<span class="tile-dot" style="background: {STATUS_COLORS[tile.status]}"></span>
			<span class="tile-label">{tile.label}</span>
			<span
				class="tile-value"
				class:tile-warn={tile.status === 'warn'}
				class:tile-err={tile.status === 'err'}
				class:tile-stale={tile.status === 'stale'}
			>{tile.value}{tile.unit ? `${tile.unit}` : ''}</span>
			{#if tileModes[tile.key] === 'text-graph' && tileHistory(tile.key).length > 1}
				<svg class="tile-spark" viewBox="0 0 30 12" preserveAspectRatio="none">
					<path
						d={sparklinePath(tileHistory(tile.key), 30, 12)}
						fill="none"
						stroke={STATUS_COLORS[tile.status]}
						stroke-width="1"
					></path>
				</svg>
			{/if}

			{#if hoveredTile === tile.key && tileHistory(tile.key).length > 0}
				{@const stats = tileStats(tile.key)}
				<div class="tile-tooltip" role="tooltip">
					<div class="tt-header">{tile.label} · {tile.source}</div>
					<div class="tt-threshold">{tile.threshold}</div>
					<svg class="tt-spark" viewBox="0 0 100 30" preserveAspectRatio="none">
						<path
							d={sparklinePath(tileHistory(tile.key), 100, 30)}
							fill="none"
							stroke={STATUS_COLORS[tile.status]}
							stroke-width="1.2"
						></path>
					</svg>
					{#if stats}
						<div class="tt-stats">
							<span>min {stats.min.toFixed(1)}</span>
							<span>avg {stats.avg.toFixed(1)}</span>
							<span>max {stats.max.toFixed(1)}</span>
						</div>
					{/if}
				</div>
			{/if}
		</button>
	{/each}
</div>

<style>
	.diag-strip {
		display: flex;
		align-items: center;
		gap: 5px;
		height: 26px;
		padding: 0 12px;
		background: #111113;
		border-bottom: 1px solid #222;
		flex-shrink: 0;
		overflow-x: auto;
	}

	/* Each tile is a discrete instrument chip (Claude polish #1) */
	.diag-tile {
		position: relative;
		display: grid;
		grid-template-columns: 8px 28px 1fr;
		align-items: center;
		gap: 6px;
		height: 18px;
		padding: 0 8px;
		background: transparent;
		border: 1px solid rgba(255, 255, 255, 0.06);
		border-radius: 3px;
		white-space: nowrap;
		cursor: pointer;
		font-family: inherit;
		color: inherit;
		transition: background 100ms ease, border-color 100ms ease;
		min-width: 78px;
	}

	.diag-tile:hover {
		background: #1a1a1e;
		border-color: rgba(255, 255, 255, 0.12);
	}

	.tile-dot {
		width: 6px;
		height: 6px;
		border-radius: 50%;
		flex-shrink: 0;
		box-shadow: 0 0 3px currentColor;
	}

	.tile-label {
		font-size: 9px;
		font-weight: 700;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #555;
		letter-spacing: 0.08em;
		text-transform: uppercase;
	}

	.tile-value {
		font-size: 11px;
		font-weight: 600;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #d4d0c8;
		text-align: right;
		justify-self: end;
	}

	.tile-warn { color: #f5a623; }
	.tile-err { color: #ef4444; }
	.tile-stale { color: #555; }

	.tile-spark {
		width: 30px;
		height: 12px;
		display: block;
	}

	.tile-tooltip {
		position: absolute;
		top: calc(100% + 6px);
		left: 0;
		min-width: 180px;
		background: rgba(22, 22, 24, 0.96);
		backdrop-filter: blur(8px);
		border: 1px solid rgba(39, 39, 42, 0.9);
		border-radius: 5px;
		padding: 8px 10px;
		display: flex;
		flex-direction: column;
		gap: 4px;
		font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
		z-index: 100;
		pointer-events: none;
		animation: tooltip-fade 100ms ease;
	}

	@keyframes tooltip-fade {
		from { opacity: 0; transform: translateY(-2px); }
		to { opacity: 1; transform: translateY(0); }
	}

	.tt-header {
		font-size: 10px;
		font-weight: 600;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #f5a623;
	}

	.tt-threshold {
		font-size: 10px;
		color: #878787;
	}

	.tt-spark {
		width: 100%;
		height: 30px;
		background: #06080c;
		border-radius: 3px;
	}

	.tt-stats {
		display: flex;
		justify-content: space-between;
		font-size: 9px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #555;
	}
</style>
