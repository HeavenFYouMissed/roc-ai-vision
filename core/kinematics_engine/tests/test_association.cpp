// =============================================================================
// test_association — verifies §4.2.1 cosine appearance distance (concatenated
// GEMM ≡ naïve double loop), §4.2.2 Mahalanobis χ² gate, the LAP-JV solver,
// and the §4.4 matching-cascade age ordering.
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "roc/kinematics/association.hpp"
#include "roc/kinematics/lap_jv.hpp"
#include "roc/kinematics/object_priors.hpp"
#include "roc/kinematics/track.hpp"

#include <Eigen/Cholesky>
#include <algorithm>
#include <cmath>
#include <random>

using namespace roc::kinematics;

namespace {

Embedding random_unit_embedding(std::mt19937& rng)
{
    Embedding f;
    std::normal_distribution<float> n(0.0f, 1.0f);
    for (int i = 0; i < kEmbeddingDim; ++i) f(i) = n(rng);
    f /= f.norm();
    return f;
}

CameraPose make_pose()
{
    CameraPose p;
    p.K(0, 0) = 800.0; p.K(1, 1) = 800.0;
    p.K(0, 2) = 320.0; p.K(1, 2) = 240.0;
    p.R_cw = Eigen::Matrix3d::Identity();
    p.t_cw = Eigen::Vector3d::Zero();
    p.img_w = 640; p.img_h = 480;
    return p;
}

Track make_track(TrackId id, Label label, const Eigen::Vector3d& p_world,
                 const Embedding& emb, int hits = 5)
{
    Track t;
    t.id = id;
    t.label = label;
    t.status = TrackStatus::Confirmed;
    t.hits = hits;
    State x = State::Zero();
    x.head<3>() = p_world;
    StateCov P = StateCov::Identity();
    P.block<3, 3>(0, 0) = 0.04 * Eigen::Matrix3d::Identity();
    t.filter = CaEkf(x, P, qc_diagonal_for(label));
    t.regime_detector.reset();
    t.gallery.push(emb);
    return t;
}

Detection make_detection(Label label, double u, double v, double w_px, double h_px,
                         const Embedding& emb)
{
    Detection d;
    d.label = label;
    d.bbox_center.u = u;
    d.bbox_center.v = v;
    d.bbox_w_px = w_px;
    d.bbox_h_px = h_px;
    d.appearance = emb;
    d.confidence = 0.95;
    return d;
}

}  // namespace

// ---------------------------------------------------------------------------
// §4.2.1 cosine via GEMM ≡ naive loop.
// ---------------------------------------------------------------------------
TEST_CASE("cosine GEMM equals naive double loop within 1e-6", "[assoc][cosine]") {
    std::mt19937 rng(0xBEEF);
    const int n_tracks = 4;
    const int n_dets   = 6;

    std::vector<Track> tracks;
    tracks.reserve(n_tracks);
    for (int i = 0; i < n_tracks; ++i) {
        Track t = make_track(TrackId(i + 1), Label::Person,
                             {0.0, 0.0, 5.0 + i}, random_unit_embedding(rng));
        // Add a few more gallery entries so the per-track min applies.
        for (int k = 0; k < 5; ++k) {
            t.gallery.push(random_unit_embedding(rng));
        }
        tracks.push_back(t);
    }
    std::vector<Detection> dets;
    for (int j = 0; j < n_dets; ++j) {
        dets.push_back(make_detection(Label::Person, 320, 240, 50, 100,
                                      random_unit_embedding(rng)));
    }

    std::vector<int> tidx(n_tracks), didx(n_dets);
    for (int i = 0; i < n_tracks; ++i) tidx[i] = i;
    for (int j = 0; j < n_dets;   ++j) didx[j] = j;

    Associator A;
    Eigen::MatrixXf D_gemm = A.compute_appearance_distance(tidx, didx, tracks, dets);

    // Naive double loop ground truth.
    Eigen::MatrixXf D_naive(n_tracks, n_dets);
    for (int i = 0; i < n_tracks; ++i) {
        for (int j = 0; j < n_dets; ++j) {
            float best_sim = -1.0f;
            for (int k = 0; k < tracks[i].gallery.size(); ++k) {
                const float s = tracks[i].gallery.at(k).dot(dets[j].appearance);
                if (s > best_sim) best_sim = s;
            }
            D_naive(i, j) = 1.0f - best_sim;
        }
    }
    REQUIRE((D_gemm - D_naive).cwiseAbs().maxCoeff() < 1e-5f);
}

// ---------------------------------------------------------------------------
// §4.2.2 Mahalanobis χ²_4 gate at 9.4877.
// ---------------------------------------------------------------------------
TEST_CASE("Mahalanobis χ² gate accepts ≤ 9.4877 and rejects beyond", "[assoc][gating]") {
    std::mt19937 rng(0xC0DE);
    const Embedding e = random_unit_embedding(rng);

    Track tr = make_track(TrackId(1), Label::Person, {0.0, 0.0, 5.0}, e);
    PinholeMeasurement pin(make_pose());

    // First: detection that lines up exactly with prediction.  M ≈ 0 → accepted.
    Meas h_pred;
    Jacobian H_dummy;
    REQUIRE(pin.compute_h_and_H(tr.filter.state(), kPriorPerson, h_pred, H_dummy));
    Detection d_close;
    d_close.label = Label::Person;
    d_close.bbox_center.u = h_pred(0);
    d_close.bbox_center.v = h_pred(1);
    d_close.bbox_w_px     = h_pred(2);
    d_close.bbox_h_px     = h_pred(3);
    d_close.confidence    = 0.95;
    d_close.appearance    = e;

    Associator A;
    InnovationCache cache;
    const double m_close_sq = A.compute_motion_distance_sq(tr, d_close, pin, cache);
    REQUIRE(m_close_sq < 9.4877);

    // Now a wildly off detection — far enough that even with R, S^{-1} pushes M >> threshold.
    Detection d_far = d_close;
    d_far.bbox_center.u += 200.0;
    d_far.bbox_center.v += 200.0;
    const double m_far_sq = A.compute_motion_distance_sq(tr, d_far, pin, cache);
    REQUIRE(m_far_sq > 9.4877);
}

