#pragma once

// =============================================================================
// roc::transport::spsc_links
//
// Typed SPSC (single-producer / single-consumer) ring-buffer aliases per the
// hot-path architecture pinned in
// `workspace_blueprint/research_notes/research_ipc_middleware_2026.md`
// §5.3 (wait-free `rigtorp::SPSCQueue` push/pop) and §8.2 (four named links).
//
// Five queues — one per directed edge in the §8.1 process diagram, plus the
// reverse OperatorCommand channel:
//
//   Camera Thread  ──[ FrameToInferenceQueue   ]──►  Inference Thread
//   Inference     ──[ InferenceToTrackerQueue ]──►  Tracker Thread
//   Tracker       ──[ TrackerToActuatorQueue  ]──►  Actuator Thread
//   Tracker       ──[ TrackerToWireQueue      ]──►  Dashboard Egress Thread
//   WS server     ──[ OperatorCommandQueue    ]──►  Tracker Thread (reverse)
//
// Capacities follow §8.2 (4 / 16 / 8 / 4 / 16).  The §5.4 cache-line-alignment
// requirement is enforced both inside the rigtorp::SPSCQueue (head/tail on
// separate lines) and on EACH payload struct (alignas(64)) so that adjacent
// slots in the ring buffer never share a cache line and false sharing is
// eliminated end-to-end.
//
// All payload types are trivially-movable so SPSC push/pop is a single
// memcpy.  Heap-owning members (cv::Mat reference-count, std::vector for the
// wire bytes) move in O(1).  No payload contains a std::span or any other
// view type, so lifetime ownership is unambiguous: pushing a payload
// transfers ownership into the ring; popping transfers it out.
// =============================================================================

#include "roc/kinematics/types.hpp"  // Detection, TrackId, Label, kEmbeddingDim
#include "roc/transport/operator_command.hpp"

#include <opencv2/core/mat.hpp>     // cv::Mat — FrameBatch payload

#include <rigtorp/SPSCQueue.h>

#include <array>
#include <cstdint>
#include <vector>

namespace roc {
namespace transport {

// ---------------------------------------------------------------------------
// Cache line size — matches the §5.4 padding assumption.  Hardware-dependent
// in general, but on every CPU we care about (x86-64, ARMv8 BIG.little) the
// line is 64 bytes.  We use a literal because:
//   * `std::hardware_destructive_interference_size` is a C++17 feature gated
//     behind compiler-specific flags (libstdc++ warns "abi-changing" when
//     used in ABIs).
//   * pinning to a concrete value also pins the static_asserts below.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kCacheLineBytes = 64;

// ---------------------------------------------------------------------------
// FrameBatch — Camera → Inference.  Owns a captured `cv::Mat` (reference-
// counted internal buffer) plus capture metadata.
// ---------------------------------------------------------------------------
struct alignas(kCacheLineBytes) FrameBatch {
    cv::Mat       frame{};         // BGR8 or any layout the inference engine accepts
    std::int64_t  capture_ns = 0;  // monotonic clock (steady_clock::time_since_epoch ns)
    std::uint32_t frame_id   = 0;  // monotonic frame counter from the camera thread

    FrameBatch() = default;
    FrameBatch(cv::Mat f, std::int64_t ns, std::uint32_t id) noexcept
        : frame(std::move(f)), capture_ns(ns), frame_id(id) {}
};

// ---------------------------------------------------------------------------
// CameraFrame — Camera Capture Thread → Inference Thread (Phase 6 part A).
//
// Distinct from `FrameBatch` (which the prior Phase 4 stub reserved for an
// inference-bound queue that the orchestrator never ended up using directly)
// — `CameraFrame` is the named typedef the Phase 6 camera-capture path uses
// and is documented in `workspace_blueprint/research_notes/
// dashboard_menu_architecture_2026.md` §11.4 + §12.1 as the live-feed payload.
//
// The cv::Mat inside is reference-counted internally (cv::UMatData) so SPSC
// push/pop is a pointer copy, not a pixel-buffer copy.  Capacity 4 means the
// camera thread drops oldest frames when the inference thread can't keep up
// — the right backpressure semantics for a real-time vision pipeline.
// ---------------------------------------------------------------------------
struct alignas(kCacheLineBytes) CameraFrame {
    cv::Mat        frame{};            // BGR8 layout from cv::VideoCapture
    std::int64_t   capture_ts_ns = 0;  // monotonic steady-clock timestamp
    std::uint32_t  frame_seq     = 0;  // monotonic counter from the camera thread

