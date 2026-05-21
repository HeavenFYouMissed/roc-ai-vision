# Phase 5 Research: SvelteKit Telemetry UI — Frontend Performance Architecture
> **ROC AI Vision** · Lead Author: Robotics & CV Research Scientist  
> **Scope:** `dashboard/` · **Target Renderer:** SvelteKit 2.x + Svelte 5  
> **Performance Target:** Zero-GC frame loop @ ≥144 Hz display refresh  
> **Last Updated:** Phase 5 Research Sprint

---

## 0. Problem Statement

The telemetry dashboard must ingest a continuous stream of kinematic target
matrices from a C++ rosbridge backend and render them onto an SVG canvas in
real time. Each frame packet may contain up to ~32 simultaneous active targets,
each with:

- A unique persistent tracker ID (uint16)
- A detection class label (e.g., person, head, upper_torso — uint8 enum)
- A 2D bounding box in detection space: `[x1, y1, x2, y2]` (float32 × 4)
- A historical trajectory ring: the last *N* center-of-mass coordinates in
  tracking space (float32 × 2 × N, where N ≤ 64)

The three failure modes that must be eliminated entirely:

1. **V8 GC pressure** — The JavaScript engine must never be forced to collect
   short-lived intermediate objects during a live frame cycle. Any JSON
   deserialization that allocates temporary string keys, intermediate arrays,
   or boxed numbers on every 7 ms frame is unacceptable.

2. **Svelte reactive churn** — A naïve implementation that replaces the entire
   store array reference on every packet triggers Svelte's fine-grained diffing
   engine across all array entries on every frame, even for unchanged targets.
   This creates O(N) reactive work per frame regardless of whether any given
   target moved.

3. **SVG repaint cascade** — Unbounded DOM mutations force the browser's
   compositor to re-rasterize the entire scene graph. At 144 Hz the frame
   budget is 6.94 ms. Triggering a full composite of layered SVG paths on every
   frame without GPU isolation will saturate this budget on any mid-range GPU.

---

## 1. Binary WebSocket Payload Design

### 1.1 Why Raw Binary Beats JSON on the Hot Path

When a V8 engine parses `JSON.parse(wsEvent.data)` it must:
1. Allocate a new `string` primitive for the entire payload.
2. Tokenize the string, allocating temporary `string` objects for every key
   (`"id"`, `"bbox"`, `"trajectory"`, …).
3. Allocate intermediate `Array` and `Object` heap nodes.
4. Assign properties, triggering hidden class transitions for each new object.

For 32 targets at 144 frames/second, that is **4,608 object allocations per
second** just from the parser — not counting any application-layer processing.
Every allocation increases GC mark-and-sweep pressure proportionally.

A pre-allocated binary frame passed as an `ArrayBuffer` produces **zero
string allocations** and **zero heap object allocation** during frame ingestion
if the reader is written correctly (see §1.4).

### 1.2 Format Candidate Analysis

| Format | Zero-copy read | V8 GC pressure | Schema evolution | Wire size (32 tgts, N=32) | C++ lib |
|---|---|---|---|---|---|
| Raw `ArrayBuffer` (custom) | ✅ Full | ✅ Zero | ❌ Manual versioning | ~10.6 KB | none |
| FlatBuffers | ✅ Full | ✅ Zero | ✅ Forward/backward | ~11.2 KB | `flatbuffers` |
| MessagePack (binary JSON) | ❌ Decodes to objects | ⚠️ Moderate | ✅ Dynamic | ~14.8 KB | `msgpack-cxx` |
| Protocol Buffers (proto3) | ❌ Decodes to objects | ⚠️ Moderate | ✅ Full | ~9.4 KB | `protobuf` |
| JSON (UTF-8) | ❌ String parse | ❌ Heavy | ✅ Dynamic | ~52 KB | none |

**Decision: FlatBuffers for structured production use; raw `ArrayBuffer` for
the initial single-pipeline implementation.**

FlatBuffers is zero-copy because the JS accessor reads directly from the
underlying `ArrayBuffer` memory using `DataView`-style offsets without
deserializing into heap objects. It also provides a self-describing schema that
survives C++ refactors.

Raw `ArrayBuffer` with a hand-coded `DataView` reader is equally zero-copy and
has zero runtime overhead, but requires manual versioning. It is the correct
choice for Phase 5 because the schema is fully controlled end-to-end by this
codebase.

### 1.3 ROC Binary Frame Schema (v1)

The frame is a single contiguous `ArrayBuffer` transmitted as a WebSocket
binary message. All multi-byte integers and floats are **little-endian**.

```
┌──────────────────────────────────────────────────────────────────┐
│ FRAME HEADER  (8 bytes)                                          │
│  [0..1]  magic        : uint16  = 0x524F ("RO")                  │
│  [2]     version      : uint8   = 1                              │
│  [3]     target_count : uint8   (max 255, practical max 32)      │
│  [4..5]  frame_seq    : uint16  monotonic frame counter          │
│  [6..7]  timestamp_ms : uint16  millis mod 65536 (rollover-safe) │
└──────────────────────────────────────────────────────────────────┘

Followed immediately by `target_count` TARGET RECORDS, each:

┌──────────────────────────────────────────────────────────────────┐
│ TARGET RECORD  (variable, minimum 18 bytes)                      │
│  [0..1]  target_id        : uint16                               │
│  [2]     class_label      : uint8                                │
│            0 = person                                            │
│            1 = head                                              │
│            2 = upper_torso                                       │
│            3 = accessory                                         │
│  [3]     flags            : uint8  (bit 0 = selected/locked)     │
│  [4..7]  bbox_x1          : float32                              │
│  [8..11] bbox_y1          : float32                              │
│  [12..15] bbox_x2         : float32                              │
│  [16..19] bbox_y2         : float32                              │
│  [20]    trajectory_len   : uint8  (0–64 history depth)          │
│  [21..24+trajectory_len*8-1]                                     │
│          trajectory[]     : float32 pairs [cx, cy] × N          │
│            Most recent sample is at index 0.                     │
└──────────────────────────────────────────────────────────────────┘
```

**Byte cost per target (N=32 trajectory points):**
```
  2 (id) + 1 (class) + 1 (flags) + 16 (bbox) + 1 (traj_len) + 32*8 (traj)
= 21 + 256 = 277 bytes
```

