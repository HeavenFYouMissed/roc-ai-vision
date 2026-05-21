#pragma once

// =============================================================================
// roc::kinematics::lap_jv
//
// Jonker–Volgenant rectangular dense linear-assignment solver (§4.5).
//
// References:
//   Jonker R., Volgenant A. — "A Shortest Augmenting Path Algorithm for
//   Dense and Sparse Linear Assignment Problems", Computing 38, 325–340
//   (1987).
//
// The implementation supports rectangular cost matrices (N rows × M cols)
// directly: when N ≠ M we pad to a square (max(N,M)) with the sentinel cost
// kForbiddenCost so that any "phantom" assignment is filtered out by the
// caller after solving.
//
// **Hard contract (§4.5 / §4.8):**
//   - Sentinel "forbidden" cost is 1e6 — *never* +inf, which breaks the
//     reduced-cost arithmetic.
//   - Row/column scan order is row-major over the original (track id, det id)
//     ordering so identical input yields identical assignment, regardless of
//     cost-tie position (§4.8 determinism).
// =============================================================================

#include <cstdint>
#include <utility>
#include <vector>

namespace roc::kinematics {

// All cost arithmetic is double — the inner products of cosine + Mahalanobis
// terms have plenty of dynamic range and double matches the rest of the
// kinematic stack.
inline constexpr double kForbiddenCost = 1.0e6;

struct LapAssignment {
    // For each row i (0..N-1), col_for_row[i] is the matched column or -1
    // if the row was paired only against a phantom (i.e., unmatched).
    std::vector<int>    col_for_row;
    // Symmetric: for each column j (0..M-1), row_for_col[j] is the matched
    // row or -1 if the column had no real partner.
    std::vector<int>    row_for_col;
    // Sum of the real (non-sentinel) matched costs.
    double              total_cost = 0.0;
};

// Solve the rectangular linear assignment problem on `cost` (size N × M,
// row-major).  N rows, M columns.  Returns assignments + total cost.
//
// Cells with cost ≥ kForbiddenCost are treated as forbidden — any row
// whose only "match" is a forbidden / phantom cell is reported with col=-1.
LapAssignment solve_lap_jv(const double* cost,
                           int N,
                           int M);

}  // namespace roc::kinematics