    CameraFrame() = default;
    CameraFrame(cv::Mat f, std::int64_t ns, std::uint32_t seq) noexcept
        : frame(std::move(f)), capture_ts_ns(ns), frame_seq(seq) {}
};

// ---------------------------------------------------------------------------
// DetectionBatch — Inference → Tracker.  Holds a fixed-capacity array of
// Detection structs so the SPSC payload is trivially-movable (no std::vector
// in the ring slot — that would defeat the wait-free guarantee).
//
// kMaxDetectionsPerBatch = 64 is sized to cover the §7.2 wire-frame design
// ceiling of N=50 confirmed tracks with 28% headroom for unmatched detections
// in a single frame.
// ---------------------------------------------------------------------------
inline constexpr int kMaxDetectionsPerBatch = 64;

struct alignas(kCacheLineBytes) DetectionBatch {
    std::array<roc::kinematics::Detection, kMaxDetectionsPerBatch> dets{};
    int           count       = 0;
    std::int64_t  capture_ns  = 0;
    std::uint32_t frame_id    = 0;

    // Pushing a Detection back; returns false if the batch is already full.
    bool push_back(const roc::kinematics::Detection& d) noexcept {
        if (count >= kMaxDetectionsPerBatch) return false;
        dets[static_cast<std::size_t>(count)] = d;
        ++count;
        return true;
    }

    void clear() noexcept { count = 0; }
};

// ---------------------------------------------------------------------------
// ActuatorCommand — Tracker → Actuator.  Compact (one cache line) per §8.2.
// ---------------------------------------------------------------------------
struct alignas(kCacheLineBytes) ActuatorCommand {
    std::uint64_t target_track_id = 0;  // 0 == no lock; actuator parks
    float         dx_px           = 0.0f;
    float         dy_px           = 0.0f;
    float         dt_predict_s    = 0.0f;
    std::int64_t  emit_ns         = 0;
};

// ---------------------------------------------------------------------------
// WireFrame — Tracker → Dashboard Egress.  Owns the encoded byte buffer.
// The Tracker hot path calls `Tracker::step(..., out_bytes)` which writes
// the §7 binary payload into a vector; we *move* that vector into the SPSC
// slot.  The egress thread `pop()`s and broadcasts via uWebSockets /
// IXWebSocket; ownership then dies on the consumer side.
// ---------------------------------------------------------------------------
struct alignas(kCacheLineBytes) WireFrame {
    std::vector<std::uint8_t> bytes{};
    std::uint32_t             frame_id = 0;
    std::int64_t              emit_ns  = 0;

