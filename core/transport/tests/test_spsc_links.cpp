// =============================================================================
// test_spsc_links — wait-free push/pop on every typed SPSC link, cache-line
// alignment guards, capacity-overflow behaviour.  Covers §5.3 / §5.4 of
// `research_ipc_middleware_2026.md`.
// =============================================================================

#include "roc/transport/spsc_links.hpp"
#include "roc/transport/operator_command.hpp"

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <thread>

using namespace roc::transport;

TEST_CASE("spsc_links: cache-line alignment guarantees (§5.4)", "[spsc][align]") {
    STATIC_REQUIRE(alignof(FrameBatch)      >= kCacheLineBytes);
    STATIC_REQUIRE(alignof(DetectionBatch)  >= kCacheLineBytes);
    STATIC_REQUIRE(alignof(ActuatorCommand) >= kCacheLineBytes);
    STATIC_REQUIRE(alignof(WireFrame)       >= kCacheLineBytes);
    STATIC_REQUIRE(sizeof(ActuatorCommand)  <= kCacheLineBytes);
}

TEST_CASE("FrameToInferenceQueue: push then pop preserves payload", "[spsc][frame]") {
    FrameToInferenceQueue q(kFrameToInferenceCapacity);
    REQUIRE(q.capacity() == kFrameToInferenceCapacity);

    cv::Mat synth(120, 160, CV_8UC3, cv::Scalar(7, 11, 13));
    FrameBatch fb{synth, 1234567890123ll, 42u};
    REQUIRE(q.try_push(std::move(fb)));
    REQUIRE_FALSE(q.empty());

    auto* head = q.front();
    REQUIRE(head != nullptr);
    REQUIRE(head->frame_id == 42u);
    REQUIRE(head->capture_ns == 1234567890123ll);
    REQUIRE(head->frame.cols == 160);
    REQUIRE(head->frame.rows == 120);
    REQUIRE(head->frame.channels() == 3);
    q.pop();
    REQUIRE(q.empty());
}

TEST_CASE("InferenceToTrackerQueue: handles DetectionBatch payload", "[spsc][det]") {
    InferenceToTrackerQueue q(kInferenceToTrackerCapacity);

    DetectionBatch batch{};
    batch.frame_id = 7;
    batch.capture_ns = 99;
    for (int i = 0; i < 5; ++i) {
        roc::kinematics::Detection d{};
        d.bbox_center.u = 100.0 + static_cast<double>(i);
        d.bbox_center.v = 200.0;
        d.bbox_w_px = 30.0;
        d.bbox_h_px = 60.0;
        d.confidence = 0.8;
        d.label = roc::kinematics::Label::Person;
        REQUIRE(batch.push_back(d));
    }
    REQUIRE(batch.count == 5);

    REQUIRE(q.try_push(std::move(batch)));
    auto* head = q.front();
    REQUIRE(head != nullptr);
    REQUIRE(head->count == 5);
    REQUIRE(head->dets[0].bbox_center.u == 100.0);
    REQUIRE(head->dets[4].bbox_center.u == 104.0);
    q.pop();
}

TEST_CASE("DetectionBatch overflow returns false from push_back", "[spsc][det][overflow]") {
    DetectionBatch batch{};
    for (int i = 0; i < kMaxDetectionsPerBatch; ++i) {
        roc::kinematics::Detection d{};
        REQUIRE(batch.push_back(d));
    }
    roc::kinematics::Detection extra{};
    REQUIRE_FALSE(batch.push_back(extra));
    REQUIRE(batch.count == kMaxDetectionsPerBatch);
}

TEST_CASE("TrackerToWireQueue: vector ownership transfers via move", "[spsc][wire]") {
    TrackerToWireQueue q(kTrackerToWireCapacity);
    std::vector<std::uint8_t> bytes(86 * 3 + 12, 0xA5);
    const auto sz = bytes.size();

    REQUIRE(q.try_push(WireFrame{std::move(bytes), 100u, 1ll}));
    REQUIRE(bytes.empty());  // moved-from

    auto* head = q.front();
    REQUIRE(head != nullptr);
    REQUIRE(head->frame_id == 100u);
    REQUIRE(head->bytes.size() == sz);
    REQUIRE(head->bytes.back() == 0xA5);
    q.pop();
}

