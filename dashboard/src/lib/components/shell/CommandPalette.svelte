<script lang="ts">
	import { CONTROLLER_DOMAINS, type ControllerDomain } from '$lib/domains/controller_matrix';
	import { PANEL_REGISTRY } from '$lib/profiles/panel_registry';
	import { SCENARIOS } from '$lib/profiles/scenarios';
	import { DEFAULT_BINDINGS } from '$lib/keybindings/defaults';

	interface Props {
		open: boolean;
		onClose: () => void;
		onSelectDestination: (d: ControllerDomain) => void;
		onSelectScenario: (id: string) => void;
	}

	let { open, onClose, onSelectDestination, onSelectScenario }: Props = $props();

	let query = $state('');
	let selectedIndex = $state(0);
	let inputEl: HTMLInputElement | undefined = $state();

	interface PaletteItem {
		id: string;
		label: string;
		category: string;
		hint: string;
		action: () => void;
	}

	const allItems = $derived<PaletteItem[]>([
		...CONTROLLER_DOMAINS.map(d => ({
			id: `dest-${d.id}`,
			label: d.label,
			category: 'Destination',
			hint: d.hotkey,
			action: () => { onSelectDestination(d.id); onClose(); }
		})),
		...SCENARIOS.map(s => ({
			id: `scenario-${s.id}`,
			label: s.label,
			category: 'Scenario',
			hint: '',
			action: () => { onSelectScenario(s.id); onClose(); }
		})),
		...PANEL_REGISTRY.map(p => ({
			id: `panel-${p.id}`,
			label: p.label,
			category: 'Panel',
			hint: '',
			action: () => { onSelectDestination('inspect'); onClose(); }
		})),
		...DEFAULT_BINDINGS.filter(b => b.category === 'global').map(b => ({
			id: `action-${b.action}`,
			label: b.label,
			category: 'Action',
			hint: b.combo,
			action: () => { onClose(); }
		}))
	]);

	const filtered = $derived(
		query.length === 0
			? allItems
			: allItems.filter(item =>
				item.label.toLowerCase().includes(query.toLowerCase()) ||
				item.category.toLowerCase().includes(query.toLowerCase())
			)
	);

	$effect(() => {
		if (open && inputEl) {
			query = '';
			selectedIndex = 0;
			setTimeout(() => inputEl?.focus(), 50);
		}
	});

	$effect(() => {
		if (filtered.length > 0 && selectedIndex >= filtered.length) {
			selectedIndex = 0;
		}
	});

	function handleKeydown(e: KeyboardEvent) {
		if (e.key === 'ArrowDown') {
			e.preventDefault();
			selectedIndex = (selectedIndex + 1) % filtered.length;
		} else if (e.key === 'ArrowUp') {
			e.preventDefault();
			selectedIndex = (selectedIndex - 1 + filtered.length) % filtered.length;
		} else if (e.key === 'Enter' && filtered.length > 0) {
			e.preventDefault();
			filtered[selectedIndex].action();
		} else if (e.key === 'Escape') {
			e.preventDefault();
			onClose();
		}
	}

	const CATEGORY_COLORS: Record<string, string> = {
		Destination: '#f5a623',
		Scenario: '#34d399',
		Panel: '#60a5fa',
		Action: '#a78bfa'
	};
</script>

{#if open}
	<div class="palette-backdrop" onclick={onClose} role="presentation">
		<div class="palette" onclick={(e) => e.stopPropagation()} onkeydown={handleKeydown} role="dialog" aria-label="Command palette">
			<div class="palette-input-wrap">
				<svg class="search-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
					<circle cx="11" cy="11" r="8" /><line x1="21" y1="21" x2="16.65" y2="16.65" />
				</svg>
				<input
					bind:this={inputEl}
					bind:value={query}
					type="text"
					placeholder="Search destinations, scenarios, panels, actions..."
					class="palette-input"
					spellcheck="false"
				/>
				<kbd class="palette-esc">ESC</kbd>
			</div>

			<div class="palette-results">
				{#if filtered.length === 0}
					<div class="palette-empty">No results for "{query}"</div>
				{:else}
					{#each filtered as item, i (item.id)}
						<button
							class="palette-item"
							class:selected={i === selectedIndex}
							onclick={() => item.action()}
							onmouseenter={() => { selectedIndex = i; }}
						>
							<span class="item-category" style="color: {CATEGORY_COLORS[item.category] ?? '#666'}">
								{item.category}
							</span>
							<span class="item-label">{item.label}</span>
							{#if item.hint}
								<kbd class="item-hint">{item.hint}</kbd>
							{/if}
						</button>
					{/each}
				{/if}
			</div>
		</div>
	</div>
{/if}

<style>
	.palette-backdrop {
		position: fixed;
		inset: 0;
		background: rgba(0, 0, 0, 0.6);
		display: flex;
		justify-content: center;
		padding-top: 15vh;
		z-index: 100;
		backdrop-filter: blur(2px);
	}

	.palette {
		width: 560px;
		max-height: 420px;
		background: #1a1a1e;
		border: 1px solid #333;
		border-radius: 10px;
		box-shadow: 0 16px 48px rgba(0, 0, 0, 0.5);
		display: flex;
		flex-direction: column;
		overflow: hidden;
	}

	.palette-input-wrap {
		display: flex;
		align-items: center;
		gap: 10px;
		padding: 12px 16px;
		border-bottom: 1px solid #27272a;
	}

	.search-icon {
		width: 18px;
		height: 18px;
		color: #555;
		flex-shrink: 0;
	}

	.palette-input {
		flex: 1;
		background: transparent;
		border: none;
		color: #d4d0c8;
		font-size: 14px;
		font-family: inherit;
		outline: none;
	}

	.palette-input::placeholder {
		color: #555;
	}

	.palette-esc {
		font-size: 9px;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		padding: 2px 6px;
		background: #27272a;
		color: #666;
		border-radius: 3px;
		border: 1px solid #333;
	}

	.palette-results {
		overflow-y: auto;
		padding: 6px;
	}

	.palette-empty {
		padding: 24px;
		text-align: center;
		color: #555;
		font-size: 13px;
	}

	.palette-item {
		display: flex;
		align-items: center;
		gap: 10px;
		width: 100%;
		padding: 8px 12px;
		background: transparent;
		border: none;
		border-radius: 6px;
		color: #d4d0c8;
		cursor: pointer;
		text-align: left;
		font-family: inherit;
		font-size: 13px;
		transition: background 80ms ease;
	}

	.palette-item.selected {
		background: #27272a;
	}

	.item-category {
		font-size: 10px;
		font-weight: 600;
		text-transform: uppercase;
		letter-spacing: 0.04em;
		width: 80px;
		flex-shrink: 0;
	}

	.item-label {
		flex: 1;
		min-width: 0;
		overflow: hidden;
		text-overflow: ellipsis;
		white-space: nowrap;
	}

	.item-hint {
		font-size: 10px;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		padding: 1px 5px;
		background: #222;
		color: #666;
		border-radius: 3px;
		border: 1px solid #333;
		flex-shrink: 0;
	}
</style>
