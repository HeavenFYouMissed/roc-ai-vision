// =============================================================================
// roc::kinematics::tracker — top-level orchestrator (§5).
// Implements the per-frame pipeline in the EXACT order:
//   Prediction → Association (label-grouped) → Update → Stationary shrinkage
//   → Hierarchical constraint → Boundary-probation maintenance → Lifecycle
//   → Egress.
// =============================================================================

#include "roc/kinematics/tracker.hpp"
#include "roc/kinematics/object_priors.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace roc::kinematics {

namespace {

// Number of label enum slots (Unknown..AccessoryBackpack), matches the
// cardinality of `Label`.  Used to size the bucket scratch arrays.
constexpr int kNumLabelBuckets = 6;

// Group indices by label so the cascade per-§4.4 runs only on same-label
// detection ↔ track pairs.  Phase-5: bucket destination is supplied by the
// caller as a stable scratch buffer; we just reset and refill it.
template <class T, class GetLabel>
void bucket_by_label(const T& items,
                     GetLabel get_label,
                     std::vector<std::vector<int>>& out_buckets)
{
    if (out_buckets.size() != kNumLabelBuckets) {
        out_buckets.assign(kNumLabelBuckets, std::vector<int>{});
    } else {
        for (auto& b : out_buckets) b.clear();
    }
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        const auto lbl = get_label(items[static_cast<std::size_t>(i)]);
        const auto idx = static_cast<std::size_t>(lbl);
        if (idx >= out_buckets.size()) continue;
        out_buckets[idx].push_back(i);
    }
}

}  // namespace

Tracker::Tracker(CameraPose pose, TrackerConfig cfg) noexcept
    : camera_pose_(pose),
      pinhole_(pose),
      cfg_(cfg),
      associator_(cfg.assoc),
      hierarchy_(),
      pending_spawn_(),
      selector_(cfg.selector_cx, cfg.selector_cy, cfg.selector_weights)
{
}

void Tracker::set_camera_pose(const CameraPose& pose) noexcept
{
    camera_pose_ = pose;
    pinhole_.set_pose(pose);
}

void Tracker::set_config(const TrackerConfig& cfg) noexcept
{
    cfg_ = cfg;
    associator_.set_config(cfg.assoc);
    selector_.set_reference(cfg.selector_cx, cfg.selector_cy);
    selector_.set_weights(cfg.selector_weights);
}

void Tracker::reset()
{
    tracks_.clear();
    pending_spawn_.clear();
    selector_.clear_state();
    next_track_id_ = 1;
    frame_counter_ = 0;
    last_t_seconds_ = 0.0;
    have_last_t_ = false;
    session_epoch_seconds_ = 0.0;
    stats_ = FrameStats{};
}

int Tracker::find_track_index_by_id(TrackId id) const noexcept
{
    for (int i = 0; i < static_cast<int>(tracks_.size()); ++i) {
        if (tracks_[static_cast<std::size_t>(i)].id == id) return i;
    }
    return -1;
}

void Tracker::predict_all(double dt)
{
    for (auto& t : tracks_) {
        t.predict(dt);
    }
}

