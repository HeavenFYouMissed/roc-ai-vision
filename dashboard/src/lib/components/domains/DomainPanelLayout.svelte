<script lang="ts">
	import type { Snippet } from 'svelte';

	let {
		title,
		subtitle,
		children,
		footer,
		reserved
	}: {
		title: string;
		subtitle: string;
		children: Snippet;
		footer?: Snippet;
		reserved?: Snippet;
	} = $props();
</script>

<article class="domain-panel">
	<header class="panel-header">
		<h2>{title}</h2>
		<p class="subtitle">{subtitle}</p>
	</header>

	<div class="panel-body">
		{@render children()}
	</div>

	{#if footer}
		<footer class="panel-footer">
			{@render footer()}
		</footer>
	{/if}

	{#if reserved}
		<section class="reserved-expansion" aria-label="Reserved for future settings">
			{@render reserved()}
		</section>
	{/if}
</article>

<style>
	.domain-panel {
		padding: 24px;
		background: #161618;
		border: 1px solid #27272a;
		border-radius: 8px;
		color: #d4d0c8;
	}

	.panel-header h2 {
		margin: 0 0 4px;
		color: #d4d0c8;
		font-size: 1rem;
		font-weight: 600;
	}

	.subtitle {
		margin: 0 0 20px;
		color: #666;
		font-size: 0.8rem;
	}

	.panel-body {
		display: flex;
		flex-direction: column;
		gap: 20px;
	}

	.panel-footer {
		margin-top: 24px;
		display: flex;
		flex-wrap: wrap;
		gap: 12px;
		justify-content: flex-end;
	}

	.reserved-expansion {
		margin-top: 28px;
		padding-top: 20px;
		border-top: 1px solid #222;
		min-height: 48px;
	}
</style>
