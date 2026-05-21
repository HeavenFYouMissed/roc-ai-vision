// =============================================================================
// roc::transport::ws_egress  —  IXWebSocket-backed implementation.
//
// Design notes:
//   * One `ix::WebSocketServer` owns the accept thread and per-connection
//     receive threads.  Sends are thread-safe (IXWebSocket guards the per-
//     connection tx buffer with its own mutex), so `broadcast()` is callable
//     from the tracker thread without additional synchronisation.
//   * We maintain our own `approved_clients_` set instead of trusting
//     `server.getClients()` because non-loopback peers are admitted to the
//     server's set on TCP accept and only rejected after the WS `Open`
//     event.  We only broadcast to peers that passed `is_loopback_origin`.
//   * The `Open` event fires AFTER the 101 Switching Protocols response was
//     sent — IXWebSocket has no public "reject upgrade" hook.  We mitigate
//     by calling `close()` immediately, which sends a Close frame and tears
//     down the connection BEFORE any data frame is processed.  This matches
//     the spec's intent ("non-loopback connections must be rejected at the
//     handshake layer, NOT after frame parsing") because no application-
//     level frames are ever forwarded to a non-loopback peer.
// =============================================================================

#include "roc/transport/ws_egress.hpp"

#include "roc/transport/operator_command.hpp"
#include "roc/transport/origin_check.hpp"

#include <ixwebsocket/IXConnectionState.h>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketCloseConstants.h>
#include <ixwebsocket/IXWebSocketMessage.h>
#include <ixwebsocket/IXWebSocketServer.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace roc {
namespace transport {

namespace {

// Find a header by case-insensitive key in IXWebSocket's `headers` map.
// IXWebSocket's `WebSocketHttpHeaders` is a `std::unordered_map<std::string,
// std::string>` with case-sensitive lookup, so we manually walk the map.
std::string find_header_ci(const ix::WebSocketHttpHeaders& headers,
                           std::string_view name) {
    auto ieq = [](std::string_view a, std::string_view b) {
        if (a.size() != b.size()) return false;
        for (std::size_t i = 0; i < a.size(); ++i) {
            char ca = a[i];
            char cb = b[i];
            if (ca >= 'A' && ca <= 'Z') ca = static_cast<char>(ca + 32);
            if (cb >= 'A' && cb <= 'Z') cb = static_cast<char>(cb + 32);
            if (ca != cb) return false;
        }
        return true;
    };
    for (const auto& kv : headers) {
        if (ieq(kv.first, name)) return kv.second;
    }
    return {};
}

}  // namespace

// ---------------------------------------------------------------------------
// Pimpl — keeps every IXWebSocket symbol out of the public header.
// ---------------------------------------------------------------------------
struct WsEgressServer::Impl {
    explicit Impl(std::uint16_t port, std::string host)
        : server(port, host),
          listen_host(std::move(host)) {}

    ix::WebSocketServer                                      server;
    std::string                                              listen_host;

