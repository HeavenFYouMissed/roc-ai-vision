---
name: roc-ekf-context
description: >-
  Provides full context for the ROC AI Vision kinematics engine: 9-D
  constant-acceleration EKF, Joseph-form update, NIS regime detector,
  Gauss-Seidel hierarchical constraint, wire format, and key file roles.
  Use when working on core/kinematics_engine/, debugging EKF behaviour,
  extending the tracker, or understanding the C++ tracking pipeline.
disable-model-invocation: true
---

# ROC Kinematics Engine Context

Load this skill before touching any file under `core/kinematics_engine/`.

## State Vector (§1.2)

9-D constant-acceleration state per tracked sub-feature, all in world frame F_W:

```
x = [ p_x  p_y  p_z | v_x  v_y  v_z | a_x  a_y  a_z ]^T ∈ R^9
      indices 0-2       indices 3-5       indices 6-8
```

- Scalar precision: `double` for filter math; `float` for wire egress and embeddings.
- Eigen types: `State (9×1)`, `StateCov (9×9)`, `Meas (4×1)`, `MeasCov (4×4)`, `Jacobian (4×9)`.
- Measurement: `[u, v, w_px, h_px]` — pixel-space bounding box centroid + size.

## Prediction (§2.1 / §2.2)

Closed-form transition matrix via nilpotent expansion:

```
F(Δt) = I_9 + [dt in (0,3)→(3,6)] + [½dt² in (0,3)→(6,8)]
```

Process noise Q(Δt) uses Continuous White-Noise Jerk (CWNJ) van-Loan integral:

```
Q = [ dt⁵/20·Qc   dt⁴/8·Qc   dt³/6·Qc ]
    [ dt⁴/8·Qc    dt³/3·Qc   dt²/2·Qc ]
    [ dt³/6·Qc    dt²/2·Qc   dt·Qc     ]
```

`Qc` is diagonal (3-vector). Each 3×3 sub-block is therefore diagonal — exploited for zero-allocation fill.

## Adaptive Process Noise (§2.6)

`NisRegimeDetector` computes EMA of the NIS scalar (ν^T S^{-1} ν):

| Regime     | η̄ range      | ρ(η̄) scale |
|------------|--------------|-------------|
| Stationary | η̄ < 1.0     | 0.05        |
| Steady     | 1.0–15.0     | 0.05 → 1.0 → 4.0 (piecewise-linear) |
| Maneuver   | η̄ > 15.0    | 4.0         |

- Init: η̄₀ = 4 (χ² mean for 4-DOF measurement). Starting at 0 would wrongly strangle acceleration.
- Stationary regime triggers zero-velocity/zero-acceleration shrinkage pseudo-measurement (§2.6.3).

## Joseph-Form Update (§3.7)

**Mandatory on every update path.** S is never explicitly inverted — Cholesky-solve only.

```cpp
// Templated kernel: joseph_update<M> handles M=2 (centroid-only),
// M=4 (full), M=6 (shrinkage), M=3 (hierarchical constraint).
P = (I - K·H)·P·(I - K·H)^T + K·R·K^T
```

Symmetrise after every predict and every Joseph update: `P = ½(P + P^T)`.

## Hierarchical Constraint (§3.11)

Two-pass Gauss-Seidel coupled pseudo-measurement between parent (Person) and sub-features (Head, UpperTorso, Accessories):

1. Pass 1: update sub-feature treating parent position as known.
2. Pass 2: update parent treating refreshed sub-feature as known.
3. N_iter = 2 outer sweeps. Convergence verified by geometric ratio ≤ 0.6/iter.

**Application gate (§3.11.4):** both filters confirmed, both `time_since_update == 0`, ‖h_rel‖ < 3σ_S. Failure emits `hierarchy_violation` event → wire status bit 3.

## Wire Format (§7)

12-byte header + 86 bytes/track. See `roc-wire-protocol` skill for byte layout.

**Key deviation:** the blueprint §7.2 table sums to 90 B (includes a `P_pos_offnorm` field). Production code pins 86 B — the Frobenius norm field was dropped. This is a documented, accepted deviation.

## Per-Frame Pipeline Order (§5 — binding)

```
predict → associate → update → stationary_shrinkage → hierarchy_constraint
→ boundary_probation → lifecycle → wire_egress
```

Re-ordering breaks the §3.11.4 gate (depends on `time_since_update == 0` post-update).

## Key Files

| File | Role |
|------|------|
| `types.hpp` | §1.2 Eigen fixed-size types, `TrackId`, `Label` enum, `Detection`, `CameraPose` |
| `ca_ekf.hpp/cpp` | 9-D EKF: `compute_F`, `compute_Q`, `CaEkf` class with predict + 4 update paths |
| `pinhole_measurement.hpp/cpp` | h(x) projection F_W→F_C→F_I, 4×9 Jacobian, selective-axis init |
| `regime_detector.hpp` | EMA-NIS, Regime enum, piecewise-linear ρ(η̄) |
| `track.hpp` | Track aggregate: filter + regime detector + EmbeddingGallery (100-entry ring) + lifecycle |
| `association.hpp/cpp` | Cosine GEMM, Mahalanobis χ²₄ gate, matching cascade, IoU fallback |
| `lap_jv.hpp/cpp` | Rectangular dense shortest-augmenting-path LAP solver (sentinel 1e6) |
| `hierarchical_constraint.hpp/cpp` | §3.11 Gauss-Seidel + plausibility gate |
| `object_priors.hpp` | Anthropometric tables (W₀, H₀), anatomical μ_S/σ_S, default R, Q_c |
| `target_selector.hpp/cpp` | Multi-target ranking + EMA-smoothed (dx, dy) |
| `pending_spawn_buffer.hpp/cpp` | §3.10.3 deferred detection holding pen (T_hold = 5) |
| `wire_format.hpp/cpp` | §7 binary serialize/deserialize, zero-copy indexed variant |
| `tracker.hpp/cpp` | Top-level §5 orchestrator |

## Common Pitfalls

1. **Never use `-ffast-math`.** It breaks Joseph-form symmetry guarantees (§9). The project uses `-fno-math-errno -fno-trapping-math` only.
2. **C++20 required** for `std::span` in `Tracker::step()` and `serialize_frame*`. The rest is C++17-safe.
3. **86 B vs 90 B wire payload.** The binding is 86 B. Any new per-track field must replace an existing one or extend the protocol version.
4. **η̄₀ = 4, not 0.** Initialising the NIS EMA at zero starves the acceleration channel for ~10 frames.
5. **No heap allocation in the hot path.** All scratch buffers are instance members grown on demand. `EIGEN_RUNTIME_NO_MALLOC` is asserted at tracker boot.
6. **Symmetrise after every P mutation.** Both predict and Joseph update must end with `symmetrize(P)`.
7. **Dt clamping.** Δt is clamped to [5 ms, 300 ms] at the tracker level (`clamp_dt`). Jitter beyond this range indicates a dropped-frame condition.
8. **EmbeddingGallery is ~51 KB per Track** (100 × 128 × 4 B). The wire egress uses `serialize_frame_indexed` to avoid copying full Track aggregates.

## Build & Test

```powershell
cd d:\roc-ai-vision\core\kinematics_engine
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
.\build\roc_kinematics_tests.exe
# Expected: 37 test cases, 503 assertions, ~25 ms
```

## Additional Resources

- Mathematical blueprint: `workspace_blueprint/research_notes/phase_2_ekf_kinematics.md`
- Architecture overview: `workspace_blueprint/architecture.md`
- Roadmap execution log: `workspace_blueprint/roadmap.md` (Phase 2/3 entries)
