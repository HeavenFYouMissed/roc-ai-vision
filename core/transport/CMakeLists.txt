cmake_minimum_required(VERSION 3.20)
project(roc_transport LANGUAGES CXX)

# =============================================================================
# ROC AI Vision — single-process hybrid IPC transport layer.
#
# Implements the Phase-4 binding architecture from
# `workspace_blueprint/research_notes/research_ipc_middleware_2026.md`:
#
#   §5  SPSC ring buffers (rigtorp/SPSCQueue, MIT, header-only C++11)
#       → ~8 ns wait-free push/pop, zero heap on hot path.
#   §6  Binary WebSocket egress to the SvelteKit dashboard.
#       The binding spec calls for `uWebSockets`, but that library's hard
#       dependency on `uSockets` (a sibling C library with its own makefile
#       build) is documented as a Windows + MinGW pain point.  This project
#       uses **IXWebSocket** (Apache-2.0, single CMakeLists, no `uSockets`)
#       as the explicitly-permitted fallback.  The wire contract is
#       identical: binary frames on `ws://127.0.0.1:8765` with origin
#       loopback enforcement.  Justification: a) one-shot FetchContent
#       build cleanly under MSYS2 mingw64, b) thread-safe `sendBinary`,
#       c) per-message `Open` callback exposes the HTTP `Origin` header
#       and peer IP for handshake-layer rejection.
# =============================================================================

# C++17 strict — matches `core/vision_pipeline/`.  We deliberately do NOT
# include `roc/kinematics/tracker.hpp` or `roc/kinematics/wire_format.hpp`
# anywhere in the transport library or its public headers, because those
# expose `std::span` (C++20).  The transport library only handles raw byte
# buffers; the orchestrator (`core/main.cpp`) is what bridges the C++20
# tracker API to the C++17 transport API.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(ROC_TRANSPORT_BUILD_TESTS "Build Catch2 v3 unit tests" ON)

# -----------------------------------------------------------------------------
# Warning + optimisation flags — identical to the existing two libraries.
# -----------------------------------------------------------------------------
set(_roc_warn_flags
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
)
set(_roc_opt_flags
    -O3
    -march=native
    -fno-math-errno
    -fno-trapping-math
)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# CMake 4.x emits warnings when sub-projects declare a minimum < 3.5.  The
# vision_pipeline subdirectory already sets this; we set it again defensively
# in case `core/transport` is configured stand-alone.
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# -----------------------------------------------------------------------------
# rigtorp/SPSCQueue — MIT-licensed wait-free SPSC ring (§5.3 binding spec).
# Header-only C++11.  We only need the include directory; no built artefact.
# -----------------------------------------------------------------------------
FetchContent_Declare(
    rigtorp_SPSCQueue
    GIT_REPOSITORY https://github.com/rigtorp/SPSCQueue.git
    GIT_TAG        v1.1
    GIT_SHALLOW    TRUE
)
FetchContent_GetProperties(rigtorp_SPSCQueue)
if(NOT rigtorp_spscqueue_POPULATED)
    FetchContent_Populate(rigtorp_SPSCQueue)
endif()
# The library ships its public header as `include/rigtorp/SPSCQueue.h`.
set(_roc_spsc_include "${rigtorp_spscqueue_SOURCE_DIR}/include")
if(NOT EXISTS "${_roc_spsc_include}/rigtorp/SPSCQueue.h")
    message(FATAL_ERROR
        "roc_transport: rigtorp/SPSCQueue header missing at ${_roc_spsc_include}")
endif()
message(STATUS "roc_transport: rigtorp/SPSCQueue at ${_roc_spsc_include}")

