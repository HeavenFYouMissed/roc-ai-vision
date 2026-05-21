// =============================================================================
// roc_vision.exe — Phase 6 part A end-to-end orchestrator.
//
// Replaces the prior Phase-4 synthetic-detection generator with a real
// camera-capture path feeding a production InferenceEngine (DirectML / CPU
// EP) and the §5 kinematics tracker.  Five threads, four SPSC links + one
// reverse OperatorCommand SPSC + the WS egress server:
//
//     ┌────────────────────────────────────────────────────────────────┐
//     │  camera_capture_thread                                         │
//     │     cv::VideoCapture(idx) → CameraFrame ──[CameraToInfer]──┐   │
//     │                                                            │   │
//     │  inference_thread                                          ▼   │
//     │     CameraFrame → InferenceEngine.run() → DetectionBatch       │
//     │                                       ──[InferToTrack]──┐      │
//     │  tracker_thread                                         ▼      │
//     │     drain OperatorCommandQueue → Tracker.step() →              │
//     │       (WireFrame ──[TrackerToWire]──→ dashboard_egress_thread) │
//     │       (ActuatorCommand ──[TrackerToActuator]──→ actuator_telemetry_thread)
//     │                                                                │
//     │  actuator_telemetry_emit_thread  (§12.4 — 500 Hz default)      │
//     │     ActuatorCommand → version=2 wire → ActuatorToEgressQueue    │
//     │  dashboard_egress_thread — sole WsEgressServer::broadcast()    │
//     │     round-robin: TrackerToWire + ActuatorToEgress (+ v3 later) │
//     └────────────────────────────────────────────────────────────────┘
//
// Hard rules upheld:
//   * tracker is the SOLE mutator of kinematics state (no contention);
//   * OperatorCommandQueue is drained inside `tracker_thread` not a
//     separate thread (same rationale as the prior Phase-4 wiring);
//   * camera frames drop at the SPSC boundary on backpressure — the camera
//     thread is never blocked by slow inference;
//   * SIGINT triggers graceful teardown in reverse-startup order.
//
// CLI (additions vs Phase 4 are marked *new*):
//   roc_vision.exe                                — run until Ctrl-C.
//   --demo-duration SEC                          — auto-exit after N seconds.
//   --port PORT                                  — WS listen port (default 8765).
//   --bind ADDR                                  — WS bind address (default 127.0.0.1).
//   --camera IDX *new*                           — webcam device index (default 0).
//   --camera-width PX *new*                      — request camera capture width.
//   --camera-height PX *new*                     — request camera capture height.
//   --model PATH *new*                           — ONNX model path (or $ROC_MODEL_ONNX).
//   --manifest PATH *new*                        — YAML manifest path; defaults to
//                                                  derived-from-model-name (`models/<basename>.yaml`).
//   --no-actuator-telemetry *new*                — skip the §12.4 actuator
//                                                  telemetry thread entirely.
//   --actuator-emit-hz HZ *new*                  — §12.4 actuator telemetry
//                                                  rate (default 500).
//   --help                                       — print usage and exit.
//
// All wire frame bytes are broadcast as binary WS frames.  The dashboard's
// existing decoder consumes version=1 frames (track telemetry); version=2
// frames (actuator telemetry per §12.4) are forward-compatible — the
// dashboard's Phase 5b decoder no-ops them until Phase 5c lands the
// MouseDynamicsInspector render path.
// =============================================================================

#include "roc/kinematics/tracker.hpp"
#include "roc/kinematics/types.hpp"

#include "roc/transport/desktop_capture.hpp"
#include "roc/transport/mjpeg_server.hpp"
#include "roc/transport/operator_command.hpp"
#include "roc/transport/output_sink.hpp"
#include "roc/transport/spsc_links.hpp"
#include "roc/transport/wire_format.hpp"
#include "roc/transport/ws_egress.hpp"

#include "roc/vision/inference_engine.hpp"
#include "roc/vision/optical_flow_sampler.hpp"
#include "roc/vision/manifest_loader.hpp"
#include "roc/vision/postprocessor_registry.hpp"
#include "roc/vision/types.hpp"

#include <onnxruntime_cxx_api.h>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <vector>

