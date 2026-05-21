// =============================================================================
// test_ws_egress — integration test for `WsEgressServer`.  Uses IXWebSocket
// (already linked) as the client driver to avoid an extra dependency.
//
// Each test binds the server to a random high port (port=0 → OS-assigned) so
// parallel CI runs don't collide.
// =============================================================================

#include "roc/transport/operator_command.hpp"
#include "roc/transport/ws_egress.hpp"

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketHttpHeaders.h>
#include <ixwebsocket/IXWebSocketMessage.h>

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace roc::transport;
using namespace std::chrono_literals;

namespace {

// Quietly wait for a predicate to become true or a deadline to expire.
template <class Pred>
bool wait_for(Pred&& p, std::chrono::milliseconds budget) {
    const auto deadline = std::chrono::steady_clock::now() + budget;
    while (!p()) {
        if (std::chrono::steady_clock::now() >= deadline) return false;
        std::this_thread::sleep_for(2ms);
    }
    return true;
}

// Build a `ws://127.0.0.1:<port>/` URL.
std::string ws_url(std::uint16_t port) {
    return "ws://127.0.0.1:" + std::to_string(port) + "/";
}

struct ClientResult {
    std::atomic<bool>          opened {false};
    std::atomic<bool>          closed {false};
    std::atomic<bool>          errored{false};
    std::atomic<std::uint32_t> rx_binary_count{0};

    std::mutex                 mu;
    std::vector<std::uint8_t>  last_binary_payload;
    std::string                last_error_reason;
    int                        last_error_status = 0;
};

}  // namespace

// Distinct high ports per test, well above the IANA registered range and
// below the dynamic / private range default Windows uses for ephemeral
// outbound connections — empirically clear on a stock MSYS2 mingw64 box.
// `IXWebSocket::SocketServer::getPort()` returns the *configured* port,
// not the OS-assigned port, so binding to 0 is not viable here.
inline constexpr std::uint16_t kPortLifecycle    = 39801;
inline constexpr std::uint16_t kPortBroadcast    = 39802;
inline constexpr std::uint16_t kPortOperatorCmd  = 39803;
inline constexpr std::uint16_t kPortOrigin       = 39804;
inline constexpr std::uint16_t kPortSafety       = 39805;

TEST_CASE("WsEgressServer: starts and stops cleanly", "[ws][lifecycle]") {
    ix::initNetSystem();
    WsEgressServer server(kPortLifecycle, "127.0.0.1");
    REQUIRE_FALSE(server.is_running());
    server.start();
    REQUIRE(server.is_running());
    const auto port = server.listen_port();
    INFO("listening on port " << port);
    REQUIRE(port == kPortLifecycle);
    server.stop();
    REQUIRE_FALSE(server.is_running());
}

TEST_CASE("WsEgressServer: loopback client accepted, broadcast round-trips bytes",
          "[ws][broadcast]") {
    ix::initNetSystem();
    WsEgressServer server(kPortBroadcast, "127.0.0.1");
    server.start();
    const auto port = server.listen_port();
    REQUIRE(port == kPortBroadcast);

    auto result = std::make_shared<ClientResult>();
    ix::WebSocket client;
    client.setUrl(ws_url(port));
    client.disablePerMessageDeflate();
    client.disableAutomaticReconnection();
    client.setOnMessageCallback([result](const ix::WebSocketMessagePtr& msg) {
        switch (msg->type) {
        case ix::WebSocketMessageType::Open:
            result->opened.store(true);
            break;
        case ix::WebSocketMessageType::Close:
            result->closed.store(true);
            break;
        case ix::WebSocketMessageType::Error:
            result->errored.store(true);
            {
                std::lock_guard<std::mutex> g(result->mu);
                result->last_error_reason = msg->errorInfo.reason;
                result->last_error_status = msg->errorInfo.http_status;
            }
            break;
        case ix::WebSocketMessageType::Message:
            if (msg->binary) {
                {
                    std::lock_guard<std::mutex> g(result->mu);
                    result->last_binary_payload.assign(
                        reinterpret_cast<const std::uint8_t*>(msg->str.data()),
                        reinterpret_cast<const std::uint8_t*>(msg->str.data()) + msg->str.size());
                }
                result->rx_binary_count.fetch_add(1);
            }
            break;
        default:
            break;
        }
    });
    client.start();

    REQUIRE(wait_for([&] { return result->opened.load(); }, 2000ms));
    REQUIRE(wait_for([&] { return server.client_count() >= 1; }, 1000ms));

    // Build a minimal §7 wire-format frame: 12-byte header + 1 fake 86-byte track.
    std::vector<std::uint8_t> payload(12 + 86, 0);
    payload[0] = 1;                  // version
    payload[1] = 0x03;                // flags
    payload[2] = 0xEF; payload[3] = 0xBE; payload[4] = 0xAD; payload[5] = 0xDE; // frame_id (LE)
    payload[6] = 0x01; payload[7] = 0x00;     // num_tracks = 1
    // t_capture_ms_offset bytes 8..11 left 0
    // Fill the per-track payload with a distinctive pattern.
    for (std::size_t i = 12; i < payload.size(); ++i) {
        payload[i] = static_cast<std::uint8_t>(i & 0xFF);
    }

    const std::size_t n_clients = server.broadcast(payload.data(), payload.size());
    REQUIRE(n_clients == 1);

    REQUIRE(wait_for([&] { return result->rx_binary_count.load() >= 1; }, 2000ms));

    {
        std::lock_guard<std::mutex> g(result->mu);
        REQUIRE(result->last_binary_payload.size() == payload.size());
        REQUIRE(result->last_binary_payload == payload);
    }

    client.stop();
    REQUIRE(wait_for([&] { return server.client_count() == 0; }, 2000ms));
    server.stop();
}