# -----------------------------------------------------------------------------
# IXWebSocket — Apache-2.0 single-CMakeLists WebSocket library (§6.3 fallback).
# -----------------------------------------------------------------------------
# We disable every optional feature we don't use to keep configure time low
# and to avoid pulling extra system libraries (OpenSSL, MbedTLS).  Loopback
# only — TLS is not needed for an `127.0.0.1` dashboard bridge.
set(USE_TLS                 OFF CACHE BOOL "" FORCE)
set(USE_OPEN_SSL            OFF CACHE BOOL "" FORCE)
set(USE_MBED_TLS            OFF CACHE BOOL "" FORCE)
set(USE_SECURE_TRANSPORT    OFF CACHE BOOL "" FORCE)
# IXWebSocket's `USE_WS` flag builds the `ws` CLI demo (which pulls in a
# vendored msgpack11 that's broken on g++ 15 — missing <cstdint> includes).
# We only need the library; disable the demo.
set(USE_WS                  OFF CACHE BOOL "" FORCE)
set(USE_TEST                OFF CACHE BOOL "" FORCE)
set(USE_ZLIB                ON  CACHE BOOL "" FORCE)
set(BUILD_DEMO              OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS       OFF CACHE BOOL "" FORCE)
set(IXWEBSOCKET_INSTALL     OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    IXWebSocket
    GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
    GIT_TAG        v11.4.5
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(IXWebSocket)

# Quiet some upstream warnings that fire under -Wpedantic / -Wshadow on g++ 15.
if(TARGET ixwebsocket)
    target_compile_options(ixwebsocket PRIVATE
        -Wno-pedantic
        -Wno-shadow
        -Wno-conversion
        -Wno-deprecated-declarations
        -Wno-unused-parameter
    )
endif()

# -----------------------------------------------------------------------------
# OpenCV — required for the `FrameBatch::frame` cv::Mat payload in
# `roc/transport/spsc_links.hpp`.  We re-use the system MSYS2 mingw64 package.
# -----------------------------------------------------------------------------
find_package(OpenCV QUIET COMPONENTS core imgcodecs)
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR
        "roc_transport: OpenCV (core + imgcodecs) not found.\n"
        "  On MSYS2 mingw64: pacman -S mingw-w64-x86_64-opencv\n"
        "  Or set -DOpenCV_DIR=<path-to-OpenCVConfig.cmake>")
endif()
message(STATUS "roc_transport: OpenCV ${OpenCV_VERSION} from ${OpenCV_DIR}")

# -----------------------------------------------------------------------------
# kinematics_engine sibling dependency — provides roc::kinematics for the
# Detection / OperatorCommand-target / TrackId vocabulary (the transport
# headers reference these in DetectionBatch and OperatorCommand).
# -----------------------------------------------------------------------------
if(NOT TARGET roc::kinematics)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../kinematics_engine
                     ${CMAKE_BINARY_DIR}/_roc_kinematics_engine_build)
endif()

# -----------------------------------------------------------------------------
# roc_transport — static library
# -----------------------------------------------------------------------------
set(_roc_transport_sources
    src/ws_egress.cpp
    src/operator_command.cpp
    src/mjpeg_server.cpp
)

# Win32 desktop / window / region capture is Windows-only by definition.
if(WIN32)
    list(APPEND _roc_transport_sources src/desktop_capture.cpp)
endif()

set(_roc_transport_headers
    include/roc/transport/spsc_links.hpp
    include/roc/transport/operator_command.hpp
    include/roc/transport/origin_check.hpp
    include/roc/transport/ws_egress.hpp
    include/roc/transport/output_sink.hpp
    include/roc/transport/wire_format.hpp
    include/roc/transport/hardware_sinks.hpp
    include/roc/transport/mjpeg_server.hpp
)

if(WIN32)
    list(APPEND _roc_transport_headers include/roc/transport/desktop_capture.hpp)
endif()

add_library(roc_transport STATIC ${_roc_transport_sources} ${_roc_transport_headers})
add_library(roc::transport ALIAS roc_transport)

