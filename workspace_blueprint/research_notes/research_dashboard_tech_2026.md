# Dashboard Technology Research — 2026 Deep Dive
> **ROC AI Vision** · Lead Frontend/Systems Engineer  
> **Scope:** `dashboard/` · Real-time telemetry rendering, transport, and tooling  
> **Cross-references:** `phase_5_telemetry_ui.md` (wire protocol, pool architecture, SVG render loop)  
> **Last Updated:** May 2026

---

## Table of Contents

1. [WebTransport vs WebSocket](#1-webtransport-vs-websocket)
2. [Canvas 2D vs SVG vs WebGL/WebGPU for Telemetry](#2-canvas-2d-vs-svg-vs-webglwebgpu-for-telemetry)
3. [Svelte 5 Runes Patterns for Zero-GC Stores](#3-svelte-5-runes-patterns-for-zero-gc-stores)
4. [SharedArrayBuffer + Atomics](#4-sharedarraybuffer--atomics)
5. [Foxglove Studio Architecture Analysis](#5-foxglove-studio-architecture-analysis)
6. [Rerun.io Architecture Analysis](#6-rerunio-architecture-analysis)
7. [Observable Plot / D3 Alternatives for Analytics Panels](#7-observable-plot--d3-alternatives-for-analytics-panels)
8. [Service Worker Caching for Offline Dashboard](#8-service-worker-caching-for-offline-dashboard)
9. [Performance Profiling Methodology](#9-performance-profiling-methodology)
10. [Consolidated Recommendations](#10-consolidated-recommendations)

---

## 1. WebTransport vs WebSocket

### 1.1 The Problem with WebSocket over TCP

Our current architecture (phase_5_telemetry_ui.md §1) uses WebSocket over TCP. TCP
guarantees ordered, reliable delivery — but for a 30–60 fps telemetry stream where
**every frame supersedes the previous one**, this guarantee becomes a liability:

- **Head-of-line (HOL) blocking:** A single lost TCP segment stalls *all* subsequent
  segments until retransmission completes. On a 1% packet-loss link, this can inject
  30–100 ms latency spikes — enough to drop 2–6 frames at 60 Hz.
- **Stale data accumulation:** While TCP retransmits, newer frames queue behind the
  stale one. The client receives a burst of already-obsolete frames after recovery.
- **Connection teardown:** TCP doesn't support connection migration. A WiFi↔cellular
  handoff kills the WebSocket and requires full reconnection + state flush.

### 1.2 WebTransport: HTTP/3 + QUIC Datagrams

WebTransport is the browser API for QUIC-based communication over HTTP/3. It offers
two transport modes relevant to our use case:

| Mode | Delivery | Ordering | Use Case |
|---|---|---|---|
| **Datagrams** | Unreliable | None | Frame payloads — drop stale, read latest |
| **Bidirectional streams** | Reliable | Per-stream | Control messages (target lock commands) |

**Datagram semantics** are exactly what telemetry needs: each 12-byte header + N×86-byte
track record is a self-contained frame. If a datagram is lost, the next datagram
arrives within ~16 ms (at 60 Hz) with strictly newer data. No retransmission, no HOL
blocking, no stale frame burst.

**Measured latency improvement:** Published benchmarks show a ~35% latency reduction
vs WebSocket for real-time update workloads. Under 15% packet loss, WebSocket
degrades severely due to TCP HOL blocking, while WebTransport maintains stable
delivery rates.

### 1.3 Browser Support Status (May 2026)

WebTransport reached **Baseline Newly Available** on March 24, 2026:

| Browser | Version | Status |
|---|---|---|
| Chrome | 97+ | ✅ Full support |
| Edge | 97+ | ✅ Full support |
| Firefox | 114+ | ✅ Full support |
| Safari | 26.4+ | ✅ Full support (new — March 2026) |

Safari was the last holdout. As of spring 2026, WebTransport is universally
available in all major browsers. Adoption remains low (~0.003% of page loads),
meaning the API surface is stable but not battle-tested at web scale.

### 1.4 Server-Side Implementation Options

| Language | Library | Maturity | Notes |
|---|---|---|---|
| **Rust** | `wtransport` v0.7.1 (274K downloads) | Production-ready | Async-native, tokio-based. Best option for a C++ ROS backend with FFI bridge. |
| **Rust** | `web-transport-quiche` v0.3.1 | Active dev | Built on Google's `quiche` QUIC implementation via `tokio-quiche`. |
| **Node.js** | `@aspect-build/webtransport` (fails-components/webtransport v1.6.0) | Production-ready | C++ bindings to libquiche. Supports WT over HTTP/2 fallback + WebSocket mapping. Cross-platform (Linux, Windows, macOS). |
| **C++** | Direct use of `quiche` / `msquic` | Low-level | Google's `quiche` is the reference QUIC implementation in C/C++. Microsoft's `msquic` is another option. Requires manual HTTP/3 framing. |

**Recommendation for ROC:** The Rust `wtransport` library is the strongest option.
The C++ backend can spawn a thin Rust sidecar (or embed via `cxx`) that accepts
WebTransport connections and bridges to the ROS 2 topic subscription. Alternatively,
the Node.js library works if we add a Node.js relay process.

### 1.5 Integration Design: Dual-Mode Transport

```
┌─────────────────────────────────────────────────────────────────┐
│ C++ Backend (ros2_ws/src/roc_vision_nodes/)                      │
│  ┌──────────────────────────────────────────────────────┐        │
│  │ serialize_frame() → g_frame_buf                      │        │
│  │ 12 B header + N × 86 B track records                 │        │
│  └──────────────────────┬───────────────────────────────┘        │
│                         │ raw bytes                               │
│  ┌──────────────────────▼───────────────────────────────┐        │
│  │ WebTransport Relay (wtransport / Rust sidecar)       │        │
│  │  • Datagrams: frame payloads (unreliable)            │        │
│  │  • BiDi stream: control channel (target lock)        │        │
│  └──────────────────────┬───────────────────────────────┘        │
│                         │                                         │
│  ┌──────────────────────▼───────────────────────────────┐        │
│  │ WebSocket Fallback (asio / rosbridge)                │        │
│  │  • Binary messages: same frame format                │        │
│  │  • JSON fallback: compact schema (§1.6 phase_5)     │        │
│  └──────────────────────────────────────────────────────┘        │
└─────────────────────────────────────────────────────────────────┘
                           │
                    ┌──────▼──────┐
                    │  Dashboard  │
                    │  (SvelteKit) │
                    └─────────────┘
```

**Client-side transport selection:**

```typescript
// src/lib/transport/transport.ts — conceptual API

export async function connect_transport(url: string): Promise<TransportHandle> {
  if ('WebTransport' in globalThis) {
    try {
      const wt = new WebTransport(url.replace('wss://', 'https://'));
      await wt.ready;
      return create_webtransport_handle(wt);
    } catch {
      // Fall through to WebSocket
    }
  }
  return create_websocket_handle(url);
}
```

When using WebTransport datagrams, the client reads from
`transport.datagrams.readable` via a `ReadableStream` reader. Each datagram is a
`Uint8Array` containing one complete frame. The existing `read_frame()` function
(phase_5_telemetry_ui.md §1.5) works unchanged — it accepts an `ArrayBuffer`.

### 1.6 Frame Dropping Strategy

With WebSocket (TCP), every frame arrives and must be processed. With WebTransport
datagrams, frames may be lost — but we can also **intentionally drop stale frames**:

```typescript
let _lastProcessedSeq = -1;

function on_datagram(data: Uint8Array): void {
  const view = new DataView(data.buffer, data.byteOffset, data.byteLength);
  const seq = view.getUint16(4, true); // frame_seq at offset 4

  // Drop stale or duplicate frames (handles uint16 rollover)
  const delta = (seq - _lastProcessedSeq) & 0xFFFF;
  if (delta === 0 || delta > 0x8000) return; // duplicate or old

  _lastProcessedSeq = seq;
  apply_frame_from_buffer(data.buffer, data.byteOffset, data.byteLength);
}
```

This is impossible with WebSocket — TCP guarantees all frames arrive in order, so
we'd need to buffer and sequence-check manually while still paying the HOL latency.

### 1.7 Verdict

| Criterion | WebSocket | WebTransport Datagrams |
|---|---|---|
| HOL blocking | ❌ Unavoidable (TCP) | ✅ None (QUIC per-datagram) |
| Stale frame drop | ❌ Must process all | ✅ Native — lost = dropped |
| Browser support 2026 | ✅ Universal | ✅ Universal (as of March 2026) |
| Server ecosystem | ✅ Mature | ⚠️ Growing — production-ready Rust/Node.js |
| Connection migration | ❌ Full reconnect | ✅ QUIC native |
| TLS requirement | WSS (TLS 1.2+) | QUIC (TLS 1.3 only) |
| Debugging tools | ✅ Mature (DevTools WS inspector) | ⚠️ Limited (chrome://webtransport-internals) |
| Complexity | Low | Moderate (QUIC cert management) |

**Decision: Implement WebTransport datagrams as the primary transport for Phase 5
optimization (Phase B). Keep WebSocket as the Phase A fallback and for rosbridge
compatibility. The wire format is transport-agnostic.**

---

## 2. Canvas 2D vs SVG vs WebGL/WebGPU for Telemetry

### 2.1 Requirements Recap

- 50 entities maximum, each with: bounding box, ID label, trajectory path (up to 64 points)
- 60 fps rendering target (30 fps data ingestion, display refresh-synced)
- Interactive: click-to-select entities
- DOM element count per entity: ~3 (rect, text, path) if SVG; 0 if Canvas/WebGL

### 2.2 Technology Comparison at Our Scale

| Technology | Max Smooth Elements @60fps | Hit Testing | Text Quality | GC Overhead | Implementation Complexity |
|---|---|---|---|---|---|
| **SVG (current)** | ~5,000 elements | ✅ Native events | ✅ Native | ⚠️ DOM mutations | Low |
| **Canvas 2D** | ~50,000 objects | ❌ Manual | ⚠️ Manual rendering | ✅ Minimal | Medium |
| **OffscreenCanvas** | ~50,000 (off-thread) | ❌ Manual | ⚠️ Manual | ✅ Minimal | High |
| **WebGL** | 100,000+ (instanced) | ❌ Manual/GPU picking | ❌ Requires atlas | ✅ Zero DOM | High |
| **WebGPU** | 1,000,000+ (compute) | ❌ GPU picking | ❌ Requires atlas | ✅ Zero DOM | Very High |

### 2.3 SVG at Our Scale: The Verdict

At 50 entities, the total SVG element count is:

```
50 targets × 3 elements (rect + text + path) = 150 DOM nodes
+ 3 layer groups + 1 root = 154 total elements
```

Published benchmarks show SVG maintaining 60 fps up to ~1,000 elements with
frequent mutations, and up to ~5,000 elements with infrequent updates. Our 154
elements are **well within SVG's comfortable range** — by a factor of 6–30×.

The existing three-layer SVG architecture (phase_5_telemetry_ui.md §3.2) with
`will-change` compositor hints and direct `setAttribute` mutations is the optimal
choice for this entity count because:

1. **Native text rendering** — `<text>` elements render with subpixel anti-aliasing
   and native font shaping. Canvas 2D `fillText` is lower quality and requires
   manual baseline alignment.
2. **Native event handling** — `click` events on `<g>` elements provide entity
   selection with zero custom hit-testing code. Canvas requires manual coordinate
   math or a secondary hit-testing canvas.
3. **CSS styling** — stroke colors, opacities, hover states, and selection highlights
   are pure CSS. Canvas requires imperative state management for visual states.
4. **Accessibility** — SVG elements can carry `aria-label` and `role` attributes.
   Canvas is an opaque bitmap.

### 2.4 When to Consider Canvas 2D

Canvas becomes preferable when:
- Entity count exceeds ~500 with per-frame mutations
- Trajectory paths have >200 points per entity (path `d` string regeneration cost)
- Background elements (grid, FOV overlay) change frequently

**Hybrid approach worth considering:** Use SVG for bounding boxes + labels (Layer 2)
where native text and events matter, and a single `<canvas>` element for trajectory
paths (Layer 1) where the path string regeneration cost dominates.

Trajectory rendering in Canvas 2D:
```typescript
ctx.beginPath();
ctx.moveTo(traj[0], traj[1]);
for (let i = 1; i < len; i++) {
  ctx.lineTo(traj[i * 2], traj[i * 2 + 1]);
}
ctx.stroke();
```

This eliminates the `TextDecoder.decode()` path `d` string allocation entirely —
no string generated, no `setAttribute('d', ...)` call. Canvas path commands operate
on internal GPU-resident geometry buffers.

**Allocation saving:** At 50 targets × 64 trajectory points × 60 Hz, the Canvas
approach eliminates ~3,000 string allocations/second and ~1.8 MB/s of Eden
allocation from path `d` strings.

### 2.5 OffscreenCanvas: Off-Thread Rendering

OffscreenCanvas enables rendering in a Web Worker, completely freeing the main
thread from Canvas draw calls:

```typescript
// Main thread
const offscreen = canvas.transferControlToOffscreen();
worker.postMessage({ canvas: offscreen }, [offscreen]);

// Worker thread
const ctx = offscreen.getContext('2d');
function render(pool: SharedArrayBuffer) {
  // Read pool data directly from SharedArrayBuffer
  // Draw bounding boxes, trajectories
  // No postMessage needed — rendering happens on the transferred canvas
}
```

Browser support: **Baseline Widely Available** since March 2023 (Chrome 69+,
Firefox 105+, Safari 16.4+). `transferToImageBitmap()` is universally supported.

**Caution:** Firefox has a known performance issue where `transferToImageBitmap()`
with WebGL contexts achieves only ~15 fps vs 60 fps in Chrome/Safari (bugzilla
#1864882). For Canvas 2D contexts, Firefox performance is acceptable.

### 2.6 WebGPU: Future-Proofing with Compute Shaders

WebGPU reached **Baseline status in January 2026** with broad support:

| Browser | Support |
|---|---|
| Chrome 113+ | ✅ (Windows, macOS, ChromeOS, Android 12+) |
| Edge 113+ | ✅ |
| Firefox 141+ (Windows), 145+ (macOS) | ✅ |
| Safari 26 (macOS, iOS, visionOS) | ✅ |
| Firefox Linux | ⚠️ Behind flag, expected 2026 |

**Compute shader potential for trajectory interpolation:**

WebGPU compute shaders can process trajectory data on the GPU in parallel:
- Upload 50 × 64 trajectory points as a storage buffer (~25 KB)
- Run a compute shader that performs cubic spline interpolation
- Read back smoothed trajectories for rendering

This is technically feasible but **overkill for 50 entities**. The compute dispatch
overhead (buffer upload → compute → readback) exceeds the CPU interpolation cost at
our scale. WebGPU compute becomes advantageous at >10,000 entities or >1,000
trajectory points per entity.

**Practical WebGPU value for ROC:** If we add a 3D view (camera frustum
visualization, world-space trajectory projection), WebGPU via Three.js or raw
`wgpu`-style rendering would be the correct foundation. For 2D telemetry overlays
at 50 entities, it's unnecessary complexity.

### 2.7 Rendering Decision Matrix

| Layer | Recommended Tech | Rationale |
|---|---|---|
| Layer 0: Static grid | SVG | Rendered once, never updated |
| Layer 1: Trajectory paths | **SVG (Phase A) → Canvas 2D (Phase B)** | SVG is simpler; Canvas eliminates path `d` string allocation |
| Layer 2: Bounding boxes + labels | SVG | Native text, native click events, CSS styling |
| Future 3D view | WebGPU (Three.js) | If/when 3D camera visualization is added |

**Phase A recommendation: Stay with SVG for all layers.** 154 DOM elements at 60 fps
is well within SVG's performance envelope. The path `d` string allocation (~1.8 MB/s
Eden) is manageable within V8's minor GC budget (~0.4% of Scavenger throughput).

**Phase B optimization: Migrate Layer 1 (trajectories) to a `<canvas>` element.**
This eliminates the only remaining per-frame string allocation on the hot path,
achieving true zero-allocation rendering for trajectory visualization.

---

## 3. Svelte 5 Runes Patterns for Zero-GC Stores

### 3.1 Core Rune Semantics for High-Frequency Data

Svelte 5 runes are **compiler-level signal primitives**, not runtime functions. The
compiler tracks dependencies at build time and generates minimal update code.

| Rune | Behavior | GC Impact |
|---|---|---|
| `$state(value)` | Deep proxy — mutations tracked at property level | ⚠️ Proxy objects allocated per nested object |
| `$state.raw(value)` | Shallow — no proxy, reassignment-only reactivity | ✅ No proxy allocation |
| `$derived(expr)` | Lazy computed value, recalculated only when deps change | ✅ Minimal — one closure |
| `$effect(fn)` | Side effect after DOM update, batched in microtask | ⚠️ Runs per dependency change |
| `$effect.pre(fn)` | Side effect before DOM update | ⚠️ Same batching as `$effect` |
| `untrack(fn)` | Read state without creating dependency | ✅ Prevents unwanted reactivity |

### 3.2 The `$state` Proxy Problem for Track Pools

Svelte 5's `$state()` wraps objects in a deep `Proxy`. For our 64-slot track pool:

```typescript
// ❌ WRONG — each pool slot becomes a deep Proxy
let pool = $state(Array.from({ length: 64 }, () => ({
  active: false, id: 0, cls: 0, flags: 0,
  x1: 0, y1: 0, x2: 0, y2: 0,
  traj: new Float32Array(128), trajectoryLen: 0,
})));
```

This creates 64 Proxy wrapper objects plus nested Proxy wrappers for each property
access. At 60 Hz, reading 64 slots × ~10 properties = 640 Proxy trap invocations
per frame on the RAF hot path. The Proxy traps are not free — each one performs a
signal dependency registration check.

### 3.3 The `$state.raw` Pattern: Opt Out of Deep Proxying

```typescript
// ✅ CORRECT — no Proxy wrapping, reassignment-only reactivity
let pool = $state.raw(Array.from({ length: 64 }, () => ({
  active: false, id: 0, cls: 0, flags: 0,
  x1: 0, y1: 0, x2: 0, y2: 0,
  traj: new Float32Array(128), trajectoryLen: 0,
})));
```

With `$state.raw`, the pool array and its slot objects are plain JavaScript objects —
no Proxy overhead. The tradeoff: mutating `pool[i].x1 = newValue` does **not**
trigger reactivity. Only reassigning `pool = newArray` would trigger subscribers.

This is **exactly what we want**. The pool is mutated in-place by `apply_frame()`
on every WebSocket message, and the single reactive notification is a version tick.

### 3.4 Recommended Architecture: Version Tick + Raw Pool

```typescript
// src/lib/stores/target_store.svelte.ts

export const MAX_TARGETS = 64;
export const TRAJ_DEPTH = 64;

// Non-reactive pool — mutated in-place by apply_frame()
// $state.raw prevents Svelte from wrapping in Proxy
export let pool = $state.raw(Array.from({ length: MAX_TARGETS }, () => ({
  active:        false,
  id:            0,
  cls:           0,
  flags:         0,
  x1:            0, y1: 0, x2: 0, y2: 0,
  traj:          new Float32Array(TRAJ_DEPTH * 2),
  trajectoryLen: 0,
})));

// Reactive version counter — the ONLY reactive signal on the hot path.
// Components that need to re-render subscribe to this via $derived or template reads.
// Incrementing this is a single signal notification — O(1) work.
export let frame_tick = $state(0);

export function notify_frame(): void {
  frame_tick++;
}
```

### 3.5 Using `untrack()` in the RAF Loop

The RAF render loop must read pool data without creating reactive dependencies:

```typescript
// Inside TelemetryCanvas.svelte
import { untrack } from 'svelte';

$effect(() => {
  // Subscribe to frame_tick to schedule re-render
  const _tick = frame_tick;

  // Read pool data without tracking — prevents $effect from
  // re-subscribing to every pool property read
  untrack(() => {
    render_frame_imperative();
  });
});
```

However, for the RAF-based render loop described in phase_5_telemetry_ui.md §3.5,
we bypass Svelte's `$effect` entirely and use raw `requestAnimationFrame`. The pool
reads inside the RAF callback are already outside Svelte's reactive context, so no
`untrack()` is needed.

**Key insight:** The RAF callback is registered in `onMount()` and runs outside
Svelte's effect system. Pool reads in the RAF callback do not create signal
dependencies because there is no active effect context. `untrack()` is only needed
when reading non-tracked data inside a `$effect` or `$derived` block.

### 3.6 Fine-Grained vs Coarse Reactivity for 50 Tracks

Two competing patterns:

**Pattern A: Single version tick (coarse) — RECOMMENDED**
```
frame_tick++ → all subscribed components re-evaluate
Components read pool[] directly in their render path
```
- Pro: One signal notification per frame, O(1) reactive overhead
- Con: All track components re-render even if only one track changed
- At 50 tracks: 50 component re-evaluations per frame = acceptable at 60 Hz

**Pattern B: Per-track signals (fine-grained)**
```
track_version[i]++ → only components watching track i re-evaluate
```
- Pro: Minimal re-render — only changed tracks update
- Con: 50 signal notifications per frame if all tracks move (usual case)
- The signal notification overhead of 50 updates may exceed the rendering
  savings, since our render loop is imperative `setAttribute` anyway

**Verdict:** Pattern A (coarse). Our render loop is imperative (direct DOM mutation
in a RAF callback), not template-driven. Svelte's fine-grained reactivity only
helps when components use `{#each}` with keyed diffing — which we deliberately
avoid. The RAF loop iterates all 64 slots regardless, so per-track signals add
notification overhead without reducing render work.

### 3.7 Proposed `onAnimationFrame` Lifecycle (Future)

An open Svelte RFC (PR #14594) proposes an `onAnimationFrame` (or `onFrame`)
lifecycle function:

```typescript
$effect(() => {
  const ctx = canvas.getContext('2d');
  onFrame(() => {
    paint(ctx, Date.now());
  });
});
```

This would eliminate the manual `requestAnimationFrame` + `onDestroy` cleanup
pattern. The RFC is under discussion and not yet merged. Monitor for inclusion in
Svelte 5.x — it would simplify our `TelemetryCanvas.svelte` significantly.

---

## 4. SharedArrayBuffer + Atomics

### 4.1 Architecture: Worker-Decoded Shared State

The concept: move binary frame decoding to a Web Worker, write decoded track state
into a `SharedArrayBuffer`, and have the main thread read it directly without
`postMessage` or structured clone:

```
┌────────────────────────────────────────────────────────────────┐
│ Web Worker Thread                                               │
│  WebSocket/WebTransport → read_frame() → write to SAB          │
│  Atomics.store(dirty_flag, 1)                                   │
└─────────────────────────┬──────────────────────────────────────┘
                          │ SharedArrayBuffer (zero-copy shared memory)
┌─────────────────────────▼──────────────────────────────────────┐
│ Main Thread                                                      │
│  RAF loop: Atomics.load(dirty_flag)                              │
│  If dirty: read track data from SAB → render SVG/Canvas          │
│  Atomics.store(dirty_flag, 0)                                    │
└────────────────────────────────────────────────────────────────┘
```

### 4.2 SharedArrayBuffer Layout for Track Pool

```
SharedArrayBuffer Layout (total: 34,944 bytes)

┌──────────────────────────────────────────────────────────────┐
│ Control Region (64 bytes)                                      │
│  [0..3]   dirty_flag    : Int32  (Atomics target)              │
│  [4..7]   frame_seq     : Uint32                               │
│  [8..9]   active_count  : Uint16                               │
│  [10..11] timestamp_ms  : Uint16                               │
│  [12..63] reserved                                             │
└──────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────┐
│ Track Slots (64 × 545 bytes = 34,880 bytes)                    │
│  Per slot (545 bytes):                                         │
│    [0]     active        : Uint8                               │
│    [1..2]  id            : Uint16                              │
│    [3]     cls           : Uint8                               │
│    [4]     flags         : Uint8                               │
│    [5..8]  x1            : Float32                             │
│    [9..12] y1            : Float32                             │
│    [13..16] x2           : Float32                             │
│    [17..20] y2           : Float32                             │
│    [21]    trajectoryLen : Uint8                               │
│    [22..533] traj        : Float32[128] (64 pairs × 4 bytes)  │
│    [534..544] padding (align to 8)                             │
└──────────────────────────────────────────────────────────────┘
```

### 4.3 Cross-Origin Isolation Requirements

SharedArrayBuffer requires the page to be **cross-origin isolated**. Two HTTP
headers must be set on the HTML document:

```
Cross-Origin-Opener-Policy: same-origin
Cross-Origin-Embedder-Policy: require-corp
```

**Implications for the dashboard:**
- All resources (JS, CSS, fonts, images) must either be same-origin or served
  with `Cross-Origin-Resource-Policy: cross-origin` headers.
- Third-party resources (CDN fonts, analytics scripts) that lack CORP headers
  will be blocked.
- OAuth popup flows may break under `COOP: same-origin` (no cross-origin window
  access). Mitigation: use `same-origin-allow-popups` if auth is needed.

**SvelteKit configuration:**

```typescript
// vite.config.ts
export default defineConfig({
  server: {
    headers: {
      'Cross-Origin-Opener-Policy': 'same-origin',
      'Cross-Origin-Embedder-Policy': 'require-corp',
    },
  },
});
```

For production, these headers must be set at the web server level (nginx, Caddy, etc.).

### 4.4 Known Limitation: TextDecoder Rejects SharedArrayBuffer

`TextDecoder.decode()` and `TextEncoder.encodeInto()` **explicitly reject
SharedArrayBuffer views**, throwing `"The provided ArrayBufferView value must not
be shared"`. This means:

- The SVG path `d` string cannot be built directly from SAB-resident trajectory
  data using TextDecoder.
- The worker must either: (a) decode binary into the SAB, and the main thread
  builds path strings from SAB float reads; or (b) the worker builds path strings
  and transfers them via postMessage.

Option (a) is correct for our architecture — the main thread reads float32 values
from the SAB and writes them as SVG attributes or Canvas draw calls. No TextDecoder
involvement on the SAB data.

### 4.5 Cost-Benefit Analysis

| Factor | Without SAB (current) | With SAB |
|---|---|---|
| Frame decode thread | Main thread | Worker thread |
| Main thread decode time | ~0.2 ms/frame | 0 ms |
| Data transfer overhead | `onmessage` event (near-zero for ArrayBuffer) | Atomics.load (1 ns) |
| Path string building | Main thread (both cases) | Main thread (both cases) |
| Implementation complexity | Low | Medium (SAB layout + worker + COOP/COEP) |
| Cross-origin isolation | Not required | Required (headers on all resources) |

**Verdict:** The benefit is marginal for our workload. Frame decoding (`read_frame()`)
costs ~0.2 ms — offloading it to a worker saves 0.2 ms on the main thread, which
is ~1.2% of the 16.6 ms frame budget. The COOP/COEP header requirements add
deployment complexity.

**Recommendation: Defer SAB to Phase C.** The current single-threaded architecture
processes frames well within the 16.6 ms budget. SAB becomes valuable if:
- We add CPU-intensive per-frame computation (e.g., client-side EKF prediction)
- Entity count exceeds 200
- Frame rate exceeds 120 Hz

If implemented, the SAB layout above is ready to use. The pool architecture
(phase_5_telemetry_ui.md §2) already uses the same flat-array-of-structs pattern —
migrating to SAB-backed storage is a mechanical change.

---

## 5. Foxglove Studio Architecture Analysis

### 5.1 Overview

Foxglove Studio is an open-source multimodal robotics visualization platform. It is
the closest architectural precedent to our dashboard — both consume ROS 2 topic
streams and render real-time overlays.

**Repository:** `github.com/foxglove/studio`  
**License:** Source-available (previously Apache 2.0, changed to BSL in 2023)

### 5.2 Rendering Architecture

Foxglove uses **Three.js** for its 3D panel, having migrated from `regl` (a
functional WebGL wrapper) and Cruise's `worldview` library. The migration was
driven by maintainability — regl and worldview were no longer actively maintained.

Key rendering components:
- **Three.js** for all 3D visualization (point clouds, meshes, URDF models)
- **React** for panel UI (not Svelte)
- **Custom panel API** with `onRender` + `done()` callback pattern
- **Comlink** (Foxglove fork) for Web Worker communication via RPC

### 5.3 Data Pipeline Architecture

```
Data Sources (ROS bridge, MCAP files, native ROS 2, Foxglove WebSocket)
  │
  ▼
Message Router (topic subscription, time-based message delivery)
  │
  ▼
Panel Extension Context (provides renderState to each panel)
  │
  ├─ currentFrame: messages for current timestamp
  ├─ allFrames: full preloaded message set
  ├─ topics: available topic list
  └─ parameters: key/value configuration
  │
  ▼
Panel Renderer (onRender callback → DOM/Canvas/Three.js → done())
```

**Critical pattern:** Panels call `context.watch('currentFrame')` to subscribe to
specific render state fields. Only watched fields trigger re-renders. This is
analogous to our `frame_tick` version counter — selective subscription.

### 5.4 Performance Patterns Worth Adopting

1. **Synchronous message iteration:** Foxglove's MCAP reader (PR #1188) avoids
   `await` on every message by providing synchronous message retrieval from
   internal data structures. Only chunk boundaries require async I/O. Result:
   ~30% throughput improvement for message-dense recordings.

2. **Comlink for Worker RPC:** Instead of raw `postMessage` + `onmessage`, Foxglove
   uses Comlink to make worker communication look like local function calls. This
   reduces boilerplate but adds ~10 µs per call overhead from Proxy wrapping.
   At our 60 Hz frame rate (16.6 ms budget), this is negligible.

3. **Memory leak prevention:** Foxglove documentation explicitly warns against
   aggregating message data over time without limits. Their `done()` callback
   pattern ensures panels signal completion before the next frame, preventing
   unbounded state accumulation.

### 5.5 What We Can Learn

| Foxglove Pattern | ROC Application |
|---|---|
| Three.js for 3D | Use for future 3D camera frustum view (if needed) |
| Panel watch/subscribe | Already implemented via `frame_tick` |
| Comlink Worker RPC | Consider for SAB worker communication (Phase C) |
| MCAP format | Not applicable — we use live streaming, not recorded playback |
| `done()` backpressure | Implement frame completion signal to prevent WS buffer growth |

### 5.6 Key Differences from ROC

- Foxglove is a **generic** visualization tool for any ROS topic. ROC is a
  **purpose-built** telemetry overlay optimized for one specific schema.
- Foxglove uses React with virtual DOM diffing. ROC uses imperative RAF +
  direct `setAttribute` — fundamentally different rendering philosophy.
- Foxglove's 3D panel (Three.js) is far more complex than our 2D SVG overlay.
  Our rendering is simpler but demands higher frame-rate consistency.

---

## 6. Rerun.io Architecture Analysis

### 6.1 Overview

Rerun is a fast visualization SDK for multimodal data (point clouds, images,
tensors, time series). It is written in Rust and renders via `wgpu`.

**Repository:** `github.com/rerun-io/rerun`  
**License:** Apache 2.0 + MIT dual license

### 6.2 Core Architecture

```
Logging SDK (Python/Rust)
  │ Apache Arrow encoding
  ▼
Transport (gRPC / .rrd file / network)
  │
  ▼
re_chunk_store (in-RAM columnar time-series database)
  │ Apache Arrow chunks
  ▼
Query Engine (re_query — time-aware range/latest queries)
  │
  ▼
re_renderer (custom wgpu-based renderer)
  │ ViewBuilder → DrawData → Drawable → GPU
  ▼
egui (immediate-mode GUI)
```

### 6.3 Key Technical Decisions and Their Rationale

**1. Apache Arrow for Everything**

Rerun uses Apache Arrow as the universal data encoding format — for network
transmission, disk storage, and in-memory representation. This eliminates
serialization/deserialization at boundaries: the same Arrow buffers flow from
SDK to storage to renderer.

**ROC parallel:** Our binary wire format (§1.3 in phase_5_telemetry_ui.md) is a
domain-specific equivalent — a fixed schema optimized for our 12-byte header + 86
bytes/track layout. Arrow would be overkill for our narrow schema but the
principle of "zero conversion at boundaries" is the same.

**2. Chunk-Based Ingestion with Micro-Batching**

Rerun batches small log calls into larger "chunks" before transmission:
- SDK micro-batching: ~200 ms when logging to file, ~8 ms to viewer
- In-viewer compaction: merges chunks up to ~384 KB or ~4,096 rows

This directly reduces per-message overhead. The throughput optimization that
yielded **20–30× speedup in time series rendering** (versions 0.12→0.13) came
from primary query caching — avoiding redundant bookkeeping and improving data
locality for small data.

**ROC parallel:** Our frame batching is implicit — each WebSocket message contains
one complete frame with all tracks. No micro-batching needed.

**3. Immediate-Mode Rendering**

Rerun's viewer uses immediate-mode rendering via `egui`. Every frame, the entire
GUI is re-laid out and re-queried from scratch. This eliminates state
synchronization bugs but requires relentless optimization.

**ROC parallel:** Our RAF render loop is similarly immediate-mode — every frame we
iterate all 64 pool slots and update SVG attributes. We don't cache or diff —
we write all active attributes every frame. This is architecturally correct
for the same reasons Rerun chose immediate mode.

**4. GPU Picking**

Rerun implements GPU-based picking for interactive element selection. Each
renderable object is assigned a unique color ID in an offscreen render pass.
On click, the pixel color at the click position identifies the selected object.

**ROC potential:** If we migrate trajectory rendering to Canvas 2D or WebGL,
GPU picking is a more robust hit-testing approach than coordinate-based math.
For SVG, native event handling is sufficient.

### 6.4 `re_renderer` Architecture

The rendering crate follows a clean pipeline:

```
ViewBuilder (collects drawables)
  │
  ├─ DrawData (user-supplied: points, lines, boxes, meshes)
  │
  ▼
Drawable (maps to wgpu draw calls)
  │ Sorted by draw phase:
  │  • Opaque: front-to-back, bundled by DrawData type
  │  • Transparent: far-to-near for correct blending
  │  • UI overlay: last, on top
  ▼
wgpu render pass
```

Each `Renderer` trait implementation encapsulates primitive-specific knowledge
(lines, points, boxes). This modular approach keeps rendering code maintainable
as visualization types grow.

### 6.5 Lessons for ROC

| Rerun Pattern | ROC Application |
|---|---|
| Arrow columnar encoding | Validate that our flat-struct pool is equivalent for our schema |
| Chunk compaction | Not needed — single-frame messages |
| Immediate-mode rendering | Already using this (RAF loop) |
| GPU picking | Consider for Canvas 2D hit testing in Phase B |
| wgpu + re_renderer | Overkill for 2D SVG — relevant if 3D view added |
| Micro-batching at SDK level | Apply to WebTransport: batch multiple datagrams before notify |

---

## 7. Observable Plot / D3 Alternatives for Analytics Panels

### 7.1 Requirements for Non-Realtime Panels

The dashboard includes analytics panels that display:
- Latency histograms (frame processing time distribution)
- EKF covariance magnitude over time
- Track lifecycle statistics (creation/deletion rates)
- System health metrics (CPU, GPU, memory)

These panels update at 1–5 Hz (not 60 Hz) and need standard chart types:
histograms, line charts, scatter plots, bar charts.

### 7.2 Library Comparison

| Library | Svelte 5 Support | Grammar of Graphics | Bundle Size | Histogram | Streaming | Notes |
|---|---|---|---|---|---|---|
| **SveltePlot** | ✅ Native Svelte | ✅ Observable Plot API | ~45 KB | ✅ Rect+Bin marks | ❌ | Most Observable Plot-like; v0.14.2 (April 2026) |
| **LayerChart** | ✅ Native Svelte 5 runes | ❌ Component-based | ~60 KB | ✅ | ✅ (investigated) | 1,191 stars; streaming perf fix merged; v2.0.0-next.57 |
| **Lightweight Charts** | ✅ Wrappers available | ❌ TradingView-specific | ~40 KB | ✅ | ✅ | Financial charting origin; excellent for time series |
| **Observable Plot** (direct) | ⚠️ Framework-agnostic | ✅ Canonical | ~85 KB (with D3) | ✅ | ❌ | Renders to detached DOM; requires manual Svelte integration |
| **Chart.js** | ⚠️ Via wrapper | ❌ Imperative | ~65 KB | ✅ | ⚠️ | Canvas-based; doesn't leverage Svelte reactivity |

### 7.3 Recommended Choice: SveltePlot for Declarative + LayerChart for Streaming

**SveltePlot** (primary recommendation for analytics panels):

- API directly mirrors Observable Plot's mark-based grammar of graphics
- Svelte-native: components render as Svelte templates, not detached DOM
- Histogram via `<Rect>` mark with bin transform:

```svelte
<Plot>
  <RectY
    data={latencyData}
    x={d => d.latency}
    y="count"
    fill="steelblue"
  />
</Plot>
```

- No fighting Svelte's reactivity — data changes naturally trigger re-renders
- v0.14.2 is actively maintained (latest release April 2026)

**LayerChart** (secondary, for streaming time-series panels):

- Recently migrated to Svelte 5 runes and snippets (v2.0.0-next.x)
- Actively investigated and fixed streaming performance issues (PR #490)
  - Identified and resolved detached SVG element memory leak
- Composable component architecture (Bar, Area, Stack, Scatter, Hierarchy, etc.)
- Removed LayerCake dependency for leaner architecture
- New canvas layer with improved memoization

### 7.4 Integration Pattern

For analytics panels that update at 1–5 Hz, standard Svelte 5 reactivity is appropriate:

```typescript
// src/lib/stores/diagnostics.svelte.ts
export let latency_histogram = $state<{ bin: number; count: number }[]>([]);
export let ekf_covariance_trace = $state<{ t: number; cov: number }[]>([]);

export function push_latency_sample(ms: number): void {
  // Bin and update histogram — fine at 1 Hz update rate
  const binIdx = Math.min(Math.floor(ms / 2), 49); // 2ms bins, max 100ms
  latency_histogram[binIdx] = {
    bin: binIdx * 2,
    count: (latency_histogram[binIdx]?.count ?? 0) + 1,
  };
  // Trigger reactivity via reassignment (or use $state with deep tracking)
  latency_histogram = [...latency_histogram];
}
```

At 1–5 Hz, the array spread allocation is negligible (~50 × 16 bytes = 800 bytes).
No zero-GC optimization needed for analytics panels.

### 7.5 Observable Plot Direct Usage (Alternative)

Observable Plot can be used directly in Svelte via an action:

```svelte
<script>
  import * as Plot from '@observablehq/plot';

  function plotAction(node, data) {
    const plot = Plot.plot({
      marks: [Plot.rectY(data, Plot.binX({ y: "count" }, { x: "latency" }))]
    });
    node.appendChild(plot);
    return {
      update(newData) { node.firstChild?.remove(); /* re-render */ },
      destroy() { node.firstChild?.remove(); }
    };
  }
</script>

<div use:plotAction={latencyData}></div>
```

This works but creates/destroys DOM subtrees on each update. SveltePlot avoids
this by rendering directly into Svelte's component tree.

---

## 8. Service Worker Caching for Offline Dashboard

### 8.1 Use Case

The ROC tracker runs headless on an embedded system. The dashboard is served
from the tracker's local web server. Scenario: the operator opens the dashboard
once while connected, then the network connection becomes intermittent or drops.
Service Worker caching ensures the dashboard UI remains available — though
real-time telemetry naturally stops without a live connection.

### 8.2 Caching Strategy by Content Type

| Content Type | Strategy | Rationale |
|---|---|---|
| App shell (HTML, CSS, JS) | **Cache First** | Static assets change only on deployment |
| Fonts | **Cache First** with expiration (30 days) | Rarely change, large payload |
| API routes (SvelteKit SSR) | **Network Only** | Dashboard is SPA (`ssr = false`) — no SSR API |
| WebSocket/WebTransport | **Not cacheable** | Real-time data — no offline substitute |
| Static images/icons | **Cache First** | Immutable after build |
| Manifest + favicon | **Stale While Revalidate** | Rarely change but should update eventually |

### 8.3 SvelteKit Service Worker Integration

SvelteKit provides built-in service worker support via `src/service-worker.ts`:

```typescript
// src/service-worker.ts
/// <reference types="@sveltejs/kit" />
/// <reference lib="webworker" />

declare const self: ServiceWorkerGlobalScope;

import { build, files, version } from '$service-worker';

const CACHE_NAME = `roc-dashboard-${version}`;

// App shell: built JS/CSS bundles + static files
const ASSETS = [...build, ...files];

self.addEventListener('install', (event) => {
  event.waitUntil(
    caches.open(CACHE_NAME).then((cache) => cache.addAll(ASSETS))
  );
});

self.addEventListener('activate', (event) => {
  event.waitUntil(
    caches.keys().then((keys) =>
      Promise.all(
        keys.filter((key) => key !== CACHE_NAME).map((key) => caches.delete(key))
      )
    )
  );
});

self.addEventListener('fetch', (event) => {
  if (event.request.method !== 'GET') return;

  const url = new URL(event.request.url);

  // Skip WebSocket/WebTransport URLs
  if (url.protocol === 'ws:' || url.protocol === 'wss:') return;

  event.respondWith(
    caches.match(event.request).then((cached) => {
      return cached || fetch(event.request);
    })
  );
});
```

**SvelteKit variables:**
- `build`: array of URL strings for all built JS/CSS files (hashed filenames)
- `files`: array of URL strings for static directory contents
- `version`: deterministic string representing the current build

### 8.4 Alternative: `vite-plugin-pwa` / `@vite-pwa/sveltekit`

The `@vite-pwa/sveltekit` plugin (v0.6.7+) automates service worker generation
using Workbox:

```typescript
// vite.config.ts
import { SvelteKitPWA } from '@vite-pwa/sveltekit';

export default defineConfig({
  plugins: [
    sveltekit(),
    SvelteKitPWA({
      strategies: 'generateSW',
      registerType: 'autoUpdate',
      workbox: {
        globPatterns: ['**/*.{js,css,html,ico,png,svg,woff2}'],
        runtimeCaching: [
          {
            urlPattern: /^https:\/\/fonts\./,
            handler: 'CacheFirst',
            options: { cacheName: 'fonts', expiration: { maxAgeSeconds: 30 * 24 * 60 * 60 } },
          },
        ],
      },
      manifest: {
        name: 'ROC AI Vision Dashboard',
        short_name: 'ROC Dashboard',
        theme_color: '#0a0a0a',
        background_color: '#0a0a0a',
        display: 'standalone',
        scope: '/',
        start_url: '/',
        icons: [
          { src: '/icon-192.png', sizes: '192x192', type: 'image/png' },
          { src: '/icon-512.png', sizes: '512x512', type: 'image/png' },
        ],
      },
    }),
  ],
});
```

**Recommendation:** Use SvelteKit's built-in `src/service-worker.ts` for simplicity.
The dashboard has minimal caching needs (just the app shell). The `vite-plugin-pwa`
approach is better for complex apps with multiple caching strategies and web app
manifest generation — but adds a dependency for functionality we can implement in
~30 lines of service worker code.

### 8.5 Offline Indicator

When the WebSocket/WebTransport connection drops, the dashboard should display a
clear offline indicator rather than showing stale data:

```typescript
// src/lib/stores/connection.svelte.ts
export let connection_state = $state<'connected' | 'reconnecting' | 'offline'>('offline');

// In transport module:
ws.onclose = () => {
  connection_state = 'reconnecting';
  flush_pool(); // Mark all pool slots inactive
  schedule_reconnect();
};
```

The offline indicator should be a persistent banner, not a toast, since the dashboard
is useless without live data.

---

## 9. Performance Profiling Methodology

### 9.1 Goal: Verify Zero Major GC at 60 fps

The performance target is: **no major (mark-sweep) GC events during a 60-second
sustained run at 60 fps with 50 active targets.** Minor (Scavenge) GC events are
acceptable if they complete within 2 ms.

### 9.2 V8 Garbage Collection Architecture

```
┌─────────────────────────────────────────────────────────┐
│ Young Generation ("New Space") — 1–8 MB                  │
│  ┌────────────────┐  ┌────────────────┐                  │
│  │ Semi-space A   │  │ Semi-space B   │                  │
│  │ (active Eden)  │  │ (empty/to-space)│                 │
│  └────────────────┘  └────────────────┘                  │
│  Minor GC (Scavenge): copies live objects A→B            │
│  Pause: 1–5 ms. Runs when Eden fills.                    │
│  Objects surviving 2 cycles → promoted to Old Space      │
└─────────────────────────────────────────────────────────┘
                         │ promotion
┌────────────────────────▼────────────────────────────────┐
│ Old Generation ("Old Space") — grows as needed           │
│  Major GC (Mark-Compact): marks all reachable objects,   │
│  sweeps unreachable, compacts remaining.                 │
│  Pause: 50–200 ms (incremental marking reduces this     │
│  but atomic sweep/compact phase still blocks).           │
│  Triggered when Old Space reaches limit.                 │
└─────────────────────────────────────────────────────────┘
```

**Key insight:** Major GC is triggered by Old Space growth. If the application
allocates objects that are long-lived enough to be promoted (survive 2 minor GC
cycles) but then become garbage, the Old Space fills with dead objects, eventually
triggering a mark-compact. Our architecture prevents this by:

1. Allocating all long-lived objects (pool, typed arrays) once at module init
2. Ensuring per-frame allocations (path strings, DataView wrappers) die within
   1–2 minor GC cycles, never reaching promotion age

### 9.3 Chrome DevTools Profiling Workflow

**Step 1: Enable Memory in Performance Panel**

1. Open Chrome DevTools → Performance tab
2. Check **Memory** checkbox (shows JS heap, Documents, Nodes, Listeners over time)
3. Click the garbage can icon (Force GC) to establish a clean baseline
4. Start recording

**Step 2: Record 30–60 Second Session**

Run the dashboard with 50 active targets at 60 fps data rate for 60 seconds.
During recording, the Performance panel captures:
- Frame timings (green bars)
- JS execution (yellow)
- Layout/Paint/Composite (purple/green)
- GC events (labeled "Minor GC" or "Major GC" in the timeline)

**Step 3: Identify GC Events**

After recording:
1. Look for **yellow blocks labeled "Minor GC"** in the timeline — these should be
   short (< 2 ms) and infrequent (every few seconds)
2. Look for **yellow blocks labeled "Major GC"** — these should be **absent entirely**
3. The Memory graph should show a sawtooth pattern (allocation → minor GC drop)
   without upward drift (which indicates memory leak / Old Space growth)

**Step 4: Allocation Timeline (if GC issues found)**

If Major GC events appear:
1. Open DevTools → Memory tab
2. Select **Allocation instrumentation on timeline**
3. Enable **Record heap allocation stack traces** in DevTools settings
4. Record for 10 seconds
5. Examine blue bars (still-live objects) — these identify allocations that
   survived long enough to be promoted to Old Space
6. Stack traces on blue bars reveal the exact call site creating leaked objects

### 9.4 Programmatic Memory Monitoring

```typescript
// src/lib/diagnostics/gc_monitor.ts

// V8 GC event monitoring via PerformanceObserver
// Available in Chrome 95+ with the 'gc' entry type (behind flag in some versions)
if ('PerformanceObserver' in globalThis) {
  const observer = new PerformanceObserver((list) => {
    for (const entry of list.getEntries()) {
      if (entry.entryType === 'gc') {
        const gcEntry = entry as any;
        console.warn(
          `[GC] ${gcEntry.name} | duration: ${gcEntry.duration.toFixed(2)} ms | ` +
          `kind: ${gcEntry.detail?.kind ?? 'unknown'}`
        );
        if (gcEntry.duration > 5) {
          console.error(`[GC] MAJOR GC detected: ${gcEntry.duration.toFixed(2)} ms`);
        }
      }
    }
  });
  try {
    observer.observe({ type: 'gc', buffered: true });
  } catch {
    // 'gc' entry type may not be available in all browsers
  }
}
```

**Note:** The `gc` PerformanceObserver entry type is not universally available. As a
fallback, use `performance.measureUserAgentSpecificMemory()` for periodic memory
snapshots (requires cross-origin isolation — same as SharedArrayBuffer).

### 9.5 Frame Budget Verification

```typescript
// src/lib/diagnostics/frame_timer.ts

let _frameCount = 0;
let _droppedFrames = 0;
let _lastFrameTime = performance.now();
const _frameDurations: Float64Array = new Float64Array(300); // 5 seconds at 60fps

function measure_frame(): void {
  const now = performance.now();
  const dt = now - _lastFrameTime;
  _lastFrameTime = now;

  _frameDurations[_frameCount % 300] = dt;
  _frameCount++;

  if (dt > 18.0) { // > 16.6ms + 1.4ms tolerance
    _droppedFrames++;
  }

  if (_frameCount % 300 === 0) {
    // Report every 5 seconds
    let sum = 0, max = 0;
    for (let i = 0; i < 300; i++) {
      sum += _frameDurations[i];
      if (_frameDurations[i] > max) max = _frameDurations[i];
    }
    const avg = sum / 300;
    console.log(
      `[PERF] avg: ${avg.toFixed(2)} ms | max: ${max.toFixed(2)} ms | ` +
      `dropped: ${_droppedFrames} / ${_frameCount}`
    );
  }
}
```

### 9.6 Chrome Command-Line Flags for Profiling

```bash
chrome --enable-precise-memory-info \
       --js-flags="--trace-gc --trace-gc-verbose" \
       --disable-extensions \
       --disable-background-timer-throttling \
       http://localhost:5173/dashboard
```

- `--trace-gc`: Logs every GC event to stdout with timing
- `--trace-gc-verbose`: Adds heap size details per GC event
- `--enable-precise-memory-info`: Makes `performance.memory` return accurate values
- `--disable-background-timer-throttling`: Prevents Chrome from throttling timers
  when the tab is in background (important for continuous profiling)

### 9.7 Profiling Checklist

- [ ] Record 60-second Performance trace with Memory enabled
- [ ] Verify: zero Major GC events in timeline
- [ ] Verify: Minor GC pauses < 2 ms each
- [ ] Verify: no upward drift in Old Space (flat or sawtooth in young gen only)
- [ ] Measure: average frame duration < 12 ms (leaving 4 ms for compositor)
- [ ] Measure: P99 frame duration < 16 ms
- [ ] Measure: zero `requestAnimationFrame` callbacks > 16.6 ms
- [ ] Verify: JS heap size stable after initial 5-second warmup period
- [ ] Run Allocation Timeline for 10 seconds — confirm no blue bars in pool code
- [ ] Test with 50 targets × 64 trajectory points (maximum load)
- [ ] Test with 1 target (minimum load) — verify no idle-time memory growth
- [ ] Test connection drop + reconnect — verify pool flush, no leaked DOM nodes

---

## 10. Consolidated Recommendations

### 10.1 Phase A (Current Implementation)

| Component | Technology | Status |
|---|---|---|
| Transport | WebSocket (binary, `arraybuffer` mode) | ✅ Implemented |
| Wire format | Custom binary (12 B header + 86 B/track) | ✅ Implemented |
| Store | `$state.raw` pool + `$state` frame_tick | ✅ In progress |
| Rendering | Three-layer SVG, RAF imperative loop | ✅ Implemented |
| Analytics charts | Not yet implemented | Pending |
| Service Worker | Not yet implemented | Pending |

### 10.2 Phase B (Performance Optimization — Recommended Next)

| Component | Change | Expected Impact |
|---|---|---|
| Transport | Add WebTransport datagram path (Rust `wtransport` sidecar) | Eliminate HOL blocking; ~35% latency reduction on lossy links |
| Rendering Layer 1 | Migrate trajectory paths from SVG `<path>` to `<canvas>` | Eliminate ~1.8 MB/s path string allocation from Eden |
| Analytics charts | Integrate SveltePlot for histograms + LayerChart for streaming time series | Complete diagnostics panel |
| Service Worker | Add SvelteKit `src/service-worker.ts` with Cache First for app shell | Offline UI availability |
| Diagnostics | Add `frame_timer.ts` + GC monitor | Continuous performance verification |

### 10.3 Phase C (Advanced Optimization — When Needed)

| Component | Change | Trigger Condition |
|---|---|---|
| Transport | SharedArrayBuffer worker decode | Entity count > 200 or frame rate > 120 Hz |
| Rendering | WebGPU compute for trajectory interpolation | Entity count > 10,000 or trajectory depth > 1,000 |
| Rendering | OffscreenCanvas for off-thread Canvas 2D | Main thread utilization > 80% at target frame rate |
| Store | Per-track fine-grained signals | Template-driven rendering replaces RAF imperative loop |

### 10.4 Technology Decisions Summary

| Decision | Choice | Rationale |
|---|---|---|
| Primary transport | WebSocket → WebTransport (Phase B) | Datagram unreliable delivery eliminates HOL blocking for telemetry |
| Rendering for 50 entities | SVG (all layers Phase A) → SVG+Canvas hybrid (Phase B) | SVG scales to ~5000 elements; our 154 elements are well within budget |
| Svelte reactivity | `$state.raw` pool + single `$state` tick | Zero Proxy overhead on pool reads; O(1) reactive notification |
| RAF vs $effect | RAF imperative loop | Bypasses Svelte's microtask scheduling; direct DOM mutation |
| SharedArrayBuffer | Deferred to Phase C | 0.2 ms decode time not worth COOP/COEP complexity yet |
| Analytics charting | SveltePlot + LayerChart | Svelte-native, Observable Plot API, active Svelte 5 support |
| Offline support | SvelteKit built-in service worker | Simpler than `vite-plugin-pwa` for our minimal caching needs |
| WebGPU | Deferred to Phase C / 3D view | Overkill for 2D overlay at 50 entities |

### 10.5 Architecture Lessons from Foxglove and Rerun

| Lesson | Source | ROC Application |
|---|---|---|
| Zero-copy columnar data encoding | Rerun (Apache Arrow) | Our binary wire format already achieves this for our schema |
| Immediate-mode rendering | Rerun (egui) | Validated: our RAF loop is architecturally equivalent |
| Chunk-based micro-batching | Rerun | Not needed — single-frame messages |
| GPU picking for hit testing | Rerun (re_renderer) | Consider for Canvas 2D entity selection in Phase B |
| `done()` backpressure callback | Foxglove | Implement: signal frame completion to prevent WS buffer growth |
| Comlink Worker RPC | Foxglove | Consider for SAB worker communication in Phase C |
| Panel-level data subscription | Foxglove (`watch()`) | Already implemented via `frame_tick` version counter |
| Three.js migration from regl | Foxglove | Validates Three.js as the correct choice if 3D view is added |
| Memory leak prevention in data panels | Foxglove | Enforce bounded data retention in analytics stores |

---

## Appendix A: WebTransport Datagram Client Example

```typescript
// src/lib/transport/webtransport_client.ts

export class WebTransportClient {
  private transport: WebTransport | null = null;
  private reader: ReadableStreamDefaultReader<Uint8Array> | null = null;

  async connect(url: string): Promise<void> {
    this.transport = new WebTransport(url);
    await this.transport.ready;

    this.reader = this.transport.datagrams.readable.getReader();
    this.read_loop();
  }

  private async read_loop(): Promise<void> {
    if (!this.reader) return;

    let lastSeq = -1;

    while (true) {
      const { value, done } = await this.reader.read();
      if (done) break;

      // value is Uint8Array — one complete frame datagram
      const view = new DataView(value.buffer, value.byteOffset, value.byteLength);

      // Validate magic
      if (view.getUint16(0, true) !== 0x524F) continue;

      // Sequence-based stale frame drop
      const seq = view.getUint16(4, true);
      const delta = (seq - lastSeq) & 0xFFFF;
      if (delta === 0 || delta > 0x8000) continue;
      lastSeq = seq;

      // Reuse existing read_frame + apply_frame pipeline
      const result = read_frame(value.buffer);
      if (result) {
        apply_frame(result.targets, result.header.targetCount);
      }
    }
  }

  async send_control(data: Uint8Array): Promise<void> {
    // Use a bidirectional stream for reliable control messages
    if (!this.transport) return;
    const stream = await this.transport.createBidirectionalStream();
    const writer = stream.writable.getWriter();
    await writer.write(data);
    writer.releaseLock();
  }

  close(): void {
    this.transport?.close();
    this.transport = null;
    this.reader = null;
  }
}
```

## Appendix B: Canvas 2D Trajectory Renderer (Phase B Reference)

```typescript
// src/lib/rendering/trajectory_canvas.ts

const _canvas: HTMLCanvasElement | null = null;
let _ctx: CanvasRenderingContext2D | null = null;

export function init_trajectory_canvas(canvas: HTMLCanvasElement): void {
  _ctx = canvas.getContext('2d', { alpha: true, desynchronized: true });
  if (_ctx) {
    _ctx.lineCap = 'round';
    _ctx.lineJoin = 'round';
    _ctx.lineWidth = 1.5;
  }
}

export function render_trajectories(
  pool: PoolSlot[],
  activeCount: number,
  colors: string[],
  width: number,
  height: number,
): void {
  if (!_ctx) return;

  _ctx.clearRect(0, 0, width, height);

  for (let i = 0; i < 64; i++) {
    const s = pool[i];
    if (!s.active || s.trajectoryLen < 2) continue;

    _ctx.strokeStyle = colors[s.cls];
    _ctx.beginPath();
    _ctx.moveTo(s.traj[0], s.traj[1]);

    for (let j = 1; j < s.trajectoryLen; j++) {
      _ctx.lineTo(s.traj[j * 2], s.traj[j * 2 + 1]);
    }

    _ctx.stroke();
  }
}
```

**Key advantage:** `ctx.lineTo(float, float)` operates on internal GPU geometry
buffers — no string allocation, no DOM mutation, no path `d` attribute. The entire
trajectory rendering path is allocation-free.

## Appendix C: SharedArrayBuffer Worker Decode (Phase C Reference)

```typescript
// src/lib/workers/frame_decoder.worker.ts

const CONTROL_OFFSET = 0;
const SLOTS_OFFSET = 64;
const SLOT_SIZE = 548; // aligned to 4 bytes

let sab: SharedArrayBuffer;
let controlView: Int32Array;
let dataView: DataView;

self.onmessage = (e: MessageEvent) => {
  if (e.data.type === 'init') {
    sab = e.data.sab;
    controlView = new Int32Array(sab, CONTROL_OFFSET, 16);
    dataView = new DataView(sab);
  } else if (e.data.type === 'frame') {
    decode_into_sab(e.data.buffer);
  }
};

function decode_into_sab(buffer: ArrayBuffer): void {
  const frame = new DataView(buffer);
  if (frame.getUint16(0, true) !== 0x524F) return;

  const count = frame.getUint8(3);
  const seq = frame.getUint16(4, true);
  const ts = frame.getUint16(6, true);

  let srcOffset = 8;

  for (let i = 0; i < count && i < 64; i++) {
    const slotBase = SLOTS_OFFSET + i * SLOT_SIZE;

    dataView.setUint8(slotBase, 1); // active
    dataView.setUint16(slotBase + 1, frame.getUint16(srcOffset, true), true); srcOffset += 2;
    dataView.setUint8(slotBase + 3, frame.getUint8(srcOffset++));
    dataView.setUint8(slotBase + 4, frame.getUint8(srcOffset++));

    // bbox: 4 × float32
    for (let f = 0; f < 4; f++) {
      dataView.setFloat32(slotBase + 5 + f * 4, frame.getFloat32(srcOffset, true), true);
      srcOffset += 4;
    }

    const trajLen = frame.getUint8(srcOffset++);
    dataView.setUint8(slotBase + 21, trajLen);

    // trajectory: trajLen × 2 × float32
    for (let j = 0; j < trajLen * 2; j++) {
      dataView.setFloat32(slotBase + 22 + j * 4, frame.getFloat32(srcOffset, true), true);
      srcOffset += 4;
    }
  }

  // Mark inactive slots
  for (let i = count; i < 64; i++) {
    dataView.setUint8(SLOTS_OFFSET + i * SLOT_SIZE, 0);
  }

  // Write control metadata and signal dirty
  Atomics.store(controlView, 1, seq);           // frame_seq
  Atomics.store(controlView, 2, count);         // active_count
  Atomics.store(controlView, 3, ts);            // timestamp_ms
  Atomics.store(controlView, 0, 1);             // dirty_flag = 1 (LAST — memory fence)
  Atomics.notify(controlView, 0);               // wake main thread if waiting
}
```

---

*End of research document. Cross-reference with `phase_5_telemetry_ui.md` for
wire protocol schema, pool implementation, and SVG render loop details.*
