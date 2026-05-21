# 01 — Orientation

## What this project is

A general-purpose computer-vision + behavioural-biometrics research
instrument with two validated applications:

1. **Counter-UAS perimeter monitoring / drone tracking** — validated at
   ~400 m range on a 1080p source.
2. **Defensive anti-cheat dataset generation** — Pointer-Lock
   mouse-dynamics recorder with Croissant-compatible JSONL export.

Three planned additional applications:

3. Ender-3 repurposed as a conveyor pick-and-place sorter
4. NDI network video receiver (broadcast pipeline integration)
5. DXGI desktop capture (perf upgrade)

## Repo layout

```
roc-ai-vision/
├── core/
│   ├── main.cpp                       (six-thread orchestrator)
│   ├── vision_pipeline/               (ONNX + DirectML inference)
│   │   ├── include/roc/vision/        (~10 headers)
│   │   ├── src/                       (~8 sources)
│   │   └── tests/                     (Catch2 v3 — 48/48 green)
│   ├── kinematics_engine/             (CA-EKF + DeepSORT, C++20)
│   │   ├── include/roc/kinematics/    (~13 headers)
│   │   ├── src/                       (~11 sources)
│   │   └── tests/                     (Catch2 v3 — 37/37 green)
│   └── transport/                     (SPSC queues + WS + MJPEG + REST)
│       ├── include/roc/transport/
│       ├── src/
│       └── tests/
├── dashboard/                         (SvelteKit 2 / Svelte 5)
│   └── src/lib/
│       ├── components/destinations/
│       ├── components/drawers/
│       ├── stores/                    (Svelte 5 $state runes)
│       ├── biometrics/                (mouse_capture + metrics + export)
│       └── telemetry/                 (wire decoders)
├── models/                            (.yaml manifests; .onnx gitignored)
├── scripts/scan_onnx_to_manifest.py   (bulk ONNX -> YAML stub)
├── docs/
└── workspace_blueprint/               (living architecture docs)
```

## Build (Windows)

```powershell
$env:PATH = "C:\msys64\mingw64\bin;C:\msys64\usr\bin;$env:PATH"

# C++ side
cmake -S core -B core/build -G Ninja `
    -DCMAKE_BUILD_TYPE=Release `
    -DORT_ROOT="$(Resolve-Path third_party/ort_dml_root)" `
    -DDIRECTML_ROOT="$(Resolve-Path third_party/directml)" `
    -DROC_VISION_HAVE_DML_EP=ON
cmake --build core/build --config Release -j

# Dashboard
cd dashboard
npm install
npm run dev          # serves http://localhost:5173/
```

Produces `core/build/roc_vision.exe` (~1.5 MB) with all runtime DLLs
bundled alongside (DirectML, ORT, OpenCV, MinGW, FFmpeg — ~180 DLLs total).

## What's already built

See `workspace_blueprint/roadmap.md` for the phase-by-phase log. Highest-
level summary as of last commit:

- Phase 1 (vision pipeline + ONNX manifest + DirectML EP) — green
- Phases 2 + 3 (kinematics + EKF + DeepSORT) — green
- Phases 4-6 (orchestrator + dashboard cockpit) — green
- Phase 6 (FOV crop pipeline) — green
- Phase 7 (FOV follow-target) — green
- Phase 8 (desktop / window / region capture, GDI BitBlt) — green
- Amendment v6 (Biometrics destination, mouse-dynamics recorder) — green
- Amendment v7 (model hot-swap + Models drawer) — green
- Amendment v8 (DXGI / NDI / motion / conveyor) — **THIS HANDOFF**
