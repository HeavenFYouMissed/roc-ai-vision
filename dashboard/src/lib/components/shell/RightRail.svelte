<!--
	roc-ai-vision · dashboard · RightRail.svelte

	The Roboflow-style 280px right column. Per active destination:
		Operate / Engage: Detection JSON Feed (top 40%) + Thresholds & Display
		                   (middle 30%) + Selected Track Inspector (bottom 30%)
		Inspect:         Focused-panel settings (Phase 3 sessions)
		Calibrate:       Step help (Phase 4 sessions)

	Collapsible via `]` hotkey, focusable via `,` hotkey.
-->
<script lang="ts">
	import { trackPool, MAX_SLOTS } from '$lib/stores/track_pool.svelte';
	import { renderSettings, type LabelDisplayMode, type BBoxStyle } from '$lib/stores/render_settings.svelte';
	import { selectionStore } from '$lib/stores/selection_store.svelte';
	import { telemetrySocket } from '$lib/telemetry/telemetry_socket.svelte';
	import { focusedPanel } from '$lib/stores/focused_panel.svelte';
	import {
		CLASS_COLORS,
		CLASS_NAMES,
		CLASS_SHORT_NAMES,
		statusFlagLabels
	} from '$lib/render/class_colors';
	import type { ControllerDomain } from '$lib/domains/controller_matrix';

	interface Props {
		activeDomain: ControllerDomain;
		collapsed: boolean;
		onToggleCollapse?: () => void;
	}

	let { activeDomain, collapsed, onToggleCollapse = () => {} }: Props = $props();

	// Active tracks for JSON feed - re-derives on every presence bump
	const activeTracks = $derived.by(() => {
		void trackPool.presenceVersion;
		const out: number[] = [];
		for (let s = 0; s < MAX_SLOTS; s++) {
			if (trackPool.presenceBitmap[s] !== 1) continue;
			const c = trackPool.confidence[s];
			const l = trackPool.label[s];
			if (c < renderSettings.confidenceThreshold) continue;
			if (!renderSettings.classVisibility[l]) continue;
			out.push(s);
		}
		return out;
	});

	const perClassCounts = $derived.by(() => {
		void trackPool.presenceVersion;
		const counts = new Array<number>(CLASS_NAMES.length).fill(0);
		for (let s = 0; s < MAX_SLOTS; s++) {
			if (trackPool.presenceBitmap[s] !== 1) continue;
			counts[trackPool.label[s]]++;
		}
		return counts;
	});

	function onSelectCard(slot: number) {
		selectionStore.select(slot);
	}

	function onHoverCard(slot: number) {
		selectionStore.setHover(slot);
	}

	function onLeaveCard() {
		selectionStore.clearHover();
	}

	function copyDetectionsJson() {
		const payload = activeTracks.map((s) => {
			const b4 = s * 4;
			const v3 = s * 3;
			return {
				track_id: trackPool.idStrings[s],
				class: CLASS_NAMES[trackPool.label[s]],
				confidence: +trackPool.confidence[s].toFixed(3),
				bbox: {
					u: +trackPool.bboxOrig[b4 + 0].toFixed(1),
					v: +trackPool.bboxOrig[b4 + 1].toFixed(1),
					w: +trackPool.bboxOrig[b4 + 2].toFixed(1),
					h: +trackPool.bboxOrig[b4 + 3].toFixed(1)
				},
				p_world: [
					+trackPool.pWorld[v3 + 0].toFixed(3),
					+trackPool.pWorld[v3 + 1].toFixed(3),
					+trackPool.pWorld[v3 + 2].toFixed(3)
				],
				v_world: [
					+trackPool.vWorld[v3 + 0].toFixed(3),
					+trackPool.vWorld[v3 + 1].toFixed(3),
					+trackPool.vWorld[v3 + 2].toFixed(3)
				],
				flags: statusFlagLabels(trackPool.statusFlags[s])
			};
		});
		try {
			navigator.clipboard.writeText(JSON.stringify(payload, null, 2));
		} catch {
			// silently fail
		}
	}

	function setConfidence(v: number) {
		renderSettings.confidenceThreshold = v;
		renderSettings.persist();
	}

	function setIou(v: number) {
		renderSettings.iouThreshold = v;
		renderSettings.persist();
	}

	function setLabelMode(m: LabelDisplayMode) {
		renderSettings.labelMode = m;
		renderSettings.persist();
	}

	function setBBoxStyle(s: BBoxStyle) {
		renderSettings.bboxStyle = s;
		renderSettings.persist();
	}

	function toggleClassVisibility(label: number) {
		const next = [...renderSettings.classVisibility];
		next[label] = !next[label];
		renderSettings.classVisibility = next;
		renderSettings.persist();
	}

	function toggleDisplay(key: keyof typeof toggleMap) {
		toggleMap[key]();
		renderSettings.persist();
	}

	const toggleMap = {
		showTrackId: () => (renderSettings.showTrackId = !renderSettings.showTrackId),
		showAnchor: () => (renderSettings.showAnchor = !renderSettings.showAnchor),
		showVelocityArrow: () => (renderSettings.showVelocityArrow = !renderSettings.showVelocityArrow),
		showUncertaintyEllipse: () =>
			(renderSettings.showUncertaintyEllipse = !renderSettings.showUncertaintyEllipse),
		showFovRing: () => (renderSettings.showFovRing = !renderSettings.showFovRing),
		showTrajectories: () => (renderSettings.showTrajectories = !renderSettings.showTrajectories),
		showHierarchy: () => (renderSettings.showHierarchy = !renderSettings.showHierarchy),
		showActuatorTrace: () => (renderSettings.showActuatorTrace = !renderSettings.showActuatorTrace)
	} as const;

	const labelModes: { value: LabelDisplayMode; label: string }[] = [
		{ value: 'confidence', label: 'Confidence' },
		{ value: 'labels', label: 'Labels' },
		{ value: 'both', label: 'Both' },
		{ value: 'none', label: 'None' }
	];

	const bboxStyles: { value: BBoxStyle; label: string }[] = [
		{ value: 'solid', label: 'Solid' },
		{ value: 'dashed', label: 'Dashed' },
		{ value: 'corners', label: 'Corners' },
		{ value: 'outline', label: 'Outline' }
	];

	// ss11.3 targeting anchor semantics — bbox-relative coordinate where
	// the tracker locks the center-of-mass. Tooltips per Claude observation #2.
	const anchorOptions = [
		{ value: 0, label: 'bbox_top', short: 'T', desc: 'TOP — edge center · hat/accessory tracking' },
		{ value: 1, label: 'bbox_top_center', short: 'TC', desc: 'TOP-CENTER — slightly above center · head/face' },
		{ value: 2, label: 'bbox_center', short: 'C', desc: 'CENTER — geometric center · body/torso (default)' },
		{ value: 3, label: 'bbox_center_lower', short: 'CL', desc: 'CENTER-LOWER — slightly below center · hip/COM' },
		{ value: 4, label: 'bbox_bottom_center', short: 'BC', desc: 'BOTTOM-CENTER — edge center · foot/ground-plane' },
		{ value: 5, label: 'custom_consensus', short: 'CS', desc: 'CUSTOM CONSENSUS — weighted avg of hierarchical bboxes' }
	];

	let selectedAnchor = $state(2); // default to bbox_center

	function setAnchor(v: number) {
		selectedAnchor = v;
		telemetrySocket.setTargetingAnchor(v);
	}

	// Selected slot data extraction
	const selectedData = $derived.by(() => {
		void trackPool.presenceVersion;
		const s = selectionStore.selectedSlot;
		if (s < 0 || trackPool.presenceBitmap[s] !== 1) return null;
		const b4 = s * 4;
		const v3 = s * 3;
		return {
			slot: s,
			idString: trackPool.idStrings[s] || `0x${trackPool.trackIdLo[s].toString(16)}`,
			label: trackPool.label[s],
			className: CLASS_NAMES[trackPool.label[s]],
			confidence: trackPool.confidence[s],
			bbox: {
				u: trackPool.bboxOrig[b4 + 0],
				v: trackPool.bboxOrig[b4 + 1],
				w: trackPool.bboxOrig[b4 + 2],
				h: trackPool.bboxOrig[b4 + 3]
			},
			pWorld: [
				trackPool.pWorld[v3 + 0],
				trackPool.pWorld[v3 + 1],
				trackPool.pWorld[v3 + 2]
			],
			vWorld: [
				trackPool.vWorld[v3 + 0],
				trackPool.vWorld[v3 + 1],
				trackPool.vWorld[v3 + 2]
			],
			aWorld: [
				trackPool.aWorld[v3 + 0],
				trackPool.aWorld[v3 + 1],
				trackPool.aWorld[v3 + 2]
			],
			pPosDiag: [
				trackPool.pPosDiag[v3 + 0],
				trackPool.pPosDiag[v3 + 1],
				trackPool.pPosDiag[v3 + 2]
			],
			parentIdLo: trackPool.parentIdLo[s],
			parentIdHi: trackPool.parentIdHi[s],
			flags: trackPool.statusFlags[s],
			flagLabels: statusFlagLabels(trackPool.statusFlags[s])
		};
	});
