# =============================================================================
# RocBundleMingwRuntime.cmake
#
# Drop the three MinGW-w64 runtime DLLs alongside any executable target so the
# resulting `.exe` is portable — it boots on a machine that does NOT have
# MSYS2 / mingw64 installed and does NOT have `C:\msys64\mingw64\bin` on PATH.
#
# Background
# ----------
# Every C++ binary compiled by mingw-w64 g++ depends at runtime on three DLLs
# that live in `<MINGW>/bin/`:
#
#   libstdc++-6.dll        ← C++ standard library
#   libgcc_s_seh-1.dll     ← GCC's exception-handling unwinder (SEH variant on x86_64)
#   libwinpthread-1.dll    ← POSIX threading shim used by libstdc++
#
# Without these, the binary aborts at process start with
# `STATUS_DLL_NOT_FOUND` (Windows code 0xC0000135) before `main()` runs.
# The standard Windows DLL search order looks in the executable's directory
# first, then System32, then PATH — so copying the three DLLs next to the
# `.exe` makes the binary trivially portable.
#
# This solves the regression the Phase 4 worker flagged as "vision suite's
# pre-existing Windows System32 ORT-DLL shadowing": the actual root cause was
# MinGW runtime DLLs missing on a clean PATH, not ORT DLLs being shadowed.
# The local on-disk ORT DLL was always being found (it's adjacent in
# `_deps/onnxruntime-src/lib/` and gets copied next to the test binary by
# `roc_vision_copy_ort_dll`); what was failing was the MinGW runtime
# resolution.
#
# Usage
# -----
#   include(${CMAKE_CURRENT_LIST_DIR}/../cmake/RocBundleMingwRuntime.cmake)
#   add_executable(my_binary main.cpp)
#   roc_bundle_mingw_runtime(my_binary)
#
# The include path is relative to the calling CMakeLists.txt so it works both
# when the subdirectory is built standalone (`cmake -S vision_pipeline -B …`)
# and when it's added as a sub-project from `core/CMakeLists.txt`.
#
# No-op on non-Windows hosts and non-GCC/non-Clang compilers (MSVC ships its
# runtime via the Visual C++ Redistributable instead — different problem).
# =============================================================================

# Capture the directory of THIS file at include time.  `CMAKE_CURRENT_LIST_DIR`
# inside the function below evaluates at the function's CALL-site, not its
# definition-site, so we'd resolve the script path against the caller's
# CMakeLists.txt instead of against this module file.  Storing the path in a
# module-scope variable at include time pins it correctly.
set(_ROC_BUNDLE_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(roc_bundle_mingw_runtime target)
    if(NOT WIN32)
        return()
    endif()
    if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
        # MSVC + Clang-cl get their runtime via the MS redistributable; skip.
        return()
    endif()

    get_filename_component(_mingw_bin "${CMAKE_CXX_COMPILER}" DIRECTORY)

    # Use CMake's `file(GET_RUNTIME_DEPENDENCIES ...)` (via a POST_BUILD
    # -P script) to walk the executable's import table + every transitive
    # DLL dependency, resolve against the MinGW bin directory, and copy the
    # whole set next to the binary.  This catches `libopencv_*.dll`,
    # `libyaml-cpp.dll`, `zlib1.dll`, etc. — anything the binary actually
    # uses — without having to hard-code names in this file.  The exclusion
    # regexes in RocBundleRuntimeDeps.cmake prevent us from accidentally
    # shadowing Windows system DLLs (KERNEL32, WS2_32, DirectML, …).
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            -D TARGET_FILE=$<TARGET_FILE:${target}>
            -D DEST_DIR=$<TARGET_FILE_DIR:${target}>
            -D SEARCH_DIRS=${_mingw_bin}
            -P "${_ROC_BUNDLE_MODULE_DIR}/RocBundleRuntimeDeps.cmake"
        COMMENT "[roc] bundling runtime DLLs for ${target}"
        VERBATIM
    )
endfunction()
