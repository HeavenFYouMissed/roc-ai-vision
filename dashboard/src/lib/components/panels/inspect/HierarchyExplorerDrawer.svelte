<!--
	roc-ai-vision · dashboard · HierarchyExplorerDrawer.svelte

	Palantir Gotham-style tree of tracks with parent_id graph. Indented
	per-row with class color dot, status icon, right-click pivot menu.
	hierarchy_violation rows get red border + red bg + warning icon.
-->
<script lang="ts">
	import { trackPool, MAX_SLOTS } from '$lib/stores/track_pool.svelte';
	import { selectionStore } from '$lib/stores/selection_store.svelte';
	import { CLASS_COLORS, CLASS_NAMES, STATUS_FLAG_BITS } from '$lib/render/class_colors';

	type FilterMode = 'all' | 'confirmed' | 'tentative' | 'violations';
	let filter = $state<FilterMode>('all');
	let search = $state('');

	let contextMenuOpen = $state(false);
	let contextSlot = $state(-1);
	let contextX = $state(0);
	let contextY = $state(0);

	interface TreeNode {
		slot: number;
		children: TreeNode[];
	}

	const tree = $derived.by(() => {
		void trackPool.presenceVersion;
		// Build parent → children index
		const present: number[] = [];
		const parentOf: number[] = new Array(MAX_SLOTS).fill(-1);
		for (let s = 0; s < MAX_SLOTS; s++) {
			if (trackPool.presenceBitmap[s] !== 1) continue;
			present.push(s);
			if (trackPool.parentIdLo[s] !== 0 || trackPool.parentIdHi[s] !== 0) {
				const p = trackPool.findSlotByTrackId(
					trackPool.parentIdLo[s],
					trackPool.parentIdHi[s]
				);
				parentOf[s] = p;
			}
		}

		// Build tree from roots (parentOf === -1)
		const childrenOf = new Map<number, number[]>();
		for (const s of present) {
			const p = parentOf[s];
			if (p >= 0) {
				const arr = childrenOf.get(p) ?? [];
				arr.push(s);
				childrenOf.set(p, arr);
			}
		}

		function build(slot: number): TreeNode {
			return {
				slot,
				children: (childrenOf.get(slot) ?? []).map(build)
			};
		}

		const roots: TreeNode[] = present.filter((s) => parentOf[s] === -1).map(build);
		return roots;
	});

	function passesFilter(slot: number): boolean {
		const flags = trackPool.statusFlags[slot];
		const matchesFilter =
			filter === 'all' ||
			(filter === 'confirmed' && (flags & STATUS_FLAG_BITS.CONFIRMED) !== 0) ||
			(filter === 'tentative' && (flags & STATUS_FLAG_BITS.CONFIRMED) === 0) ||
			(filter === 'violations' && (flags & STATUS_FLAG_BITS.HIERARCHY_VIOLATION) !== 0);
		const id = trackPool.idStrings[slot] || '';
		const matchesSearch =
			search.length === 0 ||
			id.toLowerCase().includes(search.toLowerCase()) ||
			CLASS_NAMES[trackPool.label[slot]].toLowerCase().includes(search.toLowerCase());
		return matchesFilter && matchesSearch;
	}

	function handleRowClick(slot: number) {
		selectionStore.select(slot);
	}

	function handleRowContextMenu(e: MouseEvent, slot: number) {
		e.preventDefault();
		contextSlot = slot;
		contextX = e.clientX;
		contextY = e.clientY;
		contextMenuOpen = true;
	}

	function closeContextMenu() {
		contextMenuOpen = false;
		contextSlot = -1;
	}

	const counts = $derived.by(() => {
		void trackPool.presenceVersion;
		let total = 0;
		let confirmed = 0;
		let violations = 0;
		for (let s = 0; s < MAX_SLOTS; s++) {
			if (trackPool.presenceBitmap[s] !== 1) continue;
			total++;
			if (trackPool.statusFlags[s] & STATUS_FLAG_BITS.CONFIRMED) confirmed++;
			if (trackPool.statusFlags[s] & STATUS_FLAG_BITS.HIERARCHY_VIOLATION) violations++;
		}
		return { total, confirmed, violations };
	});
