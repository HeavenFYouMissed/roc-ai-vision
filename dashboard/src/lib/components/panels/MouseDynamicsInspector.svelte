<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { actuatorPool } from '$lib/stores/actuator_pool.svelte';
	import { trackPool } from '$lib/stores/track_pool.svelte';
	import PreRecordModal from '$lib/components/PreRecordModal.svelte';
	import { JsonlWriter } from '$lib/exporter/jsonl_writer.svelte';
	import type { SessionLabel } from '$lib/profiles/scenarios';
	import { scenarioStore } from '$lib/profiles/scenario_store.svelte';

	type RenderMode = 'c' | 'd' | 't' | 's';
	let renderMode = $state<RenderMode>('s');
	let showModal = $state(false);
	let recording = $state(false);
	let bytesWritten = $state(0);
	let writer: JsonlWriter | null = null;
	let sessionLabel = $state<SessionLabel>('clean');

	let cleanPathEl: SVGPathElement | undefined;
	let dirtyPathEl: SVGPathElement | undefined;
	let rafId = 0;
	const pathScratch: string[] = [];
	const W = 480;
	const H = 280;
	const SAMPLE_CAP = 512;

	function buildPath(useClean: boolean): string {
		pathScratch.length = 0;
		const n = Math.min(actuatorPool.fill, SAMPLE_CAP);
		if (n < 2) return '';
		const start = actuatorPool.indexFromNewest(n - 1);
		let minU = Infinity;
		let maxU = -Infinity;
		let minV = Infinity;
		let maxV = -Infinity;
		for (let k = 0; k < n; k++) {
			const i = (start + k) % actuatorPool.rawU.length;
			const u = useClean ? actuatorPool.cleanU[i] : actuatorPool.rawU[i];
			const v = useClean ? actuatorPool.cleanV[i] : actuatorPool.rawV[i];
			if (!Number.isFinite(u) || !Number.isFinite(v)) continue;
			minU = Math.min(minU, u);
			maxU = Math.max(maxU, u);
			minV = Math.min(minV, v);
			maxV = Math.max(maxV, v);
		}
		const spanU = maxU - minU || 1;
		const spanV = maxV - minV || 1;
		let first = true;
		for (let k = 0; k < n; k++) {
			const i = (start + k) % actuatorPool.rawU.length;
			const u = useClean ? actuatorPool.cleanU[i] : actuatorPool.rawU[i];
			const v = useClean ? actuatorPool.cleanV[i] : actuatorPool.rawV[i];
			if (!Number.isFinite(u) || !Number.isFinite(v)) continue;
			const x = ((u - minU) / spanU) * (W - 20) + 10;
			const y = ((v - minV) / spanV) * (H - 20) + 10;
			pathScratch.push(first ? `M ${x} ${y}` : `L ${x} ${y}`);
			first = false;
		}
		return pathScratch.join(' ');
	}

	function tick() {
		void actuatorPool.presenceVersion;
		const showClean = renderMode === 'c' || renderMode === 't' || renderMode === 's';
		const showDirty = renderMode === 'd' || renderMode === 't' || renderMode === 's';
		if (cleanPathEl && showClean) cleanPathEl.setAttribute('d', buildPath(true));
		if (dirtyPathEl && showDirty) dirtyPathEl.setAttribute('d', buildPath(false));
		rafId = requestAnimationFrame(tick);
	}

	function onKey(ev: KeyboardEvent) {
		const k = ev.key.toLowerCase();
		if (k === 'c' || k === 'd' || k === 't' || k === 's') renderMode = k;
		if (k === 'r' && !recording) showModal = true;
		if (k === 'x' && recording) stopRecording();
	}

	function startRecording(label: SessionLabel) {
		sessionLabel = label;
		showModal = false;
		writer = new JsonlWriter();
		writer.writeSessionHeader({
			type: 'session',
			session_id: crypto.randomUUID(),
			label,
			started_at_ns: Date.now() * 1_000_000,
			actuator_hz: 500,
			vision_fps: 30,
			vision_frames_total: trackPool.frameSeq,
			croissant_spec_version: '1.0',
			academic_citations_attestation: true,
			profile_id: scenarioStore.active.profile.id
		});
		recording = true;
		bytesWritten = 0;
	}

	async function stopRecording() {
		if (!writer) return;
		recording = false;
		const sessionId = crypto.randomUUID();
		const blob = await writer.finalizeSessionFolderZip(sessionId, sessionLabel, {
			userAgent: navigator.userAgent,
			screen: { width: screen.width, height: screen.height }
		});
		const url = URL.createObjectURL(blob);
		const a = document.createElement('a');
		a.href = url;
		a.download = `${sessionId}.zip`;
		a.click();
		URL.revokeObjectURL(url);
		writer = null;
	}

	onMount(() => {
		window.addEventListener('keydown', onKey);
		rafId = requestAnimationFrame(tick);
	});
	onDestroy(() => {
		window.removeEventListener('keydown', onKey);
		cancelAnimationFrame(rafId);
	});

	$effect(() => {
		if (!recording || !writer) return;
		void actuatorPool.presenceVersion;
		const i = actuatorPool.indexFromNewest(0);
		writer.recordVisionFrame(trackPool.frameSeq, 0, 0);
		writer.writeSampleEvent({
			ts_lo: actuatorPool.tsLo[i],
			ts_hi: actuatorPool.tsHi[i],
			frame_id: actuatorPool.frameId[i],
			raw_u: actuatorPool.rawU[i],
			raw_v: actuatorPool.rawV[i],
			clean_u: actuatorPool.cleanU[i],
			clean_v: actuatorPool.cleanV[i],
			applied_alpha: actuatorPool.appliedAlpha[i],
			flags: actuatorPool.flags[i]
		});
		bytesWritten += 1;
	});
