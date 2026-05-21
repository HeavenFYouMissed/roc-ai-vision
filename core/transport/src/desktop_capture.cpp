// =============================================================================
// roc::transport::desktop_capture — implementation
//
// Native Windows GDI BitBlt capture. The hot path is:
//
//   1. Determine source rect for THIS capture call (monitor / window /
//      static region). For window-mode the rect re-queries each frame so
//      the capture tracks the window as the user drags / resizes it.
//   2. BitBlt the screen DC -> memory DC backed by a 32-bit DIBSection.
//   3. (Optional) CURSORINFO + DrawIcon for `capture_cursor`.
//   4. Wrap the DIBSection pixel buffer in a cv::Mat (BGRA layout — Windows
//      stores it as 0xAARRGGBB in little-endian → byte order BGRA).
//   5. cv::cvtColor(BGRA -> BGR) into the caller's out buffer.
//   6. (Optional) cv::resize to the user's downscale_max_dim.
//
// All Win32 resources are RAII-wrapped via Impl. release() is safe to call
// from the destructor and explicitly.
// =============================================================================

#include "roc/transport/desktop_capture.hpp"

#include <opencv2/imgproc.hpp>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
// Intentionally NOT including <shellscalingapi.h>: it requires WINVER >= 0x0603
// AND adds a hard link-time dependency on shcore.lib that some mingw-w64
// distros don't ship. We instead try the modern per-monitor-V2 API via
// dynamic GetProcAddress() and fall back to the universal SetProcessDPIAware
// (in user32, ships on every Windows version) when that's unavailable.
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "User32.lib")

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <iostream>
#include <mutex>
#include <thread>

