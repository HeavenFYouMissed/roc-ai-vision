<!--
	roc-ai-vision · dashboard · TargetLockModal.svelte

	Skydio Pause-slider anti-fat-finger confirmation for committing a target
	lock. User must hold the [HOLD TO CONFIRM] bar for 800ms continuously to
	commit. Release before complete = cancel. Esc cancels at any time.

	Per dashboard_pro_console_ux_2026.md Rec 1: "anti-fat-finger".
-->
<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { engageState } from '$lib/stores/engage_state.svelte';
	import { trackPool } from '$lib/stores/track_pool.svelte';
	import { CLASS_NAMES, CLASS_COLORS } from '$lib/render/class_colors';

	const HOLD_DURATION_MS = 800;

	let holdAnimId = 0;
	let holdStartMs = 0;
	let isHolding = $state(false);

	const slot = $derived(engageState.pendingLockSlot);
	const trackData = $derived.by(() => {
		if (slot < 0 || trackPool.presenceBitmap[slot] !== 1) return null;
		const b4 = slot * 4;
		return {
			idString: trackPool.idStrings[slot] || `0x${trackPool.trackIdLo[slot].toString(16)}`,
			label: trackPool.label[slot],
			className: CLASS_NAMES[trackPool.label[slot]],
			confidence: trackPool.confidence[slot],
			u: trackPool.bboxOrig[b4 + 0],
			v: trackPool.bboxOrig[b4 + 1],
			w: trackPool.bboxOrig[b4 + 2],
			h: trackPool.bboxOrig[b4 + 3],
			distFromCenter: Math.hypot(
				trackPool.bboxOrig[b4 + 0] - 960,
				trackPool.bboxOrig[b4 + 1] - 540
			)
		};
	});

	function holdTick() {
		const elapsed = performance.now() - holdStartMs;
		const progress = Math.min(1, elapsed / HOLD_DURATION_MS);
		engageState.lockConfirmHoldProgress = progress;
		if (progress >= 1) {
			isHolding = false;
			cancelAnimationFrame(holdAnimId);
			engageState.commitLock();
			return;
		}
		holdAnimId = requestAnimationFrame(holdTick);
	}

	function handleHoldStart(e: PointerEvent | TouchEvent) {
		e.preventDefault();
		if (isHolding) return;
		isHolding = true;
		holdStartMs = performance.now();
		engageState.lockConfirmHoldProgress = 0;
		holdAnimId = requestAnimationFrame(holdTick);
	}

	function handleHoldEnd() {
		if (!isHolding) return;
		isHolding = false;
		cancelAnimationFrame(holdAnimId);
		engageState.lockConfirmHoldProgress = 0;
	}

	function handleCancel() {
		handleHoldEnd();
		engageState.cancelLockModal();
	}

	function handleBackdropClick() {
		handleCancel();
	}

	function handleKeydown(e: KeyboardEvent) {
		if (e.key === 'Escape') {
			e.preventDefault();
			handleCancel();
		}
	}

	onMount(() => {
		window.addEventListener('keydown', handleKeydown);
	});

	onDestroy(() => {
		window.removeEventListener('keydown', handleKeydown);
		cancelAnimationFrame(holdAnimId);
	});
</script>

