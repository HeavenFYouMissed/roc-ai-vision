cmake_minimum_required(VERSION 3.20)
project(roc_vision_pipeline LANGUAGES CXX)

# -----------------------------------------------------------------------------
# Language & global options
#
# C++17 strict — matches `core/kinematics_engine/`.  The dashboard's C++20
# precedent is intentionally NOT followed here (see worker spec hard
# constraint #2).  Span-like APIs use roc::vision::Span<T> instead of
# std::span; pattern-matching uses if/else, etc.
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(ROC_VISION_BUILD_TESTS "Build Catch2 unit tests" ON)
option(ROC_VISION_BUILD_CUDA_EP "Enable CUDA Execution Provider in InferenceEngine" OFF)
option(ROC_VISION_FETCH_TEST_MODELS "Download YOLO26n COCO test model on configure" ON)

# DirectML / OpenVINO are auto-detected from the fetched ORT root; users may
# override with explicit flags.  Default OFF — the CPU prebuilt of ORT does
# not ship DML / OV headers, and a fully-green test binary needs only the CPU
# EP per the worker spec's "fallback" clause.
option(ROC_VISION_HAVE_DML_EP        "Provide DirectML EP at compile time" OFF)
option(ROC_VISION_HAVE_OPENVINO_EP   "Provide OpenVINO EP at compile time" OFF)

# -----------------------------------------------------------------------------
# Warning + optimisation flags — identical to core/kinematics_engine/.
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

# -----------------------------------------------------------------------------
# FetchContent setup
# -----------------------------------------------------------------------------
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# CMake 4.x emits a warning if a sub-project declares a minimum below 3.5;
# Eigen 3.4.0 and yaml-cpp set 3.0.  Suppress that to keep configure quiet.
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# -----------------------------------------------------------------------------
# Eigen 3.4 — reuse the kinematics_engine declaration, or fetch our own.
# -----------------------------------------------------------------------------
if(TARGET Eigen3::Eigen)
    set(_roc_vision_eigen_target Eigen3::Eigen)