namespace {

// ---------------------------------------------------------------------------
// Global shutdown flag set by the SIGINT/SIGTERM handler.
// ---------------------------------------------------------------------------
std::atomic<bool> g_shutdown_requested{false};

extern "C" void sigint_handler(int /*sig*/) {
    g_shutdown_requested.store(true, std::memory_order_release);
}

// ---------------------------------------------------------------------------
// Monotonic clocks — used everywhere we need a frame / wire timestamp.
// ---------------------------------------------------------------------------
std::int64_t now_steady_ns() {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}
double now_steady_seconds() {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

// ---------------------------------------------------------------------------
// CLI parsing — minimal flag walker.  Kept inline to avoid pulling in
// third-party argparse libraries.
// ---------------------------------------------------------------------------
constexpr int kActuatorEmitHz = 500;  // §12.4 full cadence

struct OrchestratorOptions {
    double         demo_duration_s   = 0.0;       // 0 == run until SIGINT
    std::uint16_t  ws_port           = 8765;
    std::string    ws_bind           = "127.0.0.1";
    int            camera_index      = 0;
    int            camera_width      = 0;          // 0 == camera default
    int            camera_height     = 0;
    std::string    model_path;                     // empty ⇒ $ROC_MODEL_ONNX
    std::string    manifest_path;                  // empty ⇒ derived from model
    // Directory scanned by the /api/models endpoint for swappable manifests.
    // Empty ⇒ derived from manifest_path's parent directory at startup.
    std::string    models_dir;
    bool           enable_actuator_tx = true;
    int            actuator_emit_hz   = kActuatorEmitHz;  // §12.4; --actuator-emit-hz
    // MJPEG-over-HTTP server for the dashboard's OperateCanvas Layer 0
    // <img> element + /sources device enumeration. 0 to disable.
    std::uint16_t  mjpeg_port        = 8766;
    bool           enable_mjpeg      = true;
    int            mjpeg_quality     = 80;          // 1..100

    // Phase-8 desktop capture (Windows-only). When `desktop_kind` is
    // anything other than "none", the orchestrator opens a DesktopCapture
    // INSTEAD of cv::VideoCapture and ignores `camera_index`.
    //   "none"     — use webcam / capture card (legacy default)
    //   "primary"  — primary monitor
    //   "monitor"  — display index (default 0); see `--capture-monitor`
    //   "window"   — top-level window matched by title substring
    //   "region"   — fixed screen-relative rectangle
    std::string    desktop_kind          = "none";
    int            desktop_monitor_index = 0;
    std::string    desktop_window_title;
    int            desktop_region_x = 0;
    int            desktop_region_y = 0;
    int            desktop_region_w = 0;
    int            desktop_region_h = 0;
    int            desktop_fps          = 60;
    int            desktop_downscale    = 0;        // 0 = native, else largest-dim cap
    bool           desktop_show_cursor  = false;

    bool           help               = false;
};

void print_usage() {
    std::cout <<
        "Usage: roc_vision.exe [options]\n"
        "  --demo-duration SECONDS      auto-exit after N seconds (default: run until Ctrl-C)\n"
        "  --port PORT                  dashboard WebSocket port (default: 8765)\n"
        "  --bind ADDRESS               WS bind address (default: 127.0.0.1)\n"
        "  --camera INDEX               webcam device index (default: 0)\n"
        "  --camera-width PX            request camera capture width (default: device native)\n"
        "  --camera-height PX           request camera capture height (default: device native)\n"
        "  --model PATH.onnx            ONNX model path (default: $ROC_MODEL_ONNX)\n"
        "  --manifest PATH.yaml         YAML manifest path (default: derived from model)\n"
        "  --no-actuator-telemetry      skip the §12.4 actuator-telemetry thread\n"
        "  --actuator-emit-hz HZ          actuator telemetry rate (default: 500, §12.4)\n"
        "  --mjpeg-port PORT            dashboard MJPEG-over-HTTP port (default: 8766)\n"
        "  --no-mjpeg                   disable the MJPEG-over-HTTP server entirely\n"
        "  --mjpeg-quality Q            JPEG quality 1..100 (default: 80, larger = bigger frames)\n"
        "  --help                       print this help and exit\n";
}

bool parse_double(const char* s, double& out) {
    if (!s || !*s) return false;
    char* end = nullptr;
    double v = std::strtod(s, &end);
    if (end == s) return false;
    out = v;
    return true;
}
bool parse_u16(const char* s, std::uint16_t& out) {
    if (!s || !*s) return false;
    char* end = nullptr;
    unsigned long v = std::strtoul(s, &end, 10);
    if (end == s || v > 65535ul) return false;
    out = static_cast<std::uint16_t>(v);
    return true;
}
bool parse_int(const char* s, int& out) {
    if (!s || !*s) return false;
    char* end = nullptr;
    long v = std::strtol(s, &end, 10);
    if (end == s) return false;
    out = static_cast<int>(v);
    return true;
}

bool parse_args(int argc, char** argv, OrchestratorOptions& opts) {
    for (int i = 1; i < argc; ++i) {
        const std::string_view a{argv[i]};
        auto need_value = [&](const char* flag, const char*& v_out) {
            if (i + 1 >= argc) {
                std::cerr << "[roc-orchestrator] missing value for " << flag << "\n";
                return false;
            }
            v_out = argv[++i];
            return true;
        };

        if (a == "--help" || a == "-h") {
            opts.help = true;
        } else if (a == "--demo-duration") {
            const char* v = nullptr;
            if (!need_value("--demo-duration", v)) return false;
            if (!parse_double(v, opts.demo_duration_s) || opts.demo_duration_s < 0.0) {
                std::cerr << "[roc-orchestrator] invalid --demo-duration: " << v << "\n";
                return false;
            }
        } else if (a == "--port") {
            const char* v = nullptr;
            if (!need_value("--port", v)) return false;
            if (!parse_u16(v, opts.ws_port)) {
                std::cerr << "[roc-orchestrator] invalid --port: " << v << "\n";
                return false;
            }
        } else if (a == "--bind") {
            const char* v = nullptr;
            if (!need_value("--bind", v)) return false;
            opts.ws_bind.assign(v);
        } else if (a == "--camera") {
            const char* v = nullptr;
            if (!need_value("--camera", v)) return false;
            if (!parse_int(v, opts.camera_index) || opts.camera_index < 0) {
                std::cerr << "[roc-orchestrator] invalid --camera: " << v << "\n";
                return false;
            }
        } else if (a == "--camera-width") {
            const char* v = nullptr;
            if (!need_value("--camera-width", v)) return false;
            if (!parse_int(v, opts.camera_width) || opts.camera_width <= 0) {
                std::cerr << "[roc-orchestrator] invalid --camera-width: " << v << "\n";
                return false;
            }
        } else if (a == "--camera-height") {
            const char* v = nullptr;
            if (!need_value("--camera-height", v)) return false;
            if (!parse_int(v, opts.camera_height) || opts.camera_height <= 0) {
                std::cerr << "[roc-orchestrator] invalid --camera-height: " << v << "\n";
                return false;
            }
        } else if (a == "--model") {
            const char* v = nullptr;
            if (!need_value("--model", v)) return false;
            opts.model_path.assign(v);
        } else if (a == "--manifest") {
            const char* v = nullptr;
            if (!need_value("--manifest", v)) return false;
            opts.manifest_path.assign(v);
        } else if (a == "--models-dir") {
            const char* v = nullptr;
            if (!need_value("--models-dir", v)) return false;
            opts.models_dir.assign(v);
        } else if (a == "--no-actuator-telemetry") {
            opts.enable_actuator_tx = false;
        } else if (a == "--actuator-emit-hz") {
            const char* v = nullptr;
            if (!need_value("--actuator-emit-hz", v)) return false;
            if (!parse_int(v, opts.actuator_emit_hz) || opts.actuator_emit_hz <= 0
                || opts.actuator_emit_hz > 10000) {
                std::cerr << "[roc-orchestrator] invalid --actuator-emit-hz: " << v << "\n";
                return false;
            }
        } else if (a == "--mjpeg-port") {
            const char* v = nullptr;
            if (!need_value("--mjpeg-port", v)) return false;
            if (!parse_u16(v, opts.mjpeg_port)) {
                std::cerr << "[roc-orchestrator] invalid --mjpeg-port: " << v << "\n";
                return false;
            }
        } else if (a == "--no-mjpeg") {
            opts.enable_mjpeg = false;
        } else if (a == "--mjpeg-quality") {
            const char* v = nullptr;
            if (!need_value("--mjpeg-quality", v)) return false;
            if (!parse_int(v, opts.mjpeg_quality) || opts.mjpeg_quality < 1
                || opts.mjpeg_quality > 100) {
                std::cerr << "[roc-orchestrator] invalid --mjpeg-quality: " << v << "\n";
                return false;
            }
        } else if (a == "--desktop-capture") {
            const char* v = nullptr;
            if (!need_value("--desktop-capture", v)) return false;
            opts.desktop_kind.assign(v);
            // Accepted: none / primary / monitor / window / region
            if (opts.desktop_kind != "none" && opts.desktop_kind != "primary" &&
                opts.desktop_kind != "monitor" && opts.desktop_kind != "window" &&
                opts.desktop_kind != "region") {
                std::cerr << "[roc-orchestrator] invalid --desktop-capture mode: " << v
                          << " (expected none|primary|monitor|window|region)\n";
                return false;
            }
        } else if (a == "--capture-monitor") {
            const char* v = nullptr;
            if (!need_value("--capture-monitor", v)) return false;
            if (!parse_int(v, opts.desktop_monitor_index) || opts.desktop_monitor_index < 0) {
                std::cerr << "[roc-orchestrator] invalid --capture-monitor: " << v << "\n";
                return false;
            }
        } else if (a == "--capture-window") {
            const char* v = nullptr;
            if (!need_value("--capture-window", v)) return false;
            opts.desktop_window_title.assign(v);
        } else if (a == "--capture-region") {
            // expected: X,Y,W,H (no spaces)
            const char* v = nullptr;
            if (!need_value("--capture-region", v)) return false;
            int xs[4]{0,0,0,0};
            const char* p = v; int idx = 0;
            while (*p && idx < 4) {
                char* endp = nullptr;
                xs[idx] = static_cast<int>(std::strtol(p, &endp, 10));
                if (endp == p) break;
                p = endp;
                if (*p == ',') ++p;
                ++idx;
            }
            if (idx != 4 || xs[2] <= 0 || xs[3] <= 0) {
                std::cerr << "[roc-orchestrator] invalid --capture-region: " << v
                          << " (expected X,Y,W,H with W>0 and H>0)\n";
                return false;
            }
            opts.desktop_region_x = xs[0];
            opts.desktop_region_y = xs[1];
            opts.desktop_region_w = xs[2];
            opts.desktop_region_h = xs[3];
        } else if (a == "--capture-fps") {
            const char* v = nullptr;
            if (!need_value("--capture-fps", v)) return false;
            if (!parse_int(v, opts.desktop_fps) || opts.desktop_fps <= 0
                || opts.desktop_fps > 240) {
                std::cerr << "[roc-orchestrator] invalid --capture-fps: " << v << "\n";
                return false;
            }
        } else if (a == "--capture-downscale") {
            const char* v = nullptr;
            if (!need_value("--capture-downscale", v)) return false;
            if (!parse_int(v, opts.desktop_downscale) || opts.desktop_downscale < 0) {
                std::cerr << "[roc-orchestrator] invalid --capture-downscale: " << v << "\n";
                return false;
            }
        } else if (a == "--capture-cursor") {
            opts.desktop_show_cursor = true;
        } else {
            std::cerr << "[roc-orchestrator] unknown argument: " << a << "\n";
            return false;
        }
    }
    return true;
}

// Apply $ROC_MODEL_ONNX as the default model path, and derive the manifest
// path by replacing the .onnx extension with .yaml (in the same directory),
// when the user didn't pass --manifest.
void resolve_model_paths(OrchestratorOptions& opts) {
    if (opts.model_path.empty()) {
        if (const char* env = std::getenv("ROC_MODEL_ONNX"); env && *env) {
            opts.model_path.assign(env);
        }
    }
    if (opts.manifest_path.empty() && !opts.model_path.empty()) {
        std::filesystem::path p = std::filesystem::path(opts.model_path);
        p.replace_extension(".yaml");
        opts.manifest_path = p.string();
    }
    if (opts.models_dir.empty() && !opts.manifest_path.empty()) {
        std::filesystem::path p = std::filesystem::path(opts.manifest_path);
        if (p.has_parent_path()) {
            opts.models_dir = p.parent_path().string();
        }
    }
}

// ---------------------------------------------------------------------------
// Camera capture thread — drives cv::VideoCapture at the device's preferred
// frame rate, packages each frame into a CameraFrame and try_push'es into the
// CameraToInferenceQueue.  Drops on full (the inference thread is the only
// consumer and may be slower than the camera).
//
// Per the worker spec hard-constraint #9: cv::VideoCapture::read is blocking;
// it MUST be in its own thread so inference latency doesn't backpressure the
// camera and produce stale frames.
// ---------------------------------------------------------------------------
void camera_capture_thread(cv::VideoCapture&                         cap,
                           roc::transport::CameraToInferenceQueue&   infer_q,
                           roc::transport::CameraToOpticalFlowQueue& optflow_q,
                           roc::transport::MjpegServer*              mjpeg_server,
                           int                                       camera_index_for_mjpeg,
                           std::atomic<bool>&                        shutdown,
                           std::atomic<std::uint64_t>&               frames_captured,
                           std::atomic<std::uint64_t>&               frames_dropped) {
    using namespace std::chrono;
    std::uint32_t frame_seq = 0;
    cv::Mat scratch;

    while (!shutdown.load(std::memory_order_acquire)) {
        const bool ok = cap.read(scratch);
        if (!ok || scratch.empty()) {
            // Driver hiccup — yield and retry rather than spinning.
            std::this_thread::sleep_for(milliseconds(5));
            continue;
        }

        ++frame_seq;
        // Move the cv::Mat into the SPSC payload — cv::Mat's move is O(1)
        // (refcount swap on the internal cv::UMatData), so this is cheap
        // even at 1080p.
        cv::Mat owned = scratch.clone();   // capture-thread-owned copy so
                                           // the V4L2/MSMF backend's next
                                           // read() can overwrite `scratch`
                                           // without aliasing into the SPSC.
        const std::int64_t ts = now_steady_ns();
        roc::transport::CameraFrame cf_infer{
            owned,
            ts,
            frame_seq
        };
        roc::transport::CameraFrame cf_optflow{
            cv::Mat(owned),
            ts,
            frame_seq
        };
        if (!infer_q.try_push(std::move(cf_infer))) {
            frames_dropped.fetch_add(1, std::memory_order_relaxed);
        }
        (void)optflow_q.try_push(std::move(cf_optflow));

        // Push to MJPEG server (best-effort, thread-safe, drops to slow clients).
        // The encode happens inside push_jpeg_frame on this thread — at JPEG
        // quality 80 + 1920x1080 it's ~5-10ms, well under the camera frame
        // budget even at 60 fps. Skipped entirely when --no-mjpeg.
        if (mjpeg_server != nullptr) {
            mjpeg_server->push_jpeg_frame(camera_index_for_mjpeg, owned);
        }

        frames_captured.fetch_add(1, std::memory_order_relaxed);
    }
}

#ifdef _WIN32
// ---------------------------------------------------------------------------
// Phase 8: desktop / window / region capture thread.
//
// Drop-in for camera_capture_thread when the operator passes
// `--desktop-capture <kind>`. Drives a DesktopCapture instead of a
// cv::VideoCapture and pushes the resulting frames into the SAME SPSC
// queues the inference + optical-flow threads consume — so downstream code
// is identical regardless of where pixels originate.
//
// Use cases enabled here: tracking targets in a YouTube video, in another
// game window, in OBS preview, in a desktop region marked with the
// `--capture-region X,Y,W,H` flag.
// ---------------------------------------------------------------------------
void desktop_capture_thread(roc::transport::DesktopCapture&           cap,
                            roc::transport::CameraToInferenceQueue&   infer_q,
                            roc::transport::CameraToOpticalFlowQueue& optflow_q,
                            roc::transport::MjpegServer*              mjpeg_server,
                            int                                       camera_index_for_mjpeg,
                            std::atomic<bool>&                        shutdown,
                            std::atomic<std::uint64_t>&               frames_captured,
                            std::atomic<std::uint64_t>&               frames_dropped) {
    using namespace std::chrono;
    std::uint32_t frame_seq = 0;
    cv::Mat scratch;

    while (!shutdown.load(std::memory_order_acquire)) {
        const bool ok = cap.read(scratch);
        if (!ok || scratch.empty()) {
            // Window minimized / monitor unplugged / DRM-protected content —
            // back off and retry rather than spinning.
            std::this_thread::sleep_for(milliseconds(33));
            continue;
        }

        ++frame_seq;
        cv::Mat owned = scratch.clone();
        const std::int64_t ts = now_steady_ns();
        roc::transport::CameraFrame cf_infer{ owned, ts, frame_seq };
        roc::transport::CameraFrame cf_optflow{ cv::Mat(owned), ts, frame_seq };
        if (!infer_q.try_push(std::move(cf_infer))) {
            frames_dropped.fetch_add(1, std::memory_order_relaxed);
        }
        (void)optflow_q.try_push(std::move(cf_optflow));

        if (mjpeg_server != nullptr) {
            mjpeg_server->push_jpeg_frame(camera_index_for_mjpeg, owned);
        }
        frames_captured.fetch_add(1, std::memory_order_relaxed);
    }
}
#endif  // _WIN32

// ---------------------------------------------------------------------------
// Inference thread — pops CameraFrame, runs the manifest-bound model, packs
// the resulting Detection[] into a DetectionBatch, pushes to the tracker.
//
// The InferenceEngine is constructed ONCE at startup and lives for the
// thread's whole lifetime, so the per-frame cost is just preprocess +
// `Ort::Session::Run` + postprocess.  Zero per-frame heap allocations in
// steady state (engine reuses scratch buffers internally).
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Phase 6: FOV crop store.
//
// Shared between the HTTP API thread (writer), the inference thread
// (reader+writer for active_crop_*), and the tracker thread (writer for
// Phase 7 follow-target). A simple mutex around a small struct is
// sufficient — both access patterns are infrequent at the scale of a
// frame interval (~7 ms at 144 Hz; mutex latency is well under that).
// Defined here BEFORE any thread function so all of them can take a
// FovStore* directly.
// ---------------------------------------------------------------------------
class FovStore {
public:
    void set(const roc::transport::FovSettings& wanted) {
        std::lock_guard<std::mutex> lock(mu_);
        settings_.enabled       = wanted.enabled;
        settings_.cx_norm       = std::clamp(wanted.cx_norm, 0.0f, 1.0f);
        settings_.cy_norm       = std::clamp(wanted.cy_norm, 0.0f, 1.0f);
        settings_.radius_px     = std::max(wanted.radius_px, 16.0f);
        settings_.follow_target = wanted.follow_target;
    }

    void update_active_crop(int src_w, int src_h,
                            int x, int y, int w, int h) {
        std::lock_guard<std::mutex> lock(mu_);
        settings_.source_w      = src_w;
        settings_.source_h      = src_h;
        settings_.active_crop_x = x;
        settings_.active_crop_y = y;
        settings_.active_crop_w = w;
        settings_.active_crop_h = h;
    }

    roc::transport::FovSettings snapshot() const {
        std::lock_guard<std::mutex> lock(mu_);
        return settings_;
    }

private:
    mutable std::mutex            mu_;
    roc::transport::FovSettings   settings_{};
};

// ---------------------------------------------------------------------------
// Compute the integer source-frame square crop from current FovSettings.
// Always centred on (cx_norm * w, cy_norm * h), clamped so the crop lies
// fully within the source frame. Returns (x, y, w, h). When FOV is
// disabled or radius_px <= 0 the returned rect is the full frame.
// ---------------------------------------------------------------------------
struct CropRect { int x = 0, y = 0, w = 0, h = 0; };

inline CropRect resolve_crop(const roc::transport::FovSettings& s, int src_w, int src_h) {
    if (!s.enabled || s.radius_px <= 0.0f) return CropRect{0, 0, src_w, src_h};
    const int   half       = static_cast<int>(std::round(s.radius_px));
    const int   cx         = static_cast<int>(std::round(s.cx_norm * src_w));
    const int   cy         = static_cast<int>(std::round(s.cy_norm * src_h));
    int x = cx - half;
    int y = cy - half;
    int w = 2 * half;
    int h = 2 * half;
    if (w > src_w) w = src_w;
    if (h > src_h) h = src_h;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + w > src_w) x = src_w - w;
    if (y + h > src_h) y = src_h - h;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    return CropRect{x, y, w, h};
}

// ---------------------------------------------------------------------------
// Tracker runtime config — written by operator commands (§12.3), read by the
// actuator-emit thread without locking.
// ---------------------------------------------------------------------------
struct TrackerRuntimeConfig {
    std::atomic<float> linear_smoothing_alpha{0.35f};
    std::atomic<float> deadband_radius_px{4.0f};
    std::atomic<float> fov_radius_px{320.0f};
    std::atomic<float> targeting_anchor{0.0f};
};

// ---------------------------------------------------------------------------
// Tracker thread — UNCHANGED in behaviour vs Phase 4 production lock: drains
// InferenceToTrackerQueue + OperatorCommandQueue, runs §5 pipeline, emits
// wire frames to TrackerToWireQueue AND pushes an ActuatorCommand to
// TrackerToActuatorQueue so the §12.4 actuator-telemetry thread can broadcast
// at its decoupled cadence.
//
// `roc::kinematics::Tracker` is NOT thread-safe by design (§5.x lock-free is
// "single mutator"); collapsing the operator-cmd consumer into this thread
// preserves that invariant.
// ---------------------------------------------------------------------------
void tracker_thread(roc::transport::InferenceToTrackerQueue&  in_q,
                    roc::transport::OperatorCommandQueue&     cmd_q,
                    roc::transport::TrackerToWireQueue&       out_wire_q,
                    roc::transport::TrackerToActuatorQueue&   out_act_q,
                    TrackerRuntimeConfig&                     runtime_cfg,
                    FovStore*                                 fov_store,
                    int                                       image_w,
                    int                                       image_h,
                    bool                                      forward_to_actuator,
                    std::atomic<bool>&                        shutdown,
                    std::atomic<std::uint64_t>&               tracker_steps,
                    std::atomic<std::uint64_t>&               tracker_dropped_frames) {
    using namespace std::chrono;

    // Identity camera intrinsics so the pinhole measurement function in
    // roc::kinematics is well-defined for the demo.  fx/fy = 0.85 * image_w
    // matches the empirical "1.2× sensor diagonal" focal-length rule for
    // commodity laptop webcams.  Phase 6 part B replaces this with a true
    // intrinsics YAML from the Calibrate destination's wizard.
    roc::kinematics::CameraPose pose{};
    const double fx_guess = 0.85 * static_cast<double>(image_w);
    pose.K(0, 0) = fx_guess;
    pose.K(1, 1) = fx_guess;
    pose.K(0, 2) = 0.5 * static_cast<double>(image_w);
    pose.K(1, 2) = 0.5 * static_cast<double>(image_h);
    pose.R_cw   = roc::kinematics::Mat3::Identity();
    pose.t_cw   = roc::kinematics::Vec3::Zero();
    pose.img_w  = image_w;
    pose.img_h  = image_h;

    roc::kinematics::TrackerConfig cfg{};
    cfg.selector_cx = 0.5 * static_cast<double>(image_w);
    cfg.selector_cy = 0.5 * static_cast<double>(image_h);
    cfg.enable_target_selector = true;

    roc::kinematics::Tracker tracker(pose, cfg);

    std::vector<std::uint8_t> wire_buf;
    wire_buf.reserve(roc::kinematics::kWireHeaderSize +
                     roc::transport::kMaxDetectionsPerBatch *
                     roc::kinematics::kWireTrackSize);

    while (!shutdown.load(std::memory_order_acquire)) {
        // ── 1. Drain operator commands. ─────────────────────────────────
        while (auto* cmd_ptr = cmd_q.front()) {
            const roc::transport::OperatorCommand& cmd = *cmd_ptr;
            switch (cmd.kind) {
            case roc::transport::OperatorCommandKind::LockTarget:
                tracker.selector().set_locked_target(
                    roc::kinematics::TrackId{cmd.target_track_id});
                break;
            case roc::transport::OperatorCommandKind::ReleaseLock:
                tracker.selector().set_locked_target(roc::kinematics::TrackId{0});
                break;
            case roc::transport::OperatorCommandKind::UpdateSelectorWeights: {
                roc::kinematics::SelectorWeights w{};
                w.w_proximity  = static_cast<double>(cmd.weight_proximity);
                w.w_confidence = static_cast<double>(cmd.weight_confidence);
                w.w_stability  = static_cast<double>(cmd.weight_stability);
                tracker.selector().set_weights(w);
                break;
            }
            case roc::transport::OperatorCommandKind::SetSmoothingAlpha:
                runtime_cfg.linear_smoothing_alpha.store(cmd.value,
                    std::memory_order_release);
                break;
            case roc::transport::OperatorCommandKind::SetDeadbandRadiusPx:
                runtime_cfg.deadband_radius_px.store(cmd.value,
                    std::memory_order_release);
                break;
            case roc::transport::OperatorCommandKind::SetFovRadiusPx:
                runtime_cfg.fov_radius_px.store(cmd.value,
                    std::memory_order_release);
                break;
            case roc::transport::OperatorCommandKind::SetTargetingAnchor:
                runtime_cfg.targeting_anchor.store(cmd.value,
                    std::memory_order_release);
                break;
            case roc::transport::OperatorCommandKind::None:
            default:
                break;
            }
            cmd_q.pop();
        }

        // ── 2. Pop one DetectionBatch. ──────────────────────────────────
        auto* batch_ptr = in_q.front();
        if (batch_ptr == nullptr) {
            std::this_thread::sleep_for(microseconds(500));
            continue;
        }
        roc::transport::DetectionBatch batch = std::move(*batch_ptr);
        in_q.pop();

        // ── 3. Run the §5 pipeline. ─────────────────────────────────────
        const double t_now = now_steady_seconds();
        std::span<const roc::kinematics::Detection> dets_span{
            batch.dets.data(),
            static_cast<std::size_t>(batch.count)
        };

        wire_buf.clear();
        try {
            (void)tracker.step(t_now, dets_span, wire_buf);
        } catch (const std::exception& e) {
            std::cerr << "[roc-orchestrator] tracker.step threw: "
                      << e.what() << "\n";
            tracker_dropped_frames.fetch_add(1, std::memory_order_relaxed);
            continue;
        }

        // ── 4. Push WireFrame downstream. ───────────────────────────────
        roc::transport::WireFrame wf{
            std::move(wire_buf),
            batch.frame_id,
            now_steady_ns()
        };
        if (!out_wire_q.try_push(std::move(wf))) {
            tracker_dropped_frames.fetch_add(1, std::memory_order_relaxed);
        }
        wire_buf.clear();
        wire_buf.reserve(roc::kinematics::kWireHeaderSize +
                         roc::transport::kMaxDetectionsPerBatch *
                         roc::kinematics::kWireTrackSize);

        // ── 5. Forward the latest selection to the actuator thread.
        //         §12.4 — actuator-telemetry-emit thread consumes raw
        //         tracker state at 30 Hz and republishes at its own
        //         decoupled cadence (500 Hz default, §12.4).
        const auto sel = tracker.selector().select(tracker.tracks());
        if (forward_to_actuator) {
            roc::transport::ActuatorCommand ac{};
            ac.target_track_id = sel.valid ? sel.chosen.value : 0u;
            ac.dx_px           = static_cast<float>(sel.dx);
            ac.dy_px           = static_cast<float>(sel.dy);
            ac.dt_predict_s    = 0.0f;     // §12.4 will set this once
                                           // velocity-feedforward lands
            ac.emit_ns         = now_steady_ns();
            (void)out_act_q.try_push(ac);  // drop if full — telemetry only
        }

        // ── 6. Phase 7 — when FOV follow-target is enabled AND a valid
        //         selection exists, project the chosen target's image-space
        //         position back into normalised FOV cx/cy. The selector's
        //         dx/dy are offsets from the image centre, so:
        //              target_u = image_w/2 + dx
        //              target_v = image_h/2 + dy
        //         We low-pass the target position with a small EMA (alpha =
        //         0.35) so the FOV crop window doesn't jitter every frame.
        //         The crop is applied by the inference thread on its next
        //         read of FovStore::snapshot().
        if (fov_store != nullptr) {
            auto s = fov_store->snapshot();
            if (s.enabled && s.follow_target && sel.valid) {
                const float ux = static_cast<float>(0.5 * image_w + sel.dx) /
                                 static_cast<float>(image_w);
                const float vy = static_cast<float>(0.5 * image_h + sel.dy) /
                                 static_cast<float>(image_h);
                constexpr float kFollowAlpha = 0.35f;
                s.cx_norm = (1.0f - kFollowAlpha) * s.cx_norm + kFollowAlpha * ux;
                s.cy_norm = (1.0f - kFollowAlpha) * s.cy_norm + kFollowAlpha * vy;
                fov_store->set(s);
            }
        }

        tracker_steps.fetch_add(1, std::memory_order_relaxed);
    }
}

// ---------------------------------------------------------------------------
// Dashboard egress thread — sole caller of WsEgressServer::broadcast (T01.5).
// Round-robin drains track wire frames and actuator telemetry wire frames.
// ---------------------------------------------------------------------------
void dashboard_egress_thread(roc::transport::TrackerToWireQueue&      track_q,
                             roc::transport::ActuatorToEgressQueue&   act_q,
                             roc::transport::BgOdometryToEgressQueue& bg_q,
                             roc::transport::WsEgressServer&           server,
                             std::atomic<bool>&                        shutdown,
                             std::atomic<std::uint64_t>&               track_frames_broadcast,
                             std::atomic<std::uint64_t>&               actuator_frames_broadcast,
                             std::atomic<std::uint64_t>&               bg_frames_broadcast) {
    using namespace std::chrono;

    while (!shutdown.load(std::memory_order_acquire)) {
        bool did_work = false;

        if (auto* wf_ptr = track_q.front()) {
            roc::transport::WireFrame wf = std::move(*wf_ptr);
            track_q.pop();
            if (!wf.bytes.empty()) {
                server.broadcast(wf.bytes.data(), wf.bytes.size());
                track_frames_broadcast.fetch_add(1, std::memory_order_relaxed);
            }
            did_work = true;
        }

        if (auto* af_ptr = act_q.front()) {
            roc::transport::ActuatorWireFrame af = std::move(*af_ptr);
            act_q.pop();
            server.broadcast(af.bytes.data(), af.bytes.size());
            actuator_frames_broadcast.fetch_add(1, std::memory_order_relaxed);
            did_work = true;
        }

        if (auto* bf_ptr = bg_q.front()) {
            roc::transport::BgOdometryWireFrame bf = std::move(*bf_ptr);
            bg_q.pop();
            server.broadcast(bf.bytes.data(), bf.bytes.size());
            bg_frames_broadcast.fetch_add(1, std::memory_order_relaxed);
            did_work = true;
        }

        if (!did_work) {
            std::this_thread::sleep_for(microseconds(200));
        }
    }

    while (auto* wf_ptr = track_q.front()) {
        (void)wf_ptr;
        track_q.pop();
    }
    while (auto* af_ptr = act_q.front()) {
        (void)af_ptr;
        act_q.pop();
    }
    while (auto* bf_ptr = bg_q.front()) {
        (void)bf_ptr;
        bg_q.pop();
    }
}

// ---------------------------------------------------------------------------
// Optical-flow thread (§13.3) — consumes camera fanout, emits v3 wire frames.
// ---------------------------------------------------------------------------
void optical_flow_thread(roc::transport::CameraToOpticalFlowQueue&  in_q,
                         roc::transport::BgOdometryToEgressQueue&   out_q,
                         std::atomic<bool>&                         shutdown,
                         std::atomic<std::uint64_t>&                bg_samples_emitted) {
    using namespace std::chrono;
    roc::vision::OpticalFlowSampler sampler;

    while (!shutdown.load(std::memory_order_acquire)) {
        auto* slot = in_q.front();
        if (slot == nullptr) {
            std::this_thread::sleep_for(microseconds(500));
            continue;
        }
        roc::transport::CameraFrame frame = std::move(*slot);
        in_q.pop();
        if (frame.frame.empty()) continue;

        const auto sample = sampler.sample(
            frame.frame, frame.frame_seq, frame.capture_ts_ns);
        if (!sample.has_value()) continue;

        roc::transport::BgOdometryWirePayload payload{};
        payload.frame_id      = sample->frame_id;
        payload.ts_ns         = static_cast<std::uint64_t>(sample->ts_ns);
        payload.feature_count = sample->feature_count;
        payload.inlier_count  = sample->inlier_count;
        payload.bg_dx_px      = sample->dx_px;
        payload.bg_dy_px      = sample->dy_px;
        payload.bg_theta_rad  = sample->theta_rad;
        payload.confidence_q15 =
            roc::transport::float_to_q15(sample->confidence);
        payload.flags = 0x04u;  // center_mask_active
        if (sample->valid_estimate) payload.flags |= 0x01u;
        if (sample->feature_count < 50) payload.flags |= 0x02u;

        roc::transport::BgOdometryWireFrame wf{};
        wf.frame_id = sample->frame_id;
        wf.emit_ns  = sample->ts_ns;
        wf.bytes    = roc::transport::pack_bg_odometry(payload);
        (void)out_q.try_push(std::move(wf));
        bg_samples_emitted.fetch_add(1, std::memory_order_relaxed);
    }

    while (auto* slot = in_q.front()) {
        (void)slot;
        in_q.pop();
    }
}

// ---------------------------------------------------------------------------
// §12.4 — actuator-telemetry-emit thread (500 Hz default).  Reads the latest
// ActuatorCommand from the
// TrackerToActuatorQueue (drained to most-recent), formats the version=2
// 36-byte wire frame (§12.4), pumps it through the NoOpSink (§11.4 stub),
// and broadcasts to the dashboard.
//
// Wire layout (little-endian, total 36 B per §12.4):
//   0:1   u8  version          (= 2)
//   1:1   u8  flags             bit 0 = smoothed-only; bit 1 = raw-only;
//                               both = both populated; bit 4 = config_change_marker.
//                               Phase 6 part A emits raw-only (bit 1 set) —
//                               smoothing pipeline lands in Phase 5c.
//   2:4   u32 frame_id
//   6:8   u64 ts_ns
//  14:4   f32 raw_u_px
//  18:4   f32 raw_v_px
//  22:4   f32 clean_u_px       (NaN — no smoothing yet)
//  26:4   f32 clean_v_px       (NaN)
//  30:2   u16 sink_kind        (= OutputSinkKind::NoOp == 0)
//  32:4   f32 applied_alpha    (= NaN — §12.3 slider feedback not wired)
// ---------------------------------------------------------------------------
constexpr std::size_t kActuatorWireFrameBytes = 36;
constexpr std::uint8_t kActuatorWireVersion   = 2;
constexpr std::uint8_t kActuatorFlagsRawOnly  = 0x02;
constexpr std::uint8_t kActuatorFlagsBoth     = 0x03;  // raw + clean populated
inline void write_u8 (std::uint8_t*  p, std::uint8_t  v) noexcept { *p = v; }
inline void write_u16(std::uint8_t*  p, std::uint16_t v) noexcept {
    p[0] = static_cast<std::uint8_t>(v & 0xFFu);
    p[1] = static_cast<std::uint8_t>((v >> 8) & 0xFFu);
}
inline void write_u32(std::uint8_t*  p, std::uint32_t v) noexcept {
    for (int i = 0; i < 4; ++i) p[i] = static_cast<std::uint8_t>((v >> (8 * i)) & 0xFFu);
}
inline void write_u64(std::uint8_t*  p, std::uint64_t v) noexcept {
    for (int i = 0; i < 8; ++i) p[i] = static_cast<std::uint8_t>((v >> (8 * i)) & 0xFFu);
}
inline void write_f32(std::uint8_t*  p, float v) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, &v, 4);
    write_u32(p, bits);
}

