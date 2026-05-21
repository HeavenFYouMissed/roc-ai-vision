// =============================================================================
// test_orchestrator_smoke — bring up the full Phase-4 wiring (synthetic
// detection generator + tracker + WS egress) for ~2 seconds and assert:
//   * at least one wire frame was broadcast to a connected loopback client,
//   * all threads shut down cleanly on the shutdown signal,
//   * no exceptions escaped any thread.
//
// Compiled at C++20 (per the per-target override in CMakeLists.txt) because
// the tracker thread calls `roc::kinematics::Tracker::step()` directly,
// whose signature exposes `std::span<const Detection>`.
// =============================================================================

#include "roc/kinematics/tracker.hpp"
#include "roc/kinematics/types.hpp"

#include "roc/transport/operator_command.hpp"
#include "roc/transport/spsc_links.hpp"
#include "roc/transport/ws_egress.hpp"

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketMessage.h>

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <thread>
#include <vector>

using namespace roc::transport;
using namespace std::chrono_literals;

namespace {

std::int64_t now_ns() {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

double now_seconds() {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

void smoke_generator(InferenceToTrackerQueue& q,
                     std::atomic<bool>&       shutdown,
                     std::atomic<std::uint64_t>& produced) {
    using namespace std::chrono;
    auto next = steady_clock::now();
    std::uint32_t fid = 0;
    while (!shutdown.load()) {
        ++fid;
        DetectionBatch b{};
        b.frame_id   = fid;
        b.capture_ns = now_ns();
        const double t = static_cast<double>(fid) * 0.033;
        roc::kinematics::Detection d{};
        d.bbox_center.u = 320.0 + 80.0 * std::cos(t);
        d.bbox_center.v = 240.0 + 80.0 * std::sin(t);
        d.bbox_w_px    = 50.0;
        d.bbox_h_px    = 140.0;
        d.confidence   = 0.9;
        d.label        = roc::kinematics::Label::Person;
        b.push_back(d);

        (void)q.try_push(std::move(b));
        produced.fetch_add(1);

        next += milliseconds(33);
        std::this_thread::sleep_until(next);
    }
}

void smoke_tracker(InferenceToTrackerQueue& in_q,
                   TrackerToWireQueue&      out_q,
                   std::atomic<bool>&       shutdown,
                   std::atomic<std::uint64_t>& steps,
                   std::atomic<std::uint64_t>& thrown) {
    roc::kinematics::CameraPose pose{};
    pose.K(0, 0) = 600.0; pose.K(1, 1) = 600.0;
    pose.K(0, 2) = 320.0; pose.K(1, 2) = 240.0;
    pose.img_w = 640; pose.img_h = 480;
    pose.R_cw = roc::kinematics::Mat3::Identity();
    pose.t_cw = roc::kinematics::Vec3::Zero();
    roc::kinematics::TrackerConfig cfg{};
    cfg.selector_cx = 320.0; cfg.selector_cy = 240.0;
    roc::kinematics::Tracker tracker(pose, cfg);

    std::vector<std::uint8_t> buf;
    while (!shutdown.load()) {
        auto* head = in_q.front();
        if (!head) {
            std::this_thread::sleep_for(500us);
            continue;
        }
        DetectionBatch batch = std::move(*head);
        in_q.pop();

        std::span<const roc::kinematics::Detection> ds{batch.dets.data(),
                                                       static_cast<std::size_t>(batch.count)};
        buf.clear();
        try {
            tracker.step(now_seconds(), ds, buf);
        } catch (...) {
            thrown.fetch_add(1);
            continue;
        }
        WireFrame wf{std::move(buf), batch.frame_id, now_ns()};
        (void)out_q.try_push(std::move(wf));
        buf.clear();
        steps.fetch_add(1);
    }
}

void smoke_egress(TrackerToWireQueue& q,
                  WsEgressServer&     server,
                  std::atomic<bool>&  shutdown,
                  std::atomic<std::uint64_t>& broadcast_count) {
    while (!shutdown.load()) {
        auto* head = q.front();
        if (!head) {
            std::this_thread::sleep_for(500us);
            continue;
        }
        WireFrame wf = std::move(*head);
        q.pop();
        if (!wf.bytes.empty()) {
            server.broadcast(wf.bytes.data(), wf.bytes.size());
            broadcast_count.fetch_add(1);
        }
    }
    while (q.front()) q.pop();
}

}  // namespace

TEST_CASE("orchestrator smoke: 2-second demo, threads + WS + tracker integration",
          "[smoke][orchestrator]") {
    ix::initNetSystem();

    // ── SPSC queues ───────────────────────────────────────────────────
    InferenceToTrackerQueue det_q(kInferenceToTrackerCapacity);
    TrackerToWireQueue      wire_q(kTrackerToWireCapacity);

    // ── WS server on a fixed high port (IXWebSocket does not expose the
    // OS-assigned port when binding to 0). ───────────────────────────────
    constexpr std::uint16_t kPortSmoke = 39811;
    WsEgressServer server(kPortSmoke, "127.0.0.1");
    server.start();
    const auto port = server.listen_port();
    REQUIRE(port == kPortSmoke);

    // ── Loopback client subscribing to the broadcast feed ─────────────
    std::atomic<std::uint32_t> client_rx{0};
    std::atomic<bool>          client_opened{false};
    ix::WebSocket client;
    client.setUrl("ws://127.0.0.1:" + std::to_string(port) + "/");
    client.disablePerMessageDeflate();
    client.disableAutomaticReconnection();
    client.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open) client_opened.store(true);
        else if (msg->type == ix::WebSocketMessageType::Message && msg->binary) {
            client_rx.fetch_add(1);
        }
    });
    client.start();

    auto wait_until = [](auto pred, std::chrono::milliseconds budget) {
        const auto deadline = std::chrono::steady_clock::now() + budget;
        while (!pred()) {
            if (std::chrono::steady_clock::now() >= deadline) return false;
            std::this_thread::sleep_for(2ms);
        }
        return true;
    };
    REQUIRE(wait_until([&] { return client_opened.load(); }, 2000ms));
    REQUIRE(wait_until([&] { return server.client_count() >= 1; }, 1000ms));

    // ── Spawn the 3 demonstration threads ─────────────────────────────
    std::atomic<bool>           shutdown{false};
    std::atomic<std::uint64_t>  produced{0};
    std::atomic<std::uint64_t>  steps{0};
    std::atomic<std::uint64_t>  thrown{0};
    std::atomic<std::uint64_t>  egress_count{0};

    std::thread t_gen   (smoke_generator, std::ref(det_q),  std::ref(shutdown),
                         std::ref(produced));
    std::thread t_track (smoke_tracker,   std::ref(det_q),  std::ref(wire_q),
                         std::ref(shutdown), std::ref(steps), std::ref(thrown));
    std::thread t_egress(smoke_egress,    std::ref(wire_q), std::ref(server),
                         std::ref(shutdown), std::ref(egress_count));

    // Run for 2 seconds.
    std::this_thread::sleep_for(2s);

    // Trigger shutdown — reverse-order join (Egress drains last).
    shutdown.store(true);
    t_gen   .join();
    t_track .join();
    t_egress.join();

    // ── Allow the WS layer ~250 ms to flush the last broadcast frames ─
    std::this_thread::sleep_for(250ms);

    // Assertions.
    REQUIRE(produced.load()      >= 30);    // ~30 frames/s for 2 s
    REQUIRE(steps.load()         >= 1);
    REQUIRE(thrown.load()        == 0);
    REQUIRE(egress_count.load()  >= 1);
    REQUIRE(client_rx.load()     >= 1);     // dashboard saw at least one frame

    // ── Clean teardown ────────────────────────────────────────────────
    client.stop();
    server.stop();
}
