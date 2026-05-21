cmake_minimum_required(VERSION 3.20)
project(roc_ai_core LANGUAGES CXX)

# =============================================================================
# ROC AI Vision — top-level orchestrator CMake project.
#
# Aggregates the three layered libraries:
#   * roc::kinematics  (core/kinematics_engine/   — C++20 EKF + DeepSORT)
#   * roc::vision      (core/vision_pipeline/     — C++17 ONNX inference)
#   * roc_transport    (core/transport/           — C++17 SPSC + IXWebSocket)
#
# And builds the top-level binary `roc_vision.exe` from `core/main.cpp`, which
# wires camera → inference → tracker → dashboard-egress via lock-free SPSC
# queues per `research_ipc_middleware_2026.md` §8 (Hybrid Architecture
# Recommendation) and §10 (Decision Record).
#
# Each subdirectory sets its own CMAKE_CXX_STANDARD so the three layers compile
# at their authored standard (kinematics = 20 because of `std::span` in
# `Tracker::step`, vision = 17, transport = 17).  The top-level binary itself
# is compiled at C++20 because it includes `roc/kinematics/tracker.hpp` which
# exposes `std::span` in its public API.
# =============================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(ROC_BUILD_ORCHESTRATOR "Build the top-level roc_vision.exe binary" ON)

# Warning + optimisation flags — identical to the existing two libraries
# (see core/kinematics_engine/CMakeLists.txt and core/vision_pipeline/
# CMakeLists.txt).  -ffast-math is intentionally OMITTED because the Joseph
# covariance form in the kinematics engine relies on strict IEEE-754
# associativity (§9 of phase_2_ekf_kinematics.md).
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

# -----------------------------------------------------------------------------
# Subdirectories — order matters:
#   1. kinematics_engine first (provides roc::kinematics + initial Catch2 fetch)
#   2. vision_pipeline (depends on roc::kinematics)
#   3. transport (depends on roc::kinematics for the wire format constants)
# -----------------------------------------------------------------------------
add_subdirectory(kinematics_engine)
add_subdirectory(vision_pipeline)
add_subdirectory(transport)

# -----------------------------------------------------------------------------
# roc_vision.exe — Phase-4 demonstration orchestrator
# (see `research_ipc_middleware_2026.md` §8.1 process diagram)
# -----------------------------------------------------------------------------
if(ROC_BUILD_ORCHESTRATOR)
    # Phase 6 part A: the orchestrator drives a real cv::VideoCapture in
    # `main.cpp`, so we must add the videoio module to its OpenCV link
    # surface.  `roc::vision` (the static library) intentionally does NOT
    # pull in videoio — videoio is a runtime concern of the orchestrator,
    # not of the inference pipeline.
    find_package(OpenCV REQUIRED COMPONENTS core imgproc videoio)
    message(STATUS "roc_vision_orchestrator: OpenCV ${OpenCV_VERSION} (videoio module required for cv::VideoCapture)")

    # NOTE on target name vs. binary name: `roc_vision` is already taken by
    # the vision_pipeline static library (see `core/vision_pipeline/`).  We
    # name the orchestrator target `roc_vision_orchestrator` so it doesn't
    # collide, then override OUTPUT_NAME so the produced binary is still
    # `roc_vision.exe` — exactly what the worker spec asks for.
    add_executable(roc_vision_orchestrator main.cpp)
    set_target_properties(roc_vision_orchestrator PROPERTIES
        OUTPUT_NAME "roc_vision"
    )

    target_link_libraries(roc_vision_orchestrator
        PRIVATE
            roc::kinematics
            roc::vision
            roc_transport
            ${OpenCV_LIBS}
    )

    target_include_directories(roc_vision_orchestrator PRIVATE
        ${OpenCV_INCLUDE_DIRS}
    )

    target_compile_options(roc_vision_orchestrator PRIVATE
        ${_roc_warn_flags}
        $<$<CONFIG:Release>:${_roc_opt_flags}>
        $<$<CONFIG:RelWithDebInfo>:${_roc_opt_flags}>
    )

    # Ensure the ORT DLL ships next to the orchestrator on Windows.  The vision
    # pipeline subdirectory created the copy_ort_dll target which deposits
    # the DLL at `${CMAKE_BINARY_DIR}/onnxruntime.dll` — the same directory
    # the orchestrator binary lives in (no per-target RUNTIME_OUTPUT_DIRECTORY
    # override).  Depending on the target enforces the build order without
    # needing a duplicate copy step.
    if(TARGET roc_vision_copy_ort_dll)
        add_dependencies(roc_vision_orchestrator roc_vision_copy_ort_dll)
    endif()

    # Bundle the MinGW runtime DLLs (libstdc++-6, libgcc_s_seh-1, libwinpthread-1)
    # next to the orchestrator so it boots on hosts without MSYS2 mingw64 on
    # PATH.  See core/cmake/RocBundleMingwRuntime.cmake for the rationale —
    # this fixes the STATUS_DLL_NOT_FOUND symptom that was misattributed as
    # "System32 ORT-DLL shadowing" in the Phase 4 worker report.
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/RocBundleMingwRuntime.cmake)
    roc_bundle_mingw_runtime(roc_vision_orchestrator)
endif()
