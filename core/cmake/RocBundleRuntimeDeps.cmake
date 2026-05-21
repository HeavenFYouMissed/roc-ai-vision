# =============================================================================
# RocBundleRuntimeDeps.cmake
#
# CMake-script-mode helper invoked at POST_BUILD by `roc_bundle_mingw_runtime`.
# Uses `file(GET_RUNTIME_DEPENDENCIES ...)` to walk the import table of the
# built executable + all its transitive DLL dependencies, resolves them
# against the MinGW bin directory (and any extra search dirs we pass in),
# and copies the resolved set next to the executable.
#
# Why a separate -P script: GET_RUNTIME_DEPENDENCIES must run AFTER the
# executable exists, and it depends on environment / objdump availability.
# A standalone CMake script invoked from POST_BUILD is the canonical pattern.
#
# Required variables (passed via -D from the caller):
#   TARGET_FILE     full path to the built .exe/.dll
#   DEST_DIR        directory to copy DLLs into (usually same dir as TARGET_FILE)
#   SEARCH_DIRS     ;-separated list of dirs to resolve dependencies from
#
# Optional:
#   ROC_BUNDLE_VERBOSE   if "ON", echo every copied DLL
# =============================================================================

if(NOT TARGET_FILE OR NOT DEST_DIR OR NOT SEARCH_DIRS)
    message(FATAL_ERROR
        "RocBundleRuntimeDeps.cmake: required variables not set "
        "(TARGET_FILE=${TARGET_FILE}, DEST_DIR=${DEST_DIR}, SEARCH_DIRS=${SEARCH_DIRS})")
endif()

# Skip Windows system DLLs entirely — they always come from System32 and we
# must never shadow them.  Skip api-ms- / ext-ms- API set names too (those
# are virtual; they resolve to ucrtbase / kernelbase at runtime).
set(_roc_pre_exclude
    "api-ms-.*"          # API set virtual DLLs
    "ext-ms-.*"          # extension API sets
    "^[Kk]ernel32.dll$"
    "^[Mm]svcrt.dll$"
    "^[Mm]svcrt[0-9]+.dll$"
    "^[Uu]crtbase.*"
    "^[Vv]cruntime.*"
    "^[Mm]svcp.*"
    "^WS2_32.dll$"
    "^WSOCK32.dll$"
    "^WINMM.dll$"
    "^USER32.dll$"
    "^GDI32.dll$"
    "^ADVAPI32.dll$"
    "^SHELL32.dll$"
    "^OLE32.dll$"
    "^OLEAUT32.dll$"
    "^COMCTL32.dll$"
    "^COMDLG32.dll$"
    "^SHLWAPI.dll$"
    "^IPHLPAPI.dll$"
    "^[Cc]rypt32.dll$"
    "^[Bb]crypt.dll$"
    "^[Nn]cryptn?.dll$"
    "^[Ss]ecur32.dll$"
    "^[Pp]sapi.dll$"
    "^[Dd]bghelp.dll$"
    "^[Dd]bgcore.dll$"
    "^d3d.*\\.dll$"
    "^DirectML\\.dll$"   # ships with Windows 10 1903+
    "^dxgi\\.dll$"
)

# Anything resolved INSIDE Windows / System32 is also excluded
set(_roc_post_exclude
    "[/\\\\]windows[/\\\\]"
    "[/\\\\]system32[/\\\\]"
    "[/\\\\]SysWOW64[/\\\\]"
)

file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES ${TARGET_FILE}
    RESOLVED_DEPENDENCIES_VAR _resolved
    UNRESOLVED_DEPENDENCIES_VAR _unresolved
    CONFLICTING_DEPENDENCIES_PREFIX _conflicting
    DIRECTORIES ${SEARCH_DIRS}
    PRE_EXCLUDE_REGEXES ${_roc_pre_exclude}
    POST_EXCLUDE_REGEXES ${_roc_post_exclude}
)

# Report and copy
foreach(_dep IN LISTS _resolved)
    get_filename_component(_dep_name "${_dep}" NAME)
    if(NOT EXISTS "${DEST_DIR}/${_dep_name}" OR "${_dep}" IS_NEWER_THAN "${DEST_DIR}/${_dep_name}")
        file(COPY "${_dep}" DESTINATION "${DEST_DIR}" FOLLOW_SYMLINK_CHAIN)
        if(ROC_BUNDLE_VERBOSE)
            message(STATUS "[roc] bundled ${_dep_name}")
        endif()
    endif()
endforeach()

# Warn about anything that didn't resolve — usually a sign the SEARCH_DIRS
# list needs another entry.  These are NOT fatal because some unresolved
# names are virtual API sets that the OS provides; the pre-exclude list
# tries to catch them but doesn't catch everything.
if(_unresolved)
    foreach(_u IN LISTS _unresolved)
        message(STATUS "[roc] unresolved runtime dep (may be a system DLL): ${_u}")
    endforeach()
endif()

if(_conflicting_FILENAMES)
    foreach(_filename IN LISTS _conflicting_FILENAMES)
        message(WARNING "[roc] conflicting runtime dep ${_filename}: ${_conflicting_${_filename}}")
    endforeach()
endif()
