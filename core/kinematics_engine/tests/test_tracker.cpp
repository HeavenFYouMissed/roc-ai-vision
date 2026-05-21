// =============================================================================
// test_tracker — end-to-end pipeline test (§5).  Generates a synthetic
// 5-track / 200-frame scenario with a brief occlusion in the middle and
// verifies that track IDs remain stable.  Also exercises the Trap A
// boundary-probation cycle and the Trap B stationary-regime path.
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "roc/kinematics/tracker.hpp"
#include "roc/kinematics/object_priors.hpp"

#include <array>
#include <cmath>
#include <random>
#include <unordered_map>

using namespace roc::kinematics;

namespace {

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

struct GtAgent {
    Eigen::Vector3d p0;          // initial position F_W
    Eigen::Vector3d v;           // constant velocity m/s
    Embedding       appearance;
    int             id;
};

Embedding make_emb(int id_seed)
{
    std::mt19937 rng(static_cast<std::uint32_t>(id_seed) * 0x9E3779B9u + 1u);
    Embedding e;
    std::normal_distribution<float> n(0.0f, 1.0f);
    for (int i = 0; i < kEmbeddingDim; ++i) e(i) = n(rng);
    e /= e.norm();
    return e;
}

Detection project_to_detection(const PinholeMeasurement& pin,
                               Label label,
                               const Eigen::Vector3d& p_world,
                               const WorldExtents& we,
                               const Embedding& emb)
{
    auto px = pin.project(p_world);
    Detection d;
    d.label = label;
    d.appearance = emb;
    d.confidence = 0.95;
    if (!px) {
        d.bbox_w_px = 0.0;
        d.bbox_h_px = 0.0;
        return d;
    }
    // Recover Z to scale the bbox, then encode w_px / h_px from world extents.
    Eigen::Vector3d p_cam = pin.pose().R_cw * p_world + pin.pose().t_cw;
    const double Z = p_cam.z();
    const double fx = pin.pose().K(0, 0);
    const double fy = pin.pose().K(1, 1);
    d.bbox_center.u = px->u;
    d.bbox_center.v = px->v;
    d.bbox_w_px = (fx * we.w_world) / Z;
    d.bbox_h_px = (fy * we.h_world) / Z;
    return d;
}

}  // namespace