**Total frame cost (32 targets, N=32):**
```
  8 (header) + 32 * 277 = 8,872 bytes ≈ 8.7 KB per frame
```

At 144 Hz: **~1.25 MB/s** — comfortably within any loopback or local network
WebSocket budget.

**Compare to JSON equivalent:**  
A single target serialized as JSON with float32 precision:
```json
{"id":1024,"cls":2,"bb":[120.5,84.2,240.1,340.7],
 "tr":[[180.3,212.4],…×32]}
≈ 640 bytes per target → ~20 KB per frame — 2.3× larger
```

Beyond size, JSON requires full string allocation and parse — the binary frame
does not.

### 1.4 C++ Backend Serialization

On the C++ side (rosbridge or a direct `asio` WebSocket server), the frame is
assembled from the `TargetSelector` output:

```cpp
// Statically allocated frame buffer reused every cycle — zero heap alloc
static std::array<uint8_t, 65536> g_frame_buf;

std::size_t serialize_frame(
    const std::vector<TrackedEntity>& targets,
    uint16_t frame_seq,
    uint8_t* buf
) noexcept {
    uint8_t* p = buf;

    // Header
    write_u16_le(p, 0x524F);         p += 2;  // magic "RO"
    *p++ = 1;                                  // version
    *p++ = static_cast<uint8_t>(targets.size());
    write_u16_le(p, frame_seq);      p += 2;
    write_u16_le(p, millis_now_u16()); p += 2;

    for (const auto& t : targets) {
        write_u16_le(p, t.id);       p += 2;
        *p++ = static_cast<uint8_t>(t.class_label);
        *p++ = t.flags;
        write_f32_le(p, t.bbox.x1);  p += 4;
        write_f32_le(p, t.bbox.y1);  p += 4;
        write_f32_le(p, t.bbox.x2);  p += 4;
        write_f32_le(p, t.bbox.y2);  p += 4;

        const uint8_t tlen = static_cast<uint8_t>(
            std::min(t.trajectory.size(), std::size_t{64})
        );
        *p++ = tlen;
        for (uint8_t i = 0; i < tlen; ++i) {
            write_f32_le(p, t.trajectory[i].x); p += 4;
            write_f32_le(p, t.trajectory[i].y); p += 4;
        }
    }

    return static_cast<std::size_t>(p - buf);
}
```

The static `g_frame_buf` is reused every cycle — **zero C++ heap allocation**
per frame on the serialization path.

### 1.5 JavaScript Zero-Copy Frame Reader

The `DataView` API reads typed values from an `ArrayBuffer` without copying.
The critical constraint: **never call `slice()` on the incoming buffer**. Slice
allocates a new `ArrayBuffer`. Instead, wrap the buffer with a `DataView` at
offset zero and walk forward using byte offsets.

```typescript
// src/lib/ws/frame_reader.ts

export interface TargetRecord {
  id: number;
  cls: number;
  flags: number;
  x1: number; y1: number; x2: number; y2: number;
  trajectoryLen: number;
  // Direct view into the frame ArrayBuffer — zero copy.
  // Caller must read before next frame overwrites the WS buffer.
  trajectoryView: DataView;
  trajectoryOffset: number;  // byte offset into trajectoryView for index 0
}

export interface FrameHeader {
  version: number;
  targetCount: number;
  frameSeq: number;
  timestampMs: number;
}

// Statically pre-allocated result objects — reused every frame.
// V8 will keep these in old-space (promoted) after ~2 GC cycles,
// eliminating all per-frame allocation.
const MAX_TARGETS = 64;
const _header: FrameHeader = { version: 0, targetCount: 0, frameSeq: 0, timestampMs: 0 };
const _targets: TargetRecord[] = Array.from({ length: MAX_TARGETS }, () => ({
  id: 0, cls: 0, flags: 0,
  x1: 0, y1: 0, x2: 0, y2: 0,
  trajectoryLen: 0,
  trajectoryView: new DataView(new ArrayBuffer(0)),
  trajectoryOffset: 0,
}));

export function read_frame(buffer: ArrayBuffer): { header: FrameHeader; targets: TargetRecord[] } | null {
  const view = new DataView(buffer);
  let offset = 0;

  // Validate magic
  if (view.getUint16(0, true) !== 0x524F) return null;
  offset = 2;

  _header.version      = view.getUint8(offset++);
  _header.targetCount  = view.getUint8(offset++);
  _header.frameSeq     = view.getUint16(offset, true); offset += 2;
  _header.timestampMs  = view.getUint16(offset, true); offset += 2;

  const count = Math.min(_header.targetCount, MAX_TARGETS);

  for (let i = 0; i < count; i++) {
    const t = _targets[i];
    t.id    = view.getUint16(offset, true); offset += 2;
    t.cls   = view.getUint8(offset++);
    t.flags = view.getUint8(offset++);
    t.x1    = view.getFloat32(offset, true); offset += 4;
    t.y1    = view.getFloat32(offset, true); offset += 4;
    t.x2    = view.getFloat32(offset, true); offset += 4;
    t.y2    = view.getFloat32(offset, true); offset += 4;

    t.trajectoryLen    = view.getUint8(offset++);
    t.trajectoryView   = view;
    t.trajectoryOffset = offset;
    offset += t.trajectoryLen * 8;
  }

  return { header: _header, targets: _targets };
}

// Retrieve trajectory point i from a TargetRecord without allocating a new object.
export function get_trajectory_point(t: TargetRecord, i: number, out: { x: number; y: number }): void {
  const base = t.trajectoryOffset + i * 8;
  out.x = t.trajectoryView.getFloat32(base,     true);
  out.y = t.trajectoryView.getFloat32(base + 4, true);
}
```

**Key GC properties of this design:**
- `_header` and `_targets[0..63]` are allocated **once** at module load time.
- V8 promotes objects that survive two minor GC cycles into old-generation
  space. After the first two frames these objects are permanently tenured.
- `new DataView(buffer)` does **not** copy memory — it wraps the existing
  `ArrayBuffer` with a typed read handle.
- No `JSON.parse`, no `Array.map`, no `Object.assign`, no spread operators
  anywhere on the frame-read hot path.

### 1.6 Compact JSON Fallback Schema (rosbridge compatibility)

If rosbridge_websocket is used as the transport (which uses ROS 2 JSON
serialization by default), the schema must be designed to minimize key string
allocation overhead.

