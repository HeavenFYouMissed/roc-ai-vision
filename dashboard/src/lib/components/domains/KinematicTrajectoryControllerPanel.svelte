<script lang="ts">
	import DomainPanelLayout from '$lib/components/domains/DomainPanelLayout.svelte';
	import { aimingConfig } from '$lib/stores/aiming_config';
	import { telemetrySocket } from '$lib/telemetry/telemetry_socket.svelte';

	function onSmoothingRelease() {
		telemetrySocket.setSmoothingAlpha(aimingConfig.smoothingCoefficient);
	}
</script>

<DomainPanelLayout
	title="Kinematic Trajectory Controller"
	subtitle="Direct adjustment of target smoothing and path tracking variables."
>
	{#snippet footer()}
		<button class="btn-secondary" type="button" onclick={() => aimingConfig.resetDefaults()}>
			Reset to Defaults
		</button>
	{/snippet}

	<div class="control-group">
		<label for="smoothing">
			<span>Smoothing Coefficient (α)</span>
			<span class="value-display">{aimingConfig.smoothingCoefficient.toFixed(2)}</span>
		</label>
		<input
			id="smoothing"
			type="range"
			min="0.05"
			max="1.00"
			step="0.05"
			bind:value={aimingConfig.smoothingCoefficient}
			onchange={onSmoothingRelease}
		/>
		<p class="help-text">
			Filter alpha / smoothing weight. Lower = more humanization; higher = lower latency.
		</p>
	</div>

	<div class="control-group">
		<label for="deadband">
			<span>NIS Gated Deadband Scaling</span>
			<span class="value-display">{aimingConfig.deadbandScaling.toFixed(1)}×</span>
		</label>
		<input
			id="deadband"
			type="range"
			min="0.5"
			max="5.0"
			step="0.1"
			bind:value={aimingConfig.deadbandScaling}
		/>
		<p class="help-text">Multiplicative NIS stabilization window threshold factor.</p>
	</div>

	<div class="control-group">
		<label for="profile">Target Profile Presets</label>
		<select id="profile" bind:value={aimingConfig.activeProfile}>
			<option value="default">Standard Kinematic Match</option>
			<option value="aggressive">High-Acceleration Tracking</option>
			<option value="humanized">Smooth Interpolation Blend</option>
		</select>
	</div>
</DomainPanelLayout>

<style>
	.control-group {
		display: flex;
		flex-direction: column;
		gap: 8px;
	}

	.control-group label {
		display: flex;
		justify-content: space-between;
		font-weight: 500;
	}

	.value-display {
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		color: #f5a623;
		font-size: 0.85rem;
	}

	.help-text {
		margin: 0;
		color: #555;
		font-size: 0.75rem;
	}

	input[type='range'] {
		width: 100%;
		height: 6px;
		appearance: none;
		background: #27272a;
		border-radius: 3px;
		outline: none;
	}

	input[type='range']::-webkit-slider-thumb {
		appearance: none;
		width: 14px;
		height: 14px;
		border-radius: 50%;
		background: #f5a623;
		cursor: pointer;
		border: 2px solid #0e0e10;
	}

	select {
		width: 100%;
		padding: 8px;
		background: #111113;
		border: 1px solid #333;
		border-radius: 4px;
		color: #d4d0c8;
		font-family: inherit;
		cursor: pointer;
	}

	select:focus-visible {
		border-color: #f5a623;
		outline: none;
	}

	.btn-secondary {
		padding: 8px 16px;
		border-radius: 4px;
		font-weight: 500;
		font-size: 12px;
		cursor: pointer;
		border: 1px solid #333;
		background: #1e1e22;
		color: #d4d0c8;
		transition: all 150ms ease;
	}

	.btn-secondary:hover {
		background: #27272a;
		border-color: #555;
	}
</style>
