<script lang="ts">
	import DomainPanelLayout from '$lib/components/domains/DomainPanelLayout.svelte';
	import { hardwareConfig } from '$lib/stores/hardware_config';
</script>

<DomainPanelLayout
	title="Hardware Actuation Interface"
	subtitle="Serial link and capture-card bounds for the physical tracking loop."
>
	{#snippet footer()}
		<button class="btn-secondary" type="button" onclick={() => hardwareConfig.resetDefaults()}>
			Reset Hardware Defaults
		</button>
		<button class="btn-primary" type="button" onclick={() => hardwareConfig.runDiagnosticPing()}>
			Test Hardware Diagnostic Ping
		</button>
	{/snippet}

	<div class="control-group">
		<label for="serial-port">Serial Port Selection</label>
		<select id="serial-port" bind:value={hardwareConfig.serialPort}>
			{#each hardwareConfig.serialPortOptions as port}
				<option value={port}>{port}</option>
			{/each}
		</select>
	</div>

	<div class="control-group">
		<label for="baud-rate">Baud Rate</label>
		<select id="baud-rate" bind:value={hardwareConfig.baudRate}>
			{#each hardwareConfig.baudRateOptions as rate}
				<option value={rate}>{rate}</option>
			{/each}
		</select>
	</div>

	<section class="crop-section">
		<h3>Capture Card Video Crop Matrix</h3>
		<div class="crop-grid">
			<div class="control-group">
				<label for="crop-x">Offset X</label>
				<input id="crop-x" type="number" min="0" step="1" bind:value={hardwareConfig.cropX} />
			</div>
			<div class="control-group">
				<label for="crop-y">Offset Y</label>
				<input id="crop-y" type="number" min="0" step="1" bind:value={hardwareConfig.cropY} />
			</div>
			<div class="control-group">
				<label for="crop-w">Width</label>
				<input id="crop-w" type="number" min="1" step="1" bind:value={hardwareConfig.cropWidth} />
			</div>
			<div class="control-group">
				<label for="crop-h">Height</label>
				<input id="crop-h" type="number" min="1" step="1" bind:value={hardwareConfig.cropHeight} />
			</div>
		</div>
		<p class="help-text">Integer bounding region for capture ingress — placeholder until V4L2/DirectShow bridge.</p>
	</section>

	{#if hardwareConfig.lastDiagnosticPing}
		<p class="ping-result" role="status">{hardwareConfig.lastDiagnosticPing}</p>
	{/if}
</DomainPanelLayout>

<style>
	.control-group {
		display: flex;
		flex-direction: column;
		gap: 8px;
	}

	.control-group label,
	.crop-section h3 {
		font-weight: 500;
		color: #d4d0c8;
	}

	.crop-section h3 {
		margin: 0 0 12px;
		font-size: 0.85rem;
	}

	.crop-grid {
		display: grid;
		grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
		gap: 12px;
	}

	select,
	input[type='number'] {
		width: 100%;
		padding: 8px;
		background: #111113;
		border: 1px solid #333;
		border-radius: 4px;
		color: #d4d0c8;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		font-size: 0.8rem;
	}

	select:focus-visible,
	input[type='number']:focus-visible {
		border-color: #f5a623;
		outline: none;
	}

	.help-text {
		margin: 12px 0 0;
		color: #555;
		font-size: 0.75rem;
	}

	.ping-result {
		margin: 0;
		padding: 12px;
		background: #1a1a1e;
		border: 1px solid #333;
		border-radius: 4px;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		font-size: 0.8rem;
		color: #f5a623;
	}

	.btn-primary,
	.btn-secondary {
		padding: 8px 16px;
		border-radius: 4px;
		font-weight: 500;
		font-size: 12px;
		cursor: pointer;
		border: 1px solid transparent;
		transition: all 150ms ease;
	}

	.btn-primary {
		background: #f5a623;
		color: #111;
		border-color: #f5a623;
	}

	.btn-primary:hover {
		background: #e69a1e;
	}

	.btn-secondary {
		background: #1e1e22;
		color: #d4d0c8;
		border-color: #333;
	}

	.btn-secondary:hover {
		background: #27272a;
		border-color: #555;
	}
</style>
