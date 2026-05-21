<script lang="ts">
	import type { SessionLabel } from '$lib/profiles/scenarios';

	interface Props {
		open: boolean;
		onSelect: (label: SessionLabel) => void;
	}

	let { open, onSelect }: Props = $props();

	const labels: { id: SessionLabel; title: string }[] = [
		{ id: 'clean', title: 'Clean' },
		{ id: 'dirty', title: 'Dirty' },
		{ id: 'ambiguous', title: 'Ambiguous' },
		{ id: 'unsupervised', title: 'Unsupervised' }
	];
</script>

{#if open}
	<div class="backdrop" role="dialog" aria-modal="true" aria-labelledby="pre-record-title">
		<div class="modal">
			<h2 id="pre-record-title">Label this session</h2>
			<p class="hint">Choose a supervision label before recording starts (required).</p>
			<div class="choices">
				{#each labels as l}
					<button type="button" class="choice" onclick={() => onSelect(l.id)}>{l.title}</button>
				{/each}
			</div>
		</div>
	</div>
{/if}

<style>
	.backdrop {
		position: fixed;
		inset: 0;
		background: rgba(0, 0, 0, 0.75);
		display: flex;
		align-items: center;
		justify-content: center;
		z-index: 1000;
	}
	.modal {
		background: #1a202c;
		border: 1px solid #2d3748;
		border-radius: 8px;
		padding: 24px;
		min-width: 320px;
	}
	.hint {
		color: #a0aec0;
		font-size: 0.875rem;
	}
	.choices {
		display: grid;
		grid-template-columns: 1fr 1fr;
		gap: 8px;
		margin-top: 16px;
	}
	.choice {
		padding: 12px;
		background: #2d3748;
		border: none;
		border-radius: 4px;
		color: #fff;
		cursor: pointer;
		font-weight: 600;
	}
	.choice:hover {
		background: #3182ce;
	}
</style>
