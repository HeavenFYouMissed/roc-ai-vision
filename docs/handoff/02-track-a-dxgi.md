# 02 — Track A: DXGI Desktop Duplication capture

## Why

The current `DesktopCapture` class in `core/transport/src/desktop_capture.cpp`
uses GDI `BitBlt`. It works universally and is simple, but it caps at
~30-60 FPS at 1080p and copies the framebuffer through system memory.

Windows' **Desktop Duplication API** (introduced in Windows 8, lives in
DXGI 1.2) does the same job ~3× faster by reading the framebuffer directly
from the GPU via `IDXGIOutputDuplication`. No system-memory round-trip
for the bulk pixel copy. This is what every modern screen recorder
(OBS, ShareX, NVIDIA ShadowPlay's fallback path, etc.) uses.

## Concrete deliverable

Add a new capture mode `DesktopCaptureKind::PrimaryMonitorDXGI` that
behaves identically to the existing `PrimaryMonitor` mode from the
caller's perspective but uses DXGI Desktop Duplication under the hood.
Operator picks via `--desktop-capture dxgi` CLI flag.

## File targets

| File | Change |
|---|---|
| `core/transport/include/roc/transport/desktop_capture.hpp` | Add `DesktopCaptureKind::PrimaryMonitorDXGI` enum value. No other API change — keep `open()` / `read()` / `release()` shape. |
| `core/transport/src/desktop_capture.cpp` | Add a new `DxgiBackend` struct alongside the existing GDI backend; `open()` dispatches on `cfg_.kind`. |
| `core/transport/CMakeLists.txt` | Link `Dxgi.lib` + `D3D11.lib` under WIN32 |
| `core/main.cpp` | Accept `--desktop-capture dxgi` in the CLI parser arm |
| `dashboard/src/lib/components/drawers/SourcesDrawer.svelte` | Add `dxgi` to the kind pill list |
| `dashboard/src/lib/stores/capture_sources_store.svelte.ts` | Add `dxgi` to the `DesktopCaptureCliArgs.desktopKind` union |

## DXGI flow (skeleton)

```cpp
// One-time at process start
ID3D11Device*           d3d_device   = nullptr;
ID3D11DeviceContext*    d3d_ctx      = nullptr;
IDXGIAdapter1*          dxgi_adapter = nullptr;
IDXGIOutput*            dxgi_output  = nullptr;
IDXGIOutput1*           dxgi_output1 = nullptr;
IDXGIOutputDuplication* dupl         = nullptr;

D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, ..., &d3d_device, ...);
// IDXGIFactory1 -> EnumAdapters1 -> EnumOutputs -> QueryInterface IDXGIOutput1
dxgi_output1->DuplicateOutput(d3d_device, &dupl);

// Per frame in read():
DXGI_OUTDUPL_FRAME_INFO frame_info{};
IDXGIResource*          desktop_resource = nullptr;
HRESULT hr = dupl->AcquireNextFrame(/*timeout_ms*/100, &frame_info, &desktop_resource);
if (hr == DXGI_ERROR_WAIT_TIMEOUT) return false; // no new frame, keep last

ID3D11Texture2D* gpu_texture = nullptr;
desktop_resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&gpu_texture);

// Copy GPU texture -> CPU-readable staging texture
ID3D11Texture2D* staging = /* create once with D3D11_USAGE_STAGING + CPU_ACCESS_READ */;
d3d_ctx->CopyResource(staging, gpu_texture);

// Map staging, memcpy into our DIBSection or directly into a cv::Mat
D3D11_MAPPED_SUBRESOURCE mapped{};
d3d_ctx->Map(staging, 0, D3D11_MAP_READ, 0, &mapped);
//   mapped.pData is BGRA top-down at mapped.RowPitch bytes per row
// cv::Mat bgra_view(h, w, CV_8UC4, mapped.pData, mapped.RowPitch);
// cv::cvtColor(bgra_view, out_bgr, cv::COLOR_BGRA2BGR);
d3d_ctx->Unmap(staging, 0);

gpu_texture->Release();
desktop_resource->Release();
dupl->ReleaseFrame();
```

## Gotchas

- **AcquireNextFrame timeout = 0** returns immediately and is your friend
  when the operator's target FPS is lower than the display refresh rate
  (just sleep the remainder of the frame budget yourself).
- **Lost-access errors** (`DXGI_ERROR_ACCESS_LOST`): the desktop session
  changed (UAC prompt, Win+L lock, fullscreen exclusive app starting).
  Tear down `dupl` and re-create it. Wrap the whole acquire in a retry
  loop.
- **Protected content** (DRM Netflix etc) returns a black frame — that's
  Windows' policy, no workaround.
- **Multi-GPU systems**: `D3D11CreateDevice` may pick the wrong adapter
  (integrated instead of discrete). Iterate `IDXGIFactory1::EnumAdapters1`
  and pick the one whose `IDXGIAdapter1::EnumOutputs(0)` returns the
  primary monitor.
- **DPI awareness**: already handled process-wide in `ensure_dpi_aware()`
  in the existing `desktop_capture.cpp`. No change needed.

## Acceptance criteria

1. `--desktop-capture dxgi` boots without throwing.
2. `inference_last_ms` in the orchestrator's shutdown summary is no
   higher than with `--desktop-capture primary` (BitBlt).
3. `camera_fps_observed` at `--capture-fps 144 --no-mjpeg --no-actuator-telemetry`
   should reach 100+ FPS on an RTX 4050. (BitBlt typically caps at
   ~30-60.) If not, profile and optimize the `Map`/memcpy step.
4. Existing 48/48 vision_pipeline tests still pass.

## Reference

Microsoft's official sample is the gold-standard reference:
https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/desktop-dup-api