TEST_CASE("WsEgressServer: receives operator command via callback", "[ws][cmd]") {
    ix::initNetSystem();
    WsEgressServer server(kPortOperatorCmd, "127.0.0.1");

    std::mutex                       cb_mu;
    std::condition_variable          cb_cv;
    std::vector<OperatorCommand>     received;

    server.set_operator_command_callback(
        [&](const OperatorCommand& c) {
            std::lock_guard<std::mutex> g(cb_mu);
            received.push_back(c);
            cb_cv.notify_all();
        });
    server.start();
    const auto port = server.listen_port();
    REQUIRE(port == kPortOperatorCmd);

    auto result = std::make_shared<ClientResult>();
    ix::WebSocket client;
    client.setUrl(ws_url(port));
    client.disablePerMessageDeflate();
    client.disableAutomaticReconnection();
    client.setOnMessageCallback([result](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open) result->opened.store(true);
    });
    client.start();
    REQUIRE(wait_for([&] { return result->opened.load(); }, 2000ms));

    OperatorCommand cmd{};
    cmd.kind            = OperatorCommandKind::LockTarget;
    cmd.target_track_id = 0xCAFEFEED12345678ull;
    const auto bytes = serialize_operator_command(cmd);
    std::string payload(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    client.sendBinary(payload);

    {
        std::unique_lock<std::mutex> g(cb_mu);
        REQUIRE(cb_cv.wait_for(g, 2s, [&] { return !received.empty(); }));
        REQUIRE(received.size() == 1);
        REQUIRE(received[0].kind == OperatorCommandKind::LockTarget);
        REQUIRE(received[0].target_track_id == 0xCAFEFEED12345678ull);
    }

    client.stop();
    server.stop();
}

TEST_CASE("WsEgressServer: rejects non-loopback Origin client", "[ws][origin]") {
    ix::initNetSystem();
    WsEgressServer server(kPortOrigin, "127.0.0.1");
    server.start();
    const auto port = server.listen_port();

    auto result = std::make_shared<ClientResult>();
    ix::WebSocket client;
    client.setUrl(ws_url(port));
    client.disablePerMessageDeflate();
    // Auto-reconnect must be OFF or the client immediately reconnects with
    // the same hostile Origin and we never observe a clean close.
    client.disableAutomaticReconnection();

    // Inject a hostile Origin header — the server should close the
    // connection immediately on the Open event (before any application
    // frames are processed).
    ix::WebSocketHttpHeaders extra;
    extra["Origin"] = "http://evil.example.com";
    client.setExtraHeaders(extra);

    client.setOnMessageCallback([result](const ix::WebSocketMessagePtr& msg) {
        switch (msg->type) {
        case ix::WebSocketMessageType::Open:  result->opened.store(true);  break;
        case ix::WebSocketMessageType::Close: result->closed.store(true);  break;
        case ix::WebSocketMessageType::Error: result->errored.store(true); break;
        default: break;
        }
    });
    client.start();

    // The TCP/WS handshake itself completes (101 Switching Protocols), so
    // `Open` may briefly fire client-side.  What MUST happen is that the
    // server kicks us before any payload flows: `client_count()` stays 0
    // and the connection ends in either `Close` (clean) or `Error`.
    const bool gone = wait_for(
        [&] { return result->closed.load() || result->errored.load(); }, 3000ms);
    REQUIRE(gone);
    REQUIRE(server.client_count() == 0);

    // Broadcast: should reach 0 clients because the hostile peer was
    // dropped before being admitted to the approved set.
    std::vector<std::uint8_t> payload(12, 0xCC);
    REQUIRE(server.broadcast(payload.data(), payload.size()) == 0);

    client.stop();
    server.stop();
}

TEST_CASE("WsEgressServer: broadcast on stopped server returns 0", "[ws][safety]") {
    WsEgressServer server(kPortSafety, "127.0.0.1");
    // Not started.
    std::vector<std::uint8_t> bytes(32, 0);
    REQUIRE(server.broadcast(bytes.data(), bytes.size()) == 0);

    server.start();
    server.stop();
    // After stop, broadcasts must be silently dropped.
    REQUIRE(server.broadcast(bytes.data(), bytes.size()) == 0);
}