void actuator_telemetry_emit_thread(
        roc::transport::TrackerToActuatorQueue& in_q,
        roc::transport::ActuatorToEgressQueue&  egress_q,
        roc::transport::NoOpSink&               sink,
        const TrackerRuntimeConfig&             runtime_cfg,
        double                                  selector_cx,
        double                                  selector_cy,
        int                                     emit_hz,
        std::atomic<bool>&                      shutdown,
        std::atomic<std::uint64_t>&             frames_emitted) {
    using namespace std::chrono;
    const auto period = nanoseconds{ 1'000'000'000LL / emit_hz };

    auto next_tick = steady_clock::now();
    roc::transport::ActuatorCommand latest{};
    bool have_latest = false;
    std::uint32_t local_frame = 0;
    float clean_u = 0.0f;
    float clean_v = 0.0f;
    bool  smoother_seeded = false;
    float last_applied_alpha = -1.0f;

    while (!shutdown.load(std::memory_order_acquire)) {
        while (auto* slot = in_q.front()) {
            latest = *slot;
            in_q.pop();
            have_latest = true;
        }

        ++local_frame;
        const float alpha = runtime_cfg.linear_smoothing_alpha.load(
            std::memory_order_acquire);
        const float raw_u = have_latest
            ? static_cast<float>(selector_cx + static_cast<double>(latest.dx_px))
            : std::numeric_limits<float>::quiet_NaN();
        const float raw_v = have_latest
            ? static_cast<float>(selector_cy + static_cast<double>(latest.dy_px))
            : std::numeric_limits<float>::quiet_NaN();

        std::uint8_t flags = kActuatorFlagsRawOnly;
        if (have_latest && std::isfinite(raw_u) && std::isfinite(raw_v)) {
            if (!smoother_seeded) {
                clean_u = raw_u;
                clean_v = raw_v;
                smoother_seeded = true;
            } else {
                const float one_minus = 1.0f - alpha;
                clean_u = alpha * raw_u + one_minus * clean_u;
                clean_v = alpha * raw_v + one_minus * clean_v;
            }
            flags = kActuatorFlagsBoth;
        }

        if (last_applied_alpha >= 0.0f
            && std::fabs(alpha - last_applied_alpha) > 1.0e-6f) {
            flags |= 0x10u;  // config_change_marker (§12.4 bit 4)
        }
        last_applied_alpha = alpha;

        roc::transport::ActuatorWireFrame awf{};
        awf.frame_id = local_frame;
        awf.emit_ns  = now_steady_ns();
        std::uint8_t* const wire = awf.bytes.data();

        write_u8 (wire +  0, kActuatorWireVersion);
        write_u8 (wire +  1, flags);
        write_u32(wire +  2, local_frame);
        write_u64(wire +  6, static_cast<std::uint64_t>(awf.emit_ns));
        write_f32(wire + 14, raw_u);
        write_f32(wire + 18, raw_v);
        const float nan_f = std::numeric_limits<float>::quiet_NaN();
        const float out_clean_u =
            (flags == kActuatorFlagsBoth) ? clean_u : nan_f;
        const float out_clean_v =
            (flags == kActuatorFlagsBoth) ? clean_v : nan_f;
        write_f32(wire + 22, out_clean_u);
        write_f32(wire + 26, out_clean_v);
        write_u16(wire + 30,
                  static_cast<std::uint16_t>(roc::transport::OutputSinkKind::NoOp));
        write_f32(wire + 32, alpha);

        roc::transport::ActuatorEmission e{};
        e.target_track_id  = have_latest ? latest.target_track_id : 0u;
        e.command_u_px     = (flags & kActuatorFlagsBoth) ? clean_u : raw_u;
        e.command_v_px     = (flags & kActuatorFlagsBoth) ? clean_v : raw_v;
        e.delta_u_per_tick = have_latest ? latest.dx_px : 0.0f;
        e.delta_v_per_tick = have_latest ? latest.dy_px : 0.0f;
        e.emit_ts_ns       = awf.emit_ns;
        e.valid            = have_latest;
        sink.emit(e);

        (void)egress_q.try_push(std::move(awf));
        frames_emitted.fetch_add(1, std::memory_order_relaxed);

        next_tick += period;
        const auto now = steady_clock::now();
        if (next_tick > now) {
            std::this_thread::sleep_until(next_tick);
        } else {
            next_tick = now + period;
        }
    }

    while (auto* slot = in_q.front()) {
        (void)slot;
        in_q.pop();
    }
}

// ---------------------------------------------------------------------------
// Camera-bringup helper — opens the requested device, applies width/height
// hints, prints the resolved capture format.  Returns false on failure with
// a human-readable message; caller propagates to the SIGINT shutdown path.
// ---------------------------------------------------------------------------
bool open_camera(cv::VideoCapture& cap, const OrchestratorOptions& opts,
                 int& resolved_w, int& resolved_h, double& resolved_fps) {
    // CAP_ANY lets OpenCV pick the best Windows backend (MSMF on Win10+).
    // Falling back to CAP_DSHOW is automatic if MSMF is missing.
    if (!cap.open(opts.camera_index, cv::CAP_ANY)) {
        std::cerr << "[roc-orchestrator] could NOT open camera index "
                  << opts.camera_index
                  << " — already in use (Teams/Zoom/OBS) or no device?\n";
        return false;
    }
    if (opts.camera_width  > 0) cap.set(cv::CAP_PROP_FRAME_WIDTH,  opts.camera_width);
    if (opts.camera_height > 0) cap.set(cv::CAP_PROP_FRAME_HEIGHT, opts.camera_height);

    // Warm-up read to force the backend to fully initialise (some MSMF
    // drivers report 0×0 until the first grab returns).
    cv::Mat probe;
    for (int i = 0; i < 10; ++i) {
        if (cap.read(probe) && !probe.empty()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    if (probe.empty()) {
        std::cerr << "[roc-orchestrator] camera opened but returned no frames\n";
        return false;
    }
    resolved_w = probe.cols;
    resolved_h = probe.rows;
    resolved_fps = cap.get(cv::CAP_PROP_FPS);
    if (!std::isfinite(resolved_fps) || resolved_fps <= 0.0) resolved_fps = 30.0;
    return true;
}

// ---------------------------------------------------------------------------
// Build a JSON snapshot of the currently active InferenceEngine's manifest.
// Used by GET /api/model/active. Hand-rolled to avoid pulling nlohmann::json.
// ---------------------------------------------------------------------------
inline std::string active_model_json(const roc::vision::InferenceEngine& engine) {
    const auto& m = engine.manifest();
    std::ostringstream os;
    os << "{";
    os << "\"name\":\""        << m.name           << "\",";
    os << "\"purpose\":\""     << (m.head_string == "osnet_reid" ? "reid" : "detection") << "\",";
    os << "\"head\":\""        << m.head_string    << "\",";
    os << "\"inputW\":"        << m.input_w()      << ",";
    os << "\"inputH\":"        << m.input_h()      << ",";
    os << "\"numClasses\":"    << m.output.num_classes << ",";
    os << "\"ep\":\""          << roc::vision::to_string(engine.ep_result().chosen) << "\",";
    os << "\"epDescription\":\"" << engine.ep_result().device_description << "\"";
    os << "}";
    return os.str();
}

// ---------------------------------------------------------------------------
// Enumerate every *.yaml in `models_dir`, attempt to load each as a manifest,
// and return a ModelInfo for every entry (including failures so the
// dashboard can show what's broken). Each manifest's matching .onnx must
// share the same stem in the same directory; missing ONNX files are marked
// `loadable=false`.
// ---------------------------------------------------------------------------
inline std::vector<roc::transport::ModelInfo>
enumerate_models(const std::filesystem::path&         models_dir,
                 const roc::vision::InferenceEngine*  active_engine) {
    std::vector<roc::transport::ModelInfo> out;
    if (models_dir.empty() || !std::filesystem::is_directory(models_dir)) {
        return out;
    }
    const std::string active_name = active_engine ? active_engine->manifest().name : "";

    std::vector<std::filesystem::path> yamls;
    std::error_code ec;
    for (auto& entry : std::filesystem::directory_iterator(models_dir, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() == ".yaml" || entry.path().extension() == ".yml") {
            yamls.push_back(entry.path());
        }
    }
    std::sort(yamls.begin(), yamls.end());

    for (const auto& yaml_path : yamls) {
        roc::transport::ModelInfo info{};
        info.manifest_path = yaml_path.string();

        std::filesystem::path onnx_candidate = yaml_path;
        onnx_candidate.replace_extension(".onnx");
        info.onnx_path = onnx_candidate.string();
        const bool onnx_exists = std::filesystem::exists(onnx_candidate);

        std::string err;
        auto opt_spec = roc::vision::ManifestLoader::load(yaml_path, &err);
        if (opt_spec) {
            const auto& m = *opt_spec;
            info.name        = m.name;
            info.head        = m.head_string;
            info.purpose     = (m.head_string == "osnet_reid") ? "reid" : "detection";
            info.input_w     = m.input_w();
            info.input_h     = m.input_h();
            info.num_classes = m.output.num_classes;
            info.loadable    = onnx_exists;
            info.status_note = onnx_exists ? "ready" : "missing .onnx alongside .yaml";
        } else {
            info.name        = yaml_path.stem().string();
            info.head        = "";
            info.purpose     = "";
            info.loadable    = false;
            info.status_note = "manifest error: " + err;
        }
        info.active = !info.name.empty() && info.name == active_name;
        out.push_back(std::move(info));
    }
    return out;
}

// ---------------------------------------------------------------------------
// Resolve a requested model NAME (manifest 'name' field) under `models_dir`
// to the (manifest_path, onnx_path) pair. Returns false if not found OR
// the ONNX is missing.
// ---------------------------------------------------------------------------
inline bool resolve_model_name(const std::filesystem::path& models_dir,
                               const std::string&           wanted_name,
                               std::filesystem::path&       out_manifest,
                               std::filesystem::path&       out_onnx,
                               std::string&                 out_err) {
    std::error_code ec;
    if (!std::filesystem::is_directory(models_dir)) {
        out_err = "models_dir is not a directory";
        return false;
    }
    for (auto& entry : std::filesystem::directory_iterator(models_dir, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;
        const auto ext = entry.path().extension();
        if (ext != ".yaml" && ext != ".yml") continue;
        std::string err;
        auto opt = roc::vision::ManifestLoader::load(entry.path(), &err);
        if (!opt) continue;
        if (opt->name != wanted_name) continue;
        out_manifest = entry.path();
        out_onnx     = entry.path();
        out_onnx.replace_extension(".onnx");
        if (!std::filesystem::exists(out_onnx)) {
            out_err = "manifest found but .onnx is missing at " + out_onnx.string();
            return false;
        }
        return true;
    }
    out_err = "no manifest named '" + wanted_name + "' found in models_dir";
    return false;
}

// ---------------------------------------------------------------------------
// inference_thread — Phase 1 originally, Phase 6 FOV-crop extension.
//
// If `fov_store` is non-null and FOV is enabled, the frame is cropped to a
// centred square (resolve_crop above) BEFORE inference. Detections returned
// from the model are then translated back into source-frame coordinates by
// adding the crop offset to bbox x/y. This means tracker / EKF / dashboard
// all continue to see source-frame coordinates and need no changes.
// ---------------------------------------------------------------------------
void inference_thread(roc::transport::CameraToInferenceQueue&   in_q,
                      roc::transport::InferenceToTrackerQueue&  out_q,
                      roc::vision::InferenceEngine&             engine,
                      FovStore*                                 fov_store,
                      std::atomic<bool>&                        shutdown,
                      std::atomic<std::uint64_t>&               frames_processed,
                      std::atomic<std::uint64_t>&               total_detections,
                      std::atomic<std::int64_t>&                last_latency_ns) {
    using namespace std::chrono;

    while (!shutdown.load(std::memory_order_acquire)) {
        auto* slot = in_q.front();
        if (slot == nullptr) {
            std::this_thread::sleep_for(microseconds(500));
            continue;
        }
        roc::transport::CameraFrame frame = std::move(*slot);
        in_q.pop();
        if (frame.frame.empty()) continue;

        // ---- FOV crop (Phase 6) -----------------------------------------
        // Snapshot the settings once per frame so the API thread can't change
        // them mid-inference. Tiny critical section.
        CropRect crop{0, 0, frame.frame.cols, frame.frame.rows};
        if (fov_store) {
            roc::transport::FovSettings s = fov_store->snapshot();
            crop = resolve_crop(s, frame.frame.cols, frame.frame.rows);
            // Publish the actual crop rect so the dashboard ring can mirror it.
            fov_store->update_active_crop(frame.frame.cols, frame.frame.rows,
                                          crop.x, crop.y, crop.w, crop.h);
        }
        const bool is_cropped =
            (crop.x != 0 || crop.y != 0 ||
             crop.w != frame.frame.cols || crop.h != frame.frame.rows);

        // cv::Mat ROI is a zero-copy view into the parent buffer, so cropping
        // costs nothing measurable. The engine's letterboxer will then
        // resize-and-pad this smaller region into the model's input shape.
        cv::Mat infer_input = is_cropped
            ? cv::Mat(frame.frame, cv::Rect(crop.x, crop.y, crop.w, crop.h))
            : frame.frame;

        const std::int64_t t0 = now_steady_ns();
        std::vector<roc::vision::Detection> dets = engine.run(infer_input);
        const std::int64_t t1 = now_steady_ns();
        last_latency_ns.store(t1 - t0, std::memory_order_relaxed);

        // ---- Un-crop: translate bbox centers back to source-frame coords -
        // Detection carries the bbox as (centre, w, h) in the coordinate
        // system of the cv::Mat fed to engine.run() — here that's the
        // crop's local frame. Translating the centre by the crop offset
        // restores source-frame coordinates; w/h are unchanged.
        if (is_cropped) {
            const double dx = static_cast<double>(crop.x);
            const double dy = static_cast<double>(crop.y);
            for (auto& d : dets) {
                d.bbox_center.u += dx;
                d.bbox_center.v += dy;
            }
        }

        roc::transport::DetectionBatch batch{};
        batch.frame_id   = frame.frame_seq;
        batch.capture_ns = frame.capture_ts_ns;
        for (const auto& d : dets) {
            if (!batch.push_back(d)) break;   // §kMaxDetectionsPerBatch guard
        }
        total_detections.fetch_add(static_cast<std::uint64_t>(batch.count),
                                   std::memory_order_relaxed);
        (void)out_q.try_push(std::move(batch));
        frames_processed.fetch_add(1, std::memory_order_relaxed);
    }
}

}  // namespace

// ===========================================================================
// main — wire up the five threads + WS server + signal handler + Ort::Env;
// block until shutdown; perform graceful teardown in reverse-startup order.
// ===========================================================================
int main(int argc, char** argv) {
    OrchestratorOptions opts{};
    if (!parse_args(argc, argv, opts)) {
        print_usage();
        return 2;
    }
    if (opts.help) {
        print_usage();
        return 0;
    }
    resolve_model_paths(opts);

    if (opts.model_path.empty()) {
        std::cerr << "[roc-orchestrator] no --model and no $ROC_MODEL_ONNX — "
                     "cannot start without an ONNX model\n";
        print_usage();
        return 2;
    }
    if (!std::filesystem::exists(opts.model_path)) {
        std::cerr << "[roc-orchestrator] model file not found: "
                  << opts.model_path << "\n";
        return 2;
    }
    if (!std::filesystem::exists(opts.manifest_path)) {
        std::cerr << "[roc-orchestrator] manifest file not found: "
                  << opts.manifest_path
                  << " — pass --manifest <path.yaml>\n";
        return 2;
    }

    std::cout << "[roc-orchestrator] ROC AI Vision — Phase 6 part A bring-up\n"
              << "[roc-orchestrator] ws=" << opts.ws_bind << ":" << opts.ws_port
              << ", demo_duration=" << opts.demo_duration_s
              << (opts.demo_duration_s == 0.0 ? " s (run until Ctrl-C)\n" : " s\n")
              << "[roc-orchestrator] camera=" << opts.camera_index
              << ", model=\""   << opts.model_path    << "\"\n"
              << "[roc-orchestrator] manifest=\"" << opts.manifest_path << "\"\n"
              << "[roc-orchestrator] actuator_telemetry="
              << (opts.enable_actuator_tx
                      ? ("on (" + std::to_string(opts.actuator_emit_hz) + " Hz)")
                      : "off")
              << "\n";

    // ── Signal handlers ─────────────────────────────────────────────────
    std::signal(SIGINT, sigint_handler);
#ifdef SIGTERM
    std::signal(SIGTERM, sigint_handler);
#endif

    // ── Open the frame source FIRST (fail fast before standing up other state).
    //    Mutually exclusive: either a cv::VideoCapture (webcam / capture card)
    //    OR a DesktopCapture (Phase 8 — screen / window / region). We never
    //    open both; the desktop path completely supersedes camera_index when
    //    --desktop-capture is something other than "none".
    cv::VideoCapture cap;
#ifdef _WIN32
    roc::transport::DesktopCapture desktop_cap;
#endif
    int    cam_w = 640, cam_h = 480;
    double cam_fps = 30.0;
    const bool use_desktop = (opts.desktop_kind != "none");

    if (use_desktop) {
#ifdef _WIN32
        roc::transport::DesktopCaptureConfig dc{};
        if      (opts.desktop_kind == "primary") dc.kind = roc::transport::DesktopCaptureKind::PrimaryMonitor;
        else if (opts.desktop_kind == "monitor") dc.kind = roc::transport::DesktopCaptureKind::Monitor;
        else if (opts.desktop_kind == "window")  dc.kind = roc::transport::DesktopCaptureKind::Window;
        else if (opts.desktop_kind == "region")  dc.kind = roc::transport::DesktopCaptureKind::Region;
        dc.monitor_index           = opts.desktop_monitor_index;
        dc.window_title_substring  = opts.desktop_window_title;
        dc.region_x                = opts.desktop_region_x;
        dc.region_y                = opts.desktop_region_y;
        dc.region_w                = opts.desktop_region_w;
        dc.region_h                = opts.desktop_region_h;
        dc.target_fps              = opts.desktop_fps;
        dc.downscale_max_dim       = opts.desktop_downscale;
        dc.capture_cursor          = opts.desktop_show_cursor;

        if (!desktop_cap.open(dc)) {
            std::cerr << "[roc-orchestrator] desktop capture failed to open: "
                      << desktop_cap.last_error() << "\n";
            return 4;
        }
        cam_w   = desktop_cap.width();
        cam_h   = desktop_cap.height();
        cam_fps = static_cast<double>(opts.desktop_fps);
        std::cout << "[roc-orchestrator] desktop capture: "
                  << desktop_cap.source_description()
                  << " @ " << opts.desktop_fps << " fps target\n";
#else
        std::cerr << "[roc-orchestrator] --desktop-capture is only supported on Windows builds.\n";
        return 4;
#endif
    } else {
        if (!open_camera(cap, opts, cam_w, cam_h, cam_fps)) {
            return 4;
        }
        std::cout << "[roc-orchestrator] camera resolved to "
                  << cam_w << "x" << cam_h
                  << " @ " << cam_fps << " fps\n";
    }

    // ── Build the ONNX runtime singletons (Ort::Env + postprocess registry). ─
    Ort::Env ort_env(ORT_LOGGING_LEVEL_WARNING, "roc_vision");
    roc::vision::PostprocessorRegistry registry;

    // ── Construct the InferenceEngine — manifest validation + EP probe + session.
    std::unique_ptr<roc::vision::InferenceEngine> engine;
    try {
        engine = std::make_unique<roc::vision::InferenceEngine>(
            ort_env,
            roc::vision::ModelPath{ std::filesystem::path(opts.model_path) },
            std::filesystem::path(opts.manifest_path),
            registry);
    } catch (const std::exception& e) {
        std::cerr << "[roc-orchestrator] failed to construct InferenceEngine: "
                  << e.what() << "\n";
        cap.release();
        return 5;
    }
    std::cout << "[roc-orchestrator] InferenceEngine ready — "
              << "manifest=\"" << engine->manifest().name
              << "\" head=\""  << engine->manifest().head_string
              << "\" ep="      << roc::vision::to_string(engine->ep_result().chosen)
              << " (" << engine->ep_result().device_description << ")\n";

    // ── SPSC queues ─────────────────────────────────────────────────────
    roc::transport::CameraToInferenceQueue  cam_q (roc::transport::kCameraToInferenceCapacity);
    roc::transport::CameraToOpticalFlowQueue optflow_q(
        roc::transport::kCameraToOpticalFlowCapacity);
    roc::transport::BgOdometryToEgressQueue bg_egress_q(
        roc::transport::kBgOdometryToEgressCapacity);
    roc::transport::InferenceToTrackerQueue det_q (roc::transport::kInferenceToTrackerCapacity);
    roc::transport::TrackerToActuatorQueue  act_q (roc::transport::kTrackerToActuatorCapacity);
    roc::transport::TrackerToWireQueue      wire_q(roc::transport::kTrackerToWireCapacity);
    roc::transport::ActuatorToEgressQueue   act_egress_q(
        roc::transport::kActuatorToEgressCapacity);
    roc::transport::OperatorCommandQueue    cmd_q (roc::transport::kOperatorCommandCapacity);
    TrackerRuntimeConfig                    runtime_cfg{};

    // ── §11.4 OutputSink — NoOpSink for Phase 6 part A.
    roc::transport::NoOpSink sink;

    // ── WS egress server. ───────────────────────────────────────────────
    roc::transport::WsEgressServer server(opts.ws_port, opts.ws_bind);
    server.set_operator_command_callback(
        [&cmd_q](const roc::transport::OperatorCommand& cmd) noexcept {
            (void)cmd_q.try_push(cmd);
        });
    try {
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "[roc-orchestrator] failed to start WS server: "
                  << e.what() << "\n";
        cap.release();
        return 3;
    }
    std::cout << "[roc-orchestrator] WS server listening on "
              << opts.ws_bind << ":" << server.listen_port() << "\n";

    // ── MJPEG-over-HTTP server (dashboard Layer 0 + /sources enumeration). ──
    // Lives on a separate TCP port from the WS bridge so HTTP fetches don't
    // contend with binary WS frames. Disabled via --no-mjpeg.
    std::unique_ptr<roc::transport::MjpegServer> mjpeg;
    if (opts.enable_mjpeg) {
        try {
            mjpeg = std::make_unique<roc::transport::MjpegServer>(
                opts.mjpeg_port, opts.ws_bind, opts.mjpeg_quality);
            // Populate the /sources device list with what the orchestrator
            // currently sees. v1: we only know the camera we just opened;
            // future versions will enumerate all cv::VideoCapture indices.
            std::vector<roc::transport::CameraSourceInfo> sources;
            roc::transport::CameraSourceInfo cur{};
            cur.id            = "webcam-" + std::to_string(opts.camera_index);
            cur.kind          = "webcam";
            cur.label         = "Active Camera (cv::VideoCapture index "
                              + std::to_string(opts.camera_index) + ")";
            cur.device_index  = opts.camera_index;
            cur.width         = cam_w;
            cur.height        = cam_h;
            cur.fps           = cam_fps;
            cur.online        = true;
            sources.push_back(cur);
            mjpeg->set_sources(std::move(sources));
            mjpeg->start();
            std::cout << "[roc-orchestrator] MJPEG server listening on "
                      << opts.ws_bind << ":" << mjpeg->listen_port()
                      << " (quality=" << opts.mjpeg_quality << ")\n"
                      << "[roc-orchestrator]   stream: http://" << opts.ws_bind
                      << ":" << opts.mjpeg_port << "/mjpeg/" << opts.camera_index << "\n"
                      << "[roc-orchestrator]   sources: http://" << opts.ws_bind
                      << ":" << opts.mjpeg_port << "/sources\n";
        } catch (const std::exception& e) {
            std::cerr << "[roc-orchestrator] MJPEG server start failed (continuing without): "
                      << e.what() << "\n";
            mjpeg.reset();
        }
    } else {
        std::cout << "[roc-orchestrator] MJPEG server disabled (--no-mjpeg)\n";
    }

    // ── Phase 6: shared FOV crop state (API thread writes, inference reads). ─
    FovStore fov_store;

    // ── Phase 4: register the HTTP REST API handlers on the MJPEG server. ──
    // The handlers capture `engine.get()`, `registry`, `opts.models_dir`, and
    // `&fov_store` by reference; they outlive the API thread because mjpeg->
    // stop() runs BEFORE these objects go out of scope at end-of-main.
    if (mjpeg) {
        roc::transport::ApiHandlers api{};

        roc::vision::InferenceEngine*           engine_ptr   = engine.get();
        roc::vision::PostprocessorRegistry*     registry_ptr = &registry;
        const std::filesystem::path             models_dir   = opts.models_dir;

        api.list_models =
            [engine_ptr, models_dir]() {
                return enumerate_models(models_dir, engine_ptr);
            };
        api.active_model_json =
            [engine_ptr]() -> std::string {
                if (!engine_ptr) return "{}";
                return active_model_json(*engine_ptr);
            };
        api.switch_model =
            [engine_ptr, registry_ptr, models_dir](const std::string& name) -> std::string {
                std::ostringstream os;
                if (!engine_ptr) {
                    os << "{\"ok\":false,\"error\":\"no engine\"}";
                    return os.str();
                }
                std::filesystem::path manifest_path, onnx_path;
                std::string err;
                if (!resolve_model_name(models_dir, name, manifest_path, onnx_path, err)) {
                    os << "{\"ok\":false,\"error\":\"" << err << "\"}";
                    return os.str();
                }
                auto result = engine_ptr->try_swap_model(
                    roc::vision::ModelPath{onnx_path}, manifest_path, *registry_ptr);
                if (!result.loaded) {
                    os << "{\"ok\":false,\"error\":\"" << result.error << "\"}";
                    return os.str();
                }
                os << "{\"ok\":true,\"active\":\"" << result.active_model
                   << "\",\"ep\":\""    << roc::vision::to_string(result.active_ep)
                   << "\"}";
                return os.str();
            };
        api.get_fov = [&fov_store]() { return fov_store.snapshot(); };
        api.set_fov = [&fov_store](const roc::transport::FovSettings& s) { fov_store.set(s); };

#ifdef _WIN32
        api.list_capture_sources = []() {
            roc::transport::CaptureSourcesSnapshot snap{};
            for (const auto& m : roc::transport::enumerate_monitors()) {
                roc::transport::CaptureMonitorInfo cm{};
                cm.index       = m.index;
                cm.x           = m.x;
                cm.y           = m.y;
                cm.w           = m.w;
                cm.h           = m.h;
                cm.is_primary  = m.is_primary;
                cm.device_name = m.device_name;
                snap.monitors.push_back(std::move(cm));
            }
            for (const auto& w : roc::transport::enumerate_top_level_windows()) {
                roc::transport::CaptureWindowInfo cw{};
                cw.hwnd      = w.hwnd;
                cw.title     = w.title;
                cw.x         = w.x;
                cw.y         = w.y;
                cw.w         = w.w;
                cw.h         = w.h;
                cw.minimized = w.minimized;
                snap.windows.push_back(std::move(cw));
            }
            return snap;
        };
#endif

        mjpeg->set_api_handlers(std::move(api));
        std::cout << "[roc-orchestrator]   models:   http://" << opts.ws_bind
                  << ":" << opts.mjpeg_port << "/api/models\n"
                  << "[roc-orchestrator]   fov:      http://" << opts.ws_bind
                  << ":" << opts.mjpeg_port << "/api/fov\n"
                  << "[roc-orchestrator]   capture:  http://" << opts.ws_bind
                  << ":" << opts.mjpeg_port << "/api/capture-sources\n";
    }

    // ── Telemetry counters ──────────────────────────────────────────────
    std::atomic<bool>          shutdown_flag{false};
    std::atomic<std::uint64_t> cam_frames_captured{0};
    std::atomic<std::uint64_t> cam_frames_dropped{0};
    std::atomic<std::uint64_t> inf_frames_processed{0};
    std::atomic<std::uint64_t> inf_total_detections{0};
    std::atomic<std::int64_t>  inf_last_latency_ns{0};
    std::atomic<std::uint64_t> tracker_steps{0};
    std::atomic<std::uint64_t> tracker_dropped{0};
    std::atomic<std::uint64_t> egress_track_frames{0};
    std::atomic<std::uint64_t> egress_actuator_frames{0};
    std::atomic<std::uint64_t> egress_bg_frames{0};
    std::atomic<std::uint64_t> bg_samples_emitted{0};
    std::atomic<std::uint64_t> act_tx_frames{0};

    // ── Threads (start order: source → inference → tracker → egress → actuator). ─
    // The source thread differs depending on the configured frame origin: a
    // cv::VideoCapture (webcam / capture card) OR a DesktopCapture (Phase 8
    // screen / window / region). Everything downstream is identical.
    std::thread t_camera;
    if (use_desktop) {
#ifdef _WIN32
        t_camera = std::thread(desktop_capture_thread,
                               std::ref(desktop_cap),
                               std::ref(cam_q),
                               std::ref(optflow_q),
                               mjpeg.get(),
                               opts.camera_index,
                               std::ref(shutdown_flag),
                               std::ref(cam_frames_captured),
                               std::ref(cam_frames_dropped));
#endif
    } else {
        t_camera = std::thread(camera_capture_thread,
                               std::ref(cap),
                               std::ref(cam_q),
                               std::ref(optflow_q),
                               mjpeg.get(),                  // MJPEG sink (nullptr if disabled)
                               opts.camera_index,            // for /mjpeg/{idx} routing
                               std::ref(shutdown_flag),
                               std::ref(cam_frames_captured),
                               std::ref(cam_frames_dropped));
    }

    std::thread t_optflow(optical_flow_thread,
                          std::ref(optflow_q),
                          std::ref(bg_egress_q),
                          std::ref(shutdown_flag),
                          std::ref(bg_samples_emitted));

    std::thread t_inference(inference_thread,
                            std::ref(cam_q),
                            std::ref(det_q),
                            std::ref(*engine),
                            &fov_store,
                            std::ref(shutdown_flag),
                            std::ref(inf_frames_processed),
                            std::ref(inf_total_detections),
                            std::ref(inf_last_latency_ns));

    std::thread t_tracker(tracker_thread,
                          std::ref(det_q),
                          std::ref(cmd_q),
                          std::ref(wire_q),
                          std::ref(act_q),
                          std::ref(runtime_cfg),
                          &fov_store,
                          cam_w,
                          cam_h,
                          opts.enable_actuator_tx,
                          std::ref(shutdown_flag),
                          std::ref(tracker_steps),
                          std::ref(tracker_dropped));

    std::thread t_egress(dashboard_egress_thread,
                         std::ref(wire_q),
                         std::ref(act_egress_q),
                         std::ref(bg_egress_q),
                         std::ref(server),
                         std::ref(shutdown_flag),
                         std::ref(egress_track_frames),
                         std::ref(egress_actuator_frames),
                         std::ref(egress_bg_frames));

    std::thread t_actuator;
    if (opts.enable_actuator_tx) {
        t_actuator = std::thread(actuator_telemetry_emit_thread,
                                 std::ref(act_q),
                                 std::ref(act_egress_q),
                                 std::ref(sink),
                                 std::ref(runtime_cfg),
                                 0.5 * static_cast<double>(cam_w),
                                 0.5 * static_cast<double>(cam_h),
                                 opts.actuator_emit_hz,
                                 std::ref(shutdown_flag),
                                 std::ref(act_tx_frames));
    }

    // ── Main loop — wait for SIGINT or duration timeout. ────────────────
    const auto start = std::chrono::steady_clock::now();
    while (!g_shutdown_requested.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (opts.demo_duration_s > 0.0) {
            const double elapsed = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - start).count();
            if (elapsed >= opts.demo_duration_s) break;
        }
    }

    std::cout << "[roc-orchestrator] shutdown requested — tearing down threads\n";

    // ── Graceful teardown — reverse of startup order. ───────────────────
    shutdown_flag.store(true, std::memory_order_release);

    if (t_actuator.joinable())  t_actuator.join();
    t_egress.join();
    t_optflow.join();
    t_tracker.join();
    t_inference.join();
    t_camera.join();

    cap.release();
#ifdef _WIN32
    desktop_cap.release();
#endif
    server.stop();
    if (mjpeg) mjpeg->stop();

    // ── Final telemetry. ────────────────────────────────────────────────
    const double elapsed = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - start).count();
    const double inf_latency_ms = static_cast<double>(
        inf_last_latency_ns.load(std::memory_order_relaxed)) / 1.0e6;
    const double cam_fps_observed = elapsed > 0.0
        ? static_cast<double>(cam_frames_captured.load()) / elapsed
        : 0.0;
    const double avg_dets_per_frame = inf_frames_processed.load() > 0
        ? static_cast<double>(inf_total_detections.load()) /
          static_cast<double>(inf_frames_processed.load())
        : 0.0;

    std::cout << "[roc-orchestrator] clean exit\n"
              << "  elapsed_s             = " << elapsed                       << "\n"
              << "  camera_frames         = " << cam_frames_captured.load()    << "\n"
              << "  camera_drops          = " << cam_frames_dropped.load()     << "\n"
              << "  camera_fps_observed   = " << cam_fps_observed              << "\n"
              << "  inference_frames      = " << inf_frames_processed.load()   << "\n"
              << "  inference_last_ms     = " << inf_latency_ms                << "\n"
              << "  avg_detections/frame  = " << avg_dets_per_frame            << "\n"
              << "  tracker_steps         = " << tracker_steps.load()          << "\n"
              << "  tracker_dropped       = " << tracker_dropped.load()        << "\n"
              << "  egress_track_frames   = " << egress_track_frames.load()    << "\n"
              << "  egress_actuator_frames= " << egress_actuator_frames.load() << "\n"
              << "  egress_bg_frames      = " << egress_bg_frames.load()      << "\n"
              << "  bg_odometry_samples   = " << bg_samples_emitted.load()    << "\n"
              << "  actuator_tx_frames    = " << act_tx_frames.load()          << "\n"
              << "  sink_emissions(NoOp)  = " << sink.emissions()              << "\n"
              << "  ws_broadcasts_total   = " << server.frames_broadcast_total() << "\n";

    return 0;
}
