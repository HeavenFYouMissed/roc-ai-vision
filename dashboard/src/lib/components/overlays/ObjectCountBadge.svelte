<!--
	roc-ai-vision · dashboard · ObjectCountBadge.svelte

	Floating bottom-right of OperateCanvas. "8 objects detected" with
	per-class color dot strip + hover popover showing per-class breakdown.
-->
<script lang="ts">
	import { trackPool, MAX_SLOTS } from '$lib/stores/track_pool.svelte';
	import { renderSettings } from '$lib/stores/render_settings.svelte';
	import { CLASS_COLORS, CLASS_NAMES } from '$lib/render/class_colors';

	let popoverOpen = $state(false);

	// Reactive per-class active counts - read from trackPool every render
	const perClassCounts = $derived.by(() => {
		// triggers on presenceVersion bump
		void trackPool.presenceVersion;
		const counts = new Array<number>(CLASS_NAMES.length).fill(0);
		for (let s = 0; s < MAX_SLOTS; s++) {
			if (trackPool.presenceBitmap[s] !== 1) continue;
			const conf = trackPool.confidence[s];
			const label = trackPool.label[s];
			if (conf < renderSettings.confidenceThreshold) continue;
			if (!renderSettings.classVisibility[label]) continue;
			counts[label]++;
		}
		return counts;
	});

	const totalVisible = $derived(perClassCounts.reduce((s, n) => s + n, 0));
</script>

<div
	class="badge-wrap"
	onmouseenter={() => (popoverOpen = true)}
	onmouseleave={() => (popoverOpen = false)}
	role="status"
	aria-label="Object count"
>
	<div class="badge">
		<div class="dot-strip">
			{#each perClassCounts as count, label}
				{#if count > 0}
					<span class="dot" style="background: {CLASS_COLORS[label]}"></span>
				{/if}
			{/each}
		</div>
		<span class="count-num">{totalVisible}</span>
		<span class="count-label">{totalVisible === 1 ? 'object' : 'objects'} detected</span>
	</div>

	{#if popoverOpen && totalVisible > 0}
		<div class="popover" role="tooltip">
			{#each perClassCounts as count, label}
				{#if count > 0}
					<div class="popover-row">
						<span class="pop-dot" style="background: {CLASS_COLORS[label]}"></span>
						<span class="pop-name">{CLASS_NAMES[label]}</span>
						<span class="pop-count">{count}</span>
					</div>
				{/if}
			{/each}
		</div>
	{/if}
</div>

<style>
	.badge-wrap {
		position: absolute;
		bottom: 12px;
		right: 12px;
		z-index: 5;
	}

	.badge {
		display: flex;
		align-items: center;
		gap: 8px;
		padding: 6px 12px;
		background: rgba(22, 22, 24, 0.92);
		backdrop-filter: blur(8px);
		border: 1px solid rgba(39, 39, 42, 0.9);
		border-radius: 14px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 11px;
		color: #d4d0c8;
	}

	.dot-strip {
		display: flex;
		gap: 3px;
	}

	.dot {
		width: 6px;
		height: 6px;
		border-radius: 50%;
		box-shadow: 0 0 4px currentColor;
	}

	.count-num {
		font-weight: 700;
		color: #f5a623;
	}

	.count-label {
		color: #878787;
		font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
		font-size: 10px;
	}

	.popover {
		position: absolute;
		bottom: calc(100% + 6px);
		right: 0;
		min-width: 180px;
		background: rgba(22, 22, 24, 0.96);
		backdrop-filter: blur(8px);
		border: 1px solid rgba(39, 39, 42, 0.9);
		border-radius: 6px;
		padding: 8px 10px;
		display: flex;
		flex-direction: column;
		gap: 4px;
		font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
	}

	.popover-row {
		display: flex;
		align-items: center;
		gap: 8px;
		font-size: 11px;
	}

	.pop-dot {
		width: 8px;
		height: 8px;
		border-radius: 50%;
	}

	.pop-name {
		flex: 1;
		color: #d4d0c8;
	}

	.pop-count {
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-weight: 600;
		color: #f5a623;
	}
</style>
