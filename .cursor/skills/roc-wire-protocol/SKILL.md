---
name: roc-wire-protocol
description: >-
  Binary wire protocol between the C++ kinematics engine and the SvelteKit
  dashboard. Covers exact byte layout, endianness, DataView decoding,
  TrackPool SoA architecture, and status-flag semantics. Use when working on
  wire_format.hpp, wire_decoder.ts, track_pool.svelte.ts, mock_frame_source.ts,
  or any code that serialises/deserialises the telemetry frame.
disable-model-invocation: true
---

# ROC Binary Wire Protocol

## Frame Layout (§7)

All multi-byte fields are **little-endian** (deployment target is x86-64).

### Header (12 bytes, §7.1)

| Offset | Size | Type  | Field                | Notes |
|--------|------|-------|----------------------|-------|
| 0      | 1    | u8    | `version`            | Must be `1` |
| 1      | 1    | u8    | `flags`              | Bit 0 = NIS-degraded frame, Bit 1 = boundary probation present |
| 2      | 4    | u32   | `frame_id`           | Monotonic frame counter |
| 6      | 2    | u16   | `num_tracks`         | Track count in this frame |
| 8      | 4    | i32   | `t_capture_ms_offset`| Signed offset from session epoch (ms) |

### Per-Track Payload (86 bytes, §7.2)

| Offset | Size | Type  | Field            | Notes |
|--------|------|-------|------------------|-------|
| 0      | 8    | u64   | `track_id`       | Split into lo(0–3) + hi(4–7) in JS to avoid BigInt |
| 8      | 8    | u64   | `parent_id`      | 0 = no parent; split lo/hi same way |
| 16     | 1    | u8    | `label`          | 0=Unknown, 1=Person, 2=Head, 3=UpperTorso, 4=Hat, 5=Backpack |
| 17     | 1    | u8    | `status_flags`   | See status bits table below |
| 18     | 4    | f32   | `confidence`     | Detection confidence [0, 1] |
| 22     | 12   | f32×3 | `p_world[3]`     | World-frame position (metres) |
| 34     | 12   | f32×3 | `v_world[3]`     | World-frame velocity (m/s) |
| 46     | 12   | f32×3 | `a_world[3]`     | World-frame acceleration (m/s²) |
| 58     | 12   | f32×3 | `P_pos_diag[3]`  | Position covariance diagonal (uncertainty) |
| 70     | 16   | f32×4 | `bbox_orig[4]`   | Pixel-space: u, v, w_px, h_px |

**Total: 8+8+1+1+4+12+12+12+12+16 = 86 bytes**

### Status Flag Bits

| Bit | Mask | Name                  | Meaning |
|-----|------|-----------------------|---------|
| 0   | 0x01 | `kConfirmed`          | Track has ≥ 3 hits (promoted from Tentative) |
| 1   | 0x02 | `kBoundaryProbation`  | Edge-clipped detection, centroid-only update active |
| 2   | 0x04 | `kStationaryRegime`   | NIS EMA below η_low, shrinkage active |
| 3   | 0x08 | `kHierarchyViolation` | Parent↔sub-feature offset exceeded 3σ gate |

## C++ Serialisation

Source: `core/kinematics_engine/include/roc/kinematics/wire_format.hpp`

```cpp
// Constants
inline constexpr int kWireHeaderSize = 12;
inline constexpr int kWireTrackSize  = 86;

// Preferred zero-copy path (avoids copying 51 KB EmbeddingGallery per track):
serialize_frame_indexed(out_buf, header, track_pool, track_indices);

// Convenience path (resizes output vector internally):
auto bytes = serialize_frame(header, tracks);
```

Buffer sizing: `kWireHeaderSize + num_tracks * kWireTrackSize`.

## JavaScript Decoding

Source: `dashboard/src/lib/telemetry/wire_decoder.ts`

### Critical Rules

1. **Always pass `true` for littleEndian** in every `DataView.get*` call. Omitting it silently corrupts data.
2. **Split u64 into two u32 reads** (lo at offset+0, hi at offset+4) to avoid BigInt allocation.
3. **Re-use the DataView** across frames when the underlying `ArrayBuffer` pointer hasn't changed.