</script>

<PreRecordModal open={showModal} onSelect={startRecording} />

<section class="inspector">
	<header class="strip">
		<span>Mode: {renderMode.toUpperCase()} (c/d/t/s)</span>
		<span>α live: {actuatorPool.lastAppliedAlpha.toFixed(3)}</span>
		<button type="button" disabled={recording} onclick={() => (showModal = true)}>Record (R)</button>
		<button type="button" disabled={!recording} onclick={stopRecording}>Stop (X)</button>
		{#if recording}<span class="rec">REC · {bytesWritten} events</span>{/if}
	</header>
	<div class="panes" class:split={renderMode === 's'}>
		{#if renderMode === 's'}
			<div class="pane">
				<h3>Clean trace</h3>
				<svg viewBox="0 0 {W} {H}" width="100%" height="240"><path bind:this={cleanPathEl} class="clean" /></svg>
			</div>
			<div class="pane">
				<h3>Dirty trace</h3>
				<svg viewBox="0 0 {W} {H}" width="100%" height="240"><path bind:this={dirtyPathEl} class="dirty" /></svg>
			</div>
		{:else}
			<svg viewBox="0 0 {W} {H}" width="100%" height="280">
				{#if renderMode === 'c' || renderMode === 't'}
					<path bind:this={cleanPathEl} class="clean" />
				{/if}
				{#if renderMode === 'd' || renderMode === 't'}
					<path bind:this={dirtyPathEl} class="dirty" />
				{/if}
			</svg>
		{/if}
	</div>
</section>

<style>
	.inspector {
		display: flex;
		flex-direction: column;
		gap: 12px;
		padding: 16px;
		color: #e2e8f0;
	}
	.strip {
		display: flex;
		flex-wrap: wrap;
		gap: 12px;
		align-items: center;
		font-size: 0.8rem;
	}
	.strip button {
		padding: 6px 12px;
		background: #2d3748;
		border: none;
		border-radius: 4px;
		color: #fff;
		cursor: pointer;
	}
	.rec {
		color: #fc8181;
		font-weight: 700;
	}
	.panes.split {
		display: grid;
		grid-template-columns: 1fr 1fr;
		gap: 12px;
	}
	.pane h3 {
		margin: 0 0 8px;
		font-size: 0.85rem;
		color: #90cdf4;
	}
	path.clean {
		fill: none;
		stroke: #4fd1c5;
		stroke-width: 2;
	}
	path.dirty {
		fill: none;
		stroke: #f6ad55;
		stroke-width: 1.5;
		stroke-dasharray: 4 2;
	}
</style>