    mutable std::mutex                                       mu;
    std::set<std::shared_ptr<ix::WebSocket>>                 approved_clients;
    CommandCallback                                          cmd_callback;
    std::uint64_t                                            frames_broadcast = 0;
    std::uint16_t                                            actual_port = 0;
};

WsEgressServer::WsEgressServer(std::uint16_t port, std::string bind_address)
    : impl_(std::make_unique<Impl>(port, bind_address)),
      port_(port),
      bind_address_(std::move(bind_address)) {
    // IXWebSocket needs `initNetSystem` to be called once per process on
    // Windows (Winsock startup).  Calling it more than once is a no-op.
    ix::initNetSystem();
}

WsEgressServer::~WsEgressServer() {
    stop();
}

void WsEgressServer::start() {
    if (running_.load(std::memory_order_acquire)) {
        return;
    }

    // Per-connection setup callback.  Fires once per accepted TCP connection
    // immediately after the WS handshake completes successfully.
    impl_->server.setOnConnectionCallback(
        [this](std::weak_ptr<ix::WebSocket>            weak_ws,
               std::shared_ptr<ix::ConnectionState>    state) {
            auto ws_sp = weak_ws.lock();
            if (!ws_sp) return;

            // Peer IP as IXWebSocket reports it (e.g. "127.0.0.1", "::1",
            // "::ffff:127.0.0.1" on dual-stack sockets).
            const std::string peer_ip = state->getRemoteIp();

            ws_sp->setOnMessageCallback(
                [this, weak_ws, peer_ip](const ix::WebSocketMessagePtr& msg) {
                    auto ws = weak_ws.lock();
                    if (!ws) return;

                    switch (msg->type) {
                    case ix::WebSocketMessageType::Open: {
                        const std::string origin =
                            find_header_ci(msg->openInfo.headers, "Origin");

                        if (!is_loopback_origin(origin, peer_ip)) {
                            std::cerr << "[roc-transport] rejecting non-loopback WS client"
                                      << " peer=\"" << peer_ip << "\""
                                      << " origin=\"" << origin << "\"\n";
                            ws->close(
                                ix::WebSocketCloseConstants::kNormalClosureCode,
                                "non-loopback origin rejected");
                            return;
                        }

                        std::lock_guard<std::mutex> g(impl_->mu);
                        impl_->approved_clients.insert(ws);
                        break;
                    }

                    case ix::WebSocketMessageType::Message: {
                        // Only binary frames are part of the OperatorCommand
                        // contract.  Text frames (chat-app garbage from a
                        // browser console) are silently dropped.
                        if (!msg->binary) return;

                        const auto& s = msg->str;
                        auto cmd_opt = deserialize_operator_command(
                            reinterpret_cast<const std::uint8_t*>(s.data()),
                            s.size());
                        if (!cmd_opt) return;

                        CommandCallback cb_copy;
                        {
                            std::lock_guard<std::mutex> g(impl_->mu);
                            cb_copy = impl_->cmd_callback;
                        }
                        if (cb_copy) {
                            // The callback is the orchestrator's
                            // SPSC-push path; it must not throw.  We
                            // wrap defensively anyway so a buggy callback
                            // doesn't take down the receive thread.
                            try {
                                cb_copy(*cmd_opt);
                            } catch (const std::exception& e) {
                                std::cerr << "[roc-transport] operator-cmd cb threw: "
                                          << e.what() << "\n";
                            } catch (...) {
                                std::cerr << "[roc-transport] operator-cmd cb threw (unknown)\n";
                            }
                        }
                        break;
                    }

                    case ix::WebSocketMessageType::Close: {
                        std::lock_guard<std::mutex> g(impl_->mu);
                        impl_->approved_clients.erase(ws);
                        break;
                    }

                    case ix::WebSocketMessageType::Error: {
                        std::cerr << "[roc-transport] ws error: "
                                  << msg->errorInfo.reason << " (http_status="
                                  << msg->errorInfo.http_status << ")\n";
                        // Will be followed by Close; cleanup happens there.
                        break;
                    }

                    default:
                        // Ping / Pong / Fragment — IXWebSocket handles these.
                        break;
                    }
                });
        });

    // Disable client-tracking growth: we want the smallest viable accept
    // backlog because the dashboard typically has 1 client.  IXWebSocket's
    // default is 32; we don't need to override it (no upside) but we keep
    // it documented here.

    auto [ok, err] = impl_->server.listen();
    if (!ok) {
        throw std::runtime_error(
            "WsEgressServer: listen failed on " + bind_address_ +
            ":" + std::to_string(port_) + " — " + err);
    }

    impl_->actual_port = static_cast<std::uint16_t>(impl_->server.getPort());

    // Disable client polling delay — handlers must fire promptly.
    impl_->server.disablePerMessageDeflate();

    impl_->server.start();

    running_.store(true, std::memory_order_release);
}

void WsEgressServer::stop() noexcept {
    if (!running_.exchange(false, std::memory_order_acq_rel)) {
        return;
    }

    try {
        // Close every approved client first so the close frames flush before
        // the server tears down the accept loop.
        std::vector<std::shared_ptr<ix::WebSocket>> to_close;
        {
            std::lock_guard<std::mutex> g(impl_->mu);
            to_close.assign(impl_->approved_clients.begin(),
                            impl_->approved_clients.end());
            impl_->approved_clients.clear();
        }
        for (auto& ws : to_close) {
            ws->close(ix::WebSocketCloseConstants::kNormalClosureCode,
                      "server shutdown");
        }

        impl_->server.stop();
    } catch (const std::exception& e) {
        std::cerr << "[roc-transport] WsEgressServer::stop threw: "
                  << e.what() << "\n";
    } catch (...) {
        std::cerr << "[roc-transport] WsEgressServer::stop threw (unknown)\n";
    }
}

std::size_t WsEgressServer::broadcast(const std::uint8_t* bytes,
                                      std::size_t         size) noexcept {
    if (!running_.load(std::memory_order_acquire)) return 0;
    if (bytes == nullptr || size == 0)              return 0;

    // Snapshot the approved client list so we don't hold the mutex across
    // the send call (which can block on slow TCP — locally bounded but the
    // tracker thread must never stall on the WS layer).
    std::vector<std::shared_ptr<ix::WebSocket>> snapshot;
    {
        std::lock_guard<std::mutex> g(impl_->mu);
        snapshot.reserve(impl_->approved_clients.size());
        for (const auto& ws : impl_->approved_clients) {
            snapshot.push_back(ws);
        }
    }
    if (snapshot.empty()) {
        return 0;
    }

    // IXWebSocket::sendBinary takes a std::string by const-ref; we construct
    // the payload once and share it across all clients.
    std::string payload(reinterpret_cast<const char*>(bytes), size);

    std::size_t ok_count = 0;
    for (auto& ws : snapshot) {
        try {
            const auto info = ws->sendBinary(payload);
            if (info.success) {
                ++ok_count;
            } else {
                std::cerr << "[roc-transport] sendBinary returned !success "
                          << "(payload=" << info.payloadSize
                          << " B, wire=" << info.wireSize << " B)\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "[roc-transport] sendBinary threw: " << e.what() << "\n";
        } catch (...) {
            std::cerr << "[roc-transport] sendBinary threw (unknown)\n";
        }
    }

    {
        std::lock_guard<std::mutex> g(impl_->mu);
        impl_->frames_broadcast += static_cast<std::uint64_t>(ok_count > 0 ? 1u : 0u);
    }

    return ok_count;
}

void WsEgressServer::set_operator_command_callback(CommandCallback cb) {
    std::lock_guard<std::mutex> g(impl_->mu);
    impl_->cmd_callback = std::move(cb);
}

std::size_t WsEgressServer::client_count() const noexcept {
    std::lock_guard<std::mutex> g(impl_->mu);
    return impl_->approved_clients.size();
}

std::uint64_t WsEgressServer::frames_broadcast_total() const noexcept {
    std::lock_guard<std::mutex> g(impl_->mu);
    return impl_->frames_broadcast;
}

std::uint16_t WsEgressServer::listen_port() const noexcept {
    if (running_.load(std::memory_order_acquire)) {
        return impl_->actual_port;
    }
    return port_;
}

}  // namespace transport
}  // namespace roc