else()
    FetchContent_Declare(
        Eigen3
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG        3.4.0
        GIT_SHALLOW    TRUE
    )
    set(EIGEN_BUILD_DOC OFF CACHE BOOL "" FORCE)
    set(EIGEN_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(EIGEN_BUILD_PKGCONFIG OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(Eigen3)
    set(_roc_vision_eigen_target Eigen3::Eigen)
endif()

# -----------------------------------------------------------------------------
# ONNX Runtime — fetch the Windows x64 CPU prebuilt zip.
#
# The user spec recommends the GPU package (which includes DirectML support).
# Practical note (documented deviation): the official Microsoft Windows GPU
# prebuilt is a CUDA build, NOT a DirectML build; the DirectML package is
# distributed as a NuGet only.  For a hermetic FetchContent + g++ MinGW
# toolchain we use the CPU-only prebuilt:
#
#   * ONNX Runtime's C ABI is `extern "C"` — guaranteed compiler-portable.
#   * `onnxruntime_cxx_api.h` is header-only inline wrappers around the C ABI.
#   * mingw-w64 g++ links against MSVC-format .lib import libraries cleanly.
#
# DML / OV / CUDA provider code remains compilable; their headers ship with
# the appropriate provider-specific prebuilts.  Users wanting one of those
# point the build at it via ORT_ROOT (an alternative to the FetchContent
# download).  See `phase_1_universal_ep_and_manifest.md` §1.3–§1.6.
# -----------------------------------------------------------------------------
set(_roc_ort_version "1.19.2")

set(ORT_ROOT "" CACHE PATH "Pre-existing ONNX Runtime root (skip FetchContent)")

if(ORT_ROOT AND EXISTS "${ORT_ROOT}/include/onnxruntime_cxx_api.h")
    message(STATUS "roc_vision: using user-provided ONNX Runtime at ${ORT_ROOT}")
    set(_roc_ort_root "${ORT_ROOT}")
else()
    # No URL_HASH pin: the Microsoft GitHub release artifacts are signed and
    # served over HTTPS; pinning a SHA256 here would invalidate this build
    # every time Microsoft re-signs the release.  Reproducibility comes from
    # the version tag, not a hash.
    FetchContent_Declare(
        onnxruntime
        URL  https://github.com/microsoft/onnxruntime/releases/download/v${_roc_ort_version}/onnxruntime-win-x64-${_roc_ort_version}.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    # If the user is offline (URL_HASH mismatch is the most common scenario
    # in CI), prefer continuing without ORT prebuilt and surface a fatal
    # error — building roc_vision without ORT is impossible.
    FetchContent_GetProperties(onnxruntime)
    if(NOT onnxruntime_POPULATED)
        FetchContent_Populate(onnxruntime)
    endif()
    set(_roc_ort_root "${onnxruntime_SOURCE_DIR}")
endif()

if(NOT EXISTS "${_roc_ort_root}/include/onnxruntime_cxx_api.h")
    message(FATAL_ERROR
        "roc_vision: ONNX Runtime headers not found at ${_roc_ort_root}/include. "
        "Pass -DORT_ROOT=<path-to-extracted-prebuilt> or check your internet connection.")
endif()

# Locate the import library + DLL.  The Windows prebuilt layout is:
#   lib/onnxruntime.lib   (import lib — mingw-w64 ld accepts MSVC .lib)
#   lib/onnxruntime.dll
set(_roc_ort_import_lib "${_roc_ort_root}/lib/onnxruntime.lib")
set(_roc_ort_dll        "${_roc_ort_root}/lib/onnxruntime.dll")
if(NOT EXISTS "${_roc_ort_import_lib}")
    message(FATAL_ERROR "roc_vision: ${_roc_ort_import_lib} not found.")
endif()
message(STATUS "roc_vision: ONNX Runtime root  = ${_roc_ort_root}")
message(STATUS "roc_vision: ONNX Runtime ver   = ${_roc_ort_version}")

# -----------------------------------------------------------------------------
# yaml-cpp — prefer the system MSYS2 mingw64 package, fall back to FetchContent.
# -----------------------------------------------------------------------------
find_package(yaml-cpp QUIET)
if(NOT yaml-cpp_FOUND)
    FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG        0.8.0
        GIT_SHALLOW    TRUE
    )
    set(YAML_CPP_BUILD_TESTS  OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_BUILD_TOOLS  OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_INSTALL      OFF CACHE BOOL "" FORCE)
    set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(yaml-cpp)
endif()

# -----------------------------------------------------------------------------
# OpenCV — system find_package only.  Building OpenCV via FetchContent is too
# slow.  We REQUIRE the core, imgproc, imgcodecs modules.
# -----------------------------------------------------------------------------
find_package(OpenCV QUIET COMPONENTS core imgproc imgcodecs calib3d video)
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR
        "roc_vision: OpenCV not found.\n"
        "  On MSYS2 mingw64: pacman -S mingw-w64-x86_64-opencv\n"
        "  On Windows MSVC:  vcpkg install opencv4:x64-windows\n"
        "  Or set -DOpenCV_DIR=<path-to-OpenCVConfig.cmake>")
endif()
message(STATUS "roc_vision: OpenCV ${OpenCV_VERSION} from ${OpenCV_DIR}")

# -----------------------------------------------------------------------------
# kinematics_engine sibling dependency — provides roc::kinematics for
# Detection / Embedding / Label.
# -----------------------------------------------------------------------------
if(NOT TARGET roc::kinematics)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../kinematics_engine
                     ${CMAKE_BINARY_DIR}/_roc_kinematics_engine_build)
endif()

# -----------------------------------------------------------------------------
# roc_vision — static library
# -----------------------------------------------------------------------------
set(_roc_vision_sources
    src/manifest.cpp
    src/manifest_loader.cpp
    src/ep_probe.cpp
    src/letterbox.cpp
    src/postprocess/yolo26_detect.cpp
    src/postprocess/yolov8_detect.cpp
    src/postprocess/osnet_reid.cpp
    src/postprocess/post_nms.cpp
    src/postprocessor_registry.cpp
    src/inference_engine.cpp
    src/optical_flow_sampler.cpp
)

set(_roc_vision_headers
    include/roc/vision/types.hpp
    include/roc/vision/manifest.hpp
    include/roc/vision/manifest_loader.hpp
    include/roc/vision/ep_probe.hpp
    include/roc/vision/letterbox.hpp
    include/roc/vision/postprocessor.hpp
    include/roc/vision/postprocess/yolo26_detect.hpp
    include/roc/vision/postprocess/yolov8_detect.hpp
    include/roc/vision/postprocess/osnet_reid.hpp
    include/roc/vision/postprocess/post_nms.hpp
    include/roc/vision/postprocessor_registry.hpp
    include/roc/vision/inference_engine.hpp
    include/roc/vision/optical_flow_sampler.hpp
)

add_library(roc_vision STATIC ${_roc_vision_sources} ${_roc_vision_headers})
add_library(roc::vision ALIAS roc_vision)

