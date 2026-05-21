#pragma once

// =============================================================================
// roc::transport::desktop_capture
//
// Native Windows desktop / window / region capture that quacks like
// `cv::VideoCapture` (open / read / release / isOpened / width / height /
// fps) so it slots into the existing `camera_capture_thread` orchestrator
// path with zero changes to call-sites that don't care about the source.
//
// Use case: feed the YOLO / EKF / FOV-crop pipeline a YouTube video, a game
// running in another window, an OBS preview — anything visible on the user's
// desktop. Combined with the FOV crop pipeline this turns the orchestrator
// into a general-purpose "watch X and detect Y" instrument.
//
// Backend: GDI BitBlt + Windows DPI-aware capture context. We deliberately
// pick GDI over Windows.Graphics.Capture for two reasons:
//   1. GDI is universally available since Win NT — no UWP / WinRT linkage,
//      no minimum-Windows-version gates, no UAC permission prompts.
//   2. The driver-side BitBlt fast path on modern GPUs delivers 60+ FPS at
//      1080p with negligible CPU. DXGI Desktop Duplication is faster on
//      paper but its callback-driven API would force a bigger refactor.
//
// Cropping is supported natively (BitBlt source rect) so we can capture a
// sub-region without doing a full-screen grab and then cropping in CPU.
//
// Threading model: NOT thread-safe. Single producer (the camera capture
// thread). Construct in the same thread that will call `read()`.
// =============================================================================

#include <opencv2/core.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace roc {
namespace transport {

// What kind of source the capture targets. Mirrors the dashboard's
// SourceKind enum so the wire format is symmetric.
enum class DesktopCaptureKind {
    PrimaryMonitor,   // primary display, full extent
    Monitor,          // specific monitor by 0-indexed display number
    Window,           // top-level window matched by title substring
    Region            // arbitrary screen-relative rectangle
};

struct DesktopCaptureConfig {
    DesktopCaptureKind kind = DesktopCaptureKind::PrimaryMonitor;

    // Used when kind == Monitor. 0-indexed; matches `EnumDisplayMonitors`
    // order. If out-of-range, falls back to primary monitor with a log line.
    int monitor_index = 0;

    // Used when kind == Window. Case-insensitive substring match against
    // the window title returned by `GetWindowTextW`. First match wins.
    // Empty string => match the foreground window (whatever the user
    // clicked last); useful for the "Capture current Chrome tab" workflow.
    std::string window_title_substring;

    // Used when kind == Region. Screen-coordinate rect (virtual desktop
    // space; on multi-monitor systems can be negative for monitors to the
    // left of the primary).
    int region_x = 0;
    int region_y = 0;
    int region_w = 0;
    int region_h = 0;

    // Target capture rate. The producer will not run faster than this; if
    // the consumer is slow, frames are simply not produced (no buffering).
    // 60 Hz matches the typical YouTube delivery rate.
    int target_fps = 60;

    // When true, scale the captured frame down so its largest dimension
    // equals `downscale_max_dim`. Useful when the screen is 4K but the
    // model only wants 1920×1080 — avoids burning preprocess time on
    // pixels that letterboxing would discard anyway. 0 disables.
    int downscale_max_dim = 0;

    // Include the mouse cursor in the captured frame? Off by default
    // because the cursor causes detection-jitter false positives.
    bool capture_cursor = false;
};

// Per-monitor metadata used by /api/capture-sources to populate the dashboard
// device list with REAL monitor counts + bounds.
struct MonitorInfo {
    int  index = 0;            // EnumDisplayMonitors order
    int  x = 0;                // virtual-desktop space top-left
    int  y = 0;
    int  w = 0;
    int  h = 0;
    bool is_primary = false;
    std::string device_name;   // "\\.\DISPLAY1" etc.
};

// Per-window metadata for the window-pick dropdown.
struct WindowInfo {
    std::uint64_t hwnd = 0;     // raw HWND as uint64 (don't deref outside Win32)
    std::string   title;
    int           x = 0;        // current screen position
    int           y = 0;
    int           w = 0;
    int           h = 0;
    bool          minimized = false;
    bool          visible = true;
};

// Stateless helpers — safe to call from any thread.
std::vector<MonitorInfo> enumerate_monitors();
std::vector<WindowInfo>  enumerate_top_level_windows();

// ---------------------------------------------------------------------------
// DesktopCapture — drop-in for cv::VideoCapture against a Windows desktop
// source. Methods chosen to match cv::VideoCapture so callers can keep
// using `cap.read(...)` / `cap.isOpened()` / `cap.get(CAP_PROP_FPS)`-style
// idioms without templated abstraction.
// ---------------------------------------------------------------------------
class DesktopCapture {
public:
    DesktopCapture();
    ~DesktopCapture();

    DesktopCapture(const DesktopCapture&) = delete;
    DesktopCapture& operator=(const DesktopCapture&) = delete;

    // Open the capture. On failure returns false and `last_error()` is set.
    bool open(const DesktopCaptureConfig& cfg);

    // Idempotent close + resource release.
    void release() noexcept;

    bool isOpened() const noexcept { return opened_; }

    // Block until next frame is ready OR target_fps interval has elapsed,
    // then write into `out_bgr` (3-channel CV_8UC3 in source dimensions
    // after optional downscale). Returns false if the source disappeared
    // (e.g. window was closed, monitor unplugged).
    bool read(cv::Mat& out_bgr);

    // Reported source dimensions AFTER any downscale; 0 until `open()`
    // succeeds for the first time.
    int    width()  const noexcept { return frame_w_; }
    int    height() const noexcept { return frame_h_; }
    double fps()    const noexcept;

    // Human-readable description of the active source, e.g.
    // "Primary monitor (1920x1080)" or "Window: Chrome - YouTube".
    const std::string& source_description() const noexcept { return desc_; }

    // Last error string (only meaningful immediately after open() returned
    // false or read() returned false).
    const std::string& last_error() const noexcept { return last_err_; }

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    bool        opened_  = false;
    int         frame_w_ = 0;
    int         frame_h_ = 0;
    DesktopCaptureConfig cfg_{};
    std::string desc_;
    std::string last_err_;
};

}  // namespace transport
}  // namespace roc
