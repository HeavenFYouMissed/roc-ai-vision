<script lang="ts">
	import DomainPanelLayout from '$lib/components/domains/DomainPanelLayout.svelte';

	const ekfPlaceholder = {
		position: { x: '—', y: '—', z: '—' },
		velocity: { x: '—', y: '—', z: '—' },
		acceleration: { x: '—', y: '—', z: '—' }
	};
</script>

<DomainPanelLayout
	title="Observer Matrix"
	subtitle="Text-based readouts of live data stream statistics (wire ingress not connected)."
>
	<div class="metrics-row">
		<div class="metric-card">
			<span class="metric-label">Ingress Data Rate</span>
			<span class="metric-value">0.00 <small>MB/s</small></span>
			<p class="metric-hint">Placeholder — binds to stream throughput once WS bridge is live.</p>
		</div>
		<div class="metric-card">
			<span class="metric-label">Frame Timing</span>
			<span class="metric-value">— <small>ms / frame</small></span>
			<div class="timing-graph" role="img" aria-label="Frame timing graph placeholder">
				{#each Array(24) as _, i}
					<div class="timing-bar" style:height={`${12 + (i % 5) * 6}px`}></div>
				{/each}
			</div>
			<p class="metric-hint">Rolling frame-interval visualization — reserved.</p>
		</div>
	</div>

	<section class="readout-section">
		<h3>EKF State Vector (Live)</h3>
		<div class="vector-grid">
			<div class="vector-block">
				<h4>Position (m)</h4>
				<dl>
					<div><dt>X</dt><dd>{ekfPlaceholder.position.x}</dd></div>
					<div><dt>Y</dt><dd>{ekfPlaceholder.position.y}</dd></div>
					<div><dt>Z</dt><dd>{ekfPlaceholder.position.z}</dd></div>
				</dl>
			</div>
			<div class="vector-block">
				<h4>Velocity (m/s)</h4>
				<dl>
					<div><dt>X</dt><dd>{ekfPlaceholder.velocity.x}</dd></div>
					<div><dt>Y</dt><dd>{ekfPlaceholder.velocity.y}</dd></div>
					<div><dt>Z</dt><dd>{ekfPlaceholder.velocity.z}</dd></div>
				</dl>
			</div>
			<div class="vector-block">
				<h4>Acceleration (m/s²)</h4>
				<dl>
					<div><dt>X</dt><dd>{ekfPlaceholder.acceleration.x}</dd></div>
					<div><dt>Y</dt><dd>{ekfPlaceholder.acceleration.y}</dd></div>
					<div><dt>Z</dt><dd>{ekfPlaceholder.acceleration.z}</dd></div>
				</dl>
			</div>
		</div>
	</section>
</DomainPanelLayout>

<style>
	.metrics-row {
		display: grid;
		grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
		gap: 16px;
	}

	.metric-card {
		padding: 16px;
		background: #1a202c;
		border-radius: 6px;
		border: 1px solid #2d3748;
	}

	.metric-label {
		display: block;
		font-size: 0.75rem;
		color: #a0aec0;
		text-transform: uppercase;
		letter-spacing: 0.04em;
		margin-bottom: 8px;
	}

	.metric-value {
		font-family: ui-monospace, monospace;
		font-size: 1.5rem;
		color: #63b3ed;
	}

	.metric-value small {
		font-size: 0.875rem;
		color: #718096;
	}

	.metric-hint {
		margin: 10px 0 0;
		font-size: 0.75rem;
		color: #718096;
	}

	.timing-graph {
		display: flex;
		align-items: flex-end;
		gap: 3px;
		height: 48px;
		margin-top: 12px;
		padding: 4px 0;
	}

	.timing-bar {
		flex: 1;
		min-width: 4px;
		background: linear-gradient(180deg, #3182ce 0%, #2c5282 100%);
		border-radius: 2px 2px 0 0;
		opacity: 0.7;
	}

	.readout-section h3 {
		margin: 0 0 12px;
		font-size: 0.9rem;
		color: #e2e8f0;
	}

	.vector-grid {
		display: grid;
		grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
		gap: 12px;
	}

	.vector-block {
		padding: 12px;
		background: #1a202c;
		border-radius: 6px;
		font-family: ui-monospace, monospace;
		font-size: 0.8125rem;
	}

	.vector-block h4 {
		margin: 0 0 8px;
		font-size: 0.75rem;
		color: #a0aec0;
		font-weight: 600;
	}

	.vector-block dl {
		margin: 0;
	}

	.vector-block div {
		display: flex;
		justify-content: space-between;
		padding: 4px 0;
		border-bottom: 1px solid #2d3748;
	}

	.vector-block dt {
		color: #718096;
	}

	.vector-block dd {
		margin: 0;
		color: #bee3f8;
	}
</style>