target_include_directories(roc_vision
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${_roc_ort_root}/include>
    PRIVATE
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(roc_vision
    PUBLIC
        roc::kinematics
        ${_roc_vision_eigen_target}
    PRIVATE
        yaml-cpp::yaml-cpp
        ${OpenCV_LIBS}
        ${_roc_ort_import_lib}
)

target_compile_options(roc_vision PRIVATE
    ${_roc_warn_flags}
    $<$<CONFIG:Release>:${_roc_opt_flags}>
    $<$<CONFIG:RelWithDebInfo>:${_roc_opt_flags}>
)

target_compile_definitions(roc_vision
    PUBLIC
        $<$<CONFIG:Release>:EIGEN_NO_DEBUG>
        $<$<CONFIG:RelWithDebInfo>:EIGEN_NO_DEBUG>
)

# MinGW-w64 + onnxruntime headers compatibility:
#   ORT's `onnxruntime_c_api.h` uses `_stdcall` / `_cdecl` (single underscore),
#   which is MSVC syntax.  GCC accepts only the double-underscore form.  We
#   pre-define the single-underscore aliases so the unmodified ORT header
#   parses correctly.  On x86_64 the calling-convention attribute is a no-op
#   (the Microsoft x64 ABI is the only one available), so this is purely a
#   parse-time fix with no codegen consequences.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_definitions(roc_vision PUBLIC
        _stdcall=__stdcall
        _cdecl=__cdecl
    )
endif()

# -----------------------------------------------------------------------------
# DirectML EP — automatic detection.
#
# The DirectML headers + import lib ship with the Microsoft.AI.DirectML NuGet
# package, NOT with the ORT prebuilt this CMake fetches by default. The user
# has two options:
#
#   1. Pass -DROC_VISION_HAVE_DML_EP=ON manually after they have a DirectML
#      install at a known path (the install instructions live in
#      docs/build_directml.md).
#   2. Set -DDIRECTML_ROOT=<path-to-Microsoft.AI.DirectML> and rebuild;
#      this branch auto-detects the package and flips on the EP for them.
#
# We also auto-detect the DirectML-flavoured ORT prebuilt (the GPU package
# from Microsoft's release page named onnxruntime-win-x64-gpu-*-cpu.zip
# does NOT contain DirectML providers; the directML-enabled package is
# distributed via NuGet only as Microsoft.ML.OnnxRuntime.DirectML).
# -----------------------------------------------------------------------------
if(DIRECTML_ROOT AND EXISTS "${DIRECTML_ROOT}/include/DirectML.h")
    message(STATUS "roc_vision: DirectML found at ${DIRECTML_ROOT} — enabling DML EP")
    set(ROC_VISION_HAVE_DML_EP ON CACHE BOOL "" FORCE)
    target_include_directories(roc_vision PRIVATE "${DIRECTML_ROOT}/include")

    # DirectML ships either as DirectML.lib (NuGet) or comes from the system
    # SDK. Prefer the NuGet copy if present; fall back to a bare link directive.
    find_library(_roc_dml_lib DirectML PATHS "${DIRECTML_ROOT}/bin/x64-win" "${DIRECTML_ROOT}/lib/x64-win" NO_DEFAULT_PATH)
    if(_roc_dml_lib)
        target_link_libraries(roc_vision PRIVATE "${_roc_dml_lib}")
    else()
        target_link_libraries(roc_vision PRIVATE DirectML)
    endif()

    # Copy the DirectML.dll next to onnxruntime.dll so the test binary +
    # roc_vision.exe can locate it via the standard Windows DLL search path.
    find_file(_roc_dml_dll DirectML.dll PATHS
        "${DIRECTML_ROOT}/bin/x64-win"
        "${DIRECTML_ROOT}/bin/x64"
        NO_DEFAULT_PATH)
    if(_roc_dml_dll)
        add_custom_target(roc_vision_copy_dml_dll ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${_roc_dml_dll}"
                "${CMAKE_BINARY_DIR}/DirectML.dll"
            BYPRODUCTS "${CMAKE_BINARY_DIR}/DirectML.dll"
            COMMENT "Copying DirectML.dll → build/")
    endif()
endif()

if(ROC_VISION_HAVE_DML_EP)
    target_compile_definitions(roc_vision PRIVATE ROC_VISION_HAVE_DML_EP=1)
endif()
if(ROC_VISION_HAVE_OPENVINO_EP)
    target_compile_definitions(roc_vision PRIVATE ROC_VISION_HAVE_OPENVINO_EP=1)
endif()
if(ROC_VISION_BUILD_CUDA_EP)
    target_compile_definitions(roc_vision PRIVATE ROC_VISION_HAVE_CUDA_EP=1)