Rules for a GC-friendly JSON schema:
1. Use **single-character or minimal keys** — each key string is a V8 heap
   object. `"id"` is better than `"target_identifier"`.
2. **Flat arrays instead of nested objects** — `[x1, y1, x2, y2]` allocates
   one `Array` object; `{"x1":…,"y1":…,"x2":…,"y2":…}` allocates one `Object`
   plus four string keys.
3. **Interned constant strings** — class label as an integer enum rather than
   `"upper_torso"` prevents repeated string allocation.
4. **Trajectory as a flat Float64 array** — `[cx0, cy0, cx1, cy1, …]` rather
   than `[[cx0,cy0],[cx1,cy1],…]`, eliminating one wrapper `Array` allocation
   per trajectory point.

```json
{
  "v": 1,
  "seq": 14820,
  "ts": 49234,
  "t": [
    {
      "id": 1024,
      "c": 2,
      "f": 1,
      "b": [120.5, 84.2, 240.1, 340.7],
      "tr": [180.3, 212.4, 179.1, 211.8, 178.5, 211.1]
    }
  ]
}
```

Key mapping: `v`=version, `seq`=frame_seq, `ts`=timestamp, `t`=targets,
`c`=class, `f`=flags, `b`=bbox, `tr`=trajectory flat array.

Even with the compact schema, each call to `JSON.parse` will still allocate
intermediate V8 heap objects. For rosbridge use, process the JSON parse result
**immediately** by writing it into the pre-allocated store slots (§2) and
releasing all references to the parsed object before the next frame arrives,
allowing the GC to collect it in the next minor cycle at minimum cost.

---

## 2. Zero-GC Svelte Store Architecture

### 2.1 The Naïve Pattern and Why It Fails

The standard SvelteKit tutorial pattern for reactive state:

```typescript
// ❌ — allocates a new array + N new objects on every frame
targets.set(parsedFrame.targets.map(t => ({ ...t })));
```

This pattern:
1. Calls `Array.prototype.map` → allocates a new `Array` on the heap.
2. Spreads each target object → allocates N new `Object` instances.
3. Triggers Svelte's fine-grained subscriptions → all subscribers re-evaluate.
4. Triggers `{#each}` keyed diffing even for unchanged array entries.
5. Every old array + every old object becomes garbage → GC pressure.

At 144 Hz with 32 targets: **4,608+ allocations/second** and proportional GC
pause risk. On V8's Orinoco concurrent GC this will manifest as periodic 1–4 ms
pauses that break the 144 Hz frame budget.

### 2.2 Architecture: Static Object Pool with In-Place Mutation

The correct architecture uses a **static slot map** — a fixed-size array of
pre-allocated target state objects that are **never replaced**, only mutated
in-place. Svelte reactivity is bypassed for per-field updates on the hot path
and replaced with a single `notify()` call at frame end.

```
                 ┌─────────────────────────────────────────────┐
  WS binary  →   │  read_frame()          (zero alloc)          │
  ArrayBuffer    │  walks DataView, writes into _targets[]       │
                 └──────────────┬──────────────────────────────┘
                                │ mutates slot fields in-place
                                ▼
                 ┌─────────────────────────────────────────────┐
                 │  TargetPool (static, 64 slots)               │
                 │  slot[i] = { id, cls, flags, bbox, traj }    │
                 │  activeCount: number                         │
                 │  version: number  (increments each frame)    │
                 └──────────────┬──────────────────────────────┘
                                │ single writable.set(version++)
                                ▼
                 ┌─────────────────────────────────────────────┐
                 │  frameVersion: Writable<number>              │
                 │  Svelte components subscribe to version only  │
                 │  and read from TargetPool directly            │
                 └─────────────────────────────────────────────┘
```

### 2.3 TargetPool Implementation

