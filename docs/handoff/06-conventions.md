# 06 — Conventions, gotchas, build quirks

## C++ standard split

- **`core/vision_pipeline`** is strict C++17. Uses `roc::vision::Span<T>`
  shim instead of `std::span`. Do not introduce C++20 features here.
- **`core/kinematics_engine`** is C++20 (uses `std::span`). The Eigen +
  Joseph-form math depends on it.
- **`core/transport`** is C++17 (matches vision_pipeline). Avoid C++20
  here too — its public headers are consumed by the C++17 vision side.
- **`core/main.cpp`** picks up whatever the orchestrator's CMake sets;
  it consumes all three libraries so it bumps to the highest common
  denominator (C++20 for `std::span` in the kinematics call sites).

## mingw-w64 quirks

- ORT's headers use single-underscore `_stdcall` / `_cdecl` MSVC macros
  that mingw-w64 g++ doesn't know. The vision_pipeline CMake adds
  `_stdcall=__stdcall` + `_cdecl=__cdecl` PUBLIC compile defs to fix.
- Don't `#include <shellscalingapi.h>`. It pulls in `shcore.lib` which
  isn't always present on mingw-w64. Use `SetProcessDPIAware()` from
  user32 instead (universal, ships everywhere).
- No `<format>`. mingw-w64's libstdc++ doesn't ship it through g++ 15.
  Use `std::snprintf` into a `char[]` buffer instead.

## Build invariants

- Always build out-of-source: `cmake -S core -B core/build -G Ninja`.
- Always Release. Debug builds work but inference is unusably slow.
- The orchestrator's runtime-DLL bundling is a custom CMake target
  (`roc_bundle_mingw_runtime`) that copies ~180 DLLs alongside any
  executable that links against `roc_transport` (because IXWebSocket
  pulls in OpenSSL / zlib / etc transitively). If a test exe fails to
  start with "DLL not found", the bundle script missed something —
  check `core/cmake/RocBundleMingwRuntime.cmake`.

## Svelte 5 patterns (this project is runes-mode only)

- Every reactive piece of state uses `$state(...)` — no `let` for state
  by accident.
- Stores are `.svelte.ts` files (the extension is required so the runes
  compile). Plain `.ts` files cannot contain `$state` — silent runtime
  error.
- Refs are bound with `bind:this={el}` and `el` must be declared
  `$state<HTMLElement | undefined>()`. Not just `let`.
- `$effect(() => { ... })` for side effects. Reads `$state` inside the
  effect track it as a dep automatically.
- Don't read a `$state` variable inside the same `$effect` that also
  writes it — Svelte will treat it as a dep, the write re-fires the
  effect, infinite loop. Hit this in `SparklinePanel.svelte` once; pass
  the value through a function arg instead.

## Wire format invariants

- All multi-byte fields are little-endian (x86_64 + ARM64 LE).
- Tracker wire frame: 12 B header + 86 B per track. The math in the
  spec sums to 90 B but the binding contract pins 86 B — the SvelteKit
  decoder agrees, so the §7.2 spec's `P_pos_offnorm` is intentionally
  not on the wire.
- `OperatorCommand` is fixed 22 bytes. If you need a new command kind
  that needs more payload (e.g. a string), do NOT widen this struct —
  add a new HTTP endpoint instead (see `/api/model/active` for the
  pattern).

## Testing

- C++ tests are Catch2 v3. Run with `--reporter compact` for clean log.
- `roc_vision_tests.exe`: 48 cases. Some need a real `.onnx` in `models/`
  — they skip-with-message if missing.
- `roc_kinematics_tests.exe`: 37 cases. All self-contained, no fixture
  dependencies.
- The "ORT warnings about preferred EP" lines during tests are NORMAL
  when DirectML is active — they mean shape/control ops were assigned
  to CPU (which is correct behaviour for any non-CPU EP).

## Living blueprint mandate

Every file added / renamed / moved REQUIRES an update to
`workspace_blueprint/project_tree.md` in the same commit. The rule
exists because the file is the only place that records *why* a given
file exists. The previous amendment headers (v6 / v7 / v8) at the top
of that file are the template for new amendments.

## REST API surface

| Endpoint | Method | What |
|---|---|---|
| `/health` | GET | Liveness |
| `/sources` | GET | Camera devices |
| `/mjpeg/{idx}` | GET | Motion-JPEG video stream |
| `/api/models` | GET | List manifests in `--models-dir` |
| `/api/model/active` | GET / POST | Currently loaded / swap to new |
| `/api/fov` | GET / POST | FOV crop settings |
| `/api/capture-sources` | GET | Live monitor + window enum |

Default ports: WebSocket binary `:8765`, HTTP `:8766`. Loopback-only by
default; `--ws-bind 0.0.0.0` for LAN exposure (rarely correct).

## Git workflow

- Main branch only; no feature branches yet.
- Commit messages: subject line + paragraph body explaining the WHY.
  Most existing commits follow this — match the tone.
- The user's GitHub identity is configured repo-locally only (NOT
  global) to a noreply email form. Don't `git config --global` anything.
- `.gitignore` excludes `third_party/`, all `build/` dirs, `*.onnx`,
  `*.dll`, `*.exe`, `node_modules/`, `.svelte-kit/`. Don't push any
  of those.
