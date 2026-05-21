<!--
	roc-ai-vision · dashboard · SourcesDrawer.svelte

	Camera / capture card / file / stream source selector. Lives below
	Settings on the left rail per UI_SPEC_SHEET.md ss8.

	Sources are cross-cutting — used by both Operate AND Engage destinations
	(both render the OperateCanvas Layer 0 <video>). Hardware actuator config
	stays in Engage because it's gated by the safety interlock.

	The /sources HTTP endpoint enumerates real devices (Phase 6+ C++ work);
	until then the built-in starter list covers the common cases.
-->
<script lang="ts">
	import { sourceStore, type SourceDevice } from '$lib/stores/source_store.svelte';
	import {
		captureSourcesStore,
		synthesizeCliFlags,
		type CaptureMonitorInfo,
		type CaptureWindowInfo
	} from '$lib/stores/capture_sources_store.svelte';
	import { eventLog } from '$lib/stores/event_log.svelte';

	interface Props {
		open: boolean;
		onClose: () => void;
	}

	let { open, onClose }: Props = $props();

	let pendingSourceId = $state(sourceStore.activeSourceId);
	let resW = $state<number>(sourceStore.active.width ?? 1280);
	let resH = $state<number>(sourceStore.active.height ?? 720);
	let resFps = $state<number>(sourceStore.active.fps ?? 30);
	let mjpegUrlOverride = $state(sourceStore.active.mjpegUrl ?? '');

	// Phase 8 desktop capture form state
	let dcKind = $state<'primary' | 'monitor' | 'window' | 'region'>('primary');
	let dcMonitorIndex = $state(0);
	let dcWindowSelection = $state<string>(''); // exact title of selected window
	let dcRegionX = $state(0);
	let dcRegionY = $state(0);
	let dcRegionW = $state(1280);
	let dcRegionH = $state(720);
	let dcFps = $state(60);
	let dcDownscale = $state(0);
	let dcCursor = $state(false);
	let dcCliCopied = $state(false);

	const dcSynthesizedCli = $derived.by(() => {
		const flags = synthesizeCliFlags({
			desktopKind: dcKind,
			monitorIndex: dcMonitorIndex,
			windowTitle: dcWindowSelection || undefined,
			regionX: dcRegionX, regionY: dcRegionY,
			regionW: dcRegionW, regionH: dcRegionH,
			fps: dcFps,
			downscale: dcDownscale,
			cursor: dcCursor
		});
		return flags.join(' ');
	});

	async function copyDcCli(): Promise<void> {
		try {
			await navigator.clipboard.writeText(dcSynthesizedCli);
			dcCliCopied = true;
			setTimeout(() => { dcCliCopied = false; }, 2000);
		} catch {
			/* clipboard API blocked — fall back silently */
		}
	}

	let hasAutoRefreshed = false;

	$effect(() => {
		// Sync local form state when the drawer opens or active source changes
		if (open) {
			pendingSourceId = sourceStore.activeSourceId;
			const a = sourceStore.active;
			resW = a.width ?? 1280;
			resH = a.height ?? 720;
			resFps = a.fps ?? 30;
			mjpegUrlOverride = a.mjpegUrl ?? '';

			// Auto-poll /sources HTTP endpoint on first open per session — gracefully
			// fails to built-in device list when C++ orchestrator offline.
			if (!hasAutoRefreshed) {
				hasAutoRefreshed = true;
				sourceStore.enumerate();
				captureSourcesStore.refresh();
			}
		}
	});

	const KIND_ICONS: Record<string, string> = {
		mock: 'M9 2v6m6-6v6M3 8h18M5 8v12a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2V8',
		webcam: 'M23 7l-7 5 7 5V7zM14 5H3a2 2 0 0 0-2 2v10a2 2 0 0 0 2 2h11a2 2 0 0 0 2-2V7a2 2 0 0 0-2-2z',
		capture_card: 'M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16Z',
		file: 'M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z M14 2v6h6 M16 13H8 M16 17H8 M10 9H8',
		rtsp: 'M21 12c0 4.97-4.03 9-9 9s-9-4.03-9-9 4.03-9 9-9 9 4.03 9 9zM3 12h18 M12 3a14 14 0 0 1 0 18 M12 3a14 14 0 0 0 0 18',
		webrtc: 'M21 12c0 4.97-4.03 9-9 9s-9-4.03-9-9 4.03-9 9-9 9 4.03 9 9zM3 12h18 M12 3a14 14 0 0 1 0 18 M12 3a14 14 0 0 0 0 18'
	};

	function pickDevice(id: string) {
		pendingSourceId = id;
		const dev = sourceStore.devices.find((d) => d.id === id);
		if (dev) {
			resW = dev.width ?? resW;
			resH = dev.height ?? resH;
			resFps = dev.fps ?? resFps;
			mjpegUrlOverride = dev.mjpegUrl ?? '';
		}
	}

	function applySource() {
		const before = sourceStore.activeSourceId;
		sourceStore.setSource(pendingSourceId);
		const dev = sourceStore.active;
		eventLog.info('user', `Source switched: ${before} → ${dev.label}`);
		if (resW && resH && resFps) {
			sourceStore.setResolutionOverride(resW, resH, resFps);
			eventLog.info('user', `Resolution override: ${resW}x${resH} @ ${resFps}fps`);
		}
		// Phase 6+: also send OperatorCommand{kSetCameraSource=8, deviceIndex, width, height}
		// to the C++ orchestrator over the WebSocket reverse channel.
		onClose();
	}

	async function refreshDevices() {
		await sourceStore.enumerate();
		eventLog.info(
			'shell',
			sourceStore.lastEnumerationError
				? `Source enumeration failed: ${sourceStore.lastEnumerationError}`
				: `Source enumeration complete: ${sourceStore.devices.length} devices`
		);
	}

	function kindLabel(kind: string): string {
		switch (kind) {
			case 'mock': return 'MOCK';
			case 'webcam': return 'WEBCAM';
			case 'capture_card': return 'CAPTURE';
			case 'file': return 'FILE';
			case 'rtsp': return 'RTSP';
			case 'webrtc': return 'WEBRTC';
			default: return kind.toUpperCase();
		}
	}

	const COMMON_RESOLUTIONS: Array<{ w: number; h: number; fps: number; label: string }> = [
		{ w: 640, h: 480, fps: 30, label: '640x480 @ 30' },
		{ w: 1280, h: 720, fps: 30, label: '1280x720 @ 30' },
		{ w: 1280, h: 720, fps: 60, label: '1280x720 @ 60' },
		{ w: 1920, h: 1080, fps: 30, label: '1920x1080 @ 30' },
		{ w: 1920, h: 1080, fps: 60, label: '1920x1080 @ 60' },
		{ w: 3840, h: 2160, fps: 30, label: '3840x2160 @ 30' }
	];

	function setResPreset(label: string) {
		const r = COMMON_RESOLUTIONS.find((p) => p.label === label);
		if (r) {
			resW = r.w;
			resH = r.h;
			resFps = r.fps;
		}
	}