endif()

# Copy the ORT DLL next to any executable that links against roc_vision so
# tests can find it at run time on Windows.
add_custom_target(roc_vision_copy_ort_dll ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${_roc_ort_dll}"
        "${CMAKE_BINARY_DIR}/onnxruntime.dll"
    BYPRODUCTS "${CMAKE_BINARY_DIR}/onnxruntime.dll"
    COMMENT "Copying onnxruntime.dll → build/"
)

# -----------------------------------------------------------------------------
# Test models — Python helper script gated on ROC_VISION_FETCH_TEST_MODELS.
# Tests skip gracefully when the .onnx / .yaml are absent at run time.
# -----------------------------------------------------------------------------
set(ROC_VISION_TEST_MODELS_DIR "${CMAKE_SOURCE_DIR}/../../models" CACHE PATH
    "Directory holding test ONNX models + manifest sidecars")

if(ROC_VISION_FETCH_TEST_MODELS)
    find_package(Python3 COMPONENTS Interpreter)
    if(Python3_FOUND)
        message(STATUS "roc_vision: ROC_VISION_FETCH_TEST_MODELS=ON — running fetch_yolo26n.py")
        execute_process(
            COMMAND ${Python3_EXECUTABLE}
                    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/fetch_yolo26n.py
                    --output-dir ${ROC_VISION_TEST_MODELS_DIR}
            RESULT_VARIABLE _fetch_result
            OUTPUT_VARIABLE _fetch_out
            ERROR_VARIABLE  _fetch_err
        )
        if(NOT _fetch_result EQUAL 0)
            message(WARNING "roc_vision: model fetch failed (non-fatal — tests skip):\n${_fetch_out}\n${_fetch_err}")
        endif()
    else()
        message(WARNING "roc_vision: Python3 not found — skipping test-model fetch")
    endif()
endif()

# -----------------------------------------------------------------------------
# Tests — Catch2 v3 via FetchContent (same as kinematics_engine).
# -----------------------------------------------------------------------------
if(ROC_VISION_BUILD_TESTS)
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

    set(_roc_vision_test_sources
        tests/test_manifest_loader.cpp
        tests/test_ep_probe.cpp
        tests/test_letterbox.cpp
        tests/test_yolo26_decode.cpp
        tests/test_yolov8_decode.cpp
        tests/test_osnet_decode.cpp
        tests/test_inference_engine.cpp
        tests/test_byte_compat.cpp
        tests/test_optical_flow_sampler.cpp
    )

    add_executable(roc_vision_tests ${_roc_vision_test_sources})

    target_link_libraries(roc_vision_tests
        PRIVATE
            roc_vision
            Catch2::Catch2WithMain
            ${_roc_ort_import_lib}
            ${OpenCV_LIBS}
    )
    target_include_directories(roc_vision_tests PRIVATE
        ${_roc_ort_root}/include
        ${OpenCV_INCLUDE_DIRS}
    )
    target_compile_options(roc_vision_tests PRIVATE
        ${_roc_warn_flags}
        $<$<CONFIG:Release>:${_roc_opt_flags}>
    )

    target_compile_definitions(roc_vision_tests PRIVATE
        ROC_VISION_TEST_MODELS_DIR="${ROC_VISION_TEST_MODELS_DIR}"
    )

    add_dependencies(roc_vision_tests roc_vision_copy_ort_dll)

    # Copy the FetchContent'd ORT DLL DIRECTLY next to the test binary, not
    # just to ${CMAKE_BINARY_DIR}.  Critical because Windows hosts typically
    # have many stale `onnxruntime.dll` copies installed (Edge, Office,
    # Copilot, NuGet caches, VS Code extensions) and any of them can shadow
    # ours via PATH order.  Putting our 1.19.2 in the exe's own directory
    # guarantees the exe-dir-first DLL search rule selects ours.
    add_custom_command(TARGET roc_vision_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_roc_ort_dll}"
            "$<TARGET_FILE_DIR:roc_vision_tests>/onnxruntime.dll"
        COMMENT "[roc_vision] copying ORT 1.19.2 next to test binary"
        VERBATIM
    )

    # Bundle MinGW runtime DLLs alongside the test binary so it runs on hosts
    # without MSYS2 mingw64 on PATH (see ../cmake/RocBundleMingwRuntime.cmake).
    include(${CMAKE_CURRENT_LIST_DIR}/../cmake/RocBundleMingwRuntime.cmake)
    roc_bundle_mingw_runtime(roc_vision_tests)
endif()