<div class="lock-backdrop" onclick={handleBackdropClick} role="presentation">
	<div class="lock-modal" onclick={(e) => e.stopPropagation()} role="dialog" aria-modal="true" aria-label="Target lock confirmation">
		<header class="lock-header">
			<h2>LOCK TARGET</h2>
			<button class="x-btn" onclick={handleCancel} aria-label="Cancel">
				<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
					<line x1="18" y1="6" x2="6" y2="18"></line>
					<line x1="6" y1="6" x2="18" y2="18"></line>
				</svg>
			</button>
		</header>

		{#if trackData}
			<div class="lock-target-info">
				<div class="info-row">
					<span class="info-key">Track</span>
					<span class="info-val">
						<span class="track-dot" style="background: {CLASS_COLORS[trackData.label]}"></span>
						{trackData.idString} <span class="track-class">{trackData.className}</span>
					</span>
				</div>
				<div class="info-row">
					<span class="info-key">Anchor</span>
					<span class="info-val mono">bbox_center</span>
				</div>
				<div class="info-row">
					<span class="info-key">Confidence</span>
					<span class="info-val mono">{(trackData.confidence * 100).toFixed(1)}%</span>
				</div>
				<div class="info-row">
					<span class="info-key">Position (px)</span>
					<span class="info-val mono">{trackData.u.toFixed(0)}, {trackData.v.toFixed(0)}</span>
				</div>
				<div class="info-row">
					<span class="info-key">BBox</span>
					<span class="info-val mono">{trackData.w.toFixed(0)} x {trackData.h.toFixed(0)}</span>
				</div>
				<div class="info-row">
					<span class="info-key">Distance from center</span>
					<span class="info-val mono">{trackData.distFromCenter.toFixed(1)} px</span>
				</div>
			</div>

			<div class="hold-section">
				<button
					class="hold-btn"
					onpointerdown={handleHoldStart}
					onpointerup={handleHoldEnd}
					onpointerleave={handleHoldEnd}
					onpointercancel={handleHoldEnd}
				>
					<div class="hold-fill" style="width: {engageState.lockConfirmHoldProgress * 100}%"></div>
					<div class="hold-text">
						{#if isHolding}
							CONFIRMING... {(engageState.lockConfirmHoldProgress * 100).toFixed(0)}%
						{:else}
							HOLD TO CONFIRM
						{/if}
					</div>
				</button>
				<p class="hold-hint">Press and hold for 800ms · Release to cancel · Esc to abort</p>
			</div>

			<div class="lock-actions">
				<button class="action-cancel" onclick={handleCancel}>CANCEL (Esc)</button>
			</div>
		{:else}
			<div class="empty">Track no longer visible</div>
			<div class="lock-actions">
				<button class="action-cancel" onclick={handleCancel}>CLOSE</button>
			</div>
		{/if}
	</div>
</div>

<style>
	.lock-backdrop {
		position: fixed;
		inset: 0;
		background: rgba(0, 0, 0, 0.7);
		display: flex;
		align-items: center;
		justify-content: center;
		z-index: 200;
		backdrop-filter: blur(4px);
	}

	.lock-modal {
		width: 460px;
		background: #161618;
		border: 1px solid #ef4444;
		border-radius: 8px;
		box-shadow: 0 24px 60px rgba(0, 0, 0, 0.6), 0 0 0 1px rgba(239, 68, 68, 0.1);
		overflow: hidden;
		display: flex;
		flex-direction: column;
		font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
	}

	.lock-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding: 14px 18px;
		background: rgba(239, 68, 68, 0.08);
		border-bottom: 1px solid #27272a;
	}

	.lock-header h2 {
		margin: 0;
		font-size: 13px;
		font-weight: 800;
		color: #ef4444;
		letter-spacing: 0.12em;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
	}

	.x-btn {
		width: 24px;
		height: 24px;
		background: transparent;
		border: none;
		color: #555;
		cursor: pointer;
		display: flex;
		align-items: center;
		justify-content: center;
		border-radius: 3px;
	}

	.x-btn:hover {
		background: #27272a;
		color: #d4d0c8;
	}

	.x-btn svg {
		width: 14px;
		height: 14px;
	}

	.lock-target-info {
		padding: 14px 18px;
		display: flex;
		flex-direction: column;
		gap: 5px;
		border-bottom: 1px solid #27272a;
	}

	.info-row {
		display: flex;
		justify-content: space-between;
		align-items: center;
		gap: 12px;
		font-size: 12px;
	}

	.info-key {
		color: #878787;
	}

	.info-val {
		display: flex;
		align-items: center;
		gap: 6px;
		color: #d4d0c8;
	}

	.info-val.mono {
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
	}

	.track-dot {
		width: 8px;
		height: 8px;
		border-radius: 50%;
		box-shadow: 0 0 4px currentColor;
	}

	.track-class {
		color: #878787;
		margin-left: 4px;
		font-size: 11px;
	}

	.hold-section {
		padding: 18px;
		display: flex;
		flex-direction: column;
		gap: 8px;
	}

	.hold-btn {
		position: relative;
		width: 100%;
		height: 48px;
		background: #0a0a0c;
		border: 2px solid #ef4444;
		border-radius: 5px;
		overflow: hidden;
		cursor: pointer;
		font-family: inherit;
		color: inherit;
		user-select: none;
		touch-action: none;
	}

	.hold-btn:hover {
		border-color: #f87171;
	}

	.hold-fill {
		position: absolute;
		inset: 0;
		background: linear-gradient(90deg, #ef4444 0%, #f87171 100%);
		transition: width 16ms linear;
		width: 0%;
	}

	.hold-text {
		position: relative;
		display: flex;
		align-items: center;
		justify-content: center;
		height: 100%;
		font-size: 14px;
		font-weight: 800;
		letter-spacing: 0.1em;
		color: #ef4444;
		mix-blend-mode: difference;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
	}

	.hold-hint {
		margin: 0;
		text-align: center;
		font-size: 10px;
		color: #555;
		font-style: italic;
	}

	.lock-actions {
		padding: 12px 18px;
		display: flex;
		justify-content: flex-end;
		border-top: 1px solid #27272a;
		background: #111113;
	}

	.action-cancel {
		padding: 6px 16px;
		background: transparent;
		color: #878787;
		border: 1px solid #2a2a2e;
		border-radius: 4px;
		font-family: inherit;
		font-size: 11px;
		font-weight: 600;
		letter-spacing: 0.06em;
		cursor: pointer;
	}

	.action-cancel:hover {
		color: #d4d0c8;
		border-color: #3a3a3e;
	}

	.empty {
		padding: 32px;
		text-align: center;
		color: #555;
		font-style: italic;
	}
</style>