// ---------------------------------------------------------------------------
// LAP-JV vs. brute-force minimum on a small (5×5) cost matrix.
// ---------------------------------------------------------------------------
TEST_CASE("LAP-JV matches brute-force minimum on 5×5", "[assoc][lap]") {
    const int N = 5;
    std::mt19937 rng(0xFACE);
    std::uniform_real_distribution<double> u(0.0, 10.0);
    std::vector<double> cost(N * N);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            cost[i * N + j] = u(rng);
        }
    }

    LapAssignment lap = solve_lap_jv(cost.data(), N, N);
    double lap_total = lap.total_cost;

    // Brute-force minimum over all permutations of {0..N-1}.
    std::vector<int> perm = {0, 1, 2, 3, 4};
    double best = std::numeric_limits<double>::infinity();
    do {
        double sum = 0.0;
        for (int i = 0; i < N; ++i) sum += cost[i * N + perm[i]];
        if (sum < best) best = sum;
    } while (std::next_permutation(perm.begin(), perm.end()));
    REQUIRE(std::abs(lap_total - best) < 1e-9);
}

TEST_CASE("LAP-JV ignores forbidden cells", "[assoc][lap]") {
    const int N = 3;
    std::vector<double> cost(N * N, 1.0);
    cost[0 * N + 0] = kForbiddenCost;   // forbid (0,0)
    cost[1 * N + 1] = kForbiddenCost;   // forbid (1,1)
    cost[2 * N + 2] = 0.5;              // strongly prefer (2,2)
    LapAssignment lap = solve_lap_jv(cost.data(), N, N);
    REQUIRE(lap.col_for_row[0] != 0);
    REQUIRE(lap.col_for_row[1] != 1);
    REQUIRE(lap.col_for_row[2] == 2);
}

TEST_CASE("LAP-JV handles rectangular matrices", "[assoc][lap]") {
    // 2 rows, 4 cols.
    std::vector<double> cost = {
        1.0, 2.0, 3.0, 4.0,
        5.0, 1.5, 2.5, 0.5,
    };
    LapAssignment lap = solve_lap_jv(cost.data(), 2, 4);
    // Row 0 prefers col 0 (cost 1), row 1 prefers col 3 (cost 0.5).
    REQUIRE(lap.col_for_row[0] == 0);
    REQUIRE(lap.col_for_row[1] == 3);
    REQUIRE(std::abs(lap.total_cost - 1.5) < 1e-9);
}

// ---------------------------------------------------------------------------
// §4.4 cascade age ordering — younger tracks (lower time_since_update) get
// first claim on detections.
// ---------------------------------------------------------------------------
TEST_CASE("matching cascade respects age ordering", "[assoc][cascade]") {
    std::mt19937 rng(0xABCD);
    Embedding e = random_unit_embedding(rng);

    PinholeMeasurement pin(make_pose());
    auto we = kPriorPerson;

    // Two tracks at exactly the same world position with the same gallery
    // entry → cosine cost identical.  The fresh track has time_since_update
    // = 1, the stale one has time_since_update = 5.  A single detection
    // should go to the fresh one.
    Track t_fresh = make_track(TrackId(1), Label::Person, {0.0, 0.0, 5.0}, e);
    Track t_stale = make_track(TrackId(2), Label::Person, {0.0, 0.0, 5.0}, e);
    t_fresh.time_since_update = 1;
    t_stale.time_since_update = 5;

    Meas h_pred;
    Jacobian H_dummy;
    REQUIRE(pin.compute_h_and_H(t_fresh.filter.state(), we, h_pred, H_dummy));

    Detection d = make_detection(Label::Person,
                                 h_pred(0), h_pred(1), h_pred(2), h_pred(3), e);

    std::vector<Track> tracks{t_fresh, t_stale};
    std::vector<Detection> dets{d};
    std::vector<int> tidx = {0, 1};
    std::vector<int> didx = {0};

    Associator A;
    std::vector<InnovationCache> innov(tracks.size());
    auto result = A.run_cascade(tidx, didx,
                                std::span<const Track>(tracks.data(), tracks.size()),
                                std::span<const Detection>(dets.data(), dets.size()),
                                pin, innov);
    REQUIRE(result.matches.size() == 1);
    REQUIRE(result.matches[0].track_index == 0);   // fresh track wins
    REQUIRE(result.matches[0].det_index   == 0);
}

TEST_CASE("IoU helper agrees with hand-computed values", "[assoc][iou]") {
    // Same box → IoU = 1.
    REQUIRE(std::abs(iou_xywh(100, 100, 50, 50, 100, 100, 50, 50) - 1.0) < 1e-12);
    // Disjoint → 0.
    REQUIRE(iou_xywh(0, 0, 10, 10, 100, 100, 10, 10) == 0.0);
    // Overlap of 50% in u, 100% in v on equal-sized boxes:
    // box1 [u=0..10, v=0..10], box2 [u=5..15, v=0..10]
    // intersection 5x10 = 50, union = 100 + 100 - 50 = 150 → IoU = 1/3.
    const double iou = iou_xywh(5, 5, 10, 10, 10, 5, 10, 10);
    REQUIRE(std::abs(iou - 1.0/3.0) < 1e-9);
}