TEST_CASE("Queue capacity overflow: try_push returns false when full", "[spsc][overflow]") {
    TrackerToWireQueue q(kTrackerToWireCapacity);
    for (std::size_t i = 0; i < kTrackerToWireCapacity; ++i) {
        WireFrame wf{std::vector<std::uint8_t>(12, 0), static_cast<std::uint32_t>(i), 0};
        REQUIRE(q.try_push(std::move(wf)));
    }
    // Now the queue is at capacity; the next try_push must fail.
    WireFrame oversized{std::vector<std::uint8_t>(12, 0), 999u, 0};
    REQUIRE_FALSE(q.try_push(std::move(oversized)));

    // Drain.
    for (std::size_t i = 0; i < kTrackerToWireCapacity; ++i) {
        REQUIRE(q.front() != nullptr);
        q.pop();
    }
    REQUIRE(q.empty());
}

TEST_CASE("OperatorCommandQueue: round-trip with all kinds", "[spsc][cmd]") {
    OperatorCommandQueue q(kOperatorCommandCapacity);

    OperatorCommand c1{OperatorCommandKind::LockTarget, 0xDEADBEEFCAFEFEEDull,
                       0.0f, 0.0f, 0.0f};
    OperatorCommand c2{OperatorCommandKind::ReleaseLock, 0,
                       0.0f, 0.0f, 0.0f};
    OperatorCommand c3{OperatorCommandKind::UpdateSelectorWeights, 0,
                       1.0f, 0.5f, 0.25f};

    REQUIRE(q.try_push(c1));
    REQUIRE(q.try_push(c2));
    REQUIRE(q.try_push(c3));

    auto h1 = q.front(); REQUIRE(h1->kind == OperatorCommandKind::LockTarget);
    REQUIRE(h1->target_track_id == 0xDEADBEEFCAFEFEEDull);
    q.pop();

    auto h2 = q.front(); REQUIRE(h2->kind == OperatorCommandKind::ReleaseLock);
    q.pop();

    auto h3 = q.front(); REQUIRE(h3->kind == OperatorCommandKind::UpdateSelectorWeights);
    REQUIRE(h3->weight_proximity  == 1.0f);
    REQUIRE(h3->weight_confidence == 0.5f);
    REQUIRE(h3->weight_stability  == 0.25f);
    q.pop();
}

TEST_CASE("SPSC wait-free producer/consumer threads make progress", "[spsc][threads]") {
    TrackerToActuatorQueue q(kTrackerToActuatorCapacity);
    std::atomic<bool> shutdown{false};
    std::atomic<std::uint64_t> produced{0};
    std::atomic<std::uint64_t> consumed{0};

    std::thread producer([&]() {
        for (int i = 0; i < 4096; ++i) {
            ActuatorCommand c{};
            c.target_track_id = static_cast<std::uint64_t>(i);
            c.dx_px = static_cast<float>(i);
            while (!q.try_push(c)) {
                if (shutdown.load()) return;
                std::this_thread::yield();
            }
            produced.fetch_add(1, std::memory_order_relaxed);
        }
    });

    std::thread consumer([&]() {
        while (consumed.load() < 4096) {
            auto* p = q.front();
            if (p == nullptr) {
                if (shutdown.load() && q.empty()) return;
                std::this_thread::yield();
                continue;
            }
            REQUIRE(p->target_track_id == consumed.load());
            q.pop();
            consumed.fetch_add(1, std::memory_order_relaxed);
        }
    });

    producer.join();
    // Give the consumer a moment to drain.
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (consumed.load() < 4096 && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    shutdown.store(true);
    consumer.join();
    REQUIRE(produced.load() == 4096);
    REQUIRE(consumed.load() == 4096);
}