void Tracker::update_matched(const std::vector<Match>& matches,
                             std::span<const Detection> detections,
                             std::vector<InnovationCache>& innov_cache)
{
    for (const Match& m : matches) {
        Track& t = tracks_[static_cast<std::size_t>(m.track_index)];
        const Detection& d = detections[static_cast<std::size_t>(m.det_index)];

        Meas z;
        z << d.bbox_center.u, d.bbox_center.v, d.bbox_w_px, d.bbox_h_px;

        // Determine whether to drop the box-dim rows (§3.10.4 / §3.9).
        const bool dim_too_small = (d.bbox_w_px < kBboxDimMin) ||
                                   (d.bbox_h_px < kBboxDimMin);
        const bool centroid_only = t.is_boundary_probation || dim_too_small;

        // Fetch the cached (h, H) from the gating pass; if absent, recompute.
        InnovationCache& cache = innov_cache[static_cast<std::size_t>(m.track_index)];
        if (!cache.valid) {
            const auto we = world_extents_for(t.label);
            Meas h_pred;
            Jacobian H;
            if (!pinhole_.compute_h_and_H(t.filter.state(), we, h_pred, H)) {
                // §3.9 — degenerate Z; skip update entirely.
                t.last_confidence = d.confidence;
                t.last_bbox_u = d.bbox_center.u;
                t.last_bbox_v = d.bbox_center.v;
                t.last_bbox_w = d.bbox_w_px;
                t.last_bbox_h = d.bbox_h_px;
                t.gallery.push(d.appearance);
                t.on_match();
                continue;
            }
            cache.h = h_pred;
            cache.H = H;
            cache.valid = true;
        }

        Meas nu = z - cache.h;
        MeasCov R = default_meas_cov(d.confidence);

        // §2.6.4 centroid-deadband when in STATIONARY.
        if (t.regime_detector.regime() == Regime::Stationary) {
            if (std::max(std::abs(nu(0)), std::abs(nu(1))) < cfg_.deadband_uv_px) {
                nu(0) = 0.0;
                nu(1) = 0.0;
            }
        }

        double nis = -1.0;
        if (centroid_only) {
            Meas2 nu_uv;
            nu_uv << nu(0), nu(1);
            Jacobian2 H_uv = cache.H.template topRows<2>();
            MeasCov2 R_uv = R.topLeftCorner<2, 2>();
            nis = t.filter.update_centroid_only(nu_uv, H_uv, R_uv);
        } else {
            nis = t.filter.update_full(nu, cache.H, R);
        }
        if (nis >= 0.0) {
            t.regime_detector.update(nis);
        }

        t.last_confidence = d.confidence;
        t.last_bbox_u = d.bbox_center.u;
        t.last_bbox_v = d.bbox_center.v;
        t.last_bbox_w = d.bbox_w_px;
        t.last_bbox_h = d.bbox_h_px;
        t.gallery.push(d.appearance);
        t.on_match();
    }
}

void Tracker::apply_stationary_shrinkage()
{
    // §2.6.3 every K=5 frames inside STATIONARY.
    const int K = static_cast<int>(cfg_.stationary_shrinkage_period_frames);
    for (auto& t : tracks_) {
        if (!t.is_confirmed()) continue;
        if (t.regime_detector.regime() != Regime::Stationary) continue;
        if (K <= 0) continue;
        if (t.frames_since_spawn % K != 0) continue;
        t.filter.update_zero_velocity_shrinkage(cfg_.sigma_v_stationary,
                                                 cfg_.sigma_a_stationary);
    }
}

void Tracker::maintain_boundary_probation(std::span<const Detection> detections,
                                          const std::vector<Match>& matches)
{
    // §3.10.4 — the probation flag clears after THREE consecutive in-frame
    // detections (with the kEpsilonBoundaryPx margin).
    const double img_w = static_cast<double>(camera_pose_.img_w);
    const double img_h = static_cast<double>(camera_pose_.img_h);

    // Build a map track_index → matched detection.
    std::vector<int> det_for_track(tracks_.size(), -1);
    for (const Match& m : matches) {
        det_for_track[static_cast<std::size_t>(m.track_index)] = m.det_index;
    }

    for (int i = 0; i < static_cast<int>(tracks_.size()); ++i) {
        Track& t = tracks_[static_cast<std::size_t>(i)];
        if (!t.is_boundary_probation) continue;
        const int dj = det_for_track[static_cast<std::size_t>(i)];
        if (dj < 0) {
            t.consecutive_in_frame_detections = 0;
            continue;
        }
        const Detection& d = detections[static_cast<std::size_t>(dj)];
        const bool x_in =
            (img_w <= 0.0) ||
            (d.left()  > PinholeMeasurement::kEpsilonBoundaryPx &&
             d.right() < img_w - PinholeMeasurement::kEpsilonBoundaryPx);
        const bool y_in =
            (img_h <= 0.0) ||
            (d.top()    > PinholeMeasurement::kEpsilonBoundaryPx &&
             d.bottom() < img_h - PinholeMeasurement::kEpsilonBoundaryPx);

        if (x_in && y_in) {
            ++t.consecutive_in_frame_detections;
            if (t.consecutive_in_frame_detections >= 3) {
                t.is_boundary_probation = false;
                t.consecutive_in_frame_detections = 0;
            }
        } else {
            t.consecutive_in_frame_detections = 0;
        }
    }
}