### Decoding Pattern

```typescript
const view = new DataView(buf);

// Header
const version   = view.getUint8(0);
const flags     = view.getUint8(1);
const frameId   = view.getUint32(2, true);
const numTracks = view.getUint16(6, true);

// Per-track
for (let i = 0; i < numTracks; i++) {
  const base = 12 + i * 86;
  const idLo      = view.getUint32(base + 0,  true);
  const idHi      = view.getUint32(base + 4,  true);
  const parentLo  = view.getUint32(base + 8,  true);
  const parentHi  = view.getUint32(base + 12, true);
  const label     = view.getUint8(base + 16);
  const status    = view.getUint8(base + 17);
  const conf      = view.getFloat32(base + 18, true);
  const px        = view.getFloat32(base + 22, true);
  // ... py @26, pz @30, vx @34, vy @38, vz @42,
  //     ax @46, ay @50, az @54, Pd0 @58, Pd1 @62, Pd2 @66,
  //     bbox_u @70, bbox_v @74, bbox_w @78, bbox_h @82
}
```

## TrackPool SoA Architecture

Source: `dashboard/src/lib/stores/track_pool.svelte.ts`

64-slot Structure-of-Arrays store (~14 KB, L1d-resident):

| Column | Type | Stride | Purpose |
|--------|------|--------|---------|
| `trackIdLo/Hi` | Uint32Array(64) | 1 | u64 track ID, split |
| `parentIdLo/Hi` | Uint32Array(64) | 1 | u64 parent ID, split |
| `label` | Uint8Array(64) | 1 | Label enum |
| `statusFlags` | Uint8Array(64) | 1 | Status bits |
| `confidence` | Float32Array(64) | 1 | Detection confidence |
| `pWorld` | Float32Array(192) | 3 | `[slot*3 + {0,1,2}]` |
| `vWorld` | Float32Array(192) | 3 | Velocity |
| `aWorld` | Float32Array(192) | 3 | Acceleration |
| `pPosDiag` | Float32Array(192) | 3 | Covariance diagonal |
| `bboxOrig` | Float32Array(256) | 4 | `[slot*4 + {0,1,2,3}]` for u,v,w,h |
| `trajU/V` | Float32Array(1024) | 16 | 16-entry trajectory ring |
| `presenceBitmap` | Uint8Array(64) | 1 | 1 = active, 0 = free |

### Slot Management

- **Find:** O(64) linear scan of `presenceBitmap` + `trackIdLo/Hi` match.
- **Allocate:** first free slot (presenceBitmap[s] === 0).
- **Reclaim:** stale after 60 frames (~2 s @ 30 Hz).
- No `Map<id, slot>` — avoids per-frame allocation.

### Reactive Surface (Svelte 5 Runes)

Only 4 `$state` runes: `presenceVersion`, `frameSeq`, `frameFlags`, `activeCount`. Typed-array reads in the RAF canvas loop are **outside** Svelte's reactivity graph — zero reactive overhead on the hot path.

## Key Files

| File | Language | Role |
|------|----------|------|
| `core/.../wire_format.hpp` | C++ | Constants, FrameHeader struct, serialize/deserialize |
| `core/.../wire_format.cpp` | C++ | Serialisation implementation |
| `dashboard/.../wire_decoder.ts` | TypeScript | Zero-alloc DataView decoder + DEV verify_layout() |
| `dashboard/.../track_pool.svelte.ts` | Svelte TS | 64-slot SoA singleton store |
| `dashboard/.../mock_frame_source.ts` | TypeScript | 30 Hz synthetic frame generator for dev |

## Extending the Protocol

1. Bump `kWireProtocolVersion` in both C++ and TypeScript.
2. Maintain 86 B/track alignment or define a new payload size constant.
3. Update both `serialize_frame*` in C++ and `decode_frame` in TypeScript **simultaneously**.
4. Run `verify_layout()` (auto-runs in DEV mode) to catch offset drift.
5. Update `test_wire_format.cpp` layout assertions.
