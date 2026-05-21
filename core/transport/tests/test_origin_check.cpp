// =============================================================================
// test_origin_check — unit tests for the loopback-origin predicate against a
// table of Origin / peer-IP pairs.  See `origin_check.hpp` for the contract.
// =============================================================================

#include "roc/transport/origin_check.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>
#include <vector>

using namespace roc::transport;

namespace {

struct Vec {
    std::string origin;
    std::string peer;
    bool        expected;
};

}  // namespace

TEST_CASE("is_loopback_peer_address accepts canonical loopback IPs", "[origin][peer]") {
    REQUIRE(is_loopback_peer_address("127.0.0.1"));
    REQUIRE(is_loopback_peer_address("::1"));
    REQUIRE(is_loopback_peer_address("[::1]"));
    REQUIRE(is_loopback_peer_address("::ffff:127.0.0.1"));
    REQUIRE_FALSE(is_loopback_peer_address(""));
    REQUIRE_FALSE(is_loopback_peer_address("10.0.0.5"));
    REQUIRE_FALSE(is_loopback_peer_address("192.168.1.100"));
    REQUIRE_FALSE(is_loopback_peer_address("2001:db8::1"));
    REQUIRE_FALSE(is_loopback_peer_address("172.16.0.42"));
    REQUIRE_FALSE(is_loopback_peer_address("8.8.8.8"));
}

TEST_CASE("is_loopback_origin_header accepts approved hosts (all schemes)", "[origin][header]") {
    // Empty Origin (curl / Python sidecar / non-browser client) is accepted.
    REQUIRE(is_loopback_origin_header(""));

    for (const char* scheme : {"http://", "https://", "ws://", "wss://"}) {
        for (const char* host : {"localhost", "127.0.0.1", "[::1]", "api.roc.internal"}) {
            const std::string with_port  = std::string(scheme) + host + ":5173/path";
            const std::string no_port    = std::string(scheme) + host;
            const std::string with_root  = std::string(scheme) + host + "/";
            INFO("scheme=" << scheme << " host=" << host);
            REQUIRE(is_loopback_origin_header(with_port));
            REQUIRE(is_loopback_origin_header(no_port));
            REQUIRE(is_loopback_origin_header(with_root));
        }
    }
}

TEST_CASE("is_loopback_origin_header rejects non-loopback hosts", "[origin][header][reject]") {
    REQUIRE_FALSE(is_loopback_origin_header("http://evil.com"));
    REQUIRE_FALSE(is_loopback_origin_header("https://example.org:8080"));
    REQUIRE_FALSE(is_loopback_origin_header("http://10.0.0.5"));
    REQUIRE_FALSE(is_loopback_origin_header("http://192.168.1.42"));
    REQUIRE_FALSE(is_loopback_origin_header("http://127.0.0.1.evil.com"));   // suffix attack
    REQUIRE_FALSE(is_loopback_origin_header("http://api.roc.internal.evil.com"));
    REQUIRE_FALSE(is_loopback_origin_header("foo://localhost"));             // unknown scheme
    REQUIRE_FALSE(is_loopback_origin_header("localhost"));                    // missing scheme
    REQUIRE_FALSE(is_loopback_origin_header("//127.0.0.1"));                  // schema-relative
}

TEST_CASE("is_loopback_origin is logical-AND of peer + header", "[origin][gate]") {
    const std::vector<Vec> table = {
        // Both pass.
        {"http://localhost:5173",         "127.0.0.1",            true},
        {"https://api.roc.internal",      "127.0.0.1",            true},
        {"",                              "::1",                  true},
        {"ws://[::1]:8765",               "::ffff:127.0.0.1",     true},

        // Origin OK, peer non-loopback.
        {"http://localhost",              "10.0.0.5",             false},
        {"https://127.0.0.1",             "8.8.8.8",              false},

        // Peer OK, origin non-loopback.
        {"http://evil.com",               "127.0.0.1",            false},
        {"https://api.roc.internal.evil", "127.0.0.1",            false},

        // Both fail.
        {"http://evil.com",               "8.8.8.8",              false},
        {"",                              "203.0.113.7",          false},
    };

    for (const auto& v : table) {
        INFO("origin=" << v.origin << " peer=" << v.peer);
        REQUIRE(is_loopback_origin(v.origin, v.peer) == v.expected);
    }
}

TEST_CASE("Case-insensitive scheme + host matching", "[origin][case]") {
    REQUIRE(is_loopback_origin_header("HTTP://localhost"));
    REQUIRE(is_loopback_origin_header("HtTpS://API.ROC.INTERNAL"));
    REQUIRE(is_loopback_origin_header("WSS://Localhost:9000/"));
}
