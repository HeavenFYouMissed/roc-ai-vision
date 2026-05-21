<!--
	roc-ai-vision · dashboard · biometrics/SessionStatsPanel.svelte

	Monospaced text grid of session statistics, matching the reference image's
	"Session" panel exactly. Refreshes at 10 Hz (NOT 60 Hz) to avoid text
	jitter; the live trace + sparklines do their own 60Hz redraw.

	Fields rendered (in order):
	  Duration: 1.806s
	  Path points: 232
	  Reports: 231
	  Dir changes >= 45deg: 38
	  Sign flips X/Y: 28 / 29
	  Flips/sec: 31.56
-->
<script lang="ts">
	import type { CaptureSlot } from '$lib/biometrics/mouse_capture.svelte';
	import { sessionStats, degToRad, type SessionStats } from '$lib/biometrics/metrics';

	interface Props {
		slot: CaptureSlot;
		version: number;
		angleThreshDeg: number;
		label?: string;
	}

	let { slot, version, angleThreshDeg, label = 'Session' }: Props = $props();

	let stats = $state<SessionStats>({
		durationS: 0,
		pathPoints: 0,
		reports: 0,
		dirChanges: 0,
		signFlipsX: 0,
		signFlipsY: 0,
		flipsPerSec: 0
	});

	// 10 Hz refresh — text panels are read by humans, not strobed.
	let lastComputeMs = 0;
	$effect(() => {
		// eslint-disable-next-line @typescript-eslint/no-unused-expressions
		version;
		const now = performance.now();
		if (now - lastComputeMs < 100) return;
		lastComputeMs = now;
		stats = sessionStats(slot, degToRad(angleThreshDeg));
	});
</script>

<div class="session-panel">
	<div class="session-header">{label}</div>
	<!--
		Each stat is a vertical "stack" (label on top, value below) rather
		than a 2-column grid. The session column is intentionally narrow
		(12fr of the 100fr row) and the longest dt token (`Dir changes >= 45°:`)
		is wider than the cell itself; a horizontal grid would push the
		dd off the right edge where overflow:hidden then clips it.
		Vertical stacking guarantees the value is always visible regardless
		of how narrow the column gets.
	-->
	<div class="session-stack">
		<div class="session-stat">
			<span class="stat-label">Duration</span>
			<span class="stat-value">{stats.durationS.toFixed(3)}s</span>
		</div>
		<div class="session-stat">
			<span class="stat-label">Path points</span>
			<span class="stat-value">{stats.pathPoints}</span>
		</div>
		<div class="session-stat">
			<span class="stat-label">Reports</span>
			<span class="stat-value">{stats.reports}</span>
		</div>
		<div class="session-stat">
			<span class="stat-label">Dir &Delta; &ge; {angleThreshDeg}&deg;</span>
			<span class="stat-value">{stats.dirChanges}</span>
		</div>
		<div class="session-stat">
			<span class="stat-label">Sign flips X / Y</span>
			<span class="stat-value">{stats.signFlipsX} / {stats.signFlipsY}</span>
		</div>
		<div class="session-stat">
			<span class="stat-label">Flips/sec</span>
			<span class="stat-value">{stats.flipsPerSec.toFixed(2)}</span>
		</div>
	</div>
</div>

<style>
	.session-panel {
		display: flex;
		flex-direction: column;
		background: #161618;
		border: 1px solid #27272a;
		border-radius: 4px;
		overflow: hidden;
		min-height: 0;
		min-width: 0;
	}

	.session-header {
		padding: 4px 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 11px;
		color: #878787;
		background: #111113;
		border-bottom: 1px solid #1d1d20;
		text-transform: lowercase;
		letter-spacing: 0.02em;
		flex-shrink: 0;
	}

	.session-stack {
		display: flex;
		flex-direction: column;
		gap: 8px;
		padding: 10px 12px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 11px;
		color: #d4d0c8;
		overflow: hidden;
		flex: 1;
		min-height: 0;
		min-width: 0;
	}

	.session-stat {
		display: flex;
		flex-direction: column;
		gap: 1px;
		min-width: 0;
	}

	.stat-label {
		color: #878787;
		font-size: 9.5px;
		text-transform: uppercase;
		letter-spacing: 0.04em;
		white-space: nowrap;
		overflow: hidden;
		text-overflow: ellipsis;
	}

	.stat-value {
		color: #d4d0c8;
		font-size: 12px;
		font-variant-numeric: tabular-nums;
		white-space: nowrap;
		overflow: hidden;
		text-overflow: ellipsis;
	}
</style>
