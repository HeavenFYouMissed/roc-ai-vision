<!--
	roc-ai-vision · dashboard · TelemetryCanvas.svelte

	Hot-path render surface for the Spatial Projection Canvas domain. Reads
	the trackPool typed-array columns inside a requestAnimationFrame loop and
	mutates pre-allocated SVG primitives in place; Svelte reactivity is
	restricted to the four $state runes that drive the header strip.

	Binding contracts:
	  · phase_2_ekf_kinematics.md §7 — track-list emission cadence + flag bits
	  · core/kinematics_engine/include/roc/kinematics/wire_format.hpp — wire
	    payload layout (status_flags bits, bbox centroid form, frame flags)
-->
<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { trackPool, MAX_SLOTS, TRAJECTORY_RING } from '$lib/stores/track_pool.svelte';
	import { decode_frame } from '$lib/telemetry/wire_decoder';
	import { mockFrameSource } from '$lib/telemetry/mock_frame_source';
	import { telemetrySocket } from '$lib/telemetry/telemetry_socket.svelte';

	const LABEL_STRINGS = ['', 'Person', 'Head', 'UpperTorso', 'Hat', 'Backpack'];

	// status_flags bits — mirror wire_format.hpp wire_status_bits.
	const FLAG_BOUNDARY = 0x02;
	const FLAG_STATIONARY = 0x04;
	const FLAG_HIERARCHY_VIOLATION = 0x08;

	// 3-frame red-flash decay for hierarchy violations: the wire flag arrives
	// as a 1-frame pulse from the C++ tracker, so we hold the red stroke
	// locally for the next two frames to make a single-frame violation
	// perceivable by a human operator.
	const violationDecay = new Uint8Array(MAX_SLOTS);

	// Reused scratch array for SVG path 'd' construction. Cleared with
	// length = 0 every tick; the backing store is retained so no per-frame
	// array reallocation happens.
	const pathScratch: string[] = [];

	const rectRefs: (SVGRectElement | undefined)[] = new Array(MAX_SLOTS);
	const textRefs: (SVGTextElement | undefined)[] = new Array(MAX_SLOTS);
	const pathRefs: (SVGPathElement | undefined)[] = new Array(MAX_SLOTS);

	let rafId = 0;

	function tick(): void {
		const pool = trackPool;

		for (let s = 0; s < MAX_SLOTS; s++) {
			// Decay the local red-flash counter unconditionally so a reclaimed
			// slot can't carry stale visual state into its next occupant.
			if (violationDecay[s] > 0) violationDecay[s]--;

			const rect = rectRefs[s];
			const text = textRefs[s];
			const path = pathRefs[s];

			if (pool.presenceBitmap[s] === 0) {
				rect?.setAttribute('display', 'none');
				text?.setAttribute('display', 'none');
				path?.setAttribute('display', 'none');
				continue;
			}

			const b4 = s * 4;
			const u = pool.bboxOrig[b4 + 0];
			const v = pool.bboxOrig[b4 + 1];
			const w = pool.bboxOrig[b4 + 2];
			const h = pool.bboxOrig[b4 + 3];
			// Wire form is centroid; SVG <rect> wants top-left.
			const x = u - w * 0.5;
			const y = v - h * 0.5;

			const flags = pool.statusFlags[s];

			// Status-flag → stroke colour priority (highest first):
			//   hierarchy_violation  → red (held for 3 frames via decay)
			//   boundary_probation   → amber + dashed
			//   default              → cyan, solid
			// stationary_regime is orthogonal and modulates the fill only.
			if (flags & FLAG_HIERARCHY_VIOLATION) {
				violationDecay[s] = 3;
			}

			if (rect) {
				rect.setAttribute('x', String(x));
				rect.setAttribute('y', String(y));
				rect.setAttribute('width', String(w));
				rect.setAttribute('height', String(h));
				rect.setAttribute('display', 'inline');

				if (violationDecay[s] > 0) {
					rect.setAttribute('stroke', '#ff3333');
					rect.removeAttribute('stroke-dasharray');
				} else if (flags & FLAG_BOUNDARY) {
					rect.setAttribute('stroke', '#ffbb00');
					rect.setAttribute('stroke-dasharray', '6 4');
				} else {
					rect.setAttribute('stroke', '#00f0ff');
					rect.removeAttribute('stroke-dasharray');
				}

				if (flags & FLAG_STATIONARY) {
					rect.setAttribute('fill', 'rgba(0,240,255,0.06)');
				} else {
					rect.setAttribute('fill', 'none');
				}
			}

			if (text) {
				text.setAttribute('x', String(x));
				text.setAttribute('y', String(y - 8));
				text.setAttribute('display', 'inline');
				const labelStr = LABEL_STRINGS[pool.label[s]] ?? '';
				const pct = Math.round(pool.confidence[s] * 100);
				text.textContent = `${pool.idStrings[s]} ${labelStr} ${pct}%`;
			}

			if (path) {
				const fill = pool.trajFill[s];
				if (fill >= 2) {
					const ringBase = s * TRAJECTORY_RING;
					// trajHead points at the NEXT write slot; the oldest valid
					// sample is `fill` steps behind it (mod RING).
					const head = pool.trajHead[s];
					const start = (head - fill + TRAJECTORY_RING) % TRAJECTORY_RING;

					pathScratch.length = 0;
					let lastU = 0;
					let lastV = 0;
					let prevU = 0;
					let prevV = 0;
					for (let k = 0; k < fill; k++) {
						const idx = ringBase + ((start + k) % TRAJECTORY_RING);
						const tu = pool.trajU[idx];
						const tv = pool.trajV[idx];
						pathScratch.push(k === 0 ? `M ${tu} ${tv}` : `L ${tu} ${tv}`);
						if (k === fill - 2) {
							prevU = tu;
							prevV = tv;
						}
						if (k === fill - 1) {
							lastU = tu;
							lastV = tv;
						}
					}
					// 3-frame forward extrapolation (image-plane velocity × 3).
					const velU = lastU - prevU;
					const velV = lastV - prevV;
					pathScratch.push(`L ${lastU + velU * 3} ${lastV + velV * 3}`);
					path.setAttribute('d', pathScratch.join(' '));
					path.setAttribute('display', 'inline');
				} else {
					path.setAttribute('display', 'none');
				}
			}
		}

		rafId = requestAnimationFrame(tick);
	}

	onMount(() => {
		// Live-priority frame ingest: the C++ orchestrator's WebSocket egress
		// (ws://127.0.0.1:8765) feeds the canvas when running; otherwise the
		// mock source keeps the viewport populated. The mock callback checks
		// the live socket's state on every tick and silently skips its decode
		// while live data is flowing — no per-frame allocation, no coordinator,
		// just one cheap branch.
		mockFrameSource.start((buf) => {
			// Live-priority gating per wire-format version:
			//   v1 (track frames)        — the live C++ orchestrator already emits these,
			//                              so drop mock v1 when live is connected.
			//   v2 (actuator telemetry)  — live side won't emit these until Phase 5c
			//                              (actuator-telemetry-emit thread bootstrap),
			//                              so mock v2 always flows.
			//   v3 (bg-odometry)         — live side won't emit these until Phase 6+
			//                              (OpticalFlowSampler), so mock v3 always flows.
			// Cheap inspection: one DataView temporary per frame is well under GC
			// pressure even at 90+ Hz combined emission rate.
			const version = new Uint8Array(buf, 0, 1)[0];
			if (version === 1 && telemetrySocket.state === 'connected') return;
			decode_frame(buf);
		});
		telemetrySocket.connect();
		rafId = requestAnimationFrame(tick);
	});

	onDestroy(() => {
		telemetrySocket.disconnect();
		mockFrameSource.stop();
		cancelAnimationFrame(rafId);
	});
