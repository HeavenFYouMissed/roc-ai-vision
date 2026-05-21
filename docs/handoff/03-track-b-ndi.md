# 03 — Track B: NDI network video receiver

## Why

NDI (Network Device Interface, NewTek) is the broadcast / live-production
industry standard for sending video over a network with sub-frame latency.
Adding an NDI receiver as a source kind unlocks:

- **Broadcast / OBS pipeline integration** — OBS Studio can output NDI
  natively; any computer on the LAN can then consume it.
- **Hardware capture cards** — most modern HDMI capture cards (Magewell,
  BlackMagic, AJA) export NDI alongside their native USB feed.
- **IP cameras / PTZ cameras** — many security and broadcast PTZ cameras
  speak NDI as their primary network protocol.
- **Counter-UAS** — long-zoom optic cameras with built-in NDI output exist
  off-the-shelf in the surveillance market.
- **Phone-as-camera** — iOS app `NDI HX Camera` turns an iPhone into an
  NDI source over WiFi for ~free.

This is purely additive (a new `DesktopCaptureKind::NetworkNDI` mode); it
does not touch any existing functionality.

## SDK install

NewTek NDI SDK is free but requires registration:
https://ndi.video/for-developers/ndi-sdk/

The Windows SDK installer drops headers + import lib at:
```
C:\Program Files\NDI\NDI 6 SDK\Include\Processing.NDI.Lib.h
C:\Program Files\NDI\NDI 6 SDK\Lib\x64\Processing.NDI.Lib.x64.lib
```

Plus a runtime DLL (`Processing.NDI.Lib.x64.dll`) that must be alongside
`roc_vision.exe`.

CMake plumbing pattern (follow the DirectML auto-detect pattern from
`core/vision_pipeline/CMakeLists.txt`):

```cmake
set(NDI_SDK_ROOT "" CACHE PATH "NewTek NDI SDK root")
if(NDI_SDK_ROOT AND EXISTS "${NDI_SDK_ROOT}/Include/Processing.NDI.Lib.h")
    message(STATUS "roc_transport: NDI SDK found at ${NDI_SDK_ROOT}")
    set(ROC_TRANSPORT_HAVE_NDI ON CACHE BOOL "" FORCE)
    target_include_directories(roc_transport PRIVATE "${NDI_SDK_ROOT}/Include")
    target_link_libraries(roc_transport PRIVATE
        "${NDI_SDK_ROOT}/Lib/x64/Processing.NDI.Lib.x64.lib")
    # copy runtime DLL alongside any consumer exe via custom command
endif()
if(ROC_TRANSPORT_HAVE_NDI)
    target_compile_definitions(roc_transport PRIVATE ROC_TRANSPORT_HAVE_NDI=1)
endif()
```

## Concrete deliverable

| File | Change |
|---|---|
| `core/transport/include/roc/transport/desktop_capture.hpp` | Add `DesktopCaptureKind::NetworkNDI` + `ndi_source_name` field on `DesktopCaptureConfig` + an `enumerate_ndi_sources() -> vector<NdiSourceInfo>` free function |
| `core/transport/src/desktop_capture.cpp` | New `NdiBackend` struct, gated behind `#ifdef ROC_TRANSPORT_HAVE_NDI`. If the SDK isn't installed, `open()` returns false with a clear `last_err_`. |
| `core/transport/CMakeLists.txt` | Auto-detect NDI per the snippet above |
| `core/main.cpp` | `--desktop-capture ndi --capture-ndi-source "Camera 1"` CLI args; pass through to config |
| `core/transport/src/mjpeg_server.cpp` | Extend `CaptureSourcesSnapshot` with an `ndi_sources` array; `/api/capture-sources` returns it |
| `dashboard/src/lib/components/drawers/SourcesDrawer.svelte` | Add `ndi` kind pill + a dropdown populated from `captureSourcesStore.ndiSources` |
| `dashboard/src/lib/stores/capture_sources_store.svelte.ts` | Add `ndiSources: NdiSourceInfo[]` + extend `synthesizeCliFlags` for the `ndi` kind |

## NDI flow (skeleton)

```cpp
#include <Processing.NDI.Lib.h>

NDIlib_initialize();

// Discovery
NDIlib_find_create_t find_cfg{};
NDIlib_find_instance_t finder = NDIlib_find_create_v2(&find_cfg);
uint32_t n_sources = 0;
const NDIlib_source_t* sources = NDIlib_find_get_current_sources(finder, &n_sources);
// each source has .p_ndi_name (UTF-8 e.g. "MYPC (OBS)")

// Receive
NDIlib_recv_create_v3_t recv_cfg{};
recv_cfg.source_to_connect_to    = sources[picked_index];
recv_cfg.color_format            = NDIlib_recv_color_format_BGRX_BGRA;
recv_cfg.bandwidth               = NDIlib_recv_bandwidth_highest;
recv_cfg.allow_video_fields      = false;
NDIlib_recv_instance_t recv = NDIlib_recv_create_v3(&recv_cfg);

// Per frame in read():
NDIlib_video_frame_v2_t video_frame{};
NDIlib_frame_type_e ft = NDIlib_recv_capture_v2(recv, &video_frame, nullptr, nullptr, /*timeout_ms*/100);
if (ft == NDIlib_frame_type_video) {
    // BGRA top-down at video_frame.line_stride_in_bytes
    cv::Mat bgra_view(video_frame.yres, video_frame.xres, CV_8UC4,
                      video_frame.p_data, video_frame.line_stride_in_bytes);
    cv::cvtColor(bgra_view, out_bgr, cv::COLOR_BGRA2BGR);
    NDIlib_recv_free_video_v2(recv, &video_frame);
    return true;
}
// On exit:
NDIlib_recv_destroy(recv);
NDIlib_find_destroy(finder);
NDIlib_destroy();
```

## Acceptance criteria

1. With OBS Studio's "NDI Output" plugin enabled, running
   `roc_vision.exe --desktop-capture ndi --capture-ndi-source "..."` shows
   the OBS scene in the dashboard's Operate view.
2. `enumerate_ndi_sources()` finds at least one source within 2 seconds
   of any NDI source going live on the LAN.
3. Source disappearing (OBS quit, camera unplugged) does NOT crash the
   orchestrator — `read()` returns false and `last_err_` says "source
   disconnected".
4. CMake configure WITHOUT `NDI_SDK_ROOT` set still builds cleanly (NDI
   support is conditional, never required).