Track* Tracker::maybe_find_parent_for(const Detection& det,
                                      const InitResult& init_res)
{
    (void)init_res;
    if (!is_subfeature_label(det.label)) return nullptr;

    // Find the closest confirmed Person whose bbox contains the
    // sub-feature's centroid (cheap and deterministic for the §3.11
    // hierarchical constraint).  We do NOT match by appearance here — the
    // wire frame's parent_id is a topological pointer, not a re-id key.
    Track* best = nullptr;
    double best_dist = std::numeric_limits<double>::infinity();
    for (Track& parent : tracks_) {
        if (!parent.is_confirmed()) continue;
        if (parent.label != Label::Person) continue;
        const double half_w = 0.5 * parent.last_bbox_w;
        const double half_h = 0.5 * parent.last_bbox_h;
        const double du = det.bbox_center.u - parent.last_bbox_u;
        const double dv = det.bbox_center.v - parent.last_bbox_v;
        if (std::abs(du) > half_w || std::abs(dv) > half_h) continue;
        const double d2 = du * du + dv * dv;
        if (d2 < best_dist) {
            best_dist = d2;
            best = &parent;
        }
    }
    return best;
}

void Tracker::spawn_track(const Detection& det, const InitResult& init_res)
{
    const auto qc = qc_diagonal_for(det.label);

    Track t;
    t.id = TrackId(next_track_id_++);
    t.label = det.label;
    t.filter = CaEkf(init_res.x0, init_res.P0, qc);
    t.regime_detector.reset();
    t.gallery.push(det.appearance);
    t.last_bbox_u = det.bbox_center.u;
    t.last_bbox_v = det.bbox_center.v;
    t.last_bbox_w = det.bbox_w_px;
    t.last_bbox_h = det.bbox_h_px;
    t.last_confidence = det.confidence;
    t.is_boundary_probation = init_res.boundary_probation;
    if (init_res.boundary_probation) {
        ++t.boundary_init_count;
    }
    t.hits = 1;
    t.age = 1;
    t.frames_since_spawn = 1;
    t.time_since_update = 0;
    t.status = (cfg_.lifecycle.confirm_hits <= 1) ? TrackStatus::Confirmed
                                                  : TrackStatus::Tentative;

    Track* parent = maybe_find_parent_for(det, init_res);
    if (parent) {
        t.parent_id = parent->id;
    }

    tracks_.push_back(std::move(t));
}

void Tracker::run_lifecycle(const std::vector<int>& unmatched_dets,
                            std::span<const Detection> detections)
{
    // ---- Spawn / classify each unmatched detection per §3.10.3 ----
    for (int j : unmatched_dets) {
        const Detection& d = detections[static_cast<std::size_t>(j)];
        const auto we = world_extents_for(d.label);

        // First check the pending-spawn buffer for a re-acquired DEFERRED detection.
        const int pending_match = pending_spawn_.find_match(d);
        if (pending_match >= 0) {
            // Use this detection as the second observation; keep the original
            // appearance/world-extents for spawn but try to obtain a proper
            // §3.10.3 init from THIS detection (whose box-dim presumably now
            // satisfies the gate — that's why we're here).
            InitFailureReason reason{};
            auto init = pinhole_.initialize_from_detection(d, we, &reason);
            if (init.has_value()) {
                spawn_track(d, *init);
                pending_spawn_.erase(pending_match);
                ++stats_.num_promoted_from_pending;
                ++stats_.num_spawned;
                continue;
            }
            // Otherwise still deferred — leave the slot in the buffer.
        }

        InitFailureReason reason{};
        auto init = pinhole_.initialize_from_detection(d, we, &reason);
        if (init.has_value()) {
            spawn_track(d, *init);
            ++stats_.num_spawned;
        } else if (reason == InitFailureReason::BothAxesClipped ||
                   reason == InitFailureReason::DepthDisagreement) {
            pending_spawn_.insert(d);
            ++stats_.num_deferred_to_pending;
        }
        // Box-dim-too-small / degenerate detections are silently dropped.
    }

    // ---- Tick pending-spawn buffer ----
    const int before = pending_spawn_.size();
    pending_spawn_.tick();
    const int after  = pending_spawn_.size();
    if (after < before) {
        stats_.num_pending_dropped_age += (before - after);
    }
}