</script>

<svelte:window onclick={closeContextMenu} />

<aside class="hier-drawer">
	<header class="hier-header">
		<span class="hier-title">TRACK HIERARCHY</span>
	</header>
	<div class="hier-filter-row">
		<input
			type="text"
			placeholder="Filter by id or class..."
			bind:value={search}
			class="filter-input"
		/>
		<select bind:value={filter} class="filter-select">
			<option value="all">All</option>
			<option value="confirmed">Confirmed</option>
			<option value="tentative">Tentative</option>
			<option value="violations">Violations</option>
		</select>
	</div>

	<div class="hier-tree">
		{#if tree.length === 0}
			<div class="hier-empty">No active tracks</div>
		{:else}
			{#each tree as node (node.slot)}
				{#snippet renderNode(n: TreeNode, depth: number)}
					{#if passesFilter(n.slot)}
						{@const label = trackPool.label[n.slot]}
						{@const isSel = selectionStore.selectedSlot === n.slot}
						{@const flags = trackPool.statusFlags[n.slot]}
						{@const isViolation = (flags & STATUS_FLAG_BITS.HIERARCHY_VIOLATION) !== 0}
						{@const isConfirmed = (flags & STATUS_FLAG_BITS.CONFIRMED) !== 0}
						<button
							class="hier-row"
							class:selected={isSel}
							class:violation={isViolation}
							style="padding-left: {8 + depth * 14}px"
							onclick={() => handleRowClick(n.slot)}
							oncontextmenu={(e) => handleRowContextMenu(e, n.slot)}
						>
							{#if n.children.length > 0}
								<span class="tree-chevron">▼</span>
							{:else}
								<span class="tree-spacer"></span>
							{/if}
							<span class="hier-dot" style="background: {CLASS_COLORS[label]}"></span>
							<span class="hier-id">{trackPool.idStrings[n.slot]}</span>
							<span class="hier-class">{CLASS_NAMES[label]}</span>
							<span class="hier-conf">{(trackPool.confidence[n.slot] * 100).toFixed(0)}%</span>
							{#if isViolation}
								<span class="hier-icon" title="Hierarchy violation">⚠</span>
							{:else if isConfirmed}
								<span class="hier-icon ok" title="Confirmed">✓</span>
							{/if}
						</button>
					{/if}
					{#each n.children as child (child.slot)}
						{@render renderNode(child, depth + 1)}
					{/each}
				{/snippet}
				{@render renderNode(node, 0)}
			{/each}
		{/if}
	</div>

	<footer class="hier-footer">
		<span>{counts.total} tracks</span>
		<span class="footer-sep">·</span>
		<span class="ok">{counts.confirmed} confirmed</span>
		{#if counts.violations > 0}
			<span class="footer-sep">·</span>
			<span class="err">{counts.violations} violations</span>
		{/if}
	</footer>
</aside>

{#if contextMenuOpen && contextSlot >= 0}
	<div
		class="context-menu"
		style="left: {contextX}px; top: {contextY}px"
		role="menu"
		onclick={(e) => e.stopPropagation()}
	>
		<button class="ctx-item" onclick={closeContextMenu}>
			<span class="ctx-icon">↺</span> Replay this track's last 30s
			<span class="ctx-hint">Phase 6+</span>
		</button>
		<button class="ctx-item" onclick={closeContextMenu}>
			<span class="ctx-icon">●</span> Lock actuator on this track
			<span class="ctx-hint">Engage</span>
		</button>
		<button class="ctx-item" onclick={closeContextMenu}>
			<span class="ctx-icon">★</span> Mark for review
		</button>
		<button class="ctx-item" onclick={closeContextMenu}>
			<span class="ctx-icon">↓</span> Export track history
		</button>
		<div class="ctx-sep"></div>
		<button class="ctx-item" onclick={() => { selectionStore.select(contextSlot); closeContextMenu(); }}>
			<span class="ctx-icon">⊕</span> Focus on canvas
		</button>
	</div>
{/if}

<style>
	.hier-drawer {
		width: 240px;
		flex-shrink: 0;
		background: #141416;
		border-right: 1px solid #1d1d20;
		display: flex;
		flex-direction: column;
		overflow: hidden;
		font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
	}

	.hier-header {
		padding: 10px 12px;
		background: #18181b;
		border-bottom: 1px solid #1d1d20;
	}

	.hier-title {
		font-size: 9px;
		font-weight: 700;
		letter-spacing: 0.08em;
		color: #878787;
		text-transform: uppercase;
	}

	.hier-filter-row {
		display: flex;
		gap: 4px;
		padding: 6px 8px;
		border-bottom: 1px solid #1d1d20;
	}

	.filter-input {
		flex: 1;
		padding: 4px 6px;
		background: #111113;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		color: #d4d0c8;
		font-size: 10px;
		font-family: inherit;
		outline: none;
	}

	.filter-input:focus-visible {
		border-color: #f5a623;
	}

	.filter-select {
		padding: 4px 6px;
		background: #111113;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		color: #d4d0c8;
		font-size: 10px;
		font-family: inherit;
		cursor: pointer;
		outline: none;
	}

	.hier-tree {
		flex: 1;
		overflow-y: auto;
		padding: 4px 0;
	}

	.hier-empty {
		padding: 20px 12px;
		text-align: center;
		color: #555;
		font-size: 10px;
		font-style: italic;
	}

	.hier-row {
		display: flex;
		align-items: center;
		gap: 6px;
		width: 100%;
		padding: 4px 8px;
		background: transparent;
		border: none;
		border-left: 3px solid transparent;
		color: #d4d0c8;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 10px;
		cursor: pointer;
		text-align: left;
		transition: background 80ms ease;
	}

	.hier-row:hover {
		background: #1d1d20;
	}

	.hier-row.selected {
		background: rgba(245, 166, 35, 0.08);
		border-left-color: #f5a623;
	}

	.hier-row.violation {
		background: rgba(239, 68, 68, 0.04);
		border-left-color: #ef4444;
	}

	.tree-chevron {
		width: 10px;
		color: #555;
		font-size: 8px;
	}

	.tree-spacer {
		width: 10px;
	}

	.hier-dot {
		width: 7px;
		height: 7px;
		border-radius: 50%;
		flex-shrink: 0;
	}

	.hier-id {
		flex-shrink: 0;
		color: #d4d0c8;
	}

	.hier-class {
		flex: 1;
		color: #878787;
		min-width: 0;
		overflow: hidden;
		text-overflow: ellipsis;
		font-size: 10px;
	}

	.hier-conf {
		font-weight: 600;
		color: #f5a623;
		font-size: 9px;
	}

	.hier-icon {
		font-size: 11px;
		color: #ef4444;
	}

	.hier-icon.ok {
		color: #34d399;
	}

	.hier-footer {
		display: flex;
		align-items: center;
		gap: 4px;
		padding: 6px 12px;
		background: #18181b;
		border-top: 1px solid #1d1d20;
		font-size: 9px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #555;
	}

	.hier-footer .ok { color: #34d399; }
	.hier-footer .err { color: #ef4444; }
	.footer-sep { color: #2a2a2e; }

	/* Context menu */
	.context-menu {
		position: fixed;
		min-width: 220px;
		background: #1a1a1e;
		border: 1px solid #333;
		border-radius: 5px;
		box-shadow: 0 8px 24px rgba(0, 0, 0, 0.5);
		padding: 4px;
		z-index: 200;
		display: flex;
		flex-direction: column;
		gap: 1px;
		font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
	}

	.ctx-item {
		display: flex;
		align-items: center;
		gap: 8px;
		padding: 6px 10px;
		background: transparent;
		border: none;
		color: #d4d0c8;
		font-size: 11px;
		text-align: left;
		cursor: pointer;
		border-radius: 3px;
		font-family: inherit;
	}

	.ctx-item:hover {
		background: #27272a;
	}

	.ctx-icon {
		font-size: 12px;
		width: 14px;
		text-align: center;
		color: #f5a623;
	}

	.ctx-hint {
		margin-left: auto;
		font-size: 9px;
		color: #555;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
	}

	.ctx-sep {
		height: 1px;
		background: #27272a;
		margin: 2px 0;
	}
</style>