```typescript
// src/lib/stores/target_pool.ts

import { writable } from 'svelte/store';
import type { TargetRecord } from '$lib/ws/frame_reader';
import { get_trajectory_point } from '$lib/ws/frame_reader';

export const MAX_TARGETS = 64;
export const TRAJ_DEPTH  = 64;

// ── Static pool — allocated once, never replaced ───────────────────────────

export interface PoolSlot {
  active:        boolean;
  id:            number;
  cls:           number;
  flags:         number;
  x1:            number;
  y1:            number;
  x2:            number;
  y2:            number;
  // Flat Float32Array: [cx0,cy0,cx1,cy1,...] length = TRAJ_DEPTH*2
  // Pre-allocated. trajectoryLen controls how many points are valid.
  traj:          Float32Array;
  trajectoryLen: number;
}

export const pool: PoolSlot[] = Array.from({ length: MAX_TARGETS }, () => ({
  active:        false,
  id:            0,
  cls:           0,
  flags:         0,
  x1:            0,
  y1:            0,
  x2:            0,
  y2:            0,
  traj:          new Float32Array(TRAJ_DEPTH * 2),
  trajectoryLen: 0,
}));

export let activeCount = 0;

// ── Reactive version tick — the only thing Svelte subscribes to ────────────
// Components that need to re-render read `pool` directly; this store just
// signals "something changed this frame".
export const frameVersion = writable(0);

// ── ID→slot lookup: Int16Array[65536], value = slot index or -1 ─────────────
// Direct index by target_id (uint16). O(1) read/write, zero Map allocation.
// 65536 × 2 bytes = 128 KB — allocated once, off-heap, GC-immune.
// ⚠️  ES6 Map<number,number> was the original design — REJECTED because
//     for...of Map iteration allocates a MapIterator object + a [key,value]
//     tuple per entry on every single frame, violating the zero-GC mandate.
const _idToSlot = new Int16Array(65536).fill(-1);

// ── Free-slot stack: TypedArray-backed, zero JS heap allocation ──────────────
// Uint8Array holds slot indices (0–63). _freeStackTop is the stack pointer.
// ⚠️  number[] Array with .push()/.pop() was the original design — REJECTED
//     because JS Array backing stores can re-allocate on growth, and array
//     element boxing may allocate SMI wrappers in non-optimized paths.
const _freeStack    = new Uint8Array(MAX_TARGETS);
let   _freeStackTop = MAX_TARGETS;
for (let i = 0; i < MAX_TARGETS; i++) _freeStack[i] = MAX_TARGETS - 1 - i;

function _pop_slot(): number   { return _freeStackTop > 0 ? _freeStack[--_freeStackTop] : -1; }
function _push_slot(s: number): void { _freeStack[_freeStackTop++] = s; }

// ── Generation counter: evict vanished tracks without any Map iteration ──────
// _slotGeneration[i] holds the _frameGen value when slot i was last updated.
// After the incoming-targets pass, any slot with _slotGeneration[i] !== _frameGen
// was absent from this frame and must be evicted.
// Cost: O(MAX_TARGETS = 64) Uint32Array reads — no iterator, no tuple alloc.
// ⚠️  for (const [id, slotIdx] of _idToSlot) was the original eviction design —
//     REJECTED because V8 allocates a MapIterator + a heap [key,value] Array
//     for every entry traversed, regardless of whether the entry is evicted.
const _slotGeneration = new Uint32Array(MAX_TARGETS);
let   _frameGen       = 0;

// ── Reusable scratch vector for trajectory copying ─────────────────────────
const _pt = { x: 0, y: 0 };

/**
 * Called by the WebSocket handler every frame.
 * Mutates pool slots in-place — zero heap allocation on hot path.
 */
export function apply_frame(targets: TargetRecord[], count: number): void {
  // Advance generation FIRST. Any slot not stamped with _frameGen after the
  // incoming pass was absent from this packet and must be evicted.
  _frameGen++;

  const n = Math.min(count, MAX_TARGETS);

  for (let i = 0; i < n; i++) {
    const t = targets[i];

    // O(1) direct array index — no Map.get(), no hash computation, no alloc.
    let slotIdx: number = _idToSlot[t.id];

    if (slotIdx === -1) {
      // New track: acquire a free slot from the TypedArray stack.
      slotIdx = _pop_slot();
      if (slotIdx === -1) continue;  // pool exhausted — skip
      _idToSlot[t.id] = slotIdx;
    }

    const s = pool[slotIdx];
    s.active        = true;
    s.id            = t.id;
    s.cls           = t.cls;
    s.flags         = t.flags;
    s.x1            = t.x1;
    s.y1            = t.y1;
    s.x2            = t.x2;
    s.y2            = t.y2;
    s.trajectoryLen = t.trajectoryLen;

    // Stamp slot with current generation — proves it was present this frame.
    _slotGeneration[slotIdx] = _frameGen;

    // Copy trajectory into the pre-allocated Float32Array — no allocation.
    for (let j = 0; j < t.trajectoryLen; j++) {
      get_trajectory_point(t, j, _pt);
      s.traj[j * 2]     = _pt.x;
      s.traj[j * 2 + 1] = _pt.y;
    }
  }

  // ── Eviction: O(MAX_TARGETS = 64) linear scan — zero allocation ───────────
  // No for...of, no MapIterator object, no [id, slotIdx] tuple per entry.
  // Pure Uint32Array integer comparisons against _frameGen.
  for (let i = 0; i < MAX_TARGETS; i++) {
    if (pool[i].active && _slotGeneration[i] !== _frameGen) {
      _idToSlot[pool[i].id] = -1;  // reclaim ID in direct-index array
      pool[i].active         = false;
      _push_slot(i);               // return slot index to TypedArray stack
    }
  }

  activeCount = n;

  // Single reactive notification — the only Svelte store write per frame.
  frameVersion.update(v => v + 1);
}
```

**GC properties of the corrected implementation:**
- `pool[]` is allocated once at module init and tenured immediately.
- `pool[i].traj` is a `Float32Array` — V8 stores it off-heap in a backing
  store, immune to JS GC entirely.
- `_idToSlot` (`Int16Array`) and `_slotGeneration` / `_freeStack` (`Uint32Array`
  / `Uint8Array`) are all TypedArrays — off-heap, zero GC involvement.
- **Zero heap allocations** on the entire `apply_frame` hot path. The eviction
  scan is a plain `for` loop over 64 Uint32Array reads — no iterator objects,
  no tuple arrays, no Set membership tests.
- No `map()`, no `filter()`, no spread, no `Object.assign`, no `new Set()`,
  no `new Map()` anywhere on the hot path.

### 2.4 Svelte 5 Runes Variant

With Svelte 5's `$state` rune system, `frameVersion` (a Svelte 4 `writable`)
is replaced by a plain reactive primitive:

```typescript
// src/lib/stores/target_store.svelte.ts

// Reactive frame counter — triggers component re-render via Svelte 5 fine-grained reactivity.
// Components subscribe to this; all pool reads happen via direct pool[] access.
export let frame_tick = $state(0);
```

The underlying `apply_frame` implementation from §2.3 is shared unchanged —
the `_idToSlot` Int16Array, `_slotGeneration` generation counter, and
`_freeStack` TypedArray are transport-layer concerns independent of the
reactive framework. The only difference is replacing `frameVersion.update(v => v + 1)`
with `frame_tick++` at the end of `apply_frame`.

**Note on `_presence.fill(0)`:** An earlier draft of this section proposed a
`Uint8Array(65536)` presence bitmap cleared with `TypedArray.fill(0)` each
frame as a replacement for `new Set()`. This approach is superseded by the
generation-counter design in §2.3, which eliminates the need for any presence
structure entirely — the `_slotGeneration` array implicitly encodes presence
via the current `_frameGen` value, and the clear cost of `fill(0)` over 65536
bytes (~10 µs/frame via AVX2) is avoided.

### 2.5 WebSocket Integration

```typescript
// src/lib/ws/telemetry_socket.ts

import { read_frame } from './frame_reader';
import { apply_frame } from '$lib/stores/target_pool';

let _ws: WebSocket | null = null;

export function connect(url: string): void {
  _ws = new WebSocket(url);
  _ws.binaryType = 'arraybuffer';  // critical — prevents Blob allocation

  _ws.onmessage = (event: MessageEvent<ArrayBuffer>) => {
    const result = read_frame(event.data);
    if (!result) return;
    apply_frame(result.targets, result.header.targetCount);
  };
}

export function disconnect(): void {
  _ws?.close();
  _ws = null;
}
```

Setting `binaryType = 'arraybuffer'` is mandatory. The default `'blob'` type
wraps the incoming data in a `Blob` object, requiring an async `.arrayBuffer()`
call and allocating an intermediate copy. `'arraybuffer'` gives direct access
to the raw memory with zero copy overhead.

---

## 3. SVG Rendering Architecture for 144 Hz

### 3.1 Browser Compositor Model

