#pragma once

// =============================================================================
// roc::transport::mjpeg_server
//
// Tiny TCP-based HTTP server that exposes:
//   GET /sources               → application/json (camera enumeration)
//   GET /mjpeg/{index}         → multipart/x-mixed-replace; boundary=rocframe
//                                Motion-JPEG stream — one boundary per camera
//                                frame produced by the orchestrator.
//   GET /health                → text/plain "ok"
//
// Used by the dashboard SourcesDrawer to populate device list and by
// OperateCanvas Layer 0 <img> to receive live video pixels behind the SVG
// overlay (matches the Roboflow visual pattern).
//
// Architecture:
//   - One accept thread on (bind, port).
//   - One client thread per connection (small N: dashboard typically has
//     one MJPEG consumer + occasional /sources or /health probes).
//   - The MJPEG stream blocks until `push_jpeg_frame()` is called by the
//     camera-capture thread with a freshly encoded JPEG; the server writes
//     each frame to all subscribed clients in a non-blocking best-effort
//     way (slow clients drop frames, never backpressure capture).
//
// Hard rules:
//   - No third-party HTTP library dependency. ixwebsocket's HttpServer is
//     request-response; MJPEG needs chunked streaming. ~250 LOC of POSIX/
//     Winsock socket code is simpler than wrestling with that constraint.
//   - All `send()` calls are MSG_NOSIGNAL on POSIX (no SIGPIPE) and use
//     non-blocking sockets with explicit timeouts.
//   - cv::imencode("jpg", ..., quality) at quality 80 produces ~50-150 KB
//     frames at 1920x1080 — fits well inside loopback bandwidth.
// =============================================================================

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// Forward declare cv::Mat so the header doesn't transitively pull in OpenCV
namespace cv { class Mat; }

namespace roc {
namespace transport {

struct CameraSourceInfo {
    std::string id;              // "webcam-0", "capture-2", etc.
    std::string kind;            // "webcam" | "capture_card" | "file"
    std::string label;           // "Laptop Webcam (camera index 0)"
    int         device_index = -1;
    int         width        = 0;
    int         height       = 0;
    double      fps          = 0.0;
    bool        online       = false;  // discovered + readable
};

// ---------------------------------------------------------------------------
// Phase 4: model browser API
// ---------------------------------------------------------------------------
struct ModelInfo {
    std::string name;              // manifest 'name' field
    std::string manifest_path;     // absolute path to the .yaml on disk
    std::string onnx_path;         // absolute path to the matching .onnx (may be missing)
    std::string purpose;           // "detection" | "reid"
    std::string head;              // postprocessor head id
    int         input_w = 0;
    int         input_h = 0;
    int         num_classes = 0;
    bool        active = false;    // currently loaded by the InferenceEngine?
    bool        loadable = true;   // false if onnx file missing on disk
    std::string status_note;       // human-readable extra ("missing ONNX", "ready", etc.)
};

// ---------------------------------------------------------------------------
// Phase 6: FOV crop API. cx_norm/cy_norm in [0, 1] are the center of the
// crop expressed as a fraction of the source frame. radius_px is the
// half-extent of the SQUARE crop in source-frame pixels. `follow_target`
// being true overrides cx/cy with the locked track's EKF position each
// frame (the orchestrator handles the override at consumption time).
// ---------------------------------------------------------------------------
struct FovSettings {
    bool   enabled = false;
    float  cx_norm = 0.5f;
    float  cy_norm = 0.5f;
    float  radius_px = 360.0f;
    bool   follow_target = false;
    // Diagnostics — read-only from the API consumer's perspective.
    int    source_w = 0;
    int    source_h = 0;
    int    active_crop_x = 0;
    int    active_crop_y = 0;
    int    active_crop_w = 0;
    int    active_crop_h = 0;
};

// ---------------------------------------------------------------------------
// Phase 8: desktop capture enumeration.
//
// Lets the dashboard populate the Source drawer's monitor & window dropdowns
// with REAL data instead of a hardcoded list. The orchestrator wires these
// to roc::transport::enumerate_monitors() / enumerate_top_level_windows().
// ---------------------------------------------------------------------------
struct CaptureMonitorInfo {
    int  index = 0;
    int  x = 0, y = 0, w = 0, h = 0;
    bool is_primary = false;
    std::string device_name;
};
struct CaptureWindowInfo {
    std::uint64_t hwnd = 0;
    std::string   title;
    int           x = 0, y = 0, w = 0, h = 0;
    bool          minimized = false;
};
struct CaptureSourcesSnapshot {
    std::vector<CaptureMonitorInfo> monitors;
    std::vector<CaptureWindowInfo>  windows;
};

// Callback shape. The orchestrator registers concrete implementations that
// touch InferenceEngine, the model registry on disk, and the live FOV state.
// All callbacks are invoked from the per-client request thread — they MUST
// be thread-safe. They are otherwise free to take a few hundred ms (a model
// swap can take ~1 s while ORT JIT-compiles for the EP).
struct ApiHandlers {
    // GET /api/models -> JSON {"models":[ ... ]}
    std::function<std::vector<ModelInfo>()> list_models;