namespace roc {
namespace transport {

namespace {

// One-time process-wide DPI awareness flip. Required so that a 1920×1080
// virtual coordinate on a 150%-scaled monitor maps to the actual 1920×1080
// physical pixels (and not a 1280×720 scaled view that would alias against
// detection bboxes).
void ensure_dpi_aware() {
    static std::once_flag once;
    std::call_once(once, []() {
        // PROCESS_PER_MONITOR_DPI_AWARE_V2 lives in user32 on Win10 1607+;
        // SetProcessDpiAwareness is the older API. Try the newer one first.
        using SetCtxFn = BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT);
        HMODULE u32 = ::GetModuleHandleW(L"user32.dll");
        if (u32) {
            auto fn = reinterpret_cast<SetCtxFn>(::GetProcAddress(u32, "SetProcessDpiAwarenessContext"));
            if (fn) {
                // -4 is DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
                if (fn(reinterpret_cast<DPI_AWARENESS_CONTEXT>(static_cast<INT_PTR>(-4)))) {
                    return;
                }
            }
        }
        // Older path: SetProcessDPIAware from user32.dll. System-DPI-aware
        // (not per-monitor) but ships on every Windows version since Vista
        // and doesn't require linking shcore.lib.
        ::SetProcessDPIAware();
    });
}

// String helpers ------------------------------------------------------------
std::string wstr_to_utf8(const std::wstring& w) {
    if (w.empty()) return {};
    const int needed = ::WideCharToMultiByte(
        CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (needed <= 0) return {};
    std::string out(static_cast<std::size_t>(needed - 1), '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, out.data(), needed, nullptr, nullptr);
    return out;
}

std::string lower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

bool contains_icase(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return false;
    return lower(haystack).find(lower(needle)) != std::string::npos;
}

// Monitor enumeration -------------------------------------------------------
struct MonitorEnumCtx {
    std::vector<MonitorInfo>* out;
    int* counter;
};

BOOL CALLBACK monitor_enum_proc(HMONITOR hMon, HDC /*hdc*/, LPRECT /*rect*/, LPARAM lparam) {
    auto* ctx = reinterpret_cast<MonitorEnumCtx*>(lparam);
    MONITORINFOEXW mi{};
    mi.cbSize = sizeof(mi);
    if (!::GetMonitorInfoW(hMon, &mi)) return TRUE;

    MonitorInfo m{};
    m.index       = (*ctx->counter)++;
    m.x           = mi.rcMonitor.left;
    m.y           = mi.rcMonitor.top;
    m.w           = mi.rcMonitor.right  - mi.rcMonitor.left;
    m.h           = mi.rcMonitor.bottom - mi.rcMonitor.top;
    m.is_primary  = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;
    m.device_name = wstr_to_utf8(mi.szDevice);
    ctx->out->push_back(m);
    return TRUE;
}

}  // namespace

std::vector<MonitorInfo> enumerate_monitors() {
    ensure_dpi_aware();
    std::vector<MonitorInfo> out;
    int counter = 0;
    MonitorEnumCtx ctx{ &out, &counter };
    ::EnumDisplayMonitors(nullptr, nullptr, &monitor_enum_proc, reinterpret_cast<LPARAM>(&ctx));
    return out;
}

// Window enumeration --------------------------------------------------------
namespace {
BOOL CALLBACK window_enum_proc(HWND hwnd, LPARAM lparam) {
    auto* out = reinterpret_cast<std::vector<WindowInfo>*>(lparam);
    if (!::IsWindow(hwnd)) return TRUE;

    const int len = ::GetWindowTextLengthW(hwnd);
    if (len <= 0) return TRUE;   // skip untitled / system overlay windows

    std::wstring title(static_cast<std::size_t>(len), L'\0');
    ::GetWindowTextW(hwnd, title.data(), len + 1);

    // Filter out invisible, tool, and child windows — leave only the
    // "stuff a user would see in the taskbar" set.
    const LONG style = ::GetWindowLongW(hwnd, GWL_STYLE);
    if ((style & WS_VISIBLE) == 0) return TRUE;
    const LONG ex_style = ::GetWindowLongW(hwnd, GWL_EXSTYLE);
    if (ex_style & WS_EX_TOOLWINDOW) return TRUE;

    RECT rc{};
    if (!::GetWindowRect(hwnd, &rc)) return TRUE;
    const int w = rc.right - rc.left;
    const int h = rc.bottom - rc.top;
    if (w <= 0 || h <= 0) return TRUE;

    WindowInfo info{};
    info.hwnd       = reinterpret_cast<std::uint64_t>(hwnd);
    info.title      = wstr_to_utf8(title);
    info.x          = rc.left;
    info.y          = rc.top;
    info.w          = w;
    info.h          = h;
    info.minimized  = ::IsIconic(hwnd) != 0;
    info.visible    = true;
    out->push_back(std::move(info));
    return TRUE;
}
}  // namespace

std::vector<WindowInfo> enumerate_top_level_windows() {
    ensure_dpi_aware();
    std::vector<WindowInfo> out;
    ::EnumWindows(&window_enum_proc, reinterpret_cast<LPARAM>(&out));
    return out;
}

// ---------------------------------------------------------------------------
// DesktopCapture::Impl — RAII'd Win32 GDI state.
// ---------------------------------------------------------------------------
struct DesktopCapture::Impl {
    HDC      screen_dc  = nullptr;     // borrowed; do NOT DeleteDC, use ReleaseDC
    HWND     screen_dc_owner = nullptr;
    HDC      mem_dc     = nullptr;
    HBITMAP  dib_bitmap = nullptr;
    void*    dib_bits   = nullptr;     // pointer into the DIBSection (BGRA)
    int      dib_w = 0;
    int      dib_h = 0;
    cv::Mat  bgra_view;                // CV_8UC4 alias over dib_bits

    std::chrono::steady_clock::time_point last_read_ts{};
    std::chrono::steady_clock::time_point ema_t0{};
    double                                ema_fps = 0.0;

    ~Impl() { teardown(); }

    void teardown() noexcept {
        bgra_view.release();
        if (mem_dc && dib_bitmap) {
            // Detach the bitmap before deleting the DC.
            ::SelectObject(mem_dc, ::GetStockObject(NULL_BRUSH));
        }
        if (dib_bitmap) { ::DeleteObject(dib_bitmap); dib_bitmap = nullptr; }
        if (mem_dc)     { ::DeleteDC(mem_dc);         mem_dc = nullptr;     }
        if (screen_dc)  { ::ReleaseDC(screen_dc_owner, screen_dc); screen_dc = nullptr; }
        screen_dc_owner = nullptr;
        dib_bits = nullptr;
        dib_w = dib_h = 0;
    }

    // (Re)allocate the DIBSection for the given pixel dimensions. Returns
    // false on allocation failure.
    bool ensure_dib(int w, int h, std::string& err) {
        if (w <= 0 || h <= 0) { err = "invalid capture dims"; return false; }
        if (dib_bitmap && dib_w == w && dib_h == h) return true;

        if (dib_bitmap) { ::DeleteObject(dib_bitmap); dib_bitmap = nullptr; dib_bits = nullptr; }

        BITMAPINFO bi{};
        bi.bmiHeader.biSize        = sizeof(bi.bmiHeader);
        bi.bmiHeader.biWidth       = w;
        bi.bmiHeader.biHeight      = -h;   // negative => top-down
        bi.bmiHeader.biPlanes      = 1;
        bi.bmiHeader.biBitCount    = 32;
        bi.bmiHeader.biCompression = BI_RGB;

        dib_bitmap = ::CreateDIBSection(mem_dc, &bi, DIB_RGB_COLORS, &dib_bits, nullptr, 0);
        if (!dib_bitmap || !dib_bits) {
            err = "CreateDIBSection failed";
            dib_bitmap = nullptr;
            dib_bits = nullptr;
            return false;
        }
        ::SelectObject(mem_dc, dib_bitmap);
        dib_w = w;
        dib_h = h;
        // Wrap the bits in a cv::Mat header — zero-copy alias over the DIB.
        bgra_view = cv::Mat(h, w, CV_8UC4, dib_bits, /*step=*/ static_cast<size_t>(w) * 4);
        return true;
    }
};

DesktopCapture::DesktopCapture()  : impl_(std::make_unique<Impl>()) {}
DesktopCapture::~DesktopCapture() { release(); }

void DesktopCapture::release() noexcept {
    if (impl_) impl_->teardown();
    opened_  = false;
    frame_w_ = 0;
    frame_h_ = 0;
    desc_.clear();
}

double DesktopCapture::fps() const noexcept {
    return impl_ ? impl_->ema_fps : 0.0;
}

namespace {

// Compute the source rect for a single capture frame, based on the active
// config. Window-mode re-queries the live HWND rect so the capture tracks
// drags / resizes; monitor-mode pins to the monitor's screen rect once.
struct SourceRect { RECT rc{}; HWND optional_owner_hwnd = nullptr; };

bool compute_source_rect(const DesktopCaptureConfig& cfg, SourceRect& out, std::string& err) {
    if (cfg.kind == DesktopCaptureKind::PrimaryMonitor) {
        auto mons = enumerate_monitors();
        for (const auto& m : mons) {
            if (m.is_primary) { out.rc = { m.x, m.y, m.x + m.w, m.y + m.h }; return true; }
        }
        // Fall through to GetSystemMetrics if no monitor was reported (rare).
        out.rc.left   = 0;
        out.rc.top    = 0;
        out.rc.right  = ::GetSystemMetrics(SM_CXSCREEN);
        out.rc.bottom = ::GetSystemMetrics(SM_CYSCREEN);
        return out.rc.right > 0 && out.rc.bottom > 0;
    }

    if (cfg.kind == DesktopCaptureKind::Monitor) {
        auto mons = enumerate_monitors();
        if (mons.empty()) { err = "no monitors enumerated"; return false; }
        int idx = cfg.monitor_index;
        if (idx < 0 || idx >= static_cast<int>(mons.size())) idx = 0;
        const auto& m = mons[static_cast<std::size_t>(idx)];
        out.rc = { m.x, m.y, m.x + m.w, m.y + m.h };
        return true;
    }

    if (cfg.kind == DesktopCaptureKind::Region) {
        if (cfg.region_w <= 0 || cfg.region_h <= 0) { err = "invalid region dims"; return false; }
        out.rc = { cfg.region_x, cfg.region_y,
                   cfg.region_x + cfg.region_w,
                   cfg.region_y + cfg.region_h };
        return true;
    }

    // Window
    HWND hwnd = nullptr;
    if (cfg.window_title_substring.empty()) {
        hwnd = ::GetForegroundWindow();
    } else {
        const auto wins = enumerate_top_level_windows();
        for (const auto& w : wins) {
            if (contains_icase(w.title, cfg.window_title_substring) && !w.minimized) {
                hwnd = reinterpret_cast<HWND>(w.hwnd);
                break;
            }
        }
    }
    if (!hwnd) { err = "no matching window found"; return false; }
    RECT rc{};
    if (!::GetWindowRect(hwnd, &rc)) { err = "GetWindowRect failed"; return false; }
    if (rc.right <= rc.left || rc.bottom <= rc.top) {
        err = "window has zero area (minimized?)";
        return false;
    }
    out.rc = rc;
    out.optional_owner_hwnd = hwnd;
    return true;
}

}  // namespace

bool DesktopCapture::open(const DesktopCaptureConfig& cfg) {
    release();
    ensure_dpi_aware();
    cfg_ = cfg;

    // Grab a screen DC. nullptr -> entire virtual screen (multi-monitor OK).
    impl_->screen_dc       = ::GetDC(nullptr);
    impl_->screen_dc_owner = nullptr;
    if (!impl_->screen_dc) {
        last_err_ = "GetDC(nullptr) failed";
        return false;
    }
    impl_->mem_dc = ::CreateCompatibleDC(impl_->screen_dc);
    if (!impl_->mem_dc) {
        last_err_ = "CreateCompatibleDC failed";
        release();
        return false;
    }

    // Probe the source once so the caller can read width()/height() before
    // the first read().
    SourceRect src{};
    std::string rect_err;
    if (!compute_source_rect(cfg_, src, rect_err)) {
        last_err_ = "source-rect probe failed: " + rect_err;
        release();
        return false;
    }
    const int raw_w = src.rc.right - src.rc.left;
    const int raw_h = src.rc.bottom - src.rc.top;
    if (!impl_->ensure_dib(raw_w, raw_h, last_err_)) { release(); return false; }

    frame_w_ = raw_w;
    frame_h_ = raw_h;
    if (cfg_.downscale_max_dim > 0) {
        const int big = std::max(raw_w, raw_h);
        if (big > cfg_.downscale_max_dim) {
            const double s = static_cast<double>(cfg_.downscale_max_dim) / big;
            frame_w_ = std::max(1, static_cast<int>(std::round(raw_w * s)));
            frame_h_ = std::max(1, static_cast<int>(std::round(raw_h * s)));
        }
    }

    char buf[256];
    switch (cfg_.kind) {
        case DesktopCaptureKind::PrimaryMonitor:
            std::snprintf(buf, sizeof(buf), "Primary monitor (%dx%d)", frame_w_, frame_h_); break;
        case DesktopCaptureKind::Monitor:
            std::snprintf(buf, sizeof(buf), "Monitor %d (%dx%d)",
                          cfg_.monitor_index, frame_w_, frame_h_); break;
        case DesktopCaptureKind::Window:
            std::snprintf(buf, sizeof(buf), "Window: %.180s (%dx%d)",
                          cfg_.window_title_substring.empty()
                              ? "(foreground)"
                              : cfg_.window_title_substring.c_str(),
                          frame_w_, frame_h_); break;
        case DesktopCaptureKind::Region:
            std::snprintf(buf, sizeof(buf), "Region @ (%d, %d) %dx%d",
                          cfg_.region_x, cfg_.region_y, frame_w_, frame_h_); break;
    }
    desc_.assign(buf);

    opened_                = true;
    impl_->last_read_ts    = std::chrono::steady_clock::now();
    impl_->ema_t0          = impl_->last_read_ts;
    impl_->ema_fps         = 0.0;
    last_err_.clear();
    return true;
}

bool DesktopCapture::read(cv::Mat& out_bgr) {
    if (!opened_) { last_err_ = "not opened"; return false; }

    // Pace to target_fps. If we got called faster than the budget, sleep
    // the difference. (Camera-capture-thread already runs lock-step with
    // the device frame rate, but for desktop sources we explicitly want
    // to rate-limit so we don't burn 100% CPU re-grabbing the same frame.)
    using clock = std::chrono::steady_clock;
    if (cfg_.target_fps > 0) {
        const auto budget = std::chrono::microseconds(1'000'000 / cfg_.target_fps);
        const auto now    = clock::now();
        const auto since  = now - impl_->last_read_ts;
        if (since < budget) {
            std::this_thread::sleep_for(budget - since);
        }
    }

    // Re-resolve the source rect on every frame so window-mode tracks
    // drags / resizes. Cheap call (one GetWindowRect or one monitor enum).
    SourceRect src{};
    std::string rect_err;
    if (!compute_source_rect(cfg_, src, rect_err)) {
        last_err_ = "source-rect resolve failed: " + rect_err;
        return false;
    }
    const int raw_w = src.rc.right - src.rc.left;
    const int raw_h = src.rc.bottom - src.rc.top;
    if (raw_w <= 0 || raw_h <= 0) { last_err_ = "source rect collapsed"; return false; }

    // Reallocate DIB if the source dims changed (window resize).
    if (raw_w != impl_->dib_w || raw_h != impl_->dib_h) {
        if (!impl_->ensure_dib(raw_w, raw_h, last_err_)) return false;
    }

    // BitBlt the requested rect from screen DC into our memory DC.
    if (!::BitBlt(impl_->mem_dc, 0, 0, raw_w, raw_h,
                  impl_->screen_dc, src.rc.left, src.rc.top,
                  SRCCOPY | CAPTUREBLT)) {
        last_err_ = "BitBlt failed (window minimized or DRM-protected?)";
        return false;
    }

    // Optional: overlay the system cursor at its current screen position.
    if (cfg_.capture_cursor) {
        CURSORINFO ci{}; ci.cbSize = sizeof(ci);
        if (::GetCursorInfo(&ci) && (ci.flags & CURSOR_SHOWING)) {
            ICONINFO ii{};
            ::GetIconInfo(ci.hCursor, &ii);
            const int local_x = ci.ptScreenPos.x - src.rc.left - ii.xHotspot;
            const int local_y = ci.ptScreenPos.y - src.rc.top  - ii.yHotspot;
            ::DrawIconEx(impl_->mem_dc, local_x, local_y, ci.hCursor,
                         0, 0, 0, nullptr, DI_NORMAL);
            if (ii.hbmMask)  ::DeleteObject(ii.hbmMask);
            if (ii.hbmColor) ::DeleteObject(ii.hbmColor);
        }
    }

    // BGRA -> BGR into out_bgr (and downscale if requested).
    cv::Mat bgr_full;
    cv::cvtColor(impl_->bgra_view, bgr_full, cv::COLOR_BGRA2BGR);

    if (cfg_.downscale_max_dim > 0) {
        const int big = std::max(raw_w, raw_h);
        if (big > cfg_.downscale_max_dim) {
            const double s = static_cast<double>(cfg_.downscale_max_dim) / big;
            cv::resize(bgr_full, out_bgr,
                       cv::Size{
                           std::max(1, static_cast<int>(std::round(raw_w * s))),
                           std::max(1, static_cast<int>(std::round(raw_h * s)))
                       },
                       0.0, 0.0, cv::INTER_AREA);
        } else {
            out_bgr = std::move(bgr_full);
        }
    } else {
        out_bgr = std::move(bgr_full);
    }
    frame_w_ = out_bgr.cols;
    frame_h_ = out_bgr.rows;

    // Maintain rolling FPS estimate (EMA over 1-second window).
    const auto now = clock::now();
    const double dt_s = std::chrono::duration<double>(now - impl_->last_read_ts).count();
    impl_->last_read_ts = now;
    if (dt_s > 0.0 && dt_s < 1.0) {
        const double inst_fps = 1.0 / dt_s;
        constexpr double alpha = 0.1;
        impl_->ema_fps = (impl_->ema_fps == 0.0)
            ? inst_fps
            : (1.0 - alpha) * impl_->ema_fps + alpha * inst_fps;
    }

    last_err_.clear();
    return true;
}

}  // namespace transport
}  // namespace roc