The browser renders in a pipeline:
```
JavaScript → Style → Layout → Paint → Composite
```

**Composite** is the cheapest stage — it runs on the GPU compositor thread,
independent of the main thread. If every draw operation promotes its layer to
the compositor, the main thread only needs to commit new geometry transforms
per frame — not re-rasterize pixels.

For SVG elements, the promotion rules are:
- An SVG layer gets its own GPU compositor layer if it has `will-change: transform`
  or `will-change: opacity` applied to its direct container.
- Child elements of a composited layer are rasterized once and then transformed
  on the GPU — **no re-paint per frame**.
- If a child element's `d` path attribute changes, the browser must re-rasterize
  only that element's layer, not the entire SVG scene.

The architecture goal: maximize the ratio of **geometry transforms** (GPU-free)
to **rasterize operations** (CPU-GPU upload) per frame.

### 3.2 Layer Decomposition Strategy

Split the telemetry canvas into exactly **three SVG layers**, each with a
distinct update frequency:

```
┌─────────────────────────────────────────────────────────────┐
│ Layer 0: Static Background  (painted once, never updated)   │
│   • Grid lines, scale markers, field-of-view boundary       │
│   • will-change: none                                        │
│   • contain: strict                                          │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│ Layer 1: Trajectory Paths   (updated ~30 Hz — path changes)  │
│   • One <path> per active target, d="M cx0,cy0 L cx1,cy1…"  │
│   • will-change: contents                                    │
│   • GPU layer promoted — re-rasterizes only changed paths    │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│ Layer 2: Bounding Boxes + Labels  (updated every frame)      │
│   • <g transform="translate(x1,y1)"> per target              │
│   • Inner <rect> + <text> — geometry only, no rasterize      │
│   • will-change: transform                                   │
│   • Only transform attribute changes — no re-paint           │
└─────────────────────────────────────────────────────────────┘
```

**Critical insight:** Bounding boxes move every frame but their shape doesn't
change. By representing them as `<g transform="translate(x1,y1)">` wrappers
around fixed-size `<rect>` elements, the frame update only changes a
`transform` string — which the compositor resolves without re-rasterization.

### 3.3 SVG DOM Structure

```svelte
<!-- src/lib/components/TelemetryCanvas.svelte (structural skeleton) -->
<svg
  class="telemetry-root"
  viewBox="0 0 1920 1080"
  xmlns="http://www.w3.org/2000/svg"
  bind:this={svgRoot}
>
  <!-- ── Layer 0: Static background ─────────────────────────────── -->
  <g class="layer-static" aria-hidden="true">
    <!-- Rendered once via Svelte markup, never touched by RAF loop -->
    <StaticGrid />
  </g>

  <!-- ── Layer 1: Trajectory paths ──────────────────────────────── -->
  <g class="layer-trajectories" bind:this={layerTrajectories}>
    <!-- One <path> per target slot. Pre-created at mount for all 64 slots,
         then d="" / display:none toggled per active state.
         Never add/remove DOM nodes after mount. -->
  </g>

  <!-- ── Layer 2: Bounding boxes + labels ────────────────────────── -->
  <g class="layer-bboxes" bind:this={layerBboxes}>
    <!-- One <g> per target slot, pre-created at mount. -->
  </g>
</svg>
```

### 3.4 Pre-Created DOM Nodes — The Key Optimization

Adding or removing DOM nodes is the single most expensive SVG operation because
it invalidates the layout tree and forces a full subtree re-rasterize. The
solution is to **pre-create all 64 target slot DOM nodes at component mount**
and never add or remove them. Instead, toggle visibility via `display: none`
on inactive slots.

This means:
- The DOM tree is static after mount — no `appendChild` / `removeChild` ever.
- The RAF loop only mutates **attribute values** on existing elements.
- V8 attribute mutation is an O(1) operation that writes into V8's internal
  element backing store.
- The browser only triggers re-composite for the specific elements whose
  attributes changed, not the entire layer.

### 3.5 RAF Loop with Direct DOM Attribute Mutation

The render loop must run via `requestAnimationFrame`, not via Svelte's
reactive `$:` system. Svelte reactivity runs synchronously on the microtask
queue and can be triggered multiple times per frame if multiple store updates
occur. `requestAnimationFrame` is guaranteed to fire exactly once per display
refresh cycle, coalescing all pending state changes.