void Tracker::prune_dead()
{
    // Promote tentative → confirmed.
    for (auto& t : tracks_) {
        const TrackStatus prev = t.status;
        t.promote_if_eligible(cfg_.lifecycle);
        if (prev != TrackStatus::Confirmed && t.status == TrackStatus::Confirmed) {
            ++stats_.num_promoted;
        }
    }
    // Delete tracks past max_age OR whose parent_id is dangling.
    const int max_age = cfg_.lifecycle.max_age;
    auto end = std::remove_if(tracks_.begin(), tracks_.end(), [&](Track& t) {
        if (t.time_since_update > max_age) {
            ++stats_.num_deleted;
            return true;
        }
        return false;
    });
    if (end != tracks_.end()) {
        tracks_.erase(end, tracks_.end());
        // Now scan for orphan sub-features whose parent disappeared.
        for (auto& t : tracks_) {
            if (!t.parent_id.has_value()) continue;
            if (find_track_index_by_id(*t.parent_id) < 0) {
                t.parent_id.reset();
            }
        }
    }
    stats_.num_tracks_confirmed = 0;
    for (const auto& t : tracks_) {
        if (t.is_confirmed()) ++stats_.num_tracks_confirmed;
    }
}

void Tracker::update_target_selector()
{
    if (!cfg_.enable_target_selector) return;
    selector_.select(std::span<const Track>(tracks_.data(), tracks_.size()));
}

void Tracker::emit_wire_frame(std::vector<std::uint8_t>& out, std::uint32_t frame_id)
{
    // Phase-5: pick the confirmed-track indices into a scratch vector instead
    // of copying entire Track aggregates (each Track carries a ~51 KB embedding
    // gallery — copying them per frame for serialisation alone is wasteful).
    wire_track_indices_scratch_.clear();
    wire_track_indices_scratch_.reserve(tracks_.size());
    bool boundary_present = false;
    bool maneuver_present = false;
    for (int i = 0; i < static_cast<int>(tracks_.size()); ++i) {
        const Track& t = tracks_[static_cast<std::size_t>(i)];
        if (!t.is_confirmed()) continue;
        wire_track_indices_scratch_.push_back(i);
        if (t.is_boundary_probation) boundary_present = true;
        if (t.regime_detector.regime() == Regime::Maneuver) maneuver_present = true;
    }

    FrameHeader h;
    h.version    = kWireProtocolVersion;
    h.frame_id   = frame_id;
    h.num_tracks = static_cast<std::uint16_t>(wire_track_indices_scratch_.size());
    h.flags      = 0;
    if (boundary_present) h.flags |= wire_frame_flag_bits::kBoundaryPresent;
    if (maneuver_present) h.flags |= wire_frame_flag_bits::kNisDegraded;
    h.t_capture_ms_offset = static_cast<std::int32_t>(
        std::lround((last_t_seconds_ - session_epoch_seconds_) * 1000.0));

    const std::size_t need = static_cast<std::size_t>(kWireHeaderSize)
        + wire_track_indices_scratch_.size() * static_cast<std::size_t>(kWireTrackSize);
    out.assign(need, 0);
    serialize_frame_indexed(
        std::span<std::uint8_t>(out.data(), out.size()), h,
        std::span<const Track>(tracks_.data(), tracks_.size()),
        std::span<const int>(wire_track_indices_scratch_.data(),
                             wire_track_indices_scratch_.size()));
}