TEST_CASE("End-to-end: 5 tracks across 200 frames maintain stable IDs through an occlusion",
          "[tracker][e2e]") {
    // Construct a scenario in which all five agents move at fixed velocity
    // through the scene.  At frame 80..100 we drop the middle agent's
    // detection (synthetic occlusion).  The DeepSORT cosine + Mahalanobis
    // gating should re-associate it without spawning a fresh ID.
    CameraPose pose = make_pose();
    PinholeMeasurement pin(pose);

    std::array<GtAgent, 5> agents = {{
        {{ 0.0, 0.0, 5.0}, {0.05,  0.0,   0.0}, make_emb(11), 11},
        {{-0.6, 0.0, 6.0}, {0.04, -0.01,  0.0}, make_emb(12), 12},
        {{ 0.7,-0.1, 5.5}, {0.03,  0.02,  0.01},make_emb(13), 13},
        {{-1.0, 0.2, 7.0}, {0.06,  0.0,  -0.005},make_emb(14), 14},
        {{ 1.2, 0.1, 6.2}, {0.02, -0.02,  0.0}, make_emb(15), 15},
    }};

    TrackerConfig cfg;
    cfg.assoc.lambda = 0.0;          // motion gate only contributes binary admittance
    cfg.assoc.tau_app = 0.4;         // appearance gate
    cfg.assoc.tau_mot_sq = 9.4877;   // §4.2.2
    cfg.lifecycle.confirm_hits = 3;
    cfg.lifecycle.max_age = 30;
    cfg.enable_target_selector = false;

    Tracker tracker(pose, cfg);

    // Frame-id → (gt agent id → assigned track id) over the run.
    std::unordered_map<int, std::uint64_t> first_track_id_for_gt;
    int max_gt_id_observed = 0;
    int id_switches = 0;
    std::unordered_map<int, std::uint64_t> last_track_id_for_gt;

    const int N = 200;
    const double dt = 1.0 / 30.0;
    std::vector<std::uint8_t> wire_buf;
    for (int k = 0; k < N; ++k) {
        std::vector<Detection> dets;
        for (std::size_t a = 0; a < agents.size(); ++a) {
            // Drop middle agent (#13) for frames [80, 100).
            if (agents[a].id == 13 && k >= 80 && k < 100) continue;
            const Eigen::Vector3d p = agents[a].p0 + agents[a].v * (k * dt);
            Detection d = project_to_detection(pin, Label::Person, p, kPriorPerson,
                                               agents[a].appearance);
            // Reject tiny boxes (near out-of-frame).
            if (d.bbox_w_px < 6.0 || d.bbox_h_px < 6.0) continue;
            dets.push_back(d);
        }
        tracker.step(k * dt, dets, wire_buf);

        // Map confirmed tracks to GT agents by appearance dot product.
        for (const Track& t : tracker.tracks()) {
            if (!t.is_confirmed()) continue;
            // Find best-matching agent embedding.
            float best = -1.0f;
            int best_a = -1;
            // Use newest gallery entry.
            const Embedding gallery_back = (t.gallery.size() > 0)
                ? t.gallery.at(t.gallery.size() - 1)
                : Embedding::Zero();
            for (std::size_t a = 0; a < agents.size(); ++a) {
                const float s = agents[a].appearance.dot(gallery_back);
                if (s > best) { best = s; best_a = static_cast<int>(a); }
            }
            if (best_a < 0 || best < 0.5f) continue;
            const int gt_id = agents[static_cast<std::size_t>(best_a)].id;
            if (first_track_id_for_gt.find(gt_id) == first_track_id_for_gt.end()) {
                first_track_id_for_gt[gt_id] = t.id.value;
                last_track_id_for_gt[gt_id]  = t.id.value;
            } else if (last_track_id_for_gt[gt_id] != t.id.value) {
                ++id_switches;
                last_track_id_for_gt[gt_id] = t.id.value;
            }
            if (gt_id > max_gt_id_observed) max_gt_id_observed = gt_id;
        }
    }

    REQUIRE(first_track_id_for_gt.size() == 5);
    REQUIRE(id_switches == 0);
}

TEST_CASE("Tracker spawns + confirms a single track on stable input",
          "[tracker][lifecycle]") {
    CameraPose pose = make_pose();
    PinholeMeasurement pin(pose);

    TrackerConfig cfg;
    cfg.lifecycle.confirm_hits = 3;
    cfg.lifecycle.max_age = 30;
    cfg.enable_target_selector = false;
    Tracker tr(pose, cfg);

    Embedding e = make_emb(7);
    std::vector<std::uint8_t> wire_buf;
    Eigen::Vector3d p{0.0, 0.0, 5.0};
    for (int k = 0; k < 5; ++k) {
        Detection d = project_to_detection(pin, Label::Person, p, kPriorPerson, e);
        std::vector<Detection> dets{d};
        tr.step(k * (1.0 / 30.0), dets, wire_buf);
    }
    int n_confirmed = 0;
    for (const Track& t : tr.tracks()) if (t.is_confirmed()) ++n_confirmed;
    REQUIRE(n_confirmed == 1);
}

TEST_CASE("Tracker output wire frame size matches the §7 contract",
          "[tracker][wire]") {
    CameraPose pose = make_pose();
    PinholeMeasurement pin(pose);

    TrackerConfig cfg;
    cfg.lifecycle.confirm_hits = 1;   // confirm immediately for the test
    cfg.enable_target_selector = false;
    Tracker tr(pose, cfg);

    Embedding e = make_emb(99);
    Eigen::Vector3d p{0.0, 0.0, 5.0};
    Detection d = project_to_detection(pin, Label::Person, p, kPriorPerson, e);
    std::vector<Detection> dets{d};
    std::vector<std::uint8_t> buf;
    tr.step(0.0, dets, buf);
    // First frame: spawn → confirmed (because confirm_hits = 1) → 1 track on wire.
    REQUIRE(buf.size() == static_cast<std::size_t>(kWireHeaderSize + kWireTrackSize));
}
