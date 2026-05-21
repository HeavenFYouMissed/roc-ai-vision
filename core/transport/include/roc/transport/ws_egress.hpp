#pragma once

// =============================================================================
// roc::transport::ws_egress
//
// Single-threaded asynchronous binary WebSocket server bridging the in-process
// SPSC ring buffers to the SvelteKit dashboard process.  Architecture per
// `research_ipc_middleware_2026.md` §6.3 (Dashboard Bridge) and §8.2
// (Dashboard Bridge protocol).
//
// Implementation library: **IXWebSocket** (Apache 2.0).  The binding spec
// names `uWebSockets`, but `uSockets` (its mandatory C dependency) is not
// FetchContent-friendly on MSYS2 MinGW — see the deviation logged in
// `core/transport/CMakeLists.txt`.  The wire contract is the same:
//
//   * Binary frames only (uWS::OpCode::BINARY equivalent =
//     ix::WebSocketSendInfo::binary = true).
//   * Loopback origin / peer enforced at the WS `Open` event (handshake
//     layer) before any frame is processed — see `origin_check.hpp`.
//   * Broadcast is thread-safe: callable from the tracker thread; the IX
//     library serialises send buffers via per-connection mutex internally.
//
// Lifetime / RAII:
//   * Constructor binds and validates the listen socket.  Throws on bind
//     failure (port already in use, etc.) so the orchestrator can pick a
//     different port or fail loudly.
//   * `start()` spawns the accept thread.  Re-entrant: calling on a started
//     server is a no-op.
//   * `stop()` closes all client sockets, stops the accept thread, joins.
//     Idempotent.  The destructor calls `stop()`.
// =============================================================================

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

namespace roc {
namespace transport {

struct OperatorCommand;  // forward decl — full include is operator_command.hpp

class WsEgressServer {
public:
    // OperatorCommand callback signature.  Invoked from the IXWebSocket
    // receive thread; the orchestrator typically pushes into an SPSC queue
    // for the tracker thread to drain on its own cadence.
    using CommandCallback = std::function<void(const OperatorCommand&)>;

    // Construct with bind address and port.  Defaults: `127.0.0.1:8765` per
    // §8.1 architecture diagram.  Does NOT bind the socket yet — bind +
    // listen happen in `start()` so a constructed-but-not-started server is
    // a valid (trivially destructible) object.
    explicit WsEgressServer(std::uint16_t port = 8765,
                            std::string   bind_address = "127.0.0.1");

    ~WsEgressServer();

    WsEgressServer(const WsEgressServer&)            = delete;
    WsEgressServer& operator=(const WsEgressServer&) = delete;
    WsEgressServer(WsEgressServer&&)                 = delete;
    WsEgressServer& operator=(WsEgressServer&&)      = delete;

    // Bind + listen + spawn accept thread.  Throws std::runtime_error on
    // bind failure (port in use, permission denied).  Idempotent: a second
    // call when `is_running()` is no-op.
    void start();

    // Graceful shutdown — close every client, stop the accept thread, join.
    // Idempotent.
    void stop() noexcept;

    bool is_running() const noexcept { return running_.load(std::memory_order_acquire); }

    // Push a binary frame to every currently-connected loopback client.
    // THREAD-SAFE: callable from the tracker thread.  Never throws — a
    // transient send failure (e.g. client disconnected mid-write) is
    // swallowed and logged via stderr, then the broadcast continues to the
    // remaining clients.  Returns the number of clients the frame was
    // successfully queued for.
    std::size_t broadcast(const std::uint8_t* bytes, std::size_t size) noexcept;

    // Same as above, accepting any contiguous byte range (vector, array, etc).
    template <class ByteRange>
    std::size_t broadcast(const ByteRange& bytes) noexcept {
        return broadcast(reinterpret_cast<const std::uint8_t*>(bytes.data()),
                         bytes.size());
    }

    // Register the operator-command callback.  Replaces any previous one.
    // Must be called BEFORE `start()` (or before any client connects after
    // start) — the implementation captures the callback under a mutex but
    // the dashboard could in principle send a command in the same TCP RTT
    // as the WS upgrade.
    void set_operator_command_callback(CommandCallback cb);

    // Diagnostic — how many clients are currently connected?
    std::size_t client_count() const noexcept;

    // Diagnostic — how many BINARY frames were dispatched in total?  Useful
    // for the orchestrator smoke test to verify the egress thread is alive.
    std::uint64_t frames_broadcast_total() const noexcept;

    // Effective listen port — useful for tests that bind to port 0 and
    // need to discover the OS-assigned port.
    std::uint16_t listen_port() const noexcept;

    std::string bind_address() const noexcept { return bind_address_; }

private:
    // Pimpl — keeps the IXWebSocket types out of the public header so
    // downstream consumers don't transitively pull in <ixwebsocket/...>.
    struct Impl;
    std::unique_ptr<Impl> impl_;

    std::uint16_t port_;
    std::string   bind_address_;
    std::atomic<bool> running_{false};
};

}  // namespace transport
}  // namespace roc