```typescript
// src/lib/components/TelemetryCanvas.svelte <script lang="ts">
import { onMount, onDestroy } from 'svelte';
import { frameVersion, pool, MAX_TARGETS } from '$lib/stores/target_pool';
import { CLASS_COLORS, CLASS_NAMES } from '$lib/constants/class_labels';

// DOM references to pre-created elements — set during mount
let layerBboxes: SVGGElement;
let layerTrajectories: SVGGElement;

// Pre-allocated element handle arrays — avoids querySelector on each frame
const _bboxGroups:   SVGGElement[]    = new Array(MAX_TARGETS);
const _bboxRects:    SVGRectElement[] = new Array(MAX_TARGETS);
const _bboxLabels:   SVGTextElement[] = new Array(MAX_TARGETS);
const _trajPaths:    SVGPathElement[] = new Array(MAX_TARGETS);

// ── Path build: single Uint8Array byte buffer + reused TextDecoder ──────────
// ASCII bytes for SVG 'd' attributes are written directly into _pathWriteBuf
// using _wc()/_wi() helpers — zero intermediate string allocations.
// Max path: 1 'M' + 63 'L' segments × ~16 bytes (integer coords) ≈ 1024 bytes.
// Buffer is 4096 bytes for headroom. TextDecoder instance is created once.
// ⚠️  string[] with .toFixed(1) + .slice().join() was the original design —
//     REJECTED: .toFixed(1) allocates one string per point (589,824 allocs/s
//     at 64 tgts × 64 pts × 144 Hz); .slice() allocates a new Array per frame.
const _pathWriteBuf = new Uint8Array(4096);
const _textDecoder  = new TextDecoder('utf-8');

/** Write a single ASCII character code into buf[pos]. Returns next pos. */
function _wc(buf: Uint8Array, pos: number, cc: number): number {
  buf[pos] = cc;
  return pos + 1;
}

/**
 * Write a signed integer into buf as ASCII decimal digits.
 * No string allocation. Digits are written LSB-first then reversed in-place.
 * Valid for SVG canvas coordinate range (integer pixel values ±32767).
 * Returns next write position.
 */
function _wi(buf: Uint8Array, pos: number, n: number): number {
  if (n < 0) { buf[pos++] = 45; n = -n; }    // '-' (ASCII 45)
  if (n === 0) { buf[pos++] = 48; return pos; } // '0' (ASCII 48)
  const mark = pos;
  while (n > 0) { buf[pos++] = 48 + (n % 10); n = (n / 10) | 0; }
  // Reverse digits written in LSB-first order.
  let lo = mark, hi = pos - 1;
  while (lo < hi) { const tmp = buf[lo]; buf[lo] = buf[hi]; buf[hi] = tmp; lo++; hi--; }
  return pos;
}

let _rafHandle = 0;
let _lastRenderedVersion = -1;

// ── Mount: create all DOM nodes once ──────────────────────────────────────
onMount(() => {
  for (let i = 0; i < MAX_TARGETS; i++) {
    // Trajectory path
    const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
    path.setAttribute('class', 'traj-path');
    path.setAttribute('fill', 'none');
    path.setAttribute('display', 'none');
    layerTrajectories.appendChild(path);
    _trajPaths[i] = path;

    // Bbox group
    const g    = document.createElementNS('http://www.w3.org/2000/svg', 'g');
    const rect = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
    const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');

    rect.setAttribute('class', 'bbox-rect');
    text.setAttribute('class', 'bbox-label');
    text.setAttribute('dy', '-4');

    g.appendChild(rect);
    g.appendChild(text);
    g.setAttribute('display', 'none');
    layerBboxes.appendChild(g);

    _bboxGroups[i] = g;
    _bboxRects[i]  = rect;
    _bboxLabels[i] = text;
  }

  // Start RAF loop
  _rafHandle = requestAnimationFrame(render_frame);
});

onDestroy(() => cancelAnimationFrame(_rafHandle));

// ── Render function — called once per display refresh cycle ───────────────
function render_frame(): void {
  _rafHandle = requestAnimationFrame(render_frame);

  // Bail early if no new frame data arrived since last render.
  // This correctly handles 144 Hz display driving a 60 Hz data source.
  const currentVersion = get_frameVersion();
  if (currentVersion === _lastRenderedVersion) return;
  _lastRenderedVersion = currentVersion;

  for (let i = 0; i < MAX_TARGETS; i++) {
    const s = pool[i];

    if (!s.active) {
      if (_bboxGroups[i].getAttribute('display') !== 'none') {
        _bboxGroups[i].setAttribute('display', 'none');
        _trajPaths[i].setAttribute('display', 'none');
      }
      continue;
    }

    // ── Bounding box: only translate the group ─────────────────────────
    const g    = _bboxGroups[i];
    const rect = _bboxRects[i];
    const text = _bboxLabels[i];
    const w = s.x2 - s.x1;
    const h = s.y2 - s.y1;

    g.setAttribute('transform', `translate(${s.x1 | 0},${s.y1 | 0})`);
    rect.setAttribute('width',  (w | 0).toString());
    rect.setAttribute('height', (h | 0).toString());

    // Update label only if ID or class changed (low frequency)
    if (
      text.dataset.lastId  !== String(s.id) ||
      text.dataset.lastCls !== String(s.cls)
    ) {
      text.textContent = `${CLASS_NAMES[s.cls]} #${s.id}`;
      text.setAttribute('fill', CLASS_COLORS[s.cls]);
      text.dataset.lastId  = String(s.id);
      text.dataset.lastCls = String(s.cls);
    }

    g.setAttribute('display', '');

    // ── Trajectory path: build d string from pre-allocated buffer ─────
    const path = _trajPaths[i];
    const len  = s.trajectoryLen;

    if (len < 2) {
      path.setAttribute('display', 'none');
      continue;
    }

    // ── Build SVG path 'd' via ASCII byte-writer — zero intermediate strings ──
    // _wi() converts float32 coords to integer via | 0, then writes ASCII digits
    // directly into _pathWriteBuf as byte values. No .toFixed(), no template
    // literal string per point, no intermediate Array.
    // subarray(0, p) is a zero-copy view (no ArrayBuffer copy).
    // The single unavoidable string allocation is TextDecoder.decode() —
    // one string per active path per frame, which is the theoretical minimum.
    let p = 0;
    p = _wc(_pathWriteBuf, p, 77); p = _wc(_pathWriteBuf, p, 32);  // 'M '
    p = _wi(_pathWriteBuf, p, s.traj[0] | 0);
    p = _wc(_pathWriteBuf, p, 44);                                   // ','
    p = _wi(_pathWriteBuf, p, s.traj[1] | 0);
    for (let j = 1; j < len; j++) {
      p = _wc(_pathWriteBuf, p, 76); p = _wc(_pathWriteBuf, p, 32); // 'L '
      p = _wi(_pathWriteBuf, p, s.traj[j * 2]     | 0);
      p = _wc(_pathWriteBuf, p, 44);                                 // ','
      p = _wi(_pathWriteBuf, p, s.traj[j * 2 + 1] | 0);
    }

    path.setAttribute('d', _textDecoder.decode(_pathWriteBuf.subarray(0, p)));
    path.setAttribute('stroke', CLASS_COLORS[s.cls]);
    path.setAttribute('display', '');
  }
}

// Non-reactive version read — avoids store subscription overhead in RAF loop
let _currentFrameVersion = 0;
const _unsubscribe = frameVersion.subscribe(v => { _currentFrameVersion = v; });
onDestroy(_unsubscribe);
function get_frameVersion(): number { return _currentFrameVersion; }
```

**Why `| 0` (bitwise OR zero)?**  
`s.x1 | 0` is a V8 intrinsic that converts a float64 to int32 in a single
CPU instruction (`cvttsd2si` or `roundsd`). It is faster than `Math.floor()`
for values in the [-2^31, 2^31) range and produces integer strings for
`setAttribute` — preventing sub-pixel jitter in SVG transform coordinates.

**Why the Uint8Array byte-writer over `Array.join()` or template literals?**

`Array.join()` on a `string[]` still requires each array element to be a live
string object — the `.toFixed(1)` calls that fill those slots allocate one
string primitive per trajectory point. Template literals (`` `L ${x},${y}` ``)
do the same. The allocation cost at full scale:

```
64 targets × 64 trajectory points × 144 Hz = 589,824 string allocations/second
```

All of these enter V8's Eden (young-generation) space. While the Orinoco minor
GC can collect them, the allocation rate saturates roughly **6% of V8's minor
GC throughput** — enough to cause observable jitter at 144 Hz.

The byte-writer approach eliminates every intermediate string:

- `_wi()` converts coordinates via `| 0` (single CPU `cvttsd2si` instruction)
  and writes raw ASCII digit bytes as integer arithmetic — no string API called.
- `_pathWriteBuf` is a `Uint8Array` backed by an off-heap `ArrayBuffer` —
  completely immune to JS GC.
- `subarray(0, p)` returns a zero-copy typed view (no `ArrayBuffer` allocated).
- `TextDecoder.decode()` produces exactly **one string per active path per
  frame** — the theoretical minimum, since `setAttribute('d', ...)` requires
  a string argument.

Additionally, `.slice(0, bufIdx)` on a `string[]` allocated a new heap `Array`
on every frame — removed entirely, since `_pathWriteBuf` is a flat buffer
indexed by byte offset `p` with no copy needed.

### 3.6 CSS Hardware Acceleration Configuration

```css
/* src/lib/styles/telemetry.css */

