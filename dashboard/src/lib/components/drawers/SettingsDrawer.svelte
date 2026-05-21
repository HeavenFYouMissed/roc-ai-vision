<script lang="ts">
	import { aimingConfig } from '$lib/stores/aiming_config';
	import { DEFAULT_BINDINGS } from '$lib/keybindings/defaults';
	import { sourceStore } from '$lib/stores/source_store.svelte';

	interface Props {
		open: boolean;
		onClose: () => void;
	}

	let { open, onClose }: Props = $props();

	let activeSection = $state<'profile' | 'hotkeys' | 'prefs'>('profile');
</script>

{#if open}
	<div class="drawer-backdrop" onclick={onClose} role="presentation">
		<aside class="drawer" onclick={(e) => e.stopPropagation()} role="dialog" aria-label="Settings">
			<div class="drawer-header">
				<h3 class="drawer-title">Settings</h3>
				<button class="drawer-close" onclick={onClose} aria-label="Close">
					<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M18 6L6 18M6 6l12 12" /></svg>
				</button>
			</div>

			<div class="section-tabs">
				<button class:active={activeSection === 'profile'} onclick={() => activeSection = 'profile'}>Profile</button>
				<button class:active={activeSection === 'hotkeys'} onclick={() => activeSection = 'hotkeys'}>Hotkeys</button>
				<button class:active={activeSection === 'prefs'} onclick={() => activeSection = 'prefs'}>Preferences</button>
			</div>

			<div class="drawer-body">
				{#if activeSection === 'profile'}
					<div class="settings-group">
						<label class="settings-label">Smoothing Alpha</label>
						<div class="settings-value">
							<input type="range" min="0.05" max="1.0" step="0.05" bind:value={aimingConfig.smoothingCoefficient} />
							<span class="val-num">{aimingConfig.smoothingCoefficient.toFixed(2)}</span>
						</div>
					</div>
					<div class="settings-group">
						<label class="settings-label">Deadband Scaling</label>
						<div class="settings-value">
							<input type="range" min="0.5" max="5.0" step="0.1" bind:value={aimingConfig.deadbandScaling} />
							<span class="val-num">{aimingConfig.deadbandScaling.toFixed(1)}x</span>
						</div>
					</div>
					<div class="settings-group">
						<label class="settings-label">Active Profile</label>
						<span class="val-tag">{aimingConfig.activeProfile}</span>
					</div>
				{:else if activeSection === 'hotkeys'}
					<div class="hotkey-list">
						{#each DEFAULT_BINDINGS as binding}
							<div class="hotkey-row">
								<kbd class="hk-key">{binding.combo}</kbd>
								<span class="hk-label">{binding.label}</span>
							</div>
						{/each}
					</div>
				{:else}
					<div class="settings-group">
						<label class="settings-label">Active Source</label>
						<span class="val-tag mono">{sourceStore.sourceLabel}</span>
						<span class="settings-hint">Sources drawer (left rail) for selection</span>
					</div>
					<div class="settings-group">
						<label class="settings-label">Render Rate</label>
						<span class="val-tag">144 Hz (vsync)</span>
					</div>
					<div class="settings-group">
						<label class="settings-label">Theme</label>
						<span class="val-tag">Dark (Cursor)</span>
					</div>
					<div class="settings-group">
						<label class="settings-label">Wire Bridge</label>
						<span class="val-tag mono">ws://127.0.0.1:8765</span>
					</div>
					<div class="settings-group">
						<label class="settings-label">MJPEG Endpoint (Phase 6+)</label>
						<span class="val-tag mono">{sourceStore.mjpegUrl ?? 'not configured'}</span>
					</div>
				{/if}
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
		width: 320px;
		height: 100vh;
		background: #161618;
		border-right: 1px solid #27272a;
		display: flex;
		flex-direction: column;
		box-shadow: 8px 0 24px rgba(0, 0, 0, 0.3);
		animation: slideIn 150ms ease;
	}

	@keyframes slideIn {
		from { transform: translateX(-20px); opacity: 0.8; }
		to { transform: translateX(0); opacity: 1; }
	}

	.drawer-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding: 14px 16px;
		border-bottom: 1px solid #222;
	}

	.drawer-title { margin: 0; font-size: 14px; font-weight: 600; color: #d4d0c8; }

	.drawer-close {
		width: 28px; height: 28px;
		display: flex; align-items: center; justify-content: center;
		background: transparent; border: none; border-radius: 4px;
		color: #666; cursor: pointer;
	}
	.drawer-close:hover { background: #27272a; color: #d4d0c8; }
	.drawer-close svg { width: 16px; height: 16px; }

	.section-tabs {
		display: flex;
		padding: 0 12px;
		border-bottom: 1px solid #222;
	}

	.section-tabs button {
		padding: 10px 14px;
		background: transparent;
		border: none;
		border-bottom: 2px solid transparent;
		color: #666;
		font-size: 12px;
		font-weight: 500;
		cursor: pointer;
		transition: all 150ms ease;
	}

	.section-tabs button:hover { color: #d4d0c8; }
	.section-tabs button.active { color: #f5a623; border-bottom-color: #f5a623; }

	.drawer-body {
		flex: 1;
		overflow-y: auto;
		padding: 16px;
		display: flex;
		flex-direction: column;
		gap: 16px;
	}

	.settings-group {
		display: flex;
		flex-direction: column;
		gap: 6px;
	}

	.settings-label {
		font-size: 11px;
		font-weight: 500;
		color: #878787;
		text-transform: uppercase;
		letter-spacing: 0.04em;
	}

	.settings-value {
		display: flex;
		align-items: center;
		gap: 10px;
	}

	.settings-value input[type='range'] {
		flex: 1;
		height: 4px;
		appearance: none;
		background: #27272a;
		border-radius: 2px;
		outline: none;
	}

	.settings-value input[type='range']::-webkit-slider-thumb {
		appearance: none;
		width: 12px; height: 12px;
		border-radius: 50%;
		background: #f5a623;
		cursor: pointer;
		border: 2px solid #161618;
	}

	.val-num {
		font-size: 12px;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		color: #f5a623;
		min-width: 40px;
		text-align: right;
	}

	.val-tag {
		font-size: 12px;
		color: #d4d0c8;
		padding: 4px 8px;
		background: #1e1e22;
		border-radius: 4px;
		border: 1px solid #27272a;
		width: fit-content;
	}

	.val-tag.mono {
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		font-size: 11px;
	}

	.settings-hint {
		font-size: 10px;
		color: #555;
		font-style: italic;
	}

	.hotkey-list {
		display: flex;
		flex-direction: column;
		gap: 4px;
	}

	.hotkey-row {
		display: flex;
		align-items: center;
		gap: 10px;
		padding: 5px 6px;
		border-radius: 3px;
	}

	.hotkey-row:hover { background: #1e1e22; }

	.hk-key {
		min-width: 60px;
		font-size: 10px;
		font-family: 'SF Mono', 'Cascadia Code', 'Consolas', monospace;
		padding: 2px 6px;
		background: #222;
		color: #d4d0c8;
		border: 1px solid #333;
		border-radius: 3px;
		text-align: center;
	}

	.hk-label { font-size: 12px; color: #878787; }
</style>