</script>

{#if !collapsed}
	<aside class="right-rail" aria-label="Right rail">
		{#if activeDomain === 'operate' || activeDomain === 'engage'}
			<!-- ============================================================ -->
			<!-- SECTION 1: DETECTION JSON FEED (Roboflow style)             -->
			<!-- ============================================================ -->
			<section class="rail-section detections-section">
				<header class="section-header">
					<span class="section-title">DETECTIONS</span>
					<span class="section-count">{activeTracks.length} OBJ</span>
					<button
						class="header-icon-btn"
						onclick={copyDetectionsJson}
						title="Copy JSON to clipboard"
						aria-label="Copy detections JSON"
					>
						<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
							<rect x="9" y="9" width="13" height="13" rx="2" ry="2"></rect>
							<path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"></path>
						</svg>
					</button>
				</header>
				<div class="detections-list">
					{#if activeTracks.length === 0}
						{#each Array(3) as _, i (i)}
							<div class="detection-card skeleton">
								<div class="dc-top">
									<span class="dc-dot skel"></span>
									<span class="dc-name skel-line"></span>
									<span class="dc-conf skel-line short"></span>
								</div>
								<div class="dc-meta skel-line"></div>
								<div class="dc-meta skel-line"></div>
								<div class="dc-meta skel-line"></div>
							</div>
						{/each}
						<div class="empty-msg">Awaiting detections...</div>
					{:else}
						{#each activeTracks as slot (slot)}
							{@const b4 = slot * 4}
							{@const v3 = slot * 3}
							{@const label = trackPool.label[slot]}
							{@const isSel = selectionStore.selectedSlot === slot}
							{@const isHov = selectionStore.hoveredSlot === slot}
							{@const isViolation = (trackPool.statusFlags[slot] & 0x08) !== 0}
							<button
								class="detection-card"
								class:selected={isSel}
								class:hovered={isHov}
								class:violation={isViolation}
								style="--class-color: {CLASS_COLORS[label]}"
								onclick={() => onSelectCard(slot)}
								onmouseenter={() => onHoverCard(slot)}
								onmouseleave={onLeaveCard}
							>
								<div class="dc-top">
									<span class="dc-dot"></span>
									<span class="dc-name">{CLASS_NAMES[label]}</span>
									<span class="dc-conf">{(trackPool.confidence[slot] * 100).toFixed(1)}%</span>
								</div>
								<!-- 2-col micro-grid for column alignment across cards (Claude polish #4) -->
								<dl class="dc-meta-grid">
									<dt>id</dt>
									<dd>{trackPool.idStrings[slot]}</dd>
									<dt>flags</dt>
									<dd>{trackPool.statusFlags[slot].toString(16).padStart(2, '0')}</dd>
									<dt>bbox</dt>
									<dd>{trackPool.bboxOrig[b4 + 0].toFixed(0)},{trackPool.bboxOrig[b4 + 1].toFixed(0)} · {trackPool.bboxOrig[b4 + 2].toFixed(0)}x{trackPool.bboxOrig[b4 + 3].toFixed(0)}</dd>
									<dt>vel</dt>
									<dd>{trackPool.vWorld[v3 + 0].toFixed(1)},{trackPool.vWorld[v3 + 1].toFixed(1)}</dd>
								</dl>
							</button>
						{/each}
					{/if}
				</div>
			</section>

			<!-- ============================================================ -->
			<!-- SECTION 2: THRESHOLDS & DISPLAY                              -->
			<!-- ============================================================ -->
			<section class="rail-section thresholds-section">
				<header class="section-header">
					<span class="section-title">THRESHOLDS &amp; DISPLAY</span>
				</header>
				<div class="thresholds-body">
					<div class="control-row">
						<label for="conf-slider" class="control-label">
							Confidence
							<span class="control-value">{(renderSettings.confidenceThreshold * 100).toFixed(0)}%</span>
						</label>
						<input
							id="conf-slider"
							type="range"
							min="0"
							max="1"
							step="0.01"
							value={renderSettings.confidenceThreshold}
							oninput={(e) => setConfidence(+e.currentTarget.value)}
						/>
					</div>

					<div class="control-row">
						<label for="iou-slider" class="control-label">
							Overlap (IoU)
							<span class="control-value">{(renderSettings.iouThreshold * 100).toFixed(0)}%</span>
						</label>
						<input
							id="iou-slider"
							type="range"
							min="0"
							max="1"
							step="0.01"
							value={renderSettings.iouThreshold}
							oninput={(e) => setIou(+e.currentTarget.value)}
						/>
					</div>

					<div class="control-row">
						<label class="control-label">Label Display</label>
						<select
							class="dropdown"
							value={renderSettings.labelMode}
							onchange={(e) => setLabelMode(e.currentTarget.value as LabelDisplayMode)}
						>
							{#each labelModes as m}
								<option value={m.value}>{m.label}</option>
							{/each}
						</select>
					</div>

					<div class="control-row">
						<label class="control-label">BBox Style</label>
						<div class="segmented">
							{#each bboxStyles as s}
								<button
									class="seg-btn"
									class:active={renderSettings.bboxStyle === s.value}
									onclick={() => setBBoxStyle(s.value)}
								>{s.label}</button>
							{/each}
						</div>
					</div>

					<div class="control-row">
						<label class="control-label">Class Visibility</label>
						<div class="class-list">
							{#each CLASS_NAMES as name, label}
								<label class="class-row">
									<input
										type="checkbox"
										checked={renderSettings.classVisibility[label]}
										onchange={() => toggleClassVisibility(label)}
									/>
									<span class="class-swatch" style="background: {CLASS_COLORS[label]}"></span>
									<span class="class-name">{name}</span>
									<span class="class-count">{perClassCounts[label]}</span>
								</label>
							{/each}
						</div>
					</div>

					<div class="control-row">
						<label class="control-label">Display Toggles</label>
						<div class="toggle-grid">
							<button
								class="tog-btn"
								class:active={renderSettings.showTrackId}
								onclick={() => toggleDisplay('showTrackId')}
							>ID</button>
							<button
								class="tog-btn"
								class:active={renderSettings.showTrajectories}
								onclick={() => toggleDisplay('showTrajectories')}
							>Trail</button>
							<button
								class="tog-btn"
								class:active={renderSettings.showAnchor}
								onclick={() => toggleDisplay('showAnchor')}
							>Anchor</button>
							<button
								class="tog-btn"
								class:active={renderSettings.showVelocityArrow}
								onclick={() => toggleDisplay('showVelocityArrow')}
							>Vel</button>
							<button
								class="tog-btn"
								class:active={renderSettings.showUncertaintyEllipse}
								onclick={() => toggleDisplay('showUncertaintyEllipse')}
							>Ellipse</button>
							<button
								class="tog-btn"
								class:active={renderSettings.showHierarchy}
								onclick={() => toggleDisplay('showHierarchy')}
							>Hier</button>
							<button
								class="tog-btn"
								class:active={renderSettings.showFovRing}
								onclick={() => toggleDisplay('showFovRing')}
							>FOV</button>
							<button
								class="tog-btn"
								class:active={renderSettings.showActuatorTrace}
								onclick={() => toggleDisplay('showActuatorTrace')}
							>Trace</button>
						</div>
					</div>
				</div>
			</section>

			<!-- ============================================================ -->
			<!-- SECTION 3: SELECTED TRACK INSPECTOR                          -->
			<!-- ============================================================ -->
			<section class="rail-section selected-section">
				<header class="section-header">
					<span class="section-title">SELECTED TRACK</span>
					{#if selectedData}
						<button
							class="header-icon-btn"
							onclick={() => selectionStore.deselect()}
							title="Deselect"
							aria-label="Deselect"
						>
							<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
								<line x1="18" y1="6" x2="6" y2="18"></line>
								<line x1="6" y1="6" x2="18" y2="18"></line>
							</svg>
						</button>
					{/if}
				</header>
				<div class="selected-body">
					{#if !selectedData}
						<div class="empty-target">
							<svg class="target-icon" viewBox="0 0 48 48" fill="none" stroke="currentColor" stroke-width="1.5" stroke-dasharray="3 3">
								<circle cx="24" cy="24" r="20"></circle>
								<circle cx="24" cy="24" r="12"></circle>
								<circle cx="24" cy="24" r="4" stroke-dasharray="0" fill="currentColor" fill-opacity="0.2"></circle>
								<line x1="24" y1="0" x2="24" y2="6"></line>
								<line x1="24" y1="42" x2="24" y2="48"></line>
								<line x1="0" y1="24" x2="6" y2="24"></line>
								<line x1="42" y1="24" x2="48" y2="24"></line>
							</svg>
							<div class="empty-target-text">
								Click a <span class="accent-word">detection</span><br />
								on the canvas or in the feed above
							</div>
						</div>
					{:else}
						<div class="sel-id-row">
							<span class="sel-id" style="color: {CLASS_COLORS[selectedData.label]}">
								{selectedData.idString}
							</span>
							<span class="sel-class">{selectedData.className}</span>
						</div>
						<div class="sel-stats">
							<div class="stat-row"><span class="stat-key">conf</span><span class="stat-val">{(selectedData.confidence * 100).toFixed(1)}%</span></div>
							<div class="stat-row"><span class="stat-key">bbox</span><span class="stat-val">{selectedData.bbox.u.toFixed(0)}, {selectedData.bbox.v.toFixed(0)}, {selectedData.bbox.w.toFixed(0)} x {selectedData.bbox.h.toFixed(0)}</span></div>
							<div class="stat-row"><span class="stat-key">pos m</span><span class="stat-val">{selectedData.pWorld[0].toFixed(2)}, {selectedData.pWorld[1].toFixed(2)}, {selectedData.pWorld[2].toFixed(2)}</span></div>
							<div class="stat-row"><span class="stat-key">vel m/s</span><span class="stat-val">{selectedData.vWorld[0].toFixed(2)}, {selectedData.vWorld[1].toFixed(2)}, {selectedData.vWorld[2].toFixed(2)}</span></div>
							<div class="stat-row"><span class="stat-key">acc</span><span class="stat-val">{selectedData.aWorld[0].toFixed(2)}, {selectedData.aWorld[1].toFixed(2)}, {selectedData.aWorld[2].toFixed(2)}</span></div>
							<div class="stat-row"><span class="stat-key">uncert</span><span class="stat-val">{selectedData.pPosDiag[0].toFixed(2)}, {selectedData.pPosDiag[1].toFixed(2)}, {selectedData.pPosDiag[2].toFixed(2)}</span></div>
							{#if selectedData.parentIdLo !== 0 || selectedData.parentIdHi !== 0}
								<div class="stat-row"><span class="stat-key">parent</span><span class="stat-val">0x{selectedData.parentIdLo.toString(16).padStart(8, '0')}</span></div>
							{/if}
						</div>
						{#if selectedData.flagLabels.length > 0}
							<div class="flag-row">
								{#each selectedData.flagLabels as f}
									<span class="flag-pill" class:violation={f === 'HIERARCHY'}>{f}</span>
								{/each}
							</div>
						{/if}
						<div class="anchor-section">
							<div class="anchor-label">TARGETING ANCHOR</div>
							<div class="anchor-segments">
								{#each anchorOptions as opt}
									<button
										class="anchor-seg"
										class:active={selectedAnchor === opt.value}
										onclick={() => setAnchor(opt.value)}
										title={opt.desc}
									>{opt.short}</button>
								{/each}
							</div>
							<div class="anchor-hover-hint">
								{anchorOptions[selectedAnchor]?.desc ?? ''}
							</div>
						</div>
						<div class="sel-actions">
							<button class="action-btn primary" disabled>Lock</button>
							<button class="action-btn">Export</button>
							<button class="action-btn">Mark</button>
						</div>
					{/if}
				</div>
			</section>
		{:else if activeDomain === 'inspect'}
			<section class="rail-section panel-settings-section">
				<header class="section-header">
					<span class="section-title">PANEL SETTINGS</span>
					{#if focusedPanel.focusedId}
						<button class="header-icon-btn" onclick={() => focusedPanel.clear()} title="Clear focus">
							<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
								<line x1="18" y1="6" x2="6" y2="18"></line>
								<line x1="6" y1="6" x2="18" y2="18"></line>
							</svg>
						</button>
					{/if}
				</header>
				<div class="panel-settings-body">
					{#if !focusedPanel.focusedId}
						<div class="empty-msg compact">
							Click any panel header<br />
							to view its settings
						</div>
					{:else if focusedPanel.focusedId === 'mouse-dynamics-inspector'}
						<div class="ps-section">
							<div class="ps-label">DATA SOURCE</div>
							<div class="ps-val">actuatorPool · 4096 samples · 500 Hz</div>
						</div>
						<div class="ps-section">
							<div class="ps-label">TRACE MODE (c/d/t/s)</div>
							<div class="ps-modes">
								<button class="ps-mode active">c · Clean</button>
								<button class="ps-mode">d · Dirty</button>
								<button class="ps-mode">t · Overlap</button>
								<button class="ps-mode">s · Side-by-side</button>
							</div>
						</div>
						<div class="ps-section">
							<div class="ps-label">RECORDING</div>
							<div class="ps-val">Press <kbd>R</kbd> to start · <kbd>X</kbd> to stop · <kbd>P</kbd> to pause</div>
						</div>
						<div class="ps-section">
							<div class="ps-label">BUFFER WINDOW</div>
							<select class="ps-select">
								<option>500 samples · 1.0 s @ 500 Hz</option>
								<option>1000 samples · 2.0 s @ 500 Hz</option>
								<option selected>2000 samples · 4.0 s @ 500 Hz</option>
								<option>4096 samples · 8.2 s @ 500 Hz (full)</option>
							</select>
						</div>
					{:else if focusedPanel.focusedId === 'velocity-profile'}
						<div class="ps-section">
							<div class="ps-label">FEATURE</div>
							<div class="ps-val">F01 · Tangential velocity (px/s)</div>
						</div>
						<div class="ps-section">
							<div class="ps-label">WINDOW SIZE</div>
							<div class="ps-modes">
								<button class="ps-mode">100 ms</button>
								<button class="ps-mode">250 ms</button>
								<button class="ps-mode">500 ms</button>
								<button class="ps-mode active">1000 ms</button>
							</div>
						</div>
						<div class="ps-section">
							<div class="ps-label">SMOOTHING</div>
							<label class="ps-check">
								<input type="checkbox" checked />
								Overlay smoothing band
							</label>
						</div>
					{:else if focusedPanel.focusedId === 'fitts-residual'}
						<div class="ps-section">
							<div class="ps-label">FEATURE</div>
							<div class="ps-val">F21 · Fitts engagement-time residual</div>
						</div>
						<div class="ps-section">
							<div class="ps-label">REGRESSION TYPE</div>
							<select class="ps-select">
								<option selected>linear · MT = a + b·ID</option>
								<option>power-law · MT = a·ID^b</option>
								<option>per-user (requires session ≥30s)</option>
							</select>
						</div>
						<div class="ps-section">
							<div class="ps-label">OUTLIER THRESHOLD (σ)</div>
							<input type="range" min="1" max="5" step="0.5" value="3" />
							<div class="ps-hint">Points >3σ below regression line shown red</div>
						</div>
					{:else if focusedPanel.focusedId === 'subpixel'}
						<div class="ps-section">
							<div class="ps-label">FEATURE</div>
							<div class="ps-val">F16 · Sub-pixel sampling ratio</div>
						</div>
						<div class="ps-section">
							<div class="ps-label">WINDOW</div>
							<div class="ps-modes">
								<button class="ps-mode">500 samples</button>
								<button class="ps-mode active">1000 samples</button>
								<button class="ps-mode">2000 samples</button>
							</div>
						</div>
						<div class="ps-section">
							<div class="ps-label">REFERENCE BANDS</div>
							<div class="ps-val">Human: 0.70–0.95 · Bot: 0.00–0.10</div>
						</div>
					{:else if focusedPanel.focusedId === 'jerk-spectrum'}
						<div class="ps-section">
							<div class="ps-label">FEATURES</div>
							<div class="ps-val">F03 jerk · F08 FFT peak · F09 tremor band</div>
						</div>
						<div class="ps-section">
							<div class="ps-label">MAGNITUDE SCALE</div>
							<div class="ps-modes">
								<button class="ps-mode">Linear</button>
								<button class="ps-mode active">Log</button>
							</div>
						</div>
						<div class="ps-section">
							<div class="ps-label">TREMOR BAND HIGHLIGHT</div>
							<label class="ps-check">
								<input type="checkbox" checked />
								Highlight 8–12 Hz band
							</label>
						</div>
					{:else if focusedPanel.focusedId === 'spectator-telemetry-matrix'}
						<div class="ps-section">
							<div class="ps-label">DATA SOURCES</div>
							<div class="ps-val">actuatorPool (cyan) vs bgOdometryPool (amber)</div>
						</div>
						<div class="ps-section">
							<div class="ps-label">CORRELATION WINDOW (l/[/])</div>
							<div class="ps-modes">
								<button class="ps-mode">4 s</button>
								<button class="ps-mode active">8 s</button>
								<button class="ps-mode">16 s</button>
								<button class="ps-mode">32 s</button>
							</div>
						</div>
						<div class="ps-section">
							<div class="ps-label">Y-AXIS SCALE</div>
							<div class="ps-modes">
								<button class="ps-mode active">Linear</button>
								<button class="ps-mode">Log (l)</button>
							</div>
						</div>
						<div class="ps-section">
							<div class="ps-label">AUTOMATION ALERT</div>
							<div class="ps-val">ρ &lt; 0.6 OR lag drift &gt; +3 frames</div>
						</div>
					{:else if focusedPanel.focusedId === 'pipeline-graph'}
						<div class="ps-section">
							<div class="ps-label">PIPELINE</div>
							<div class="ps-val">Camera → Inference → Tracker → OutputSink</div>
						</div>
						<div class="ps-section">
							<div class="ps-label">HEALTH POLICY</div>
							<div class="ps-val">Per-node ● OK / Warn / Err derived from latency thresholds</div>
						</div>
					{:else if focusedPanel.focusedId === 'model-inspector'}
						<div class="ps-section">
							<div class="ps-label">ACTIVE MODEL</div>
							<div class="ps-val">yolo26m-roc-humanoid v0.1</div>
						</div>
						<div class="ps-section">
							<div class="ps-label">EP COLOR LEGEND</div>
							<div class="ps-val">CUDA · TensorRT · DML · OpenVINO · CPU</div>
						</div>
					{:else if focusedPanel.focusedId === 'topic-graph'}
						<div class="ps-section">
							<div class="ps-label">TOPICS</div>
							<div class="ps-val">v1 tracks · v2 actuator · v3 bg-odometry</div>
						</div>
					{/if}
				</div>
			</section>
		{:else if activeDomain === 'calibrate'}
			<section class="rail-section">
				<header class="section-header">
					<span class="section-title">STEP HELP</span>
				</header>
				<div class="step-help">
					<p>Calibrate the system step-by-step. Each step persists to the active Scenario when saved.</p>
					<p>Step 3 sliders apply in real-time via OperatorCommand. The actuator trace on the Operate canvas updates within 2ms.</p>
				</div>
			</section>
		{/if}
	</aside>
{:else}
	<div class="rail-collapsed-tab">
		<button class="expand-btn" onclick={onToggleCollapse} title="Expand right rail (])">
			<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
				<polyline points="15 18 9 12 15 6"></polyline>
			</svg>
		</button>
	</div>
{/if}

<style>
	.right-rail {
		width: 280px;
		flex-shrink: 0;
		background: #111113;
		border-left: 1px solid #27272a;
		display: flex;
		flex-direction: column;
		overflow: hidden;
		font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
	}

	.rail-section {
		display: flex;
		flex-direction: column;
		border-bottom: 1px solid #1d1d20;
		overflow: hidden;
	}

	.detections-section {
		flex: 1 1 40%;
		min-height: 200px;
	}

	.thresholds-section {
		flex: 0 0 auto;
		max-height: 50vh;
		overflow-y: auto;
	}

	.selected-section {
		flex: 1 1 30%;
		min-height: 220px;
	}

	.section-header {
		display: flex;
		align-items: center;
		gap: 8px;
		padding: 8px 12px;
		background: #18181b;
		border-bottom: 1px solid #1d1d20;
		min-height: 30px;
	}

	.section-title {
		font-size: 9px;
		font-weight: 700;
		letter-spacing: 0.08em;
		color: #878787;
		text-transform: uppercase;
		flex: 1;
	}

	.section-count {
		font-size: 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-weight: 700;
		color: #f5a623;
		padding: 1px 6px;
		background: rgba(245, 166, 35, 0.1);
		border-radius: 3px;
	}

	.header-icon-btn {
		background: transparent;
		border: none;
		color: #555;
		cursor: pointer;
		padding: 2px;
		display: flex;
		align-items: center;
	}

	.header-icon-btn:hover {
		color: #d4d0c8;
	}

	.header-icon-btn svg {
		width: 14px;
		height: 14px;
	}

	/* ---- Detection cards ---- */
	.detections-list {
		flex: 1;
		overflow-y: auto;
		padding: 6px;
		display: flex;
		flex-direction: column;
		gap: 4px;
	}

	.detection-card {
		display: flex;
		flex-direction: column;
		gap: 3px;
		padding: 8px 10px;
		background: #161618;
		border: 1px solid #1d1d20;
		border-left: 3px solid transparent;
		border-radius: 4px;
		text-align: left;
		cursor: pointer;
		font-family: inherit;
		color: #d4d0c8;
		transition: all 100ms ease;
	}

	.detection-card:hover,
	.detection-card.hovered {
		border-left-color: #f5a623;
		background: #1d1d22;
	}

	.detection-card.selected {
		border-left-color: #f5a623;
		border-left-width: 4px;
		background: rgba(245, 166, 35, 0.06);
	}

	.detection-card.violation {
		border-left-color: #ef4444;
		background: rgba(239, 68, 68, 0.04);
	}

	.detection-card.skeleton {
		pointer-events: none;
		opacity: 0.35;
	}

	.dc-top {
		display: flex;
		align-items: center;
		gap: 6px;
	}

	.dc-dot {
		width: 8px;
		height: 8px;
		border-radius: 50%;
		background: var(--class-color, #6b6b6f);
		box-shadow: 0 0 4px var(--class-color, #6b6b6f);
	}

	.dc-dot.skel {
		background: #2a2a2e;
		box-shadow: none;
	}

	.dc-name {
		flex: 1;
		font-size: 12px;
		font-weight: 500;
		color: #d4d0c8;
	}

	.dc-conf {
		font-size: 12px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-weight: 600;
		color: #f5a623;
	}

	/* 2-col micro-grid: column-aligned label/value pairs across cards */
	.dc-meta-grid {
		display: grid;
		grid-template-columns: 32px 1fr;
		gap: 1px 8px;
		margin: 0;
		font-size: 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #878787;
	}

	.dc-meta-grid dt {
		color: #555;
		font-weight: 400;
	}

	.dc-meta-grid dd {
		margin: 0;
		color: #d4d0c8;
		overflow: hidden;
		text-overflow: ellipsis;
		white-space: nowrap;
	}

	.skel-line {
		display: inline-block;
		height: 10px;
		background: linear-gradient(90deg, #2a2a2e 25%, #36363a 50%, #2a2a2e 75%);
		background-size: 200% 100%;
		border-radius: 2px;
		flex: 1;
		animation: shimmer 1.4s ease infinite;
	}

	.skel-line.short {
		max-width: 40px;
	}

	@keyframes shimmer {
		0% { background-position: -200% 0; }
		100% { background-position: 200% 0; }
	}

	.empty-msg {
		padding: 20px 12px;
		text-align: center;
		color: #555;
		font-size: 11px;
		font-style: italic;
	}

	.empty-msg.compact {
		padding: 24px 8px;
		line-height: 1.6;
	}

	/* Selected Track empty state — target icon + orange accent word */
	.empty-target {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
		padding: 28px 16px;
		gap: 12px;
		text-align: center;
	}

	.target-icon {
		width: 48px;
		height: 48px;
		color: #2a2a2e;
		opacity: 0.7;
	}

	.empty-target-text {
		font-size: 11px;
		color: #555;
		line-height: 1.6;
		font-style: italic;
	}

	.accent-word {
		color: #f5a623;
		font-style: normal;
		font-weight: 500;
	}

	/* ---- Thresholds ---- */
	.thresholds-body {
		padding: 10px 12px;
		display: flex;
		flex-direction: column;
		gap: 10px;
	}

	.control-row {
		display: flex;
		flex-direction: column;
		gap: 4px;
	}

	.control-label {
		display: flex;
		justify-content: space-between;
		align-items: center;
		font-size: 10px;
		font-weight: 500;
		color: #878787;
		text-transform: uppercase;
		letter-spacing: 0.04em;
	}

	.control-value {
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-weight: 600;
		color: #f5a623;
		text-transform: none;
		letter-spacing: 0;
	}

	input[type='range'] {
		width: 100%;
		height: 4px;
		appearance: none;
		background: #27272a;
		border-radius: 2px;
		outline: none;
		cursor: pointer;
	}

	input[type='range']::-webkit-slider-thumb {
		appearance: none;
		width: 12px;
		height: 12px;
		border-radius: 50%;
		background: #f5a623;
		cursor: pointer;
		border: 2px solid #111113;
	}

	input[type='range']::-moz-range-thumb {
		width: 12px;
		height: 12px;
		border-radius: 50%;
		background: #f5a623;
		cursor: pointer;
		border: 2px solid #111113;
	}

	.dropdown {
		width: 100%;
		padding: 5px 8px;
		background: #1a1a1e;
		color: #d4d0c8;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		font-size: 11px;
		font-family: inherit;
		cursor: pointer;
		outline: none;
	}

	.dropdown:focus-visible {
		border-color: #f5a623;
	}

	.segmented {
		display: flex;
		gap: 2px;
	}

	.seg-btn {
		flex: 1;
		padding: 4px 6px;
		background: #1a1a1e;
		color: #878787;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		font-size: 10px;
		font-family: inherit;
		cursor: pointer;
		transition: all 100ms ease;
	}

	.seg-btn:hover {
		color: #d4d0c8;
		border-color: #3a3a3e;
	}

	.seg-btn.active {
		background: rgba(245, 166, 35, 0.1);
		color: #f5a623;
		border-color: rgba(245, 166, 35, 0.4);
	}

	.class-list {
		display: flex;
		flex-direction: column;
		gap: 2px;
	}

	.class-row {
		display: flex;
		align-items: center;
		gap: 6px;
		padding: 3px 4px;
		border-radius: 3px;
		cursor: pointer;
		font-size: 11px;
		color: #d4d0c8;
	}

	.class-row:hover {
		background: #1d1d20;
	}

	.class-row input[type='checkbox'] {
		width: 12px;
		height: 12px;
		accent-color: #f5a623;
	}

	.class-swatch {
		width: 10px;
		height: 10px;
		border-radius: 2px;
		box-shadow: 0 0 3px currentColor;
	}

	.class-name {
		flex: 1;
	}

	.class-count {
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 10px;
		color: #555;
		min-width: 18px;
		text-align: right;
	}

	.toggle-grid {
		display: grid;
		grid-template-columns: repeat(4, 1fr);
		gap: 3px;
	}

	.tog-btn {
		padding: 4px 0;
		background: #1a1a1e;
		color: #555;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		font-size: 10px;
		font-family: inherit;
		cursor: pointer;
		transition: all 100ms ease;
	}

	.tog-btn:hover {
		color: #d4d0c8;
		border-color: #3a3a3e;
	}

	.tog-btn.active {
		background: rgba(245, 166, 35, 0.12);
		color: #f5a623;
		border-color: rgba(245, 166, 35, 0.4);
	}

	/* ---- Selected Track Inspector ---- */
	.selected-body {
		flex: 1;
		overflow-y: auto;
		padding: 10px 12px;
	}

	.sel-id-row {
		display: flex;
		align-items: baseline;
		gap: 8px;
		margin-bottom: 8px;
	}

	.sel-id {
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 12px;
		font-weight: 700;
	}

	.sel-class {
		font-size: 11px;
		color: #d4d0c8;
	}

	.sel-stats {
		display: flex;
		flex-direction: column;
		gap: 2px;
		margin-bottom: 8px;
	}

	.stat-row {
		display: flex;
		align-items: center;
		gap: 6px;
		font-size: 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
	}

	.stat-key {
		color: #555;
		min-width: 44px;
	}

	.stat-val {
		color: #d4d0c8;
		flex: 1;
	}

	.flag-row {
		display: flex;
		gap: 4px;
		flex-wrap: wrap;
		margin-bottom: 8px;
	}

	.flag-pill {
		font-size: 9px;
		font-weight: 600;
		padding: 2px 5px;
		background: rgba(0, 240, 255, 0.08);
		color: #00f0ff;
		border-radius: 2px;
		text-transform: uppercase;
		letter-spacing: 0.04em;
	}

	.flag-pill.violation {
		background: rgba(239, 68, 68, 0.12);
		color: #ef4444;
	}

	.anchor-section {
		margin-top: 8px;
		padding-top: 8px;
		border-top: 1px solid #1d1d20;
	}

	.anchor-label {
		font-size: 9px;
		font-weight: 700;
		color: #878787;
		text-transform: uppercase;
		letter-spacing: 0.04em;
		margin-bottom: 4px;
	}

	.anchor-segments {
		display: grid;
		grid-template-columns: repeat(6, 1fr);
		gap: 2px;
	}

	.anchor-seg {
		padding: 4px 0;
		background: #1a1a1e;
		color: #555;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		font-size: 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-weight: 600;
		cursor: pointer;
		transition: all 100ms ease;
	}

	.anchor-seg:hover {
		color: #d4d0c8;
	}

	.anchor-seg.active {
		background: rgba(245, 166, 35, 0.12);
		color: #f5a623;
		border-color: rgba(245, 166, 35, 0.4);
	}

	/* Persistent description of the active anchor — supplements per-button tooltips */
	.anchor-hover-hint {
		margin-top: 4px;
		font-size: 9px;
		font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
		color: #555;
		font-style: italic;
		line-height: 1.4;
		min-height: 12px;
	}

	.sel-actions {
		display: flex;
		gap: 4px;
		margin-top: 10px;
		padding-top: 8px;
		border-top: 1px solid #1d1d20;
	}

	.action-btn {
		flex: 1;
		padding: 5px 8px;
		background: #1a1a1e;
		color: #878787;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		font-size: 10px;
		font-family: inherit;
		font-weight: 500;
		cursor: pointer;
		transition: all 100ms ease;
	}

	.action-btn:hover:not(:disabled) {
		color: #d4d0c8;
		border-color: #3a3a3e;
	}

	.action-btn.primary {
		color: #f5a623;
		border-color: rgba(245, 166, 35, 0.4);
	}

	.action-btn:disabled {
		opacity: 0.4;
		cursor: not-allowed;
	}

	.step-help {
		padding: 12px;
		font-size: 11px;
		color: #878787;
		line-height: 1.5;
	}

	.step-help p {
		margin: 0 0 8px;
	}

	/* Inspect panel-settings styling */
	.panel-settings-section {
		flex: 1 1 auto;
		overflow: hidden;
	}

	.panel-settings-body {
		flex: 1;
		overflow-y: auto;
		padding: 10px 12px;
		display: flex;
		flex-direction: column;
		gap: 12px;
	}

	.ps-section {
		display: flex;
		flex-direction: column;
		gap: 4px;
	}

	.ps-label {
		font-size: 9px;
		font-weight: 700;
		text-transform: uppercase;
		letter-spacing: 0.06em;
		color: #555;
	}

	.ps-val {
		font-size: 11px;
		color: #d4d0c8;
	}

	.ps-modes {
		display: flex;
		flex-wrap: wrap;
		gap: 3px;
	}

	.ps-mode {
		flex: 1;
		min-width: 0;
		padding: 4px 6px;
		background: #1a1a1e;
		color: #878787;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		font-size: 10px;
		font-family: inherit;
		cursor: pointer;
		transition: all 100ms ease;
		white-space: nowrap;
		overflow: hidden;
		text-overflow: ellipsis;
	}

	.ps-mode:hover {
		color: #d4d0c8;
		border-color: #3a3a3e;
	}

	.ps-mode.active {
		background: rgba(245, 166, 35, 0.12);
		color: #f5a623;
		border-color: rgba(245, 166, 35, 0.4);
	}

	.ps-select {
		padding: 5px 8px;
		background: #1a1a1e;
		color: #d4d0c8;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		font-size: 11px;
		font-family: inherit;
		cursor: pointer;
		outline: none;
	}

	.ps-select:focus-visible {
		border-color: #f5a623;
	}

	.ps-check {
		display: flex;
		align-items: center;
		gap: 6px;
		font-size: 11px;
		color: #d4d0c8;
		cursor: pointer;
	}

	.ps-check input {
		accent-color: #f5a623;
	}

	.ps-hint {
		font-size: 10px;
		color: #555;
		font-style: italic;
		margin-top: 2px;
	}

	.ps-val kbd {
		display: inline-block;
		padding: 1px 5px;
		background: #222;
		color: #d4d0c8;
		border: 1px solid #333;
		border-radius: 3px;
		font-size: 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		margin: 0 2px;
	}

	/* Collapsed tab */
	.rail-collapsed-tab {
		width: 14px;
		flex-shrink: 0;
		background: #111113;
		border-left: 1px solid #27272a;
		display: flex;
		align-items: center;
		justify-content: center;
	}

	.expand-btn {
		width: 14px;
		height: 40px;
		background: transparent;
		border: none;
		color: #555;
		cursor: pointer;
		display: flex;
		align-items: center;
		justify-content: center;
	}

	.expand-btn:hover {
		color: #f5a623;
	}

	.expand-btn svg {
		width: 12px;
		height: 12px;
	}
</style>
