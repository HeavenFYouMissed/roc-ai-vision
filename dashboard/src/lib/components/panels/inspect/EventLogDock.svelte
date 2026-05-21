<!--
	roc-ai-vision · dashboard · EventLogDock.svelte

	Bottom dock across Inspect destination. Collapsed 32px shows last event;
	expanded 240px shows scrolling event list with tabs / filter / search.
	Hotkey: ` (backtick) toggles.
-->
<script lang="ts">
	import { eventLog, type EventLevel } from '$lib/stores/event_log.svelte';

	const LEVEL_COLORS: Record<EventLevel, string> = {
		debug: '#555',
		info: '#00f0ff',
		warn: '#f5a623',
		err: '#ef4444',
		critical: '#ef4444'
	};

	function formatTs(ms: number): string {
		const d = new Date(ms);
		const hh = String(d.getHours()).padStart(2, '0');
		const mm = String(d.getMinutes()).padStart(2, '0');
		const ss = String(d.getSeconds()).padStart(2, '0');
		const ms3 = String(d.getMilliseconds()).padStart(3, '0');
		return `${hh}:${mm}:${ss}.${ms3}`;
	}

	const filtered = $derived(eventLog.filtered);
	const last = $derived(eventLog.lastEvent);
</script>

<div class="event-dock" class:collapsed={eventLog.collapsed}>
	{#if eventLog.collapsed}
		<button class="dock-collapsed-bar" onclick={() => eventLog.expand()}>
			{#if last}
				<span class="last-ts">{formatTs(last.tsMs)}</span>
				<span class="last-level" style="color: {LEVEL_COLORS[last.level]}">
					[{last.level.toUpperCase()}]
				</span>
				<span class="last-source">[{last.source}]</span>
				<span class="last-msg">{last.message}</span>
			{:else}
				<span class="last-msg">No events</span>
			{/if}
			<span class="dock-hint">` to expand</span>
		</button>
	{:else}
		<header class="dock-header">
			<div class="dock-tabs">
				<button
					class="tab-btn"
					class:active={eventLog.activeTab === 'console'}
					onclick={() => (eventLog.activeTab = 'console')}
				>Console</button>
				<button
					class="tab-btn"
					class:active={eventLog.activeTab === 'network'}
					onclick={() => (eventLog.activeTab = 'network')}
				>Network</button>
				<button
					class="tab-btn"
					class:active={eventLog.activeTab === 'performance'}
					onclick={() => (eventLog.activeTab = 'performance')}
				>Performance</button>
			</div>

			<div class="dock-filters">
				<input
					type="text"
					placeholder="Filter..."
					bind:value={eventLog.searchFilter}
					class="search-input"
				/>
				<select bind:value={eventLog.levelFilter} class="level-select">
					<option value="all">All</option>
					<option value="debug">Debug+</option>
					<option value="info">Info+</option>
					<option value="warn">Warn+</option>
					<option value="err">Error+</option>
					<option value="critical">Critical</option>
				</select>
				<label class="autoscroll-label">
					<input type="checkbox" bind:checked={eventLog.autoScroll} />
					Auto
				</label>
				<button class="header-btn" onclick={() => eventLog.clear()} title="Clear non-pinned events">
					Clear
				</button>
				<button class="header-btn" onclick={() => eventLog.collapse()} title="Collapse (`)">
					Hide
				</button>
			</div>
		</header>

		<div class="dock-body">
			{#if eventLog.activeTab === 'console'}
				{#each filtered as ev (ev.id)}
					<div class="ev-row" class:pinned={ev.pinned} class:critical={ev.level === 'critical'}>
						<span class="ev-ts">{formatTs(ev.tsMs)}</span>
						<span
							class="ev-level"
							style="color: {LEVEL_COLORS[ev.level]}"
						>[{ev.level.toUpperCase()}]</span>
						<span class="ev-source">[{ev.source}]</span>
						<span class="ev-msg">{ev.message}</span>
						{#if ev.pinned}
							<span class="pin-icon">★</span>
						{/if}
					</div>
				{/each}
				{#if filtered.length === 0}
					<div class="ev-empty">No events match the filter</div>
				{/if}
			{:else if eventLog.activeTab === 'network'}
				<div class="ev-tab-placeholder">
					WS bandwidth + frames-per-second-per-version (Phase 6+)
				</div>
			{:else}
				<div class="ev-tab-placeholder">
					RAF tick histogram + render lag percentiles (Phase 6+)
				</div>
			{/if}
		</div>
	{/if}
</div>

<style>
	.event-dock {
		background: #0e0e10;
		border-top: 1px solid #27272a;
		display: flex;
		flex-direction: column;
		transition: height 150ms ease;
		flex-shrink: 0;
	}

	.event-dock:not(.collapsed) {
		height: 240px;
	}

	.event-dock.collapsed {
		height: 30px;
	}

	.dock-collapsed-bar {
		display: flex;
		align-items: center;
		gap: 8px;
		width: 100%;
		height: 100%;
		padding: 0 14px;
		background: transparent;
		border: none;
		color: inherit;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 10px;
		text-align: left;
		cursor: pointer;
		transition: background 80ms ease;
	}

	.dock-collapsed-bar:hover {
		background: #161618;
	}

	.last-ts {
		color: #555;
	}

	.last-level {
		font-weight: 700;
	}

	.last-source {
		color: #878787;
	}

	.last-msg {
		flex: 1;
		color: #d4d0c8;
		overflow: hidden;
		text-overflow: ellipsis;
		white-space: nowrap;
	}

	.dock-hint {
		color: #555;
		font-size: 9px;
	}

	.dock-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding: 6px 10px;
		background: #161618;
		border-bottom: 1px solid #1d1d20;
		flex-shrink: 0;
		gap: 12px;
	}

	.dock-tabs {
		display: flex;
		gap: 2px;
	}

	.tab-btn {
		padding: 4px 10px;
		background: transparent;
		border: none;
		border-bottom: 2px solid transparent;
		color: #555;
		font-size: 11px;
		font-weight: 500;
		font-family: inherit;
		cursor: pointer;
	}

	.tab-btn:hover {
		color: #d4d0c8;
	}

	.tab-btn.active {
		color: #f5a623;
		border-bottom-color: #f5a623;
	}

	.dock-filters {
		display: flex;
		align-items: center;
		gap: 6px;
	}

	.search-input {
		padding: 3px 8px;
		background: #111113;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		color: #d4d0c8;
		font-size: 10px;
		font-family: inherit;
		outline: none;
		width: 120px;
	}

	.search-input:focus-visible {
		border-color: #f5a623;
	}

	.level-select {
		padding: 3px 6px;
		background: #111113;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		color: #d4d0c8;
		font-size: 10px;
		font-family: inherit;
		cursor: pointer;
		outline: none;
	}

	.autoscroll-label {
		display: flex;
		align-items: center;
		gap: 4px;
		font-size: 10px;
		color: #878787;
		cursor: pointer;
	}

	.autoscroll-label input {
		accent-color: #f5a623;
	}

	.header-btn {
		padding: 3px 8px;
		background: transparent;
		color: #878787;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		font-size: 10px;
		font-family: inherit;
		cursor: pointer;
	}

	.header-btn:hover {
		color: #d4d0c8;
		border-color: #3a3a3e;
	}

	.dock-body {
		flex: 1;
		overflow-y: auto;
		padding: 4px 0;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
	}

	.ev-row {
		display: flex;
		align-items: center;
		gap: 8px;
		padding: 2px 14px;
		font-size: 10px;
		color: #d4d0c8;
	}

	.ev-row:hover {
		background: #161618;
	}

	.ev-row.pinned {
		background: rgba(245, 166, 35, 0.05);
	}

	.ev-row.critical {
		background: rgba(239, 68, 68, 0.08);
		animation: critical-pulse 1.5s ease-in-out infinite;
	}

	@keyframes critical-pulse {
		0%, 100% { background: rgba(239, 68, 68, 0.08); }
		50% { background: rgba(239, 68, 68, 0.15); }
	}

	.ev-ts {
		color: #555;
		flex-shrink: 0;
	}

	.ev-level {
		font-weight: 700;
		min-width: 56px;
	}

	.ev-source {
		color: #878787;
		min-width: 80px;
	}

	.ev-msg {
		flex: 1;
		color: #d4d0c8;
	}

	.pin-icon {
		color: #f5a623;
		font-size: 11px;
	}

	.ev-empty {
		padding: 20px;
		text-align: center;
		color: #555;
		font-style: italic;
	}

	.ev-tab-placeholder {
		padding: 24px;
		text-align: center;
		color: #555;
		font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
		font-size: 11px;
		font-style: italic;
	}
</style>