    WireFrame() = default;
    WireFrame(std::vector<std::uint8_t> b, std::uint32_t id, std::int64_t ns) noexcept
        : bytes(std::move(b)), frame_id(id), emit_ns(ns) {}
};

// ---------------------------------------------------------------------------
// ActuatorWireFrame — Actuator-emit thread → Dashboard egress (§12.4).
// Fixed 36-byte §12.4 payload; egress is the sole `broadcast()` caller (T01.5).
// ---------------------------------------------------------------------------
inline constexpr std::size_t kActuatorWirePayloadBytes = 36;

struct alignas(kCacheLineBytes) ActuatorWireFrame {
    std::array<std::uint8_t, kActuatorWirePayloadBytes> bytes{};
    std::uint32_t frame_id = 0;
    std::int64_t  emit_ns  = 0;
};

// ---------------------------------------------------------------------------
// BgOdometryWireFrame — Optical-flow thread → Dashboard egress (§13.4).
// ---------------------------------------------------------------------------
inline constexpr std::size_t kBgOdometryWirePayloadBytes = 32;

struct alignas(kCacheLineBytes) BgOdometryWireFrame {
    std::array<std::uint8_t, kBgOdometryWirePayloadBytes> bytes{};
    std::uint32_t frame_id = 0;
    std::int64_t  emit_ns  = 0;
};

// ---------------------------------------------------------------------------
// SPSC queue aliases — capacities per §8.2.
//
// `rigtorp::SPSCQueue<T>` provides wait-free push/pop (§5.3) with head and
// tail on separate cache lines (§5.4) so the producer and consumer never
// contend on the same line.  Capacity is a constructor argument; we expose
// const-expr defaults below so the orchestrator constructs queues with the
// correct shape without re-stating magic numbers.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kFrameToInferenceCapacity    = 4;
inline constexpr std::size_t kInferenceToTrackerCapacity  = 16;
inline constexpr std::size_t kTrackerToActuatorCapacity   = 8;
inline constexpr std::size_t kTrackerToWireCapacity       = 4;
inline constexpr std::size_t kOperatorCommandCapacity     = 16;
inline constexpr std::size_t kCameraToInferenceCapacity   = 4;
inline constexpr std::size_t kActuatorToEgressCapacity    = 1024;
inline constexpr std::size_t kCameraToOpticalFlowCapacity   = 4;
inline constexpr std::size_t kBgOdometryToEgressCapacity    = 64;

using FrameToInferenceQueue   = rigtorp::SPSCQueue<FrameBatch>;
using InferenceToTrackerQueue = rigtorp::SPSCQueue<DetectionBatch>;
using TrackerToActuatorQueue  = rigtorp::SPSCQueue<ActuatorCommand>;
using TrackerToWireQueue      = rigtorp::SPSCQueue<WireFrame>;
using OperatorCommandQueue    = rigtorp::SPSCQueue<OperatorCommand>;
using CameraToInferenceQueue  = rigtorp::SPSCQueue<CameraFrame>;
using ActuatorToEgressQueue     = rigtorp::SPSCQueue<ActuatorWireFrame>;
using CameraToOpticalFlowQueue  = rigtorp::SPSCQueue<CameraFrame>;
using BgOdometryToEgressQueue   = rigtorp::SPSCQueue<BgOdometryWireFrame>;

// ---------------------------------------------------------------------------
// Compile-time guards for the §5.4 cache-line alignment promise.  These run
// at parse time on every TU that includes this header so a future field
// addition that breaks alignment fails the build immediately.
// ---------------------------------------------------------------------------
static_assert(alignof(FrameBatch)        >= kCacheLineBytes,
              "FrameBatch must be cache-line aligned (§5.4 false-sharing)");
static_assert(alignof(CameraFrame)       >= kCacheLineBytes,
              "CameraFrame must be cache-line aligned (§5.4 false-sharing)");
static_assert(alignof(DetectionBatch)    >= kCacheLineBytes,
              "DetectionBatch must be cache-line aligned");
static_assert(alignof(ActuatorCommand)   >= kCacheLineBytes,
              "ActuatorCommand must be cache-line aligned");
static_assert(alignof(WireFrame)         >= kCacheLineBytes,
              "WireFrame must be cache-line aligned");
static_assert(alignof(ActuatorWireFrame) >= kCacheLineBytes,
              "ActuatorWireFrame must be cache-line aligned");
static_assert(alignof(BgOdometryWireFrame) >= kCacheLineBytes,
              "BgOdometryWireFrame must be cache-line aligned");

// ActuatorCommand should fit in a single cache line (it's tiny).  WireFrame
// has a std::vector inside, so its size depends on the libstdc++ vector
// layout — guarantee only alignment, not size.
static_assert(sizeof(ActuatorCommand) <= kCacheLineBytes,
              "ActuatorCommand grew past a cache line — re-pack the struct");

}  // namespace transport
}  // namespace roc
