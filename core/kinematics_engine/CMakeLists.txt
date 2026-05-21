cmake_minimum_required(VERSION 3.20)
project(roc_kinematics_engine LANGUAGES CXX)

# -----------------------------------------------------------------------------
# Language & global options
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Note: rule 100-core-kinematics.mdc nominally pins us to C++17.  We bump to
# C++20 so we can use std::span (which the worker spec explicitly requested
# in the Tracker::step signature).  No other C++20 features are used; the
# code remains C++17-portable except for the std::span declaration.

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(ROC_KINEMATICS_BUILD_TESTS "Build Catch2 v3 unit tests" ON)
option(ROC_KINEMATICS_RUNTIME_NO_MALLOC
       "Trip Eigen runtime malloc assertions in hot-path TUs (debug aid)" OFF)

# -----------------------------------------------------------------------------
# Warning + optimisation flags (per phase_2_ekf_kinematics.md §9 checklist)
# -----------------------------------------------------------------------------
# NOTE: -ffast-math is intentionally OMITTED.  The CWNJ Q(Δt) and Joseph
# covariance update rely on strict IEEE-754 associativity to preserve
# symmetry / positive-definiteness; -ffast-math allows the compiler to
# re-associate FMAs and silently destroy that invariant.  We do enable the
# two trap flags explicitly permitted by §9 (errno + trapping math elision).
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

# Slightly relaxed flag set for the bundled third-party Catch2 / Eigen so we
# don't drown in upstream noise.
set(_roc_thirdparty_flags
    -Wno-shadow
    -Wno-conversion
    -Wno-pedantic
    -Wno-deprecated-declarations
)

# -----------------------------------------------------------------------------
# Dependencies — Eigen 3.4 + Catch2 v3 via FetchContent
# -----------------------------------------------------------------------------
# We use FetchContent (rather than find_package) so the build is hermetic
# on Windows where neither Eigen nor Catch2 are guaranteed to be installed
# system-wide.  Fallback: if a working `find_package(Eigen3 3.4)` exists on
# the host, set ROC_KINEMATICS_USE_SYSTEM_EIGEN=ON and the FetchContent for
# Eigen will be skipped.
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

option(ROC_KINEMATICS_USE_SYSTEM_EIGEN "Prefer find_package(Eigen3) over FetchContent" OFF)
set(_roc_eigen_target "")
if(ROC_KINEMATICS_USE_SYSTEM_EIGEN)
    find_package(Eigen3 3.4 REQUIRED NO_MODULE)
    set(_roc_eigen_target Eigen3::Eigen)
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
    set(_roc_eigen_target Eigen3::Eigen)
endif()

# -----------------------------------------------------------------------------
# roc_kinematics — static library
# -----------------------------------------------------------------------------
set(_roc_kinematics_sources
    src/ca_ekf.cpp
    src/pinhole_measurement.cpp
    src/regime_detector.cpp
    src/track.cpp
    src/association.cpp
    src/lap_jv.cpp
    src/hierarchical_constraint.cpp
    src/target_selector.cpp
    src/pending_spawn_buffer.cpp
    src/wire_format.cpp
    src/tracker.cpp
)

set(_roc_kinematics_headers
    include/roc/kinematics/types.hpp
    include/roc/kinematics/object_priors.hpp
    include/roc/kinematics/ca_ekf.hpp
    include/roc/kinematics/pinhole_measurement.hpp
    include/roc/kinematics/regime_detector.hpp
    include/roc/kinematics/track.hpp
    include/roc/kinematics/association.hpp
    include/roc/kinematics/lap_jv.hpp
    include/roc/kinematics/hierarchical_constraint.hpp
    include/roc/kinematics/target_selector.hpp
    include/roc/kinematics/pending_spawn_buffer.hpp
    include/roc/kinematics/wire_format.hpp
    include/roc/kinematics/tracker.hpp
)

add_library(roc_kinematics STATIC ${_roc_kinematics_sources} ${_roc_kinematics_headers})
add_library(roc::kinematics ALIAS roc_kinematics)

target_include_directories(roc_kinematics
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(roc_kinematics PUBLIC ${_roc_eigen_target})

target_compile_options(roc_kinematics PRIVATE
    ${_roc_warn_flags}
    $<$<CONFIG:Release>:${_roc_opt_flags}>
    $<$<CONFIG:RelWithDebInfo>:${_roc_opt_flags}>
)

# Eigen compile-time hardening on the hot-path TUs:
#   EIGEN_NO_DEBUG       — strip Eigen runtime asserts in release
#   EIGEN_RUNTIME_NO_MALLOC + Eigen::internal::set_is_malloc_allowed(false) at
#                          tracker boot — proves zero heap allocation in
#                          predict / update / association
target_compile_definitions(roc_kinematics
    PUBLIC
        $<$<CONFIG:Release>:EIGEN_NO_DEBUG>
        $<$<CONFIG:RelWithDebInfo>:EIGEN_NO_DEBUG>
)
if(ROC_KINEMATICS_RUNTIME_NO_MALLOC)
    target_compile_definitions(roc_kinematics PUBLIC EIGEN_RUNTIME_NO_MALLOC)
endif()

# -----------------------------------------------------------------------------
# roc_kinematics_tests — Catch2 v3 test executable
# -----------------------------------------------------------------------------
if(ROC_KINEMATICS_BUILD_TESTS)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.4
        GIT_SHALLOW    TRUE
    )
    set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
    set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(Catch2)

    set(_roc_test_sources
        tests/test_ca_ekf.cpp
        tests/test_pinhole.cpp
        tests/test_regime.cpp
        tests/test_hierarchy.cpp
        tests/test_association.cpp
        tests/test_wire_format.cpp
        tests/test_tracker.cpp
    )

    add_executable(roc_kinematics_tests ${_roc_test_sources})

    target_link_libraries(roc_kinematics_tests
        PRIVATE
            roc_kinematics
            Catch2::Catch2WithMain
    )

    target_compile_options(roc_kinematics_tests PRIVATE
        ${_roc_warn_flags}
        $<$<CONFIG:Release>:${_roc_opt_flags}>
    )

    # Per-test discovery so CTest can drive individual cases.
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
    include(CTest)
    include(Catch)
    catch_discover_tests(roc_kinematics_tests
        REPORTER console
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/test_logs
    )

    # Bundle MinGW runtime DLLs alongside the test binary so it runs on hosts
    # without MSYS2 mingw64 on PATH (see ../cmake/RocBundleMingwRuntime.cmake).
    include(${CMAKE_CURRENT_LIST_DIR}/../cmake/RocBundleMingwRuntime.cmake)
    roc_bundle_mingw_runtime(roc_kinematics_tests)
endif()