    // POST /api/model/active body: {"name":"yolo26m-roc-humanoid"}
    // Returns {"ok": bool, "active": "...", "ep": "DirectML|OpenVino|Cpu", "error": "..."}.
    std::function<std::string(const std::string& requested_name)> switch_model;

    // GET /api/model/active -> JSON snapshot of currently-loaded manifest.
    std::function<std::string()> active_model_json;

    // GET /api/fov -> JSON snapshot. POST writes new settings.
    std::function<FovSettings()> get_fov;
    std::function<void(const FovSettings& wanted)> set_fov;

    // GET /api/capture-sources -> JSON snapshot of available monitors AND
    // top-level visible windows on the user's desktop. Phase 8.
    std::function<CaptureSourcesSnapshot()> list_capture_sources;
};

class MjpegServer {
public:
    explicit MjpegServer(std::uint16_t port = 8766,
                         std::string   bind_address = "127.0.0.1",
                         int           jpeg_quality = 80);
    ~MjpegServer();

    MjpegServer(const MjpegServer&) = delete;
    MjpegServer& operator=(const MjpegServer&) = delete;

    // Bind + accept loop. Throws std::runtime_error on bind failure.
    void start();

    // Graceful shutdown; idempotent. Closes all clients, joins threads.
    void stop() noexcept;

    bool is_running() const noexcept { return running_.load(std::memory_order_acquire); }

    // Called from the camera-capture thread once per frame. Encodes JPEG +
    // dispatches to all subscribed clients for that camera_index.
    // THREAD-SAFE. Best-effort: drops to slow clients silently.
    void push_jpeg_frame(int camera_index, const cv::Mat& frame_bgr) noexcept;

    // Update the JSON returned by /sources. The C++ side calls this once at
    // startup (after probing cameras) and on every reload.
    void set_sources(std::vector<CameraSourceInfo> sources);

    // Install the Phase 4/6 REST handlers. Safe to call before or after start().
    void set_api_handlers(ApiHandlers handlers);

    // Diagnostic: how many active MJPEG client connections?
    std::size_t mjpeg_client_count() const noexcept;

    std::uint16_t listen_port() const noexcept { return port_; }

private:
    // Per-client handler — invoked from a detached thread spawned by the
    // accept loop. Parses the HTTP request line and dispatches to /health,
    // /sources, or /mjpeg/{idx} streaming.
    void handle_client(std::int64_t client_handle) noexcept;

    struct Impl;
    std::unique_ptr<Impl> impl_;

    std::uint16_t port_;
    std::string   bind_address_;
    int           jpeg_quality_;
    std::atomic<bool> running_{false};
};

}  // namespace transport
}  // namespace roc
