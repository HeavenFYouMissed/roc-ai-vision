// =============================================================================
// roc::kinematics::lap_jv — rectangular dense linear assignment.
//
// We implement the **shortest-augmenting-path** variant of the Hungarian /
// Kuhn–Munkres algorithm (sometimes referred to as the JV-SAP form, since
// Jonker & Volgenant 1987 §3 describes the same column-potential update
// rule).  The algorithm maintains row potentials u[i] and column potentials
// v[j] such that
//     cost[i][j] - u[i] - v[j] >= 0 for all (i, j),
// with equality on matched edges.  Each outer iteration picks one
// unassigned row and runs Dijkstra on the bipartite reduced-cost graph to
// find the shortest augmenting path; potentials are then updated using the
// Dijkstra distances and the path is augmented.  The implementation runs
// in O(N · max(N, M)²) which at the design ceiling (max ≈ 256) is well
// inside the per-frame budget; on the typical cascade slice of 30×30 it
// completes in single-digit microseconds.
//
// Tie-breaking: column scan order is left-to-right (§4.8), so when two
// columns share the same reduced cost the lower-indexed column wins.  This
// guarantees deterministic output for identical input regardless of the
// floating-point representation of cost values that compare equal.
//
// The Jonker–Volgenant reference is satisfied by the column-potential
// update logic; the rectangular case (N != M) is handled by padding the
// shorter dimension with kForbiddenCost cells which the post-processing
// pass discards.
// =============================================================================

#include "roc/kinematics/lap_jv.hpp"

#include <algorithm>
#include <limits>
#include <vector>

namespace roc::kinematics {

namespace {

constexpr double kPosInf = std::numeric_limits<double>::infinity();

// Square Hungarian — n × n cost matrix, returns a complete assignment.
// (row_assignment[i] = column matched to row i.)  Cost cells of value
// kForbiddenCost are honored as "do-not-match" — they only "win" when no
// other admissible cell exists; the caller filters those out.
void hungarian_square(const std::vector<double>& cost,
                      int n,
                      std::vector<int>& row_assignment,
                      std::vector<int>& col_assignment)
{
    // 1-indexed buffers per the e-maxx Hungarian template; index 0 is the
    // dummy row used to anchor the predecessor chain.
    std::vector<double> u(static_cast<std::size_t>(n) + 1, 0.0);
    std::vector<double> v(static_cast<std::size_t>(n) + 1, 0.0);
    std::vector<int>    p(static_cast<std::size_t>(n) + 1, 0);   // p[j] = row matched to col j
    std::vector<int>    way(static_cast<std::size_t>(n) + 1, 0); // backtrack: way[j] = predecessor col
    std::vector<double> minv(static_cast<std::size_t>(n) + 1, kPosInf);
    std::vector<char>   used(static_cast<std::size_t>(n) + 1, 0);

    for (int i = 1; i <= n; ++i) {
        p[0] = i;
        int j0 = 0;
        std::fill(minv.begin(), minv.end(), kPosInf);
        std::fill(used.begin(), used.end(), 0);

        // Dijkstra inner loop — terminates when an unassigned column is
        // reached (p[j0] == 0 after relaxation).
        do {
            used[static_cast<std::size_t>(j0)] = 1;
            const int i0 = p[static_cast<std::size_t>(j0)];
            double delta = kPosInf;
            int j1 = -1;
            for (int j = 1; j <= n; ++j) {
                if (used[static_cast<std::size_t>(j)]) continue;
                const double cur =
                    cost[static_cast<std::size_t>(i0 - 1) * n + (j - 1)]
                    - u[static_cast<std::size_t>(i0)]
                    - v[static_cast<std::size_t>(j)];
                if (cur < minv[static_cast<std::size_t>(j)]) {
                    minv[static_cast<std::size_t>(j)] = cur;
                    way[static_cast<std::size_t>(j)]  = j0;
                }
                if (minv[static_cast<std::size_t>(j)] < delta) {
                    delta = minv[static_cast<std::size_t>(j)];
                    j1 = j;
                }
            }
            // delta should be finite because the cost matrix is square and
            // padded with kForbiddenCost cells (a finite, large number).

            for (int j = 0; j <= n; ++j) {
                if (used[static_cast<std::size_t>(j)]) {
                    u[static_cast<std::size_t>(p[static_cast<std::size_t>(j)])] += delta;
                    v[static_cast<std::size_t>(j)]                              -= delta;
                } else {
                    minv[static_cast<std::size_t>(j)] -= delta;
                }
            }
            j0 = j1;
        } while (p[static_cast<std::size_t>(j0)] != 0);

        // Augment along the predecessor chain.
        do {
            const int j1 = way[static_cast<std::size_t>(j0)];
            p[static_cast<std::size_t>(j0)] = p[static_cast<std::size_t>(j1)];
            j0 = j1;
        } while (j0 != 0);
    }

    // Translate to 0-indexed output assignments.
    row_assignment.assign(static_cast<std::size_t>(n), -1);
    col_assignment.assign(static_cast<std::size_t>(n), -1);
    for (int j = 1; j <= n; ++j) {
        const int i = p[static_cast<std::size_t>(j)];
        if (i > 0) {
            row_assignment[static_cast<std::size_t>(i - 1)] = j - 1;
            col_assignment[static_cast<std::size_t>(j - 1)] = i - 1;
        }
    }
}

}  // namespace

LapAssignment solve_lap_jv(const double* cost_in, int N, int M)
{
    LapAssignment out;
    out.col_for_row.assign(static_cast<std::size_t>(N), -1);
    out.row_for_col.assign(static_cast<std::size_t>(M), -1);
    out.total_cost = 0.0;
    if (N == 0 || M == 0) return out;

    // Pad to square n×n with kForbiddenCost on the phantom rows/cols.
    const int n = std::max(N, M);
    std::vector<double> cost(static_cast<std::size_t>(n) * n, kForbiddenCost);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < M; ++j) {
            const double c = cost_in[static_cast<std::size_t>(i) * M + j];
            // Defensively clamp +inf inputs to the sentinel — the algorithm
            // requires finite values to do dual-potential arithmetic.
            cost[static_cast<std::size_t>(i) * n + j] =
                (c >= kForbiddenCost) ? kForbiddenCost : c;
        }
    }

    std::vector<int> row_assignment;
    std::vector<int> col_assignment;
    hungarian_square(cost, n, row_assignment, col_assignment);

    // Filter the padded / sentinel-cost cells out of the final answer.
    double total = 0.0;
    for (int i = 0; i < N; ++i) {
        const int j = row_assignment[static_cast<std::size_t>(i)];
        if (j < 0 || j >= M) continue;
        const double c = cost_in[static_cast<std::size_t>(i) * M + j];
        if (c >= kForbiddenCost) continue;
        out.col_for_row[static_cast<std::size_t>(i)] = j;
        out.row_for_col[static_cast<std::size_t>(j)] = i;
        total += c;
    }
    out.total_cost = total;
    return out;
}

}  // namespace roc::kinematics