target_include_directories(roc_transport
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${_roc_spsc_include}>
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(roc_transport
    PUBLIC
        roc::kinematics
        ${OpenCV_LIBS}
    PRIVATE
        ixwebsocket
)

target_compile_options(roc_transport PRIVATE
    ${_roc_warn_flags}
    $<$<CONFIG:Release>:${_roc_opt_flags}>
    $<$<CONFIG:RelWithDebInfo>:${_roc_opt_flags}>
)

# IXWebSocket links against ws2_32 / iphlpapi on Windows; propagate that to
# any consumer linking us privately.  CMake usually handles this via the
# imported target, but we name them explicitly for the MinGW + Ninja path.
if(WIN32)
    # ws2_32 / iphlpapi for IXWebSocket; Gdi32 / User32 for desktop_capture.cpp
    # (BitBlt, EnumDisplayMonitors, EnumWindows, GetWindowText, DrawIconEx,
    # SetProcessDPIAware). We avoid linking shcore — the modern per-monitor
    # DPI API is loaded dynamically via GetProcAddress, the older user32
    # SetProcessDPIAware is the universal fallback.
    target_link_libraries(roc_transport PRIVATE ws2_32 iphlpapi Gdi32 User32)
endif()

# -----------------------------------------------------------------------------
# roc_transport_tests — Catch2 v3 test executable
# -----------------------------------------------------------------------------
if(ROC_TRANSPORT_BUILD_TESTS)
    if(NOT TARGET Catch2::Catch2WithMain)
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG        v3.5.4
            GIT_SHALLOW    TRUE
        )
        set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
        set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(Catch2)
    endif()

    # Resolve the Catch2 source directory so we can find `extras/Catch.cmake`
    # (which provides `catch_discover_tests`).  Both `catch2_SOURCE_DIR`
    # (set by FetchContent_MakeAvailable in the parent scope where Catch2
    # was first fetched) and `Catch2_SOURCE_DIR` (cmake-cache-style) need
    # to be considered, because the original fetch happened inside the
    # kinematics_engine subdirectory and those variables don't propagate
    # upward by default.
    if(NOT DEFINED catch2_SOURCE_DIR)
        FetchContent_GetProperties(Catch2 SOURCE_DIR catch2_SOURCE_DIR)
    endif()
    if(NOT catch2_SOURCE_DIR OR NOT EXISTS "${catch2_SOURCE_DIR}/extras/Catch.cmake")
        # Fall back to the cache-style upper-case form some CMake versions use.
        if(EXISTS "${Catch2_SOURCE_DIR}/extras/Catch.cmake")
            set(catch2_SOURCE_DIR "${Catch2_SOURCE_DIR}")
        endif()
    endif()
    if(NOT catch2_SOURCE_DIR OR NOT EXISTS "${catch2_SOURCE_DIR}/extras/Catch.cmake")
        # Last-ditch search under the binary directory.
        file(GLOB _catch_candidate "${CMAKE_BINARY_DIR}/_deps/catch2-src")
        if(_catch_candidate)
            set(catch2_SOURCE_DIR "${_catch_candidate}")
        endif()
    endif()
    if(NOT catch2_SOURCE_DIR)
        message(FATAL_ERROR
            "roc_transport: cannot locate Catch2 source directory after FetchContent")
    endif()
    message(STATUS "roc_transport: Catch2 source = ${catch2_SOURCE_DIR}")

    set(_roc_transport_test_sources
        tests/test_spsc_links.cpp
        tests/test_ws_egress.cpp
        tests/test_origin_check.cpp
        tests/test_operator_command.cpp
        tests/test_wire_format.cpp
        tests/test_hardware_sinks.cpp
        tests/test_orchestrator_smoke.cpp
    )

    add_executable(roc_transport_tests ${_roc_transport_test_sources})

    # The orchestrator smoke test calls `roc::kinematics::Tracker::step`
    # directly which exposes `std::span<const Detection>` — that requires
    # C++20.  We compile the entire test driver at C++20 (cheaper / simpler
    # than per-file overrides which fight CMAKE_CXX_STANDARD).  The static
    # library `roc_transport` itself stays at C++17 by design.
    set_target_properties(roc_transport_tests PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    target_link_libraries(roc_transport_tests
        PRIVATE
            roc_transport
            roc::kinematics
            Catch2::Catch2WithMain
            ixwebsocket
            ${OpenCV_LIBS}
    )
    target_include_directories(roc_transport_tests PRIVATE
        ${OpenCV_INCLUDE_DIRS}
    )

    target_compile_options(roc_transport_tests PRIVATE
        ${_roc_warn_flags}
        $<$<CONFIG:Release>:${_roc_opt_flags}>
    )

    if(WIN32)
        target_link_libraries(roc_transport_tests PRIVATE ws2_32 iphlpapi)
    endif()

    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
    include(CTest)
    include(Catch)
    catch_discover_tests(roc_transport_tests
        REPORTER console
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/test_logs
    )

    # Bundle MinGW runtime DLLs alongside the test binary so it runs on hosts
    # without MSYS2 mingw64 on PATH (see ../cmake/RocBundleMingwRuntime.cmake).
    include(${CMAKE_CURRENT_LIST_DIR}/../cmake/RocBundleMingwRuntime.cmake)
    roc_bundle_mingw_runtime(roc_transport_tests)
endif()