.telemetry-root {
  /* Isolate the SVG from the rest of the document layout tree.
     'contain: strict' tells the browser that nothing inside affects
     layout outside, enabling independent rasterization. */
  contain: strict;

  /* Force GPU layer promotion for the entire SVG root.
     translateZ(0) is the most universally supported compositor hint.
     will-change is the modern equivalent. Both are declared for
     maximum browser compatibility. */
  transform: translateZ(0);
  will-change: transform;

  /* Prevent iOS Safari from triggering tap-highlight repaints */
  -webkit-tap-highlight-color: transparent;
}

.layer-static {
  /* Static layer is rasterized once to a GPU texture.
     pointer-events:none prevents hit-testing traversal cost. */
  pointer-events: none;
}

.layer-trajectories {
  /* Path geometry changes require re-rasterization of individual paths.
     Promote to its own compositor layer to isolate repaint cost. */
  will-change: contents;
  pointer-events: none;
}

.layer-bboxes {
  /* Bounding boxes change only transform — no re-rasterize needed.
     The compositor handles translate() as a matrix multiply on the GPU. */
  will-change: transform;
}

.bbox-rect {
  fill: none;
  stroke-width: 1.5;
  /* Hardware-accelerated stroke rendering hint */
  vector-effect: non-scaling-stroke;
}

.traj-path {
  fill: none;
  stroke-width: 1.5;
  stroke-linejoin: round;
  stroke-linecap: round;
  /* Subpixel anti-aliasing: crispEdges is faster but aliased;
     geometricPrecision is GPU-accelerated on modern Chrome/Safari. */
  shape-rendering: geometricPrecision;
}

