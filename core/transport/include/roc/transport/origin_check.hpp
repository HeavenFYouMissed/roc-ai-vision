#pragma once

// =============================================================================
// roc::transport::origin_check
//
// Loopback-only enforcement for the dashboard WebSocket bridge.  The user's
// security posture pins the dashboard to `api.roc.internal` (a host-file
// rewrite of `127.0.0.1`), and explicitly rejects any non-loopback peer at
// the WS handshake layer.  See `research_ipc_middleware_2026.md` §8.2
// "Command Path" + the project vision document.
//
// Predicate is pure / header-only / noexcept — no global state, no logging.
// Callers (notably `ws_egress.cpp`) emit a diagnostic when a connection is
// rejected; this header just answers yes/no.
//
// Accepted Origin patterns (case-insensitive scheme, case-sensitive host):
//   * empty (Origin omitted — non-browser client, e.g. Python sidecar)
//   * `http://localhost`            [: <port>] [/...]
//   * `http://127.0.0.1`            [: <port>] [/...]
//   * `http://[::1]`                [: <port>] [/...]
//   * `http://api.roc.internal`     [: <port>] [/...]
//   * `https://` variants of all of the above
//   * `ws://`, `wss://` variants of all of the above (CLI tools may set
//     Origin = the websocket URL).
//
// Accepted peer-address patterns (string form as IXWebSocket exposes it):
//   * `127.0.0.1`
//   * `::1`     (canonical IPv6 loopback)
//   * `::ffff:127.0.0.1`  (IPv4-mapped IPv6 — common on dual-stack sockets)
//
// Everything else returns false.
// =============================================================================

#include <cstddef>
#include <string_view>

namespace roc {
namespace transport {

namespace detail {

// ASCII-only case-insensitive equality.
constexpr bool iequal(std::string_view a, std::string_view b) noexcept {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        char ca = a[i];
        char cb = b[i];
        if (ca >= 'A' && ca <= 'Z') ca = static_cast<char>(ca + 32);
        if (cb >= 'A' && cb <= 'Z') cb = static_cast<char>(cb + 32);
        if (ca != cb) return false;
    }
    return true;
}

// Strip the URL scheme.  Returns the host[:port]/path portion or empty if no
// scheme is found.  Recognised schemes: http, https, ws, wss.
//
// NOTE: `static constexpr` locals would simplify this, but they require
// C++23.  We instead unfold the four schemes by hand at the top of the
// function — each `iequal` call is constexpr-friendly under C++17.
constexpr std::string_view strip_scheme(std::string_view origin) noexcept {
    constexpr std::string_view kHttp  {"http://"};
    constexpr std::string_view kHttps {"https://"};
    constexpr std::string_view kWs    {"ws://"};
    constexpr std::string_view kWss   {"wss://"};

    if (origin.size() >= kHttp.size()  && iequal(origin.substr(0, kHttp.size()),  kHttp))  return origin.substr(kHttp.size());
    if (origin.size() >= kHttps.size() && iequal(origin.substr(0, kHttps.size()), kHttps)) return origin.substr(kHttps.size());
    if (origin.size() >= kWs.size()    && iequal(origin.substr(0, kWs.size()),    kWs))    return origin.substr(kWs.size());
    if (origin.size() >= kWss.size()   && iequal(origin.substr(0, kWss.size()),   kWss))   return origin.substr(kWss.size());

    return {};  // unknown / missing scheme — caller should reject.
}

// Extract the host portion (everything before ':' / '/' / end).  Handles
// bracketed IPv6 literals: `[::1]:8080/path` → `[::1]`.
constexpr std::string_view extract_host(std::string_view host_and_more) noexcept {
    if (host_and_more.empty()) return {};
    if (host_and_more.front() == '[') {
        // IPv6 literal — find the closing bracket.
        auto end = host_and_more.find(']');
        if (end == std::string_view::npos) return {};
        return host_and_more.substr(0, end + 1);
    }
    // IPv4 / hostname — terminate at ':' or '/'.
    std::size_t end = host_and_more.size();
    for (std::size_t i = 0; i < host_and_more.size(); ++i) {
        char c = host_and_more[i];
        if (c == ':' || c == '/') { end = i; break; }
    }
    return host_and_more.substr(0, end);
}

}  // namespace detail

// ---------------------------------------------------------------------------
// Is the supplied Origin header value a permitted loopback origin?
// An empty origin is permitted (non-browser callers don't send one).
// ---------------------------------------------------------------------------
constexpr bool is_loopback_origin_header(std::string_view origin) noexcept {
    if (origin.empty()) return true;

    auto rest = detail::strip_scheme(origin);
    if (rest.empty()) return false;

    auto host = detail::extract_host(rest);
    if (host.empty()) return false;

    return detail::iequal(host, "localhost")
        || detail::iequal(host, "127.0.0.1")
        || detail::iequal(host, "[::1]")
        || detail::iequal(host, "api.roc.internal");
}

// ---------------------------------------------------------------------------
// Is the supplied peer address (as IXWebSocket reports it) a loopback peer?
// ---------------------------------------------------------------------------
constexpr bool is_loopback_peer_address(std::string_view peer) noexcept {
    if (peer.empty()) return false;
    return detail::iequal(peer, "127.0.0.1")
        || detail::iequal(peer, "::1")
        || detail::iequal(peer, "[::1]")
        || detail::iequal(peer, "::ffff:127.0.0.1");
}

// ---------------------------------------------------------------------------
// Combined gate — both predicates must pass.
// ---------------------------------------------------------------------------
constexpr bool is_loopback_origin(std::string_view origin_header,
                                  std::string_view client_addr) noexcept {
    return is_loopback_peer_address(client_addr)
        && is_loopback_origin_header(origin_header);
}

}  // namespace transport
}  // namespace roc
