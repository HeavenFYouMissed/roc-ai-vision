# IPC & Middleware Architectural Analysis — ROC AI Vision

> **Phase 4 Research Sprint — May 2026**
> Decision document for the inter-component communication layer on a single
> Windows x64 host: one camera → one inference pipeline → one kinematics
> engine → one actuator → one SvelteKit dashboard.

---

## Table of Contents

1. [ROS 2 on Windows — Current State](#1-ros-2-on-windows--current-state)
2. [Eclipse iceoryx (v1)](#2-eclipse-iceoryx-v1)
3. [Eclipse iceoryx2 (Hourglass)](#3-eclipse-iceoryx2-hourglass)
4. [ZeroMQ / nanomsg / nng](#4-zeromq--nanomsg--nng)
5. [Direct Shared Memory + Lock-Free Queues](#5-direct-shared-memory--lock-free-queues)
6. [gRPC / Cap'n Proto / FlatBuffers (Dashboard Bridge)](#6-grpc--capn-proto--flatbuffers-dashboard-bridge)
7. [Comparison Matrix](#7-comparison-matrix)
8. [Hybrid Architecture Recommendation](#8-hybrid-architecture-recommendation)
9. [Migration Effort Assessment](#9-migration-effort-assessment)
10. [Decision Record](#10-decision-record)

---

## 1. ROS 2 on Windows — Current State

### 1.1 Best-Supported Distro (2026)

**ROS 2 Jazzy Jalisco** is the current LTS (May 2024 → May 2029) and the
strongest choice for Windows:

| Property | Value |
|---|---|
| Windows tier | **Tier 1** (amd64, VS 2019) |
| Default RMW | `rmw_fastrtps_cpp` (eProsima Fast-DDS) |
| Alt RMW | `rmw_cyclonedds_cpp`, `rmw_zenoh_cpp` (Quality Level 2) |
| Patch cadence | Patch 7 shipped Jan 2026; active maintenance through 2029 |

Rolling Ridley tracks HEAD and is useful for development but **not recommended
for production** — breaking API changes land without notice.

### 1.2 The colcon + MSVC Pain

The Windows build story for ROS 2 in 2026 remains the weakest link in the
ecosystem. Documented, persistent issues include:

| Pain Point | Detail |
|---|---|
| **Path length limits** | Windows 260-char MAX_PATH. Must use short roots (`C:\dev\ros2`) and `--merge-install`. Deep `colcon` package paths routinely hit the ceiling. |
| **Symbol visibility** | MSVC requires explicit `__declspec(dllexport/dllimport)` on every public symbol. Linux packages that forget `VISIBILITY_CONTROL` headers fail to link. Many third-party packages are never tested on Windows. |
| **CycloneDDS warnings→errors** | CycloneDDS enum mismatches trigger C5286/C5287 with `/W4`. Suppressing them with `/wd` introduces C2220/C4530 in other translation units. |
| **colcon crawl failures** | `colcon` package discovery on Windows triggers `WNDPROC` → `LRESULT` type errors on some machines (colcon-core #543). No reliable fix; workaround is to pin colcon versions. |
| **Missing binary packages** | Pre-built `.zip` installer includes only `ros-base` + partial `desktop`. Packages like `image_transport`, `cv_bridge`, and `rosbridge_suite` require source builds, which themselves fail due to the above issues. |
| **No conda support** | Still a "future goal" as of late 2025. |
| **Admin install path** | Binary installer hard-codes `C:\opt\ros\jazzy` and requires admin elevation. No relocatable installs. |

### 1.3 rosbridge_server — The Dashboard Bottleneck

The standard ROS 2 → WebSocket bridge is `rosbridge_server` (Python,
`rclpy` + `tornado`). Known production issues:

- **Memory leaks**: ~1 GB/day on continuous streaming (documented by the
  `rws` project).
- **CPU overhead**: JSON serialization of every message on the Python GIL.
- **No binary frames**: Default protocol is JSON over text frames. CBOR
  support exists but is poorly documented.
- **Latency**: 2–10 ms per message hop through the Python bridge, on top
  of DDS latency.

Alternatives exist (`rws` in C++, Foxglove Bridge), but they still route
through DDS, adding serialization/deserialization at the ROS boundary.

### 1.4 DDS Latency on a Single Machine

Even with intra-process composition (all nodes in one `rclcpp` process),
ROS 2 DDS middleware adds measurable overhead:

| Scenario | Fast-DDS Latency | CycloneDDS Latency |
|---|---|---|
| 2 MB @ 30 Hz (intra-process) | ~1.06 ms | ~1.71 ms |
| Small payload (< 1 KB) | ~50–100 µs | ~80–150 µs |
| Zero-copy (shared memory transport) | ~10–30 µs | N/A (not supported) |

For reference, a raw `memcpy` of our 86-byte wire frame takes **< 20 ns**.

### 1.5 What ROS 2 *Would* Give Us

- **Ecosystem**: `rviz2`, `rosbag2`, `rqt` debugging tools.
- **Standard message types**: `sensor_msgs`, `geometry_msgs`.
- **Parameter server**: Runtime reconfiguration.
- **Lifecycle nodes**: Managed startup/shutdown.
- **Multi-machine ready**: If we ever distribute across hosts.

### 1.6 Verdict

For a single-machine system where every component already exists as a C++20
library (kinematics engine) or will be a C++ module (inference engine,
actuator driver), ROS 2 on Windows introduces:

- **~50–100 µs of unnecessary latency** per hop (vs. < 100 ns for shared memory).
- **Days of build system friction** fighting colcon/MSVC/DLL export issues.
- **A Python dependency** (rosbridge) in the only network-facing path (dashboard).
- **~200 MB of disk** for a minimal ROS 2 install.

The ecosystem benefits (rviz2, rosbag2) are valuable for multi-machine
distributed robotics but **provide minimal value for our architecture** where
all telemetry is already visualized in the SvelteKit dashboard and all
data is consumed in the same process address space.

---

## 2. Eclipse iceoryx (v1)

### 2.1 Architecture

iceoryx v1 is a true zero-copy IPC middleware. Its core design:

```
┌─────────┐     ┌─────────┐     ┌─────────┐
│ App A   │     │  RouDi   │     │ App B   │
│Publisher│◄───►│ (Daemon) │◄───►│Subscriber│
└────┬────┘     └────┬────┘     └────┬────┘
     │               │               │
     └───────────────┼───────────────┘
              Shared Memory Pool
           (pre-allocated chunks)
```

- **RouDi daemon**: Central orchestrator that creates and manages shared
  memory segments. Must be running before any application.
- **Loan-based API**: Publisher calls `loan()` → gets a pointer into shared
  memory → writes data in-place → calls `publish()`. No copy.
- **Subscriber**: Calls `take()` → gets a pointer to the same memory. Zero
  copy, zero serialization.

### 2.2 API Surface

```cpp
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"

// Publisher side
iox::popo::Publisher<TrackFrame> publisher({"ROC", "Vision", "Tracks"});
publisher.loan()
    .and_then([](auto& sample) {
        sample->header = ...;
        sample->tracks[0] = ...;
        sample.publish();
    });

// Subscriber side
iox::popo::Subscriber<TrackFrame> subscriber({"ROC", "Vision", "Tracks"});
subscriber.take()
    .and_then([](const auto& sample) {
        process(sample->header, sample->tracks);
    });
```

### 2.3 Windows Support

iceoryx v1 has **experimental** Windows support. The POSIX shared memory
abstraction layer works via `CreateFileMapping`, but:

- RouDi daemon must run as a separate process.
- Windows builds require CMake + MSVC or MinGW.
- Some POSIX features (named semaphores, `mmap` flags) are emulated
  imperfectly.
- The project is **in maintenance mode** — new development targets iceoryx2.

### 2.4 Performance

| Metric | Value |
|---|---|
| Latency (small payload, Linux) | ~1 µs |
| Latency (large payload, Linux) | ~1 µs (zero-copy — size-independent) |
| Throughput | Limited by memory bandwidth, not protocol |
| Memory overhead | Pre-allocated pools (configurable) |

### 2.5 Limitations for ROC

- **RouDi daemon** adds operational complexity (must start before apps).
- **Static pool configuration** requires knowing message sizes at build time.
- **Inter-process only** — for inter-thread communication within the same
  process, the shared memory machinery is unnecessary overhead.
- **Maintenance mode** — no new features, bugs may not be fixed promptly.

---

## 3. Eclipse iceoryx2 (Hourglass)

### 3.1 Architecture — Daemon-Free

The key architectural improvement over v1: **no RouDi daemon**.

```
┌─────────┐                    ┌─────────┐
│ App A   │   Shared Memory    │ App B   │
│Publisher│◄──────────────────►│Subscriber│
└─────────┘   (per-publisher   └─────────┘
               memory pools)
```

Each publisher manages its own memory pool. Discovery is peer-to-peer via
shared memory metadata files. This eliminates the single point of failure
and startup ordering dependency of iceoryx v1.

### 3.2 Current Status (May 2026)

| Property | Value |
|---|---|
| Latest release | v0.8.1 (Jan 2026 pre-release) |
| Core language | Rust (~71% of codebase) |
| C++ bindings | Available (C++14 minimum, ~23% of codebase) |
| C bindings | Available |
| Python bindings | Available |
| Windows support | **Supported** (64-bit); 32-bit WIP |
| Messaging patterns | Pub/sub, events, request-response, blackboard |

### 3.3 Performance

From the iceoryx2 benchmark discussions (2024–2025):

| Metric | Value |
|---|---|
| Small payload latency | ~50 ns (measured on optimized Linux; CPU perf governor) |
| Large payload latency | ~50 ns (true zero-copy — size-independent) |
| Lock-free | Yes (wait-free for the fast path) |
| Memory ordering | Acquire-release semantics |

### 3.4 Windows Caveats

- Stale shared memory state files can accumulate (fixed in v0.6.0).
- Maximum shared memory size was artificially limited (increased in v0.6.0).
- Win32 API event handling had bugs (fixed in v0.6.0).
- 32-bit builds not yet fully working.
- CI coverage for Windows is less comprehensive than Linux.

### 3.5 C++ Binding Quality

The C++ bindings wrap the Rust core via C FFI. The API follows the builder
pattern:

```cpp
#include "iox2/service.hpp"

auto node = NodeBuilder().create<ServiceType::Ipc>().expect("");
auto service = node.service_builder(ServiceName::create("ROC/Tracks").expect(""))
    .publish_subscribe<TrackFrame>()
    .open_or_create()
    .expect("");

auto publisher = service.publisher_builder().create().expect("");
auto sample = publisher.loan_uninit().expect("");
// Write directly into shared memory — zero copy
new (sample.payload_mut()) TrackFrame{...};
sample.assume_init().send().expect("");
```

### 3.6 Assessment for ROC

**Pros:**
- True zero-copy with ~50 ns latency.
- No daemon — simpler operational model.
- Windows 64-bit supported.
- Active development with regular releases.
- Request-response pattern available (useful for dashboard commands).

**Cons:**
- Rust toolchain required for building (even for C++ consumers).
- C++ bindings are functional but less ergonomic than native C++.
- Windows support is maturing but not battle-tested.
- Only useful for **inter-process** communication; for inter-thread within
  the same process, it's unnecessary overhead.
- Community is smaller than ROS 2 or ZeroMQ.

---

## 4. ZeroMQ / nanomsg / nng

### 4.1 Architecture Overview

These are **brokerless** messaging libraries providing socket-like abstractions
over multiple transports (TCP, IPC, inproc, WebSocket).

| Library | Language | Transport | Pattern Support |
|---|---|---|---|
| **ZeroMQ (libzmq)** | C with C++ binding (cppzmq) | tcp, ipc, inproc, pgm, tipc | PUB/SUB, REQ/REP, PUSH/PULL, DEALER/ROUTER, PAIR |
| **nanomsg** | C | tcp, ipc, inproc, ws | Same patterns; **discontinued** |
| **nng** | C (nanomsg-next-gen) | tcp, ipc, inproc, ws, tls | Same patterns + native WebSocket |

### 4.2 Performance (2025 Benchmark — Hitachi Energy / arXiv:2508.07934)

| Metric | ZeroMQ | nng | Notes |
|---|---|---|---|
| Throughput (PUSH/PULL) | ~700K msg/s | ~35K msg/s | ZeroMQ 20x faster |
| Latency (small message) | ~68 µs | ~140 µs | Over `ipc://` transport |
| Latency (inproc) | ~5–15 µs | ~20–40 µs | In-process thread-to-thread |
| Memory footprint | Moderate | Small | nng is more lightweight |

### 4.3 Transport Flexibility

The key advantage: transparent transport switching.

```cpp
// Inter-thread (zero-copy within process)
zmq_bind(pub, "inproc://tracks");

// Inter-process (same machine, Unix domain socket / named pipe)
zmq_bind(pub, "ipc:///tmp/roc-tracks");

// Network (if we ever go multi-machine)
zmq_bind(pub, "tcp://*:5555");
```

nng adds native WebSocket transport:
```c
nng_listen(sock, "ws://0.0.0.0:8080/tracks", NULL, 0);
```

### 4.4 Wire Protocol

ZeroMQ/nng are **message-oriented** — they frame messages but impose **no
serialization format**. We can send our existing 86-byte wire format
directly as a ZeroMQ message with zero modifications.

### 4.5 Windows Support

| Library | Windows | Build System | MSVC |
|---|---|---|---|
| ZeroMQ | Full (Tier 1) | CMake | Native MSVC support |
| nng | Full | CMake | Native MSVC support |
| nanomsg | Full (deprecated) | CMake | Supported |

ZeroMQ on Windows is rock-solid — used in production by financial trading
systems, Microsoft's own tooling, and countless Windows applications.

### 4.6 Assessment for ROC

**ZeroMQ** is the clear winner in this category:

**Pros:**
- Mature, battle-tested, excellent Windows support.
- `inproc://` transport gives ~5–15 µs latency (adequate, not optimal).
- Zero serialization overhead — pass raw bytes.
- Can send our existing wire format unchanged.
- Native multi-pattern support (pub/sub for telemetry, req/rep for commands).
- Easy to add a WebSocket gateway (or use nng's native ws:// transport).

**Cons:**
- `inproc://` still involves a memcpy (~5–15 µs), not true zero-copy.
- Adds a library dependency (~2 MB).
- Message passing semantics, not shared memory — each subscriber gets a copy.
- Overkill for inter-thread communication where a lock-free queue suffices.

---

## 5. Direct Shared Memory + Lock-Free Queues

### 5.1 When This Is the Right Answer

When all communicating components live in the **same process** on the **same
machine**, and the data path is **latency-critical** (inference → tracker →
actuator), there is no reason to involve any middleware at all.

Our hot path:

```
Camera Thread → [SPSC Queue] → Inference Thread → [SPSC Queue] →
    Tracker Thread → [SPSC Queue] → Actuator Thread
                  └──[SPSC Queue]──→ Dashboard Egress Thread
```

### 5.2 Library Options

| Library | Type | Latency | Allocation | Header-only |
|---|---|---|---|---|
| **rigtorp::SPSCQueue** | Wait-free SPSC | ~8 ns | Pre-allocated ring | Yes (C++11) |
| **moodycamel::ReaderWriterQueue** | Lock-free SPSC | ~10 ns | Pre-allocated ring | Yes (C++11) |
| **moodycamel::ConcurrentQueue** | Lock-free MPMC | ~20–40 ns | Per-thread sub-queues | Yes (C++11) |
| **boost::lockfree::spsc_queue** | Lock-free SPSC | ~15 ns | Pre-allocated ring | Header-only (Boost) |
| **folly::ProducerConsumerQueue** | Wait-free SPSC | ~8 ns | Pre-allocated ring | Requires folly |

### 5.3 Implementation Detail

For the ROC hot path, we need exactly **SPSC** (one producer, one consumer
per link). The implementation is trivial:

```cpp
#include "rigtorp/SPSCQueue.h"

// 64-slot ring buffer, pre-allocated, zero heap on hot path
rigtorp::SPSCQueue<DetectionBatch> inference_to_tracker{64};

// Producer (inference thread)
inference_to_tracker.push(batch);  // wait-free, ~8 ns

// Consumer (tracker thread)
if (auto* batch = inference_to_tracker.front()) {
    process(*batch);
    inference_to_tracker.pop();
}
```

### 5.4 Cache-Line Optimization

All quality SPSC implementations place the read index and write index on
**separate cache lines** (64 bytes apart) to eliminate false sharing:

```
Cache Line 0:  [write_idx | padding...]   ← producer-hot
Cache Line 1:  [read_idx  | padding...]   ← consumer-hot
Cache Line 2+: [ring buffer data...]      ← shared
```

This is why `rigtorp::SPSCQueue` achieves ~8 ns per operation — the
producer and consumer never contend on the same cache line.

### 5.5 For Inter-Process (If Needed)

If we ever split into separate processes, the SPSC queue can be placed in
Windows shared memory:

```cpp
HANDLE hMap = CreateFileMappingW(
    INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, size, L"ROC_Tracks");
void* buf = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, size);

// Placement-new the SPSC queue into shared memory
auto* queue = new (buf) SPSCQueue<TrackFrame>(64);
```

Or use `boost::interprocess::managed_windows_shared_memory` for a
higher-level API with named object construction.

### 5.6 Assessment for ROC

**This is the optimal choice for the hot path.**

| Metric | Value |
|---|---|
| Latency | ~8 ns per operation |
| Throughput | >100M messages/s |
| Memory | Zero heap allocation at steady state |
| Complexity | Single header file, ~200 LOC |
| Windows support | Native (pure C++ atomics, no OS dependencies) |
| Serialization | None — pass C++ structs by value or pointer |

The only thing SPSC queues **cannot** do is communicate across a network
boundary (i.e., to the browser dashboard). That requires a complementary
transport.

---

## 6. gRPC / Cap'n Proto / FlatBuffers (Dashboard Bridge)

The dashboard is the **only component** that requires a network protocol,
because the browser runs in a separate process with its own memory space
and communicates via WebSocket (or WebTransport).

### 6.1 Current State: Custom Wire Format

The kinematics engine already defines a binary wire format
(`wire_format.hpp`):

```
┌────────────────────┬────────────────┬─────────────────┐
│  Header (12 B)     │  Track 0 (86 B)│  Track 1 (86 B) │ ...
│  frame_id: u32     │  track_id: u16 │                 │
│  timestamp: f64    │  label: u8     │                 │
│  track_count: u16  │  status: u8    │                 │
│  reserved: u16     │  x,y,z: f32×3  │                 │
│                    │  vx,vy,vz: f32 │                 │
│                    │  ax,ay,az: f32 │                 │
│                    │  bbox: f32×4   │                 │
│                    │  conf: f32     │                 │
│                    │  embed: f16×8  │                 │
│                    │  cov_diag: f32×3│                 │
│                    │  flags: u16    │                 │
└────────────────────┴────────────────┴─────────────────┘
```

At N=50 tracks: 12 + 50×86 = **4,312 bytes** — fits in 3 Ethernet MTUs.

### 6.2 Option Analysis

#### gRPC

| Aspect | Assessment |
|---|---|
| Browser support | gRPC-Web requires a proxy (Envoy/grpc-web). No native browser gRPC. |
| Streaming | Server-side streaming works well for telemetry push. |
| Overhead | Protobuf serialization + HTTP/2 framing. ~10–50 µs per message. |
| Complexity | Requires protobuf schema, code generation, gRPC server setup. |
| **Verdict** | **Rejected.** The proxy requirement and protobuf overhead provide no benefit over raw WebSocket + binary frames for a single dashboard. |

#### Cap'n Proto

| Aspect | Assessment |
|---|---|
| Browser support | Cap'n Web (Cloudflare, 2025) provides TypeScript RPC over WebSocket — but uses **JSON**, not binary. Original Cap'n Proto has no maintained JS implementation. |
| Zero-copy | In C++: yes. In JavaScript: no (must deserialize from ArrayBuffer). |
| Complexity | Schema compiler, C++ generated code, custom RPC server. |
| **Verdict** | **Rejected.** The JavaScript story is fragmented (Cap'n Web ≠ Cap'n Proto). The C++ schema compiler adds build complexity for minimal gain over our existing wire format. |

#### FlatBuffers

| Aspect | Assessment |
|---|---|
| Browser support | **Native TypeScript/JavaScript** support. 2.7M weekly npm downloads. |
| Zero-copy (C++) | Yes — `GetRoot<T>(buf)` reads directly from buffer, no deserialization. |
| Zero-copy (JS) | **Partial** — uses `ByteBuffer` wrapper over `Uint8Array`. Field access is lazy (no upfront parse), but each access calls `DataView.getFloat32()` etc. |
| Wire efficiency | Schema-defined, includes vtable overhead. A FlatBuffers encoding of our 86-byte track record would be ~100–110 bytes (vtable + alignment padding). |
| Schema evolution | Forwards/backwards compatible field addition. |
| Code generation | `flatc` compiler generates C++ headers and TypeScript classes. |

**FlatBuffers vs. Custom Wire Format — Detailed Comparison:**

| Metric | Custom Wire (current) | FlatBuffers |
|---|---|---|
| Bytes per track | **86** | ~100–110 (vtable overhead) |
| Frame @ 50 tracks | **4,312 B** | ~5,200–5,700 B |
| C++ write cost | Raw `memcpy` (~20 ns) | Builder allocation + `CreateVector` (~200–500 ns) |
| C++ read cost | Cast + offset (~5 ns) | Accessor call chain (~10–20 ns) |
| JS read cost | `DataView.getFloat32()` (~10 ns/field) | `flatbuffers.ByteBuffer` + accessor (~15 ns/field) |
| Schema evolution | Manual versioning | Built-in forwards/backwards compat |
| Type safety | Manual (`static_assert` on sizes) | Generated accessors, compile-time checked |

**Verdict:** FlatBuffers is a reasonable choice **if** we need schema evolution
or multi-language type safety. For our use case — a fixed 86-byte struct
that's already defined, implemented, tested, and fits in 1 MTU — FlatBuffers
adds ~20–30% wire overhead and build-time code generation for no functional
benefit. **Keep the custom wire format.**

### 6.3 Recommended Dashboard Transport

**Native C++ WebSocket server** sending raw binary frames of the existing
wire format.

Best library options:

| Library | Language | Binary Frames | Perf | Windows |
|---|---|---|---|---|
| **uWebSockets** | C++17 | Yes (`BINARY` opcode) | 12x Node.js, 100k+ concurrent | Yes (via µSockets) |
| **Boost.Beast** | C++11 | Yes | Good, mature | Yes (native Boost) |
| **libwebsockets** | C | Yes | Good, wide platform support | Yes |
| **IXWebSocket** | C++11 | Yes | Good, simple API | Yes |

**uWebSockets** is the recommended choice: highest performance, header-only
C++17, and minimal dependency footprint. Single-threaded async model aligns
perfectly with our architecture (one dashboard egress thread).

### 6.4 WebTransport — Future Option

WebTransport (HTTP/3 over QUIC) became **Baseline Newly Available** in March
2026 (Chrome 97+, Edge 97+, Firefox 114+, Safari 26.4+). It offers:

- **Unreliable datagrams** — perfect for telemetry where occasional loss is
  acceptable (30 Hz updates, losing 1 frame is invisible).
- **Multiple concurrent streams** — separate channels for tracks, commands,
  config.
- **Lower head-of-line blocking** than TCP-based WebSocket.

However, no mature C++ WebTransport server library exists yet. The closest
is `ngtcp2` (C, requires C++23 for examples). **Revisit in late 2026** when
server libraries mature. For now, WebSocket is the pragmatic choice.

---

## 7. Comparison Matrix

### 7.1 Hot Path (Inference → Tracker → Actuator)

| Option | Latency/hop | Throughput | Serialization | Windows | Complexity | Dependencies |
|---|---|---|---|---|---|---|
| **SPSC Queue** | **~8 ns** | **>100M msg/s** | **None** | **Native** | **Trivial** | **1 header** |
| iceoryx2 (IPC) | ~50 ns | Very high | None (zero-copy) | Supported | Moderate | Rust toolchain + C++ bindings |
| iceoryx v1 (IPC) | ~1 µs | High | None (zero-copy) | Experimental | Moderate | RouDi daemon |
| ZeroMQ (inproc) | ~5–15 µs | ~700K msg/s | None (raw bytes) | Full | Low | libzmq (~2 MB) |
| ROS 2 (intra-proc) | ~50–100 µs | Varies | CDR serialization | Tier 1 (painful) | **High** | Full ROS 2 install (~200 MB) |
| ROS 2 (inter-proc) | ~1–2 ms | Varies | CDR + DDS | Tier 1 (painful) | **High** | Full ROS 2 install |

### 7.2 Dashboard Bridge (C++ → Browser)

| Option | Latency | Wire Overhead | Browser Support | Windows | Complexity |
|---|---|---|---|---|---|
| **WS + Custom Binary** | **~0.1–1 ms** | **0 bytes** | **Native** | **Full** | **Low** |
| WS + FlatBuffers | ~0.1–1 ms | +20–30% | Native (npm) | Full | Medium (flatc codegen) |
| gRPC-Web | ~1–5 ms | Protobuf + HTTP/2 | Via proxy only | Full | High |
| rosbridge_server | ~2–10 ms | JSON text | Via roslib.js | Via ROS 2 | High |
| WebTransport | ~0.05–0.5 ms | 0 bytes | Baseline 2026 | No C++ server | High (immature) |

### 7.3 Overall Scoring (1=best, 5=worst)

| Criterion | ROS 2 | iceoryx2 | ZeroMQ | SPSC + WS | Hybrid (SPSC+WS) |
|---|---|---|---|---|---|
| Hot-path latency | 5 | 2 | 3 | **1** | **1** |
| Dashboard latency | 4 | 3 | 2 | **1** | **1** |
| Windows build pain | 5 | 3 | **1** | **1** | **1** |
| Dependency weight | 5 | 3 | 2 | **1** | **1** |
| Learning curve | 4 | 3 | 2 | **1** | **1** |
| Ecosystem/tooling | **1** | 4 | 3 | 5 | 4 |
| Multi-machine ready | **1** | 2 | **1** | 5 | 3 |
| **Total** | **25** | **20** | **14** | **15** | **12** |

The hybrid approach wins because it combines the optimal tool for each path.

---

## 8. Hybrid Architecture Recommendation

### 8.1 The Architecture

```
                        SINGLE PROCESS (roc_vision.exe)
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│  Camera Thread ──[SPSC]──► Inference Thread ──[SPSC]──► Tracker    │
│                                                          Thread     │
│                                                            │        │
│                                                   ┌────────┼────┐   │
│                                                   │ [SPSC] │    │   │
│                                                   ▼        ▼    │   │
│                                              Actuator   Dashboard│   │
│                                              Thread     Egress   │   │
│                                                │       Thread    │   │
│                                                │         │       │   │
└────────────────────────────────────────────────┼─────────┼───────┘   │
                                                 │         │           │
                                            Serial Port   WebSocket   │
                                            (COM3)        :8765       │
                                                           │           │
                                                      ┌────▼─────┐    │
                                                      │ Browser  │    │
                                                      │Dashboard │    │
                                                      └──────────┘    │
```

### 8.2 Component Details

#### Hot Path: SPSC Ring Buffers

| Link | Queue Type | Capacity | Payload |
|---|---|---|---|
| Camera → Inference | `rigtorp::SPSCQueue<FrameBatch>` | 4 slots | Raw frame pointer + metadata |
| Inference → Tracker | `rigtorp::SPSCQueue<DetectionBatch>` | 16 slots | Detection array (flat struct) |
| Tracker → Actuator | `rigtorp::SPSCQueue<ActuatorCommand>` | 8 slots | Target ID + (dx, dy) + intercept |
| Tracker → Dashboard Egress | `rigtorp::SPSCQueue<WireFrame>` | 4 slots | Pre-serialized 86 B/track binary |

Total latency, camera-to-actuator: **~32 ns** (4 hops × ~8 ns/hop).
Compare to ROS 2: **~200–400 µs** (4 hops × 50–100 µs/hop).

#### Dashboard Bridge: uWebSockets + Custom Binary

```cpp
// Dashboard egress thread
uWS::App().ws<PerSocketData>("/*", {
    .compression = uWS::DISABLED,  // 86 B/track, compression adds latency
    .maxPayloadLength = 16 * 1024,
    .open = [](auto* ws) { /* register client */ },
    .message = [](auto* ws, std::string_view msg, uWS::OpCode) {
        // Handle commands from dashboard (target lock, config)
    }
}).listen(8765, [](auto* listenSocket) { /* ... */ }).run();

// In the egress loop:
// 1. Pop WireFrame from SPSC queue
// 2. ws->send(wire_frame_bytes, uWS::BINARY)
```

The dashboard JavaScript already has the `DataView`-based decoder
(designed in `phase_5_telemetry_ui.md`). No changes needed.

#### Command Path: Dashboard → C++

For commands (target lock, config changes), the WebSocket `message`
handler on the C++ side deserializes a small JSON or binary command
and pushes it into a `SPSCQueue<Command>` that the Tracker thread
consumes. Latency budget is generous here (human reaction time is
~100 ms); even JSON parsing is fine.

### 8.3 Why Not ZeroMQ?

ZeroMQ would work, but for our architecture it's unnecessary:

1. **All hot-path components are threads in one process** — `inproc://`
   transport still copies data (~5–15 µs), while SPSC queues are ~8 ns.
2. **The only network boundary is the dashboard** — a WebSocket library
   is simpler and more direct than ZeroMQ's `ws://` transport.
3. **ZeroMQ adds a 2 MB dependency** for functionality we don't use
   (multi-transport, broker patterns, multi-peer fan-out).

**When ZeroMQ *would* be useful:** If we later split components into
separate processes (e.g., inference on GPU process, tracker on CPU
process), ZeroMQ's `ipc://` transport is an excellent upgrade path.
The SPSC queue interfaces can be swapped for ZeroMQ sockets with
minimal code changes. This is a future option, not a current need.

### 8.4 Why Not iceoryx2?

iceoryx2 is impressive technology but solves a problem we don't have:

1. **Same-process threads don't need shared memory IPC** — they already
   share an address space. SPSC queues give the same zero-copy semantics
   at ~8 ns vs. ~50 ns.
2. **Rust toolchain dependency** for building C++ bindings adds build
   complexity.
3. **Windows support is good but not battle-tested** — we'd be early
   adopters.

**When iceoryx2 *would* be useful:** If we split into separate processes
AND need zero-copy for large payloads (e.g., raw camera frames between
processes). For our 86-byte tracks, even a memcpy is free.

---

## 9. Migration Effort Assessment

### 9.1 From Current Architecture → Hybrid (SPSC + WebSocket)

| Component | Change Required | Effort |
|---|---|---|
| **Kinematics Engine** | None. Already produces `WireFrame` via `serialize_frame_indexed()`. Connect output to SPSC queue instead of (planned) ROS 2 publisher. | **~0 hours** |
| **Inference Engine** (future) | Design output as `DetectionBatch` struct pushed to SPSC queue. No ROS 2 message types needed. | **~2 hours** (write queue integration) |
| **Actuator Driver** (future) | Read `ActuatorCommand` from SPSC queue, write to serial. No ROS 2 node wrapping. | **~2 hours** |
| **Dashboard Backend** | Replace planned rosbridge_server with uWebSockets binary egress. Single .cpp file. | **~4–8 hours** |
| **Dashboard Frontend** | **Zero changes.** The `DataView` decoder already expects the binary wire format. Swap `ws://rosbridge:9090` URL for `ws://localhost:8765`. | **~5 minutes** |
| **ros2_ws/ directory** | **Delete entirely.** No ROS 2 workspace, no colcon, no package.xml, no .msg files, no launch files. | **~0 hours** (just `rm -rf`) |
| **Build system** | Single top-level CMakeLists.txt. FetchContent for Eigen, Catch2, rigtorp/SPSCQueue, uWebSockets. No colcon. | **~4 hours** (one-time) |

**Total estimated migration from the planned ROS 2 architecture: ~12–16 hours.**

Critically, this is **less effort than the ROS 2 integration would have been**,
because we're deleting complexity rather than adding it.

### 9.2 From Hybrid → ZeroMQ (Future, If Needed)

If we later need inter-process communication:

| Change | Effort |
|---|---|
| Replace SPSC queue with ZeroMQ `inproc://` socket | ~2 hours per link |
| Split into separate executables | ~4–8 hours |
| Switch `inproc://` to `ipc://` for inter-process | ~5 minutes (URL change) |
| Add ZeroMQ as CMake dependency | ~1 hour |

### 9.3 From Hybrid → iceoryx2 (Future, If Needed)

| Change | Effort |
|---|---|
| Add iceoryx2 C++ bindings to build | ~4 hours (Rust toolchain setup) |
| Replace SPSC queue with iceoryx2 pub/sub | ~4 hours per link |
| Configure shared memory segments | ~2 hours |

### 9.4 From Hybrid → ROS 2 (Future, If Needed)

If multi-machine distribution is ever required, ROS 2 can be layered on
**at the process boundary** without changing the internal architecture:

| Change | Effort |
|---|---|
| Wrap each process in a ROS 2 node | ~8 hours per node |
| Define .msg files from existing C++ structs | ~4 hours |
| Add rosbridge for dashboard | ~2 hours |
| Total for 4-node system | ~30–40 hours |

The hybrid architecture **preserves the upgrade path** to ROS 2 while
avoiding its costs today.

---

## 10. Decision Record

### Decision: Replace Phase 4 "ROS 2 Integration" with "Hybrid IPC Architecture"

**Date:** 2026-05-17

**Status:** PROPOSED — awaiting user review

**Context:**
The ROC AI Vision system runs entirely on a single Windows x64 machine.
The pipeline is: camera → inference → tracker → actuator, with a SvelteKit
dashboard for telemetry visualization. All C++ components will run as
threads within a single process. The only network boundary is the browser
WebSocket connection.

**Decision:**
1. **Hot path (camera → inference → tracker → actuator):** `rigtorp::SPSCQueue`
   lock-free ring buffers for inter-thread communication. ~8 ns/hop latency,
   zero heap allocation, zero serialization, single-header dependency.

2. **Dashboard bridge (tracker → browser):** `uWebSockets` C++17 WebSocket
   server sending the existing 86 B/track binary wire format as binary
   frames. Zero protocol changes from the Phase 5 design.

3. **Command path (browser → tracker):** WebSocket text/binary messages
   parsed in the dashboard egress thread and forwarded via SPSC queue to
   the tracker thread.

4. **Deferred:** ROS 2, iceoryx2, and ZeroMQ remain available as future
   upgrade paths if the system is ever distributed across multiple machines
   or processes.

**Consequences:**
- **Positive:** ~1000x lower latency than ROS 2 on the hot path. Zero build
  system complexity (no colcon, no DDS, no Python). Smaller binary, fewer
  dependencies, faster build times. Dashboard wire format unchanged.
- **Negative:** No built-in recording/playback (rosbag2 equivalent must be
  custom-built if needed). No rviz2 visualization (replaced by SvelteKit
  dashboard). No standard message type ecosystem.
- **Mitigation:** The wire format already supports recording to file. The
  SvelteKit dashboard provides richer, application-specific visualization
  than rviz2. Standard message types are unnecessary when all components
  are authored by the same team.

**Dependencies Added:**
- `rigtorp/SPSCQueue` — MIT license, single header, 312 stars, header-only
  C++11. [github.com/rigtorp/SPSCQueue](https://github.com/rigtorp/SPSCQueue)
- `uWebSockets` — Apache 2.0 license, header-only C++17, 18k+ stars.
  [github.com/uNetworking/uWebSockets](https://github.com/uNetworking/uWebSockets)

**Dependencies Removed:**
- ROS 2 Jazzy (~200 MB install)
- colcon build system
- Fast-DDS / CycloneDDS
- rosbridge_server (Python)
- rosbridge_suite, roslib.js

---

## Appendix A: Quick Reference — Library Links

| Library | Repository | License |
|---|---|---|
| rigtorp::SPSCQueue | https://github.com/rigtorp/SPSCQueue | MIT |
| moodycamel::ReaderWriterQueue | https://github.com/cameron314/readerwriterqueue | BSD |
| moodycamel::ConcurrentQueue | https://github.com/cameron314/concurrentqueue | BSD |
| uWebSockets | https://github.com/uNetworking/uWebSockets | Apache 2.0 |
| ZeroMQ (libzmq) | https://github.com/zeromq/libzmq | LGPL-3.0 |
| nng | https://github.com/nanomsg/nng | MIT |
| Eclipse iceoryx2 | https://github.com/eclipse-iceoryx/iceoryx2 | Apache 2.0 |
| FlatBuffers | https://github.com/google/flatbuffers | Apache 2.0 |

## Appendix B: Latency Scale Reference

```
1 ns   ─ L1 cache reference
8 ns   ─ SPSC queue push/pop (rigtorp)
20 ns  ─ memcpy of 86 bytes
50 ns  ─ iceoryx2 zero-copy IPC
100 ns ─ L2 cache reference
1 µs   ─ iceoryx v1 shared memory IPC
5 µs   ─ ZeroMQ inproc://
15 µs  ─ ZeroMQ ipc://
50 µs  ─ ROS 2 intra-process (Fast-DDS, small payload)
100 µs ─ ROS 2 intra-process (CycloneDDS)
1 ms   ─ ROS 2 inter-process (2 MB payload)
2 ms   ─ rosbridge_server (Python JSON bridge)
100 ms ─ Human reaction time
```

## Appendix C: Cited Research

1. **Hitachi Energy messaging benchmark (2025)** — arXiv:2508.07934.
   Systematic evaluation of ZeroMQ, NanoMsg, and NNG across throughput,
   latency, CPU, and memory.

2. **HPRM: High-Performance Robotic Middleware (2024)** — arXiv:2412.01799.
   Lingua Franca-based deterministic coordination, 173x lower latency than
   ROS 2 for large messages.

3. **Agnocast (2025)** — arXiv:2506.16882. True zero-copy intra-process
   communication for ROS 2, 16% avg / 25% worst-case improvement over
   standard ROS 2 intra-process.

4. **HORUS (2025)** — docs.horusrobotics.dev. Shared-memory tick-based
   middleware, ~85 ns latency vs. ROS 2's ~50–100 µs.

5. **eProsima Fast-DDS Performance Reports** — eprosima.com. Official
   benchmarks for Fast-DDS vs. CycloneDDS intra-process latency.