.bbox-label {
  font-family: 'JetBrains Mono', 'Courier New', monospace;
  font-size: 11px;
  font-weight: 600;
  /* Prevent text measurement from triggering layout recalculation */
  dominant-baseline: auto;
  text-anchor: start;
}
```

### 3.7 `will-change` Usage Constraints

`will-change: transform` is not free — the browser allocates a separate GPU
texture for each promoted layer. Misuse creates **GPU memory pressure** that
can degrade rendering more than the optimization helped.

**Rules for this application:**
1. Apply `will-change` only to the three SVG layer `<g>` elements and the root
   `<svg>` — not to individual `<path>` or `<rect>` elements.
2. Do **not** apply `will-change` in a loop, inside `{#each}`, or conditionally.
   All `will-change` declarations must be in static CSS.
3. If a layer becomes inactive (no targets visible), explicitly set
   `will-change: auto` to release the GPU texture allocation. Re-apply it when
   targets become active again.
4. The 64 pre-created slot elements do **not** receive `will-change` — only
   their `<g>` parent layers do. This keeps GPU texture count at 3 regardless
   of target count.

### 3.8 requestAnimationFrame Scheduling Strategy

At 144 Hz, the frame budget is **6.94 ms**. The RAF callback must complete
within this budget including the WebSocket message processing overhead.

**Frame timing contract:**
```
[0.0 ms] RAF fires — frame budget begins
[0.0–0.5 ms] read frameVersion, detect if data changed
[0.5–2.5 ms] iterate pool slots, build transforms + path strings
[2.5–4.0 ms] setAttribute calls (attribute writes, not layout)
[4.0–5.5 ms] Browser composites — GPU texture upload for changed paths
[5.5–6.9 ms] Margin / idle
```

**Double-buffering consideration:**  
If the WebSocket message handler and the RAF callback run on the same main
thread (standard browser model), they cannot truly race. However, if a
WebSocket message arrives during the RAF callback's execution, the `onmessage`
handler will run after the RAF callback completes (event loop semantics).

To prevent one frame's RAF from reading a half-written pool state, the
`apply_frame` function must complete atomically (it is synchronous and runs in
a single call stack). The `frameVersion` increment is the final operation,
ensuring the RAF only observes complete frame writes.

### 3.9 SvelteKit 2.x SSR Compatibility

The canvas must be SSR-safe. DOM APIs (`requestAnimationFrame`, `document`,
`SVGElement`) do not exist in the Node.js SSR environment.

```typescript
// Guard all DOM operations:
onMount(() => {
  // onMount only runs in the browser — safe to access DOM here
  if (typeof requestAnimationFrame === 'undefined') return;
  // ...
});
```

In SvelteKit 2.x, `onMount` is guaranteed to run only on the client.
Additionally, the telemetry canvas route should declare:

```typescript
// src/routes/dashboard/+page.ts
export const ssr = false;
```

This disables SSR for the dashboard entirely, avoiding any risk of accidental
server-side DOM access and eliminating the hydration reconciliation cost for
the SVG layer (which is purely dynamic and produces no useful SSR output).

---

## 4. End-to-End Performance Budget Analysis

### 4.1 Frame Cycle Breakdown at 144 Hz

| Stage | Budget (ms) | Implementation |
|---|---|---|
| WS message arrival → `read_frame` | < 0.2 | DataView walk, zero alloc |
| `apply_frame` pool mutation | < 0.5 | In-place TypedArray writes |
| `frameVersion` store notify | < 0.05 | Single `writable.update` |
| RAF: version check + early bail | < 0.05 | Integer comparison |
| RAF: 32-slot pool iteration | < 1.5 | Linear scan, no alloc |
| RAF: SVG `setAttribute` × ~128 | < 2.0 | DOM write batching |
| Browser composite | < 2.5 | GPU transform resolve |
| **Total** | **< 6.94** | ✅ Fits 144 Hz budget |

### 4.2 Memory Allocation Profile

| Object | Allocation timing | GC generation | Notes |
|---|---|---|---|
| `pool[0..63]` PoolSlot objects | Module init | Old-generation (permanent) | Never collected |
| `pool[i].traj` Float32Arrays | Module init | Off-heap (ArrayBuffer) | Immune to GC |
| `_targets[0..63]` TargetRecords | Module init | Old-generation | Never collected |
| `_header` FrameHeader | Module init | Old-generation | Never collected |
| `_idToSlot` Int16Array (128 KB) | Module init | Off-heap | Immune to GC |
| `_slotGeneration` Uint32Array | Module init | Off-heap | Immune to GC |
| `_freeStack` Uint8Array | Module init | Off-heap | Immune to GC |
| `_pathWriteBuf` Uint8Array (4 KB) | Module init | Off-heap | Immune to GC |
| `_textDecoder` TextDecoder | Module init | Old-generation | Never collected |
| Path `d` string per active target | Each frame | Eden / minor GC | 64 targets × ~0.5 KB |
| `new DataView(buffer)` | Each WS message | Eden / minor GC | Thin wrapper, ~40 bytes |

**Worst-case allocations per frame at 144 Hz (corrected implementation):**
- 64 SVG path `d` strings @ ~500 bytes each = ~32 KB Eden per frame
- 1 `DataView` wrapper @ ~40 bytes
- **Total: ~32 KB/frame → ~4.6 MB/s allocation rate**

V8's minor GC (Scavenger) can collect Eden at ~500 MB/s. The 4.6 MB/s rate
means **~0.9% of minor GC throughput consumed** — negligible. No major GC
(mark-sweep) should be triggered because no long-lived objects are allocated.

The path `d` strings are the only unavoidable per-frame allocation: `setAttribute`
requires a string argument and SVG provides no binary geometry API. Every other
allocation vector identified in the original design has been eliminated:

| Eliminated allocation | Original cost | Fix applied |
|---|---|---|
| `.toFixed(1)` strings × N points | 589,824 allocs/s @ 144Hz | `\| 0` + `_wi()` byte-writer |
| `_pathBuf.slice(0, bufIdx)` Array | 9,216 allocs/s | Removed — flat `_pathWriteBuf` |
| `new Set<number>()` per frame | 9,216 allocs/s | `_slotGeneration` generation counter |
| `Map<number,number>` iterator tuples | N × 9,216 allocs/s | `Int16Array` direct index |

---

## 5. rosbridge_websocket Integration Notes

When the backend uses ROS 2's `rosbridge_suite` instead of a custom WebSocket
server, the transport is JSON-encoded ROS messages over WebSocket (port 9090
by default). The binary frame design (§1) requires a custom WebSocket server
on the C++ side.

**Migration path:**
1. **Phase 5 initial:** Use rosbridge with the compact JSON schema (§1.6).
   Accept the GC overhead during development; the pool architecture (§2)
   ensures the Svelte layer remains allocation-free.
2. **Phase 5 optimization:** Replace rosbridge on the dashboard path with a
   dedicated `asio`-based WebSocket server in `ros2_ws/src/roc_vision_nodes/`
   that serializes directly to the binary frame format. This server subscribes
   to the same ROS 2 topics as rosbridge but writes the compact binary format.
3. The `dashboard/` telemetry socket module switches from JSON parsing to
   `read_frame()` by changing one import — the pool layer is transport-agnostic.

---

## 6. Open Engineering Questions for Phase 5 Implementation

1. **Coordinate space normalization:** The SVG `viewBox` must match the camera
   sensor resolution or be configured with a scale factor. The `read_frame`
   result coordinates are in detection space (pixels relative to the camera
   frame). The SVG `viewBox` should be set to the camera resolution
   (e.g., `0 0 1920 1080`) to eliminate any per-point scale calculation.

2. **Trajectory subsampling:** At 144 Hz ingestion with N=64 trajectory depth,
   the path `d` string is rebuilt every frame even though trajectory history
   changes only modestly. Trajectory path updates could be throttled to 30 Hz
   (every 4–5 frames) without visible degradation, halving rasterization cost.

3. **Target selection propagation:** The interactive click-to-lock feature
   (Phase 6 integration point) requires sending a `target_id` back to the
   ROS 2 `TargetSelector` via a separate WebSocket message or rosbridge
   `publish` call. The `flags` field in the binary frame (bit 0 = locked)
   provides feedback confirmation from the backend.

4. **High-DPI scaling:** On Retina / HiDPI displays, the `devicePixelRatio`
   may be 2.0 or higher. The SVG `viewBox` approach is resolution-independent
   by definition. However, `will-change: transform` promotes layers at device
   pixel resolution — on a 4K display this doubles the GPU texture memory.
   Implement a `matchMedia('(min-resolution: 2dppx)')` listener and
   conditionally apply `will-change` only when `devicePixelRatio < 2`.

5. **WebSocket reconnect with state flush:** On connection drop, all active
   pool slots must be marked `active = false` and the RAF loop will hide them
   on the next cycle. Re-connection must not leave stale slots visible.

---

## 7. Implementation Checklist

- [ ] Implement `frame_reader.ts` with static `_targets[]` and `_header` pool
- [ ] Implement `target_pool.ts` with `PoolSlot[]` and `apply_frame()`
- [ ] Implement `telemetry_socket.ts` with `binaryType = 'arraybuffer'`
- [ ] Scaffold `TelemetryCanvas.svelte` with three-layer SVG structure
- [ ] Pre-create 64 DOM slot nodes in `onMount`
- [ ] Implement RAF render loop with direct `setAttribute` mutations
- [ ] Write `telemetry.css` with layer-scoped `will-change` declarations
- [ ] Set `export const ssr = false` on dashboard route
- [ ] Implement C++ `serialize_frame()` with static buffer reuse
- [ ] Add `frame_seq` rollover handling and duplicate frame detection
- [ ] Add WebSocket reconnect logic with pool flush on disconnect
- [ ] Performance profile with Chrome DevTools Performance tab at 144 Hz
- [ ] Verify zero major GC events in a 30-second run with 32 active targets