</script>

{#if open}
	<div class="drawer-backdrop" onclick={onClose} role="presentation">
		<aside class="drawer" onclick={(e) => e.stopPropagation()} role="dialog" aria-label="Sources">
			<div class="drawer-header">
				<h3 class="drawer-title">Sources</h3>
				<button class="drawer-icon-btn" onclick={refreshDevices} title="Refresh device list (HTTP /sources)" disabled={sourceStore.enumerating}>
					<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
						<polyline points="23 4 23 10 17 10"></polyline>
						<polyline points="1 20 1 14 7 14"></polyline>
						<path d="M3.51 9a9 9 0 0 1 14.85-3.36L23 10M1 14l4.64 4.36A9 9 0 0 0 20.49 15"></path>
					</svg>
				</button>
				<button class="drawer-close" onclick={onClose} aria-label="Close">
					<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M18 6L6 18M6 6l12 12"></path></svg>
				</button>
			</div>

			<div class="drawer-section">
				<div class="section-label">AVAILABLE DEVICES</div>
				<div class="device-list">
					{#each sourceStore.devices as device (device.id)}
						<button
							class="device-card"
							class:selected={pendingSourceId === device.id}
							class:offline={!device.online}
							onclick={() => pickDevice(device.id)}
						>
							<svg class="device-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8">
								<path d={KIND_ICONS[device.kind] ?? KIND_ICONS.webcam}></path>
							</svg>
							<div class="device-info">
								<div class="device-top">
									<span class="device-label">{device.label}</span>
									<span class="device-kind">{kindLabel(device.kind)}</span>
								</div>
								<div class="device-meta">
									{#if device.deviceIndex !== undefined}
										<span class="meta-pair"><span class="meta-key">idx</span>{device.deviceIndex}</span>
									{/if}
									{#if device.backend}
										<span class="meta-pair"><span class="meta-key">drv</span>{device.backend}</span>
									{/if}
									{#if device.width}
										<span class="meta-pair"><span class="meta-key">res</span>{device.width}x{device.height}@{device.fps}</span>
									{/if}
									<span class="meta-pair status-pair" class:online={device.online}>
										<span class="status-dot"></span>
										{device.online ? 'online' : 'offline'}
									</span>
								</div>
							</div>
						</button>
					{/each}
				</div>
				{#if sourceStore.lastEnumerationError}
					<div class="enum-error">
						<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"></circle><line x1="12" y1="8" x2="12" y2="12"></line><line x1="12" y1="16" x2="12.01" y2="16"></line></svg>
						/sources endpoint unreachable: {sourceStore.lastEnumerationError}
						<br/>
						<span class="enum-hint">Built-in device list shown until the C++ orchestrator's HTTP server is up.</span>
					</div>
				{/if}
			</div>

			<div class="drawer-section">
				<div class="section-label">RESOLUTION + FPS</div>
				<div class="res-presets">
					{#each COMMON_RESOLUTIONS as r}
						<button
							class="res-preset"
							class:active={resW === r.w && resH === r.h && resFps === r.fps}
							onclick={() => setResPreset(r.label)}
						>{r.label}</button>
					{/each}
				</div>
				<div class="res-custom">
					<div class="res-field">
						<label>W</label>
						<input type="number" bind:value={resW} min="1" max="7680" />
					</div>
					<div class="res-field">
						<label>H</label>
						<input type="number" bind:value={resH} min="1" max="4320" />
					</div>
					<div class="res-field">
						<label>FPS</label>
						<input type="number" bind:value={resFps} min="1" max="240" />
					</div>
				</div>
			</div>

			<div class="drawer-section">
				<div class="section-label-row">
					<span class="section-label">DESKTOP CAPTURE (PHASE 8)</span>
					<button
						type="button"
						class="rescan-btn"
						disabled={captureSourcesStore.loading}
						onclick={() => captureSourcesStore.refresh()}
						title="Re-scan monitors and visible windows"
					>
						{captureSourcesStore.loading ? '⟳ scanning…' : '⟳ rescan'}
					</button>
				</div>

				{#if captureSourcesStore.lastError}
					<div class="dc-banner-error">
						Orchestrator offline: {captureSourcesStore.lastError}
					</div>
				{:else}
					<div class="dc-summary">
						{captureSourcesStore.monitorCount} monitor{captureSourcesStore.monitorCount === 1 ? '' : 's'}
						· {captureSourcesStore.windowCount} visible window{captureSourcesStore.windowCount === 1 ? '' : 's'}
					</div>
				{/if}

				<div class="dc-kind-row">
					{#each ['primary', 'monitor', 'window', 'region'] as k}
						<button
							type="button"
							class="dc-kind-pill"
							class:dc-kind-active={dcKind === k}
							onclick={() => (dcKind = k as typeof dcKind)}
						>{k}</button>
					{/each}
				</div>

				{#if dcKind === 'monitor' && captureSourcesStore.monitors.length > 0}
					<div class="dc-pickbox">
						<label class="dc-row">
							<span class="dc-row-label">Monitor</span>
							<select bind:value={dcMonitorIndex}>
								{#each captureSourcesStore.monitors as m (m.index)}
									<option value={m.index}>
										#{m.index} {m.isPrimary ? '(primary)' : ''} — {m.w}×{m.h} @ ({m.x}, {m.y})
									</option>
								{/each}
							</select>
						</label>
					</div>
				{/if}

				{#if dcKind === 'window'}
					<div class="dc-pickbox">
						<label class="dc-row">
							<span class="dc-row-label">Window</span>
							<select bind:value={dcWindowSelection}>
								<option value="">(foreground window)</option>
								{#each captureSourcesStore.windows as w (w.hwnd)}
									<option value={w.title}>{w.title}  —  {w.w}×{w.h}</option>
								{/each}
							</select>
						</label>
						<div class="dc-hint">
							Match is case-insensitive substring. Pick from above OR type a fragment in the
							command-line directly (e.g. <code>--capture-window "YouTube"</code>).
						</div>
					</div>
				{/if}

				{#if dcKind === 'region'}
					<div class="dc-pickbox dc-region-grid">
						<label><span>X</span><input type="number" bind:value={dcRegionX} /></label>
						<label><span>Y</span><input type="number" bind:value={dcRegionY} /></label>
						<label><span>W</span><input type="number" bind:value={dcRegionW} min="32" /></label>
						<label><span>H</span><input type="number" bind:value={dcRegionH} min="32" /></label>
					</div>
				{/if}

				<div class="dc-pickbox dc-opt-grid">
					<label>
						<span>FPS</span>
						<input type="number" bind:value={dcFps} min="1" max="240" />
					</label>
					<label>
						<span>Downscale max-dim</span>
						<input type="number" bind:value={dcDownscale} min="0" max="4096" placeholder="0 = native" />
					</label>
					<label class="dc-checkbox">
						<input type="checkbox" bind:checked={dcCursor} />
						<span>Include mouse cursor</span>
					</label>
				</div>

				<div class="dc-cli-block">
					<div class="dc-cli-label">Restart command (copy + paste into your orchestrator boot)</div>
					<code class="dc-cli">{dcSynthesizedCli}</code>
					<button type="button" class="dc-copy-btn" onclick={copyDcCli}>
						{dcCliCopied ? '✓ Copied' : 'Copy CLI'}
					</button>
				</div>

				<div class="dc-hint">
					Live source-swap (no restart) is on the Phase-8.5 wishlist; for now,
					stop the orchestrator and re-launch with the flags above.
				</div>
			</div>

			<div class="drawer-section">
				<div class="section-label">MJPEG URL OVERRIDE</div>
				<input
					type="text"
					class="url-input"
					placeholder="http://127.0.0.1:8766/mjpeg/0"
					bind:value={mjpegUrlOverride}
				/>
				<div class="url-hint">
					Layer 0 of OperateCanvas binds &lt;video&gt; src to this URL when set.
					Leave blank for SVG-only overlay (MOCK source default).
				</div>
			</div>

			<div class="drawer-footer">
				<button class="apply-btn" onclick={applySource}>
					Apply Source
					<span class="apply-hint">→ updates Layer 0</span>
				</button>
			</div>
		</aside>
	</div>
{/if}

<style>
	.drawer-backdrop {
		position: fixed;
		inset: 0;
		background: rgba(0, 0, 0, 0.4);
		z-index: 50;
	}

	.drawer {
		position: fixed;
		top: 0;
		left: 56px;
		width: 360px;
		height: 100vh;
		background: #161618;
		border-right: 1px solid #27272a;
		display: flex;
		flex-direction: column;
		box-shadow: 8px 0 24px rgba(0, 0, 0, 0.3);
		animation: slideIn 150ms ease;
		overflow: hidden;
	}

	@keyframes slideIn {
		from { transform: translateX(-20px); opacity: 0.8; }
		to { transform: translateX(0); opacity: 1; }
	}

	.drawer-header {
		display: flex;
		align-items: center;
		gap: 8px;
		padding: 14px 16px;
		border-bottom: 1px solid #222;
	}

	.drawer-title {
		flex: 1;
		margin: 0;
		font-size: 14px;
		font-weight: 600;
		color: #d4d0c8;
	}

	.drawer-icon-btn,
	.drawer-close {
		width: 28px;
		height: 28px;
		display: flex;
		align-items: center;
		justify-content: center;
		background: transparent;
		border: none;
		border-radius: 4px;
		color: #555;
		cursor: pointer;
		transition: all 100ms ease;
	}

	.drawer-icon-btn:hover:not(:disabled),
	.drawer-close:hover {
		background: #27272a;
		color: #f5a623;
	}

	.drawer-icon-btn:disabled {
		opacity: 0.4;
		cursor: not-allowed;
	}

	.drawer-icon-btn svg,
	.drawer-close svg {
		width: 14px;
		height: 14px;
	}

	.drawer-section {
		padding: 12px 16px;
		border-bottom: 1px solid #1d1d20;
	}

	.section-label {
		font-size: 9px;
		font-weight: 700;
		color: #878787;
		letter-spacing: 0.08em;
		text-transform: uppercase;
		margin-bottom: 8px;
	}

	.device-list {
		display: flex;
		flex-direction: column;
		gap: 4px;
		max-height: 320px;
		overflow-y: auto;
	}

	.device-card {
		display: flex;
		align-items: flex-start;
		gap: 10px;
		padding: 9px 11px;
		background: #1a1a1e;
		border: 1px solid #222;
		border-left: 3px solid transparent;
		border-radius: 4px;
		cursor: pointer;
		text-align: left;
		font-family: inherit;
		color: #d4d0c8;
		transition: all 100ms ease;
	}

	.device-card:hover {
		background: #1d1d22;
		border-color: #2a2a2e;
	}

	.device-card.selected {
		background: rgba(245, 166, 35, 0.06);
		border-color: rgba(245, 166, 35, 0.4);
		border-left-color: #f5a623;
		border-left-width: 4px;
	}

	.device-card.offline {
		opacity: 0.65;
	}

	.device-icon {
		width: 18px;
		height: 18px;
		color: #878787;
		flex-shrink: 0;
		margin-top: 1px;
	}

	.device-card.selected .device-icon {
		color: #f5a623;
	}

	.device-info {
		flex: 1;
		min-width: 0;
		display: flex;
		flex-direction: column;
		gap: 4px;
	}

	.device-top {
		display: flex;
		align-items: baseline;
		gap: 6px;
	}

	.device-label {
		flex: 1;
		font-size: 11px;
		font-weight: 500;
		color: #d4d0c8;
		overflow: hidden;
		text-overflow: ellipsis;
		white-space: nowrap;
	}

	.device-kind {
		font-size: 9px;
		font-weight: 700;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #555;
		letter-spacing: 0.06em;
	}

	.device-card.selected .device-kind {
		color: #f5a623;
	}

	.device-meta {
		display: flex;
		flex-wrap: wrap;
		gap: 4px 10px;
		font-size: 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #878787;
	}

	.meta-pair {
		display: inline-flex;
		align-items: center;
		gap: 3px;
	}

	.meta-key {
		color: #555;
	}

	.status-pair {
		margin-left: auto;
		color: #555;
	}

	.status-pair .status-dot {
		width: 6px;
		height: 6px;
		border-radius: 50%;
		background: #444;
	}

	.status-pair.online {
		color: #34d399;
	}

	.status-pair.online .status-dot {
		background: #34d399;
		box-shadow: 0 0 4px rgba(52, 211, 153, 0.4);
	}

	.enum-error {
		margin-top: 8px;
		padding: 8px 10px;
		background: rgba(245, 166, 35, 0.06);
		border: 1px solid rgba(245, 166, 35, 0.2);
		border-radius: 4px;
		font-size: 10px;
		color: #f5a623;
		display: flex;
		align-items: flex-start;
		gap: 6px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
	}

	.enum-error svg {
		width: 12px;
		height: 12px;
		flex-shrink: 0;
		margin-top: 2px;
	}

	.enum-hint {
		color: #878787;
		font-style: italic;
	}

	.res-presets {
		display: grid;
		grid-template-columns: 1fr 1fr;
		gap: 3px;
		margin-bottom: 8px;
	}

	.res-preset {
		padding: 5px 6px;
		background: #1a1a1e;
		color: #878787;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		font-size: 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		cursor: pointer;
		transition: all 100ms ease;
	}

	.res-preset:hover {
		background: #1d1d22;
		color: #d4d0c8;
	}

	.res-preset.active {
		background: rgba(245, 166, 35, 0.12);
		color: #f5a623;
		border-color: rgba(245, 166, 35, 0.4);
	}

	.res-custom {
		display: grid;
		grid-template-columns: 1fr 1fr 1fr;
		gap: 6px;
	}

	.res-field {
		display: flex;
		flex-direction: column;
		gap: 3px;
	}

	.res-field label {
		font-size: 9px;
		font-weight: 700;
		color: #555;
		letter-spacing: 0.04em;
	}

	.res-field input {
		padding: 5px 7px;
		background: #111113;
		color: #d4d0c8;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		font-size: 11px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		outline: none;
	}

	.res-field input:focus-visible {
		border-color: #f5a623;
	}

	.url-input {
		width: 100%;
		padding: 6px 8px;
		background: #111113;
		color: #d4d0c8;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		font-size: 11px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		outline: none;
		margin-bottom: 4px;
	}

	.url-input:focus-visible {
		border-color: #f5a623;
	}

	.url-hint {
		font-size: 9px;
		color: #555;
		font-style: italic;
		line-height: 1.4;
	}

	/* ===== Phase 8 desktop capture controls ===== */
	.section-label-row {
		display: flex;
		align-items: center;
		justify-content: space-between;
		gap: 8px;
	}

	.rescan-btn {
		font-size: 9px;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		padding: 3px 8px;
		background: transparent;
		color: #777;
		border: 1px solid #333;
		border-radius: 3px;
		cursor: pointer;
	}
	.rescan-btn:hover:not(:disabled) {
		color: #00f0ff;
		border-color: #00f0ff;
	}
	.rescan-btn:disabled { opacity: 0.45; cursor: wait; }

	.dc-banner-error {
		font-size: 10px;
		color: #ef4444;
		background: rgba(239, 68, 68, 0.1);
		padding: 6px 8px;
		border-radius: 4px;
		border-left: 3px solid #ef4444;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
	}

	.dc-summary {
		font-size: 10px;
		color: #6b6b6b;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
	}

	.dc-kind-row {
		display: flex;
		gap: 4px;
	}

	.dc-kind-pill {
		flex: 1;
		padding: 4px 8px;
		background: #111113;
		color: #777;
		border: 1px solid #2a2a2e;
		border-radius: 4px;
		font-size: 10px;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		text-transform: uppercase;
		letter-spacing: 0.04em;
		cursor: pointer;
		transition: all 120ms ease;
	}
	.dc-kind-pill:hover { color: #d4d0c8; border-color: #444; }
	.dc-kind-pill.dc-kind-active {
		background: rgba(0, 240, 255, 0.08);
		color: #00f0ff;
		border-color: rgba(0, 240, 255, 0.5);
	}

	.dc-pickbox {
		display: flex;
		flex-direction: column;
		gap: 6px;
		padding: 8px;
		background: #111113;
		border: 1px solid #1d1d20;
		border-radius: 4px;
	}

	.dc-row {
		display: flex;
		align-items: center;
		gap: 8px;
		font-size: 11px;
		color: #d4d0c8;
	}
	.dc-row-label {
		min-width: 60px;
		color: #777;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
	}
	.dc-pickbox select {
		flex: 1;
		min-width: 0;
		background: #1a1a1e;
		color: #d4d0c8;
		border: 1px solid #333;
		border-radius: 3px;
		font-size: 11px;
		font-family: inherit;
		padding: 4px 6px;
	}

	.dc-region-grid {
		display: grid;
		grid-template-columns: 1fr 1fr;
		gap: 6px;
	}
	.dc-region-grid label,
	.dc-opt-grid label {
		display: flex;
		flex-direction: column;
		gap: 2px;
		font-size: 10px;
		color: #777;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
	}
	.dc-region-grid input,
	.dc-opt-grid input[type='number'] {
		padding: 4px 6px;
		background: #1a1a1e;
		color: #d4d0c8;
		border: 1px solid #333;
		border-radius: 3px;
		font-size: 11px;
		font-family: inherit;
	}

	.dc-opt-grid {
		display: grid;
		grid-template-columns: 1fr 1fr;
		gap: 6px;
		padding: 8px;
		background: #111113;
		border: 1px solid #1d1d20;
		border-radius: 4px;
	}
	.dc-checkbox {
		grid-column: 1 / -1;
		flex-direction: row !important;
		align-items: center !important;
		gap: 6px !important;
		color: #d4d0c8 !important;
		font-size: 11px !important;
	}

	.dc-cli-block {
		display: flex;
		flex-direction: column;
		gap: 6px;
		padding: 8px;
		background: #0a0a0c;
		border: 1px solid #222;
		border-radius: 4px;
	}
	.dc-cli-label {
		font-size: 9px;
		color: #6b6b6b;
		text-transform: uppercase;
		letter-spacing: 0.05em;
	}
	.dc-cli {
		display: block;
		padding: 6px 8px;
		background: #050507;
		color: #00f0ff;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		font-size: 10.5px;
		border-radius: 3px;
		word-break: break-all;
		white-space: pre-wrap;
		line-height: 1.5;
	}
	.dc-copy-btn {
		align-self: flex-end;
		padding: 4px 10px;
		background: transparent;
		color: #d4d0c8;
		border: 1px solid #444;
		border-radius: 3px;
		font-size: 10px;
		font-family: inherit;
		cursor: pointer;
	}
	.dc-copy-btn:hover { border-color: #00f0ff; color: #00f0ff; }

	.dc-hint {
		font-size: 9px;
		color: #555;
		font-style: italic;
		line-height: 1.4;
	}
	.dc-hint code {
		color: #f5a623;
		font-style: normal;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
	}

	.drawer-footer {
		margin-top: auto;
		padding: 12px 16px;
		border-top: 1px solid #1d1d20;
		background: #111113;
	}

	.apply-btn {
		width: 100%;
		display: flex;
		align-items: center;
		justify-content: center;
		gap: 8px;
		padding: 10px 12px;
		background: rgba(245, 166, 35, 0.14);
		color: #f5a623;
		border: 1px solid rgba(245, 166, 35, 0.4);
		border-radius: 4px;
		font-family: inherit;
		font-size: 12px;
		font-weight: 600;
		letter-spacing: 0.04em;
		cursor: pointer;
		transition: all 120ms ease;
	}

	.apply-btn:hover {
		background: rgba(245, 166, 35, 0.22);
		border-color: #f5a623;
	}

	.apply-hint {
		font-size: 10px;
		color: rgba(245, 166, 35, 0.7);
		font-weight: 400;
	}
</style>