</script>

<div class="telemetry-shell">
	<header class="status-strip" data-version={trackPool.presenceVersion}>
		SRC: <span class:src-live={telemetrySocket.state === 'connected'}
			>{telemetrySocket.state === 'connected' ? 'LIVE' : 'MOCK'}</span
		> · SEQ: {trackPool.frameSeq} · ACT: {trackPool.activeCount} · NIS:
		{(trackPool.frameFlags & 0x01) ? 'DEGRADED' : 'OK'} · BND:
		{(trackPool.frameFlags & 0x02) ? 'PRESENT' : 'OK'} · WS:
		{telemetrySocket.framesReceived}/{telemetrySocket.reconnectAttempt}
		{#if telemetrySocket.lastError}
			<span class="status-error" title={telemetrySocket.lastError}>
				· ERR: {telemetrySocket.lastError.slice(0, 48)}{telemetrySocket.lastError.length > 48
					? '…'
					: ''}
			</span>
		{/if}
	</header>

	<svg
		class="telemetry-root"
		viewBox="0 0 1920 1080"
		preserveAspectRatio="xMidYMid meet"
		xmlns="http://www.w3.org/2000/svg"
	>
		<g class="layer-grid">
			{#each Array(16) as _, i (i)}
				<line
					x1={i * 120}
					y1="0"
					x2={i * 120}
					y2="1080"
					stroke="#1b2a3f"
					stroke-width="1"
				/>
			{/each}
			{#each Array(9) as _, i (i)}
				<line
					x1="0"
					y1={i * 120}
					x2="1920"
					y2={i * 120}
					stroke="#1b2a3f"
					stroke-width="1"
				/>
			{/each}
		</g>

		<g class="layer-trajectory">
			{#each Array(MAX_SLOTS) as _, i (i)}
				<path
					bind:this={pathRefs[i]}
					fill="none"
					stroke="rgba(0,240,255,0.35)"
					stroke-width="2"
					display="none"
				/>
			{/each}
		</g>

		<g class="layer-tracks">
			{#each Array(MAX_SLOTS) as _, i (i)}
				<rect
					bind:this={rectRefs[i]}
					fill="none"
					stroke="#00f0ff"
					stroke-width="2"
					display="none"
				/>
			{/each}
			{#each Array(MAX_SLOTS) as _, i (i)}
				<text
					bind:this={textRefs[i]}
					fill="#00f0ff"
					font-family="monospace"
					font-size="14"
					display="none"
				/>
			{/each}
		</g>
	</svg>
</div>

<style>
	.telemetry-shell {
		position: relative;
		width: 100%;
		height: 100%;
		display: flex;
		flex-direction: column;
		background: #020617;
		overflow: hidden;
	}

	.status-strip {
		background: #0f172a;
		color: #cbd5e1;
		font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
		font-size: 0.85rem;
		padding: 8px 16px;
		border-bottom: 1px solid #1e293b;
		letter-spacing: 0.02em;
		flex-shrink: 0;
	}

	.status-strip .src-live {
		color: #00f0ff;
		font-weight: 600;
	}

	.status-strip .status-error {
		color: #ff8a4c;
		margin-left: 6px;
	}

	.telemetry-root {
		flex: 1;
		width: 100%;
		height: 100%;
		display: block;
		user-select: none;
	}

	.layer-grid,
	.layer-trajectory,
	.layer-tracks {
		will-change: transform;
	}
</style>