std::size_t Tracker::step(double t_now_seconds,
                          std::span<const Detection> detections,
                          std::vector<std::uint8_t>& out_wire_frame)
{
    stats_ = FrameStats{};

    // §0 — Δt comes from a monotonic clock at the caller; we still clamp.
    double dt = 0.0;
    if (have_last_t_) {
        dt = t_now_seconds - last_t_seconds_;
    } else {
        dt = 1.0 / 30.0;   // bootstrap with the nominal 30 Hz cadence
        session_epoch_seconds_ = t_now_seconds;
        have_last_t_ = true;
    }
    dt = clamp_dt(dt);
    last_t_seconds_ = t_now_seconds;
    stats_.dt_used = dt;

    // ── Phase 1: Prediction ───────────────────────────────────────────
    predict_all(dt);
    stats_.num_tracks_predicted = static_cast<int>(tracks_.size());

    // ── Phase 2: Association (per-label cascade + IoU fallback) ───────
    bucket_by_label(tracks_, [](const Track& t) { return t.label; },
                    bucket_tracks_scratch_);
    bucket_by_label(detections, [](const Detection& d) { return d.label; },
                    bucket_dets_scratch_);

    matches_scratch_.clear();
    unmatched_tracks_scratch_.clear();
    unmatched_dets_scratch_.clear();
    innov_cache_scratch_.assign(tracks_.size(), InnovationCache{});

    for (std::size_t lbl = 1; lbl < bucket_tracks_scratch_.size(); ++lbl) {
        const auto& tb = bucket_tracks_scratch_[lbl];
        const auto& db = bucket_dets_scratch_[lbl];
        if (tb.empty() && db.empty()) continue;

        innov_label_scratch_.assign(tracks_.size(), InnovationCache{});
        AssociationResult cas = associator_.run_cascade(
            tb, db,
            std::span<const Track>(tracks_.data(), tracks_.size()),
            detections, pinhole_, innov_label_scratch_);

        // Merge cascade results into the per-frame accumulators.
        for (const Match& m : cas.matches) {
            matches_scratch_.push_back(m);
            innov_cache_scratch_[static_cast<std::size_t>(m.track_index)] =
                innov_label_scratch_[static_cast<std::size_t>(m.track_index)];
        }
        // IoU fallback over (tentative tracks of this label) ∪ (cascade-unmatched confirmed tracks).
        iou_tracks_scratch_.clear();
        for (int idx : tb) {
            const Track& t = tracks_[static_cast<std::size_t>(idx)];
            if (!t.is_confirmed()) iou_tracks_scratch_.push_back(idx);
        }
        iou_tracks_scratch_.insert(iou_tracks_scratch_.end(),
                                   cas.unmatched_tracks.begin(),
                                   cas.unmatched_tracks.end());

        AssociationResult iou_res = associator_.run_iou_fallback(
            iou_tracks_scratch_, cas.unmatched_dets,
            std::span<const Track>(tracks_.data(), tracks_.size()),
            detections);

        for (const Match& m : iou_res.matches) {
            matches_scratch_.push_back(m);
            ++stats_.num_iou_matches;
        }
        for (int idx : iou_res.unmatched_tracks) unmatched_tracks_scratch_.push_back(idx);
        for (int j   : iou_res.unmatched_dets)   unmatched_dets_scratch_.push_back(j);
    }

    stats_.num_matches            = static_cast<int>(matches_scratch_.size());
    stats_.num_unmatched_tracks   = static_cast<int>(unmatched_tracks_scratch_.size());
    stats_.num_unmatched_dets     = static_cast<int>(unmatched_dets_scratch_.size());

    // ── Phase 3: Update ───────────────────────────────────────────────
    update_matched(matches_scratch_, detections, innov_cache_scratch_);

    // ── Phase 4: Stationary-regime shrinkage (every K frames) ─────────
    apply_stationary_shrinkage();

    // ── Phase 5: Hierarchical constraint ──────────────────────────────
    {
        int   pair_count = 0;
        int   violation_count = 0;
        hierarchy_.set_event_callback([&](TrackId, TrackId) {
            ++violation_count;
        });
        for (auto& t : tracks_) {
            if (t.parent_id.has_value() && is_subfeature_label(t.label)) {
                ++pair_count;
            }
        }
        hierarchy_.apply(std::span<Track>(tracks_.data(), tracks_.size()));
        stats_.num_constraint_pairs = pair_count;
        stats_.num_hierarchy_violations = violation_count;
    }

    // ── Phase 6: Boundary-probation maintenance ───────────────────────
    maintain_boundary_probation(detections, matches_scratch_);

    // ── Phase 7: Lifecycle (spawn / promote / prune) ──────────────────
    run_lifecycle(unmatched_dets_scratch_, detections);
    prune_dead();

    // Selector update (the actuator node consumes the smoothed (dx, dy)).
    update_target_selector();

    // ── Phase 8: Egress wire frame ────────────────────────────────────
    ++frame_counter_;
    emit_wire_frame(out_wire_frame, frame_counter_);
    return out_wire_frame.size();
}

}  // namespace roc::kinematics
