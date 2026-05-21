// =============================================================================
// roc::transport::mjpeg_server implementation.
//
// Minimal cross-platform TCP server (Winsock on Windows, BSD sockets on
// POSIX). One accept thread + one thread per active client. JPEG encoding
// via OpenCV's cv::imencode.
// =============================================================================

#include "roc/transport/mjpeg_server.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  pragma comment(lib, "ws2_32.lib")
   using socket_t = SOCKET;
#  define ROC_SOCKET_INVALID INVALID_SOCKET
#  define ROC_SOCKET_ERROR_RET SOCKET_ERROR
#  define ROC_CLOSESOCKET(s) ::closesocket(s)
#  define ROC_SEND_FLAGS 0
#else
#  include <arpa/inet.h>
#  include <netinet/in.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <unistd.h>
#  include <netinet/tcp.h>
   using socket_t = int;
#  define ROC_SOCKET_INVALID (-1)
#  define ROC_SOCKET_ERROR_RET (-1)
#  define ROC_CLOSESOCKET(s) ::close(s)
#  define ROC_SEND_FLAGS MSG_NOSIGNAL
#endif

namespace roc {
namespace transport {

namespace {

// One Winsock initialisation per process — refcounted.
struct WinsockGuard {
#ifdef _WIN32
    WinsockGuard() {
        WSADATA wsa{};
        WSAStartup(MAKEWORD(2, 2), &wsa);
    }
    ~WinsockGuard() { WSACleanup(); }
#endif
};

void set_socket_keepalive(socket_t s) {
    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&yes), sizeof(yes));
#ifdef TCP_NODELAY
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&yes), sizeof(yes));
#endif
}

// Send all `n` bytes or return false on any failure. Best-effort blocking
// send with a small loop in case the kernel sends partial.
bool send_all(socket_t s, const char* data, std::size_t n) {
    std::size_t sent = 0;
    while (sent < n) {
        const int written = ::send(s, data + sent,
                                   static_cast<int>(n - sent),
                                   ROC_SEND_FLAGS);
        if (written <= 0) return false;
        sent += static_cast<std::size_t>(written);
    }
    return true;
}

bool send_str(socket_t s, const std::string& str) {
    return send_all(s, str.data(), str.size());
}

// Read up to `max` bytes, stop at the first \r\n\r\n. Returns the request
// headers as a single string. Empty on failure.
std::string read_http_request_headers(socket_t s, std::size_t max = 4096) {
    std::string out;
    out.reserve(512);
    char buf[512];
    while (out.size() < max) {
        const int got = ::recv(s, buf, sizeof(buf), 0);
        if (got <= 0) break;
        out.append(buf, static_cast<std::size_t>(got));
        if (out.find("\r\n\r\n") != std::string::npos) break;
    }
    return out;
}

// Pull the request path from a "GET /path HTTP/1.1\r\n..." string.
std::string parse_request_path(const std::string& req) {
    const auto eol = req.find("\r\n");
    if (eol == std::string::npos) return {};
    const std::string line = req.substr(0, eol);
    // expected: METHOD SP PATH SP VERSION
    const auto sp1 = line.find(' ');
    if (sp1 == std::string::npos) return {};
    const auto sp2 = line.find(' ', sp1 + 1);
    if (sp2 == std::string::npos) return {};
    return line.substr(sp1 + 1, sp2 - sp1 - 1);
}

// Pull the HTTP method from the request-line ("GET", "POST", "OPTIONS"...).
std::string parse_request_method(const std::string& req) {
    const auto sp = req.find(' ');
    if (sp == std::string::npos) return {};
    return req.substr(0, sp);
}

// Read a case-insensitive header value from the request headers blob.
std::string find_header(const std::string& req, const std::string& name_lower) {
    std::size_t pos = req.find("\r\n");
    while (pos != std::string::npos) {
        const std::size_t end = req.find("\r\n", pos + 2);
        if (end == std::string::npos) break;
        const std::string line = req.substr(pos + 2, end - (pos + 2));
        const std::size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            for (char& c : key) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            if (key == name_lower) {
                // Trim leading whitespace from value.
                std::size_t v = colon + 1;
                while (v < line.size() && (line[v] == ' ' || line[v] == '\t')) ++v;
                return line.substr(v);
            }
        }
        pos = end;
    }
    return {};
}

// Read the POST body up to Content-Length bytes after the header terminator.
// `header_blob` includes both headers AND any bytes the kernel already
// returned past "\r\n\r\n" in the initial recv (very common on small POSTs).
std::string read_post_body(socket_t s, const std::string& header_blob) {
    const auto end_of_headers = header_blob.find("\r\n\r\n");
    if (end_of_headers == std::string::npos) return {};
    const std::string len_str = find_header(header_blob, "content-length");
    if (len_str.empty()) return {};
    const std::size_t want = static_cast<std::size_t>(std::strtoul(len_str.c_str(), nullptr, 10));
    if (want == 0) return {};

    // Bytes already buffered after the header terminator.
    const std::size_t already = header_blob.size() - (end_of_headers + 4);
    std::string body = header_blob.substr(end_of_headers + 4, already);
    body.reserve(want);
    while (body.size() < want) {
        char buf[1024];
        const std::size_t need = want - body.size();
        const int got = ::recv(s, buf, static_cast<int>(std::min(sizeof(buf), need)), 0);
        if (got <= 0) break;
        body.append(buf, static_cast<std::size_t>(got));
    }
    return body;
}

// Extract a quoted string value for a top-level key in flat JSON. Hand-rolled
// to avoid pulling nlohmann::json. Accepts only the very simple shape
// `{"name":"yolo26m-roc-humanoid"}`. Returns empty on parse fail.
std::string extract_json_string_field(const std::string& body, const std::string& key) {
    const std::string needle = "\"" + key + "\"";
    auto p = body.find(needle);
    if (p == std::string::npos) return {};
    p = body.find(':', p + needle.size());
    if (p == std::string::npos) return {};
    p = body.find('"', p + 1);
    if (p == std::string::npos) return {};
    const auto end = body.find('"', p + 1);
    if (end == std::string::npos) return {};
    return body.substr(p + 1, end - (p + 1));
}

// Extract a numeric value (int or float) for a top-level key.
bool extract_json_number_field(const std::string& body, const std::string& key, double& out) {
    const std::string needle = "\"" + key + "\"";
    auto p = body.find(needle);
    if (p == std::string::npos) return false;
    p = body.find(':', p + needle.size());
    if (p == std::string::npos) return false;
    ++p;
    while (p < body.size() && (body[p] == ' ' || body[p] == '\t')) ++p;
    char* endp = nullptr;
    const double v = std::strtod(body.c_str() + p, &endp);
    if (endp == body.c_str() + p) return false;
    out = v;
    return true;
}

bool extract_json_bool_field(const std::string& body, const std::string& key, bool& out) {
    const std::string needle = "\"" + key + "\"";
    auto p = body.find(needle);
    if (p == std::string::npos) return false;
    p = body.find(':', p + needle.size());
    if (p == std::string::npos) return false;
    ++p;
    while (p < body.size() && (body[p] == ' ' || body[p] == '\t')) ++p;
    if (body.compare(p, 4, "true") == 0)  { out = true;  return true; }
    if (body.compare(p, 5, "false") == 0) { out = false; return true; }
    return false;
}

// Render a complete HTTP/1.1 response (close-connection style) — convenience.
std::string make_response(int status, const std::string& status_text,
                          const std::string& content_type,
                          const std::string& body,
                          const std::string& cors_header,
                          const std::vector<std::string>& extra_headers = {}) {
    std::ostringstream resp;
    resp << "HTTP/1.1 " << status << ' ' << status_text << "\r\n"
         << cors_header
         << "Content-Type: " << content_type << "\r\n"
         << "Content-Length: " << body.size() << "\r\n"
         << "Connection: close\r\n";
    for (const auto& h : extra_headers) resp << h;
    resp << "\r\n" << body;
    return resp.str();
}

// Minimal JSON serialiser for CameraSourceInfo — avoids pulling nlohmann::json.
std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

std::string sources_to_json(const std::vector<CameraSourceInfo>& sources) {
    std::ostringstream os;
    os << "{\"devices\":[";
    bool first = true;
    for (const auto& d : sources) {
        if (!first) os << ',';
        first = false;
        os << "{";
        os << "\"id\":\""    << json_escape(d.id)    << "\",";
        os << "\"kind\":\""  << json_escape(d.kind)  << "\",";
        os << "\"label\":\"" << json_escape(d.label) << "\",";
        os << "\"deviceIndex\":" << d.device_index   << ",";
        os << "\"width\":"   << d.width              << ",";
        os << "\"height\":"  << d.height             << ",";
        os << "\"fps\":"     << d.fps                << ",";
        os << "\"online\":"  << (d.online ? "true" : "false");
        if (d.device_index >= 0) {
            os << ",\"mjpegUrl\":\"http://127.0.0.1:8766/mjpeg/"
               << d.device_index << "\"";
        }
        os << "}";
    }
    os << "]}";
    return os.str();
}

// ---- JSON serialisers for the Phase 4 / 6 API responses -----------------
std::string models_to_json(const std::vector<ModelInfo>& models) {
    std::ostringstream os;
    os << "{\"models\":[";
    bool first = true;
    for (const auto& m : models) {
        if (!first) os << ',';
        first = false;
        os << "{";
        os << "\"name\":\""          << json_escape(m.name)          << "\",";
        os << "\"manifestPath\":\""  << json_escape(m.manifest_path) << "\",";
        os << "\"onnxPath\":\""      << json_escape(m.onnx_path)     << "\",";
        os << "\"purpose\":\""       << json_escape(m.purpose)       << "\",";
        os << "\"head\":\""          << json_escape(m.head)          << "\",";
        os << "\"inputW\":"          << m.input_w                    << ",";
        os << "\"inputH\":"          << m.input_h                    << ",";
        os << "\"numClasses\":"      << m.num_classes                << ",";
        os << "\"active\":"          << (m.active ? "true" : "false") << ",";
        os << "\"loadable\":"        << (m.loadable ? "true" : "false") << ",";
        os << "\"statusNote\":\""    << json_escape(m.status_note)   << "\"";
        os << "}";
    }
    os << "]}";
    return os.str();
}

std::string capture_sources_to_json(const CaptureSourcesSnapshot& s) {
    std::ostringstream os;
    os << "{\"monitors\":[";
    bool first = true;
    for (const auto& m : s.monitors) {
        if (!first) os << ',';
        first = false;
        os << "{";
        os << "\"index\":"      << m.index      << ",";
        os << "\"x\":"          << m.x          << ",";
        os << "\"y\":"          << m.y          << ",";
        os << "\"w\":"          << m.w          << ",";
        os << "\"h\":"          << m.h          << ",";
        os << "\"isPrimary\":"  << (m.is_primary ? "true" : "false") << ",";
        os << "\"deviceName\":\"" << json_escape(m.device_name) << "\"";
        os << "}";
    }
    os << "],\"windows\":[";
    first = true;
    for (const auto& w : s.windows) {
        if (!first) os << ',';
        first = false;
        os << "{";
        os << "\"hwnd\":"      << w.hwnd << ",";
        os << "\"title\":\""   << json_escape(w.title) << "\",";
        os << "\"x\":"         << w.x << ",";
        os << "\"y\":"         << w.y << ",";
        os << "\"w\":"         << w.w << ",";
        os << "\"h\":"         << w.h << ",";
        os << "\"minimized\":" << (w.minimized ? "true" : "false");
        os << "}";
    }
    os << "]}";
    return os.str();
}

std::string fov_to_json(const FovSettings& f) {
    std::ostringstream os;
    os << "{";
    os << "\"enabled\":"        << (f.enabled ? "true" : "false") << ",";
    os << "\"cxNorm\":"         << f.cx_norm        << ",";
    os << "\"cyNorm\":"         << f.cy_norm        << ",";
    os << "\"radiusPx\":"       << f.radius_px      << ",";
    os << "\"followTarget\":"   << (f.follow_target ? "true" : "false") << ",";
    os << "\"sourceW\":"        << f.source_w       << ",";
    os << "\"sourceH\":"        << f.source_h       << ",";
    os << "\"activeCropX\":"    << f.active_crop_x  << ",";
    os << "\"activeCropY\":"    << f.active_crop_y  << ",";
    os << "\"activeCropW\":"    << f.active_crop_w  << ",";
    os << "\"activeCropH\":"    << f.active_crop_h;
    os << "}";
    return os.str();
}

// Extract camera index from path "/mjpeg/{N}". Returns -1 on parse fail.
int parse_camera_index_from_mjpeg_path(const std::string& path) {
    const std::string prefix = "/mjpeg/";
    if (path.size() <= prefix.size()) return -1;
    if (path.compare(0, prefix.size(), prefix) != 0) return -1;
    char* end = nullptr;
    const long idx = std::strtol(path.c_str() + prefix.size(), &end, 10);
    if (end == path.c_str() + prefix.size()) return -1;
    return static_cast<int>(idx);
}

}  // namespace

// ---------------------------------------------------------------------------
// Per-camera frame slot — protected by mutex, condvar-signalled when a new
// JPEG arrives. Each MJPEG client thread polls its assigned camera's slot.
// ---------------------------------------------------------------------------
struct FrameSlot {
    std::mutex                mu;
    std::condition_variable   cv;
    std::vector<std::uint8_t> jpeg_bytes;
    std::uint64_t             generation = 0;   // bumps on every push
};

struct MjpegServer::Impl {
    socket_t        listen_sock = ROC_SOCKET_INVALID;
    std::thread     accept_thread;
    std::atomic<bool> stop_flag{false};

    std::vector<std::thread> client_threads;
    std::mutex               client_threads_mu;

    // One slot per camera index — sized lazily up to a reasonable cap.
    std::vector<std::unique_ptr<FrameSlot>> frame_slots;
    std::mutex                              frame_slots_mu;

    std::vector<CameraSourceInfo> sources;
    std::mutex                    sources_mu;

    ApiHandlers              api;
    std::mutex               api_mu;   // guards `api` against set_api_handlers

    std::atomic<std::size_t> mjpeg_clients{0};
    std::uint16_t            port = 0;
    int                      jpeg_quality = 80;

#ifdef _WIN32
    WinsockGuard winsock_guard;
#endif

    FrameSlot& slot_for(int camera_index) {
        std::lock_guard<std::mutex> lock(frame_slots_mu);
        if (camera_index < 0) camera_index = 0;
        if (static_cast<std::size_t>(camera_index) >= frame_slots.size()) {
            frame_slots.resize(static_cast<std::size_t>(camera_index) + 1);
        }
        auto& slot = frame_slots[static_cast<std::size_t>(camera_index)];
        if (!slot) slot = std::make_unique<FrameSlot>();
        return *slot;
    }
};

MjpegServer::MjpegServer(std::uint16_t port, std::string bind_address, int jpeg_quality)
    : impl_(std::make_unique<Impl>()),
      port_(port),
      bind_address_(std::move(bind_address)),
      jpeg_quality_(jpeg_quality) {
    impl_->port = port;
    impl_->jpeg_quality = jpeg_quality;
}

MjpegServer::~MjpegServer() {
    stop();
}

void MjpegServer::start() {
    if (running_.load(std::memory_order_acquire)) return;

    impl_->listen_sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (impl_->listen_sock == ROC_SOCKET_INVALID) {
        throw std::runtime_error("mjpeg_server: socket() failed");
    }

    int reuse = 1;
    setsockopt(impl_->listen_sock, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    if (inet_pton(AF_INET, bind_address_.c_str(), &addr.sin_addr) <= 0) {
        ROC_CLOSESOCKET(impl_->listen_sock);
        impl_->listen_sock = ROC_SOCKET_INVALID;
        throw std::runtime_error("mjpeg_server: inet_pton failed for bind address");
    }

    if (::bind(impl_->listen_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == ROC_SOCKET_ERROR_RET) {
        ROC_CLOSESOCKET(impl_->listen_sock);
        impl_->listen_sock = ROC_SOCKET_INVALID;
        throw std::runtime_error("mjpeg_server: bind() failed (port in use?)");
    }

    if (::listen(impl_->listen_sock, 8) == ROC_SOCKET_ERROR_RET) {
        ROC_CLOSESOCKET(impl_->listen_sock);
        impl_->listen_sock = ROC_SOCKET_INVALID;
        throw std::runtime_error("mjpeg_server: listen() failed");
    }

    running_.store(true, std::memory_order_release);
    impl_->stop_flag.store(false, std::memory_order_release);

    impl_->accept_thread = std::thread([this]() {
        while (!impl_->stop_flag.load(std::memory_order_acquire)) {
            sockaddr_in peer{};
            socklen_t   peer_len = sizeof(peer);
            socket_t client = ::accept(impl_->listen_sock,
                                       reinterpret_cast<sockaddr*>(&peer),
                                       &peer_len);
            if (client == ROC_SOCKET_INVALID) {
                if (impl_->stop_flag.load(std::memory_order_acquire)) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }
            // Loopback-only enforcement — defence-in-depth.
            // 127.x.x.x in network byte order — top byte is 127.
            const std::uint32_t addr_h = ntohl(peer.sin_addr.s_addr);
            if ((addr_h & 0xff000000u) != 0x7f000000u) {
                ROC_CLOSESOCKET(client);
                continue;
            }
            set_socket_keepalive(client);

            // Spawn a per-client handler thread. Lifetime joined by stop().
            std::lock_guard<std::mutex> lock(impl_->client_threads_mu);
            impl_->client_threads.emplace_back([this, client]() {
                handle_client(static_cast<std::int64_t>(client));
            });
        }
    });
}

void MjpegServer::stop() noexcept {
    if (!running_.exchange(false, std::memory_order_acq_rel)) return;
    impl_->stop_flag.store(true, std::memory_order_release);

    // Wake any client thread blocked on a condvar.
    {
        std::lock_guard<std::mutex> lock(impl_->frame_slots_mu);
        for (auto& slot : impl_->frame_slots) {
            if (!slot) continue;
            std::lock_guard<std::mutex> sl(slot->mu);
            slot->cv.notify_all();
        }
    }

    if (impl_->listen_sock != ROC_SOCKET_INVALID) {
        ROC_CLOSESOCKET(impl_->listen_sock);
        impl_->listen_sock = ROC_SOCKET_INVALID;
    }
    if (impl_->accept_thread.joinable()) impl_->accept_thread.join();

    std::vector<std::thread> threads;
    {
        std::lock_guard<std::mutex> lock(impl_->client_threads_mu);
        threads.swap(impl_->client_threads);
    }
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }
}

void MjpegServer::push_jpeg_frame(int camera_index, const cv::Mat& frame_bgr) noexcept {
    if (!running_.load(std::memory_order_acquire)) return;
    if (frame_bgr.empty()) return;

    std::vector<std::uint8_t> encoded;
    const std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, jpeg_quality_};
    try {
        if (!cv::imencode(".jpg", frame_bgr, encoded, params)) return;
    } catch (...) {
        return;
    }
    if (encoded.empty()) return;

    auto& slot = impl_->slot_for(camera_index);
    {
        std::lock_guard<std::mutex> lock(slot.mu);
        slot.jpeg_bytes = std::move(encoded);
        slot.generation += 1;
    }
    slot.cv.notify_all();
}

void MjpegServer::set_sources(std::vector<CameraSourceInfo> sources) {
    std::lock_guard<std::mutex> lock(impl_->sources_mu);
    impl_->sources = std::move(sources);
}

void MjpegServer::set_api_handlers(ApiHandlers handlers) {
    std::lock_guard<std::mutex> lock(impl_->api_mu);
    impl_->api = std::move(handlers);
}

std::size_t MjpegServer::mjpeg_client_count() const noexcept {
    return impl_->mjpeg_clients.load(std::memory_order_relaxed);
}

// ---------------------------------------------------------------------------
// Per-client handler — parses the request line, dispatches to /sources,
// /health, or the /mjpeg/{idx} streaming path.
// ---------------------------------------------------------------------------
void MjpegServer::handle_client(std::int64_t client_handle) noexcept {
    const socket_t client = static_cast<socket_t>(client_handle);
    auto* const impl = impl_.get();
    auto* server = this;

    const std::string req = read_http_request_headers(client);
    if (req.empty()) {
        ROC_CLOSESOCKET(client);
        return;
    }
    const std::string method = parse_request_method(req);
    const std::string path   = parse_request_path(req);

    // CORS — allow the dashboard origin to fetch /sources from the browser.
    const std::string cors =
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Access-Control-Max-Age: 600\r\n";

    // CORS preflight — answer 204 No Content for any path.
    if (method == "OPTIONS") {
        const std::string resp =
            "HTTP/1.1 204 No Content\r\n" + cors + "Connection: close\r\n\r\n";
        send_str(client, resp);
        ROC_CLOSESOCKET(client);
        return;
    }

    // ---- Phase 4: model browser & swap ------------------------------------
    if (path == "/api/models" && method == "GET") {
        std::function<std::vector<ModelInfo>()> fn;
        {
            std::lock_guard<std::mutex> lock(impl->api_mu);
            fn = impl->api.list_models;
        }
        if (!fn) {
            send_str(client, make_response(503, "Service Unavailable",
                                           "text/plain", "list_models handler not registered",
                                           cors));
        } else {
            const std::string body = models_to_json(fn());
            send_str(client, make_response(200, "OK", "application/json", body, cors));
        }
        ROC_CLOSESOCKET(client);
        return;
    }

    if (path == "/api/model/active" && method == "GET") {
        std::function<std::string()> fn;
        {
            std::lock_guard<std::mutex> lock(impl->api_mu);
            fn = impl->api.active_model_json;
        }
        if (!fn) {
            send_str(client, make_response(503, "Service Unavailable",
                                           "text/plain", "active_model_json handler not registered",
                                           cors));
        } else {
            send_str(client, make_response(200, "OK", "application/json", fn(), cors));
        }
        ROC_CLOSESOCKET(client);
        return;
    }

    if (path == "/api/model/active" && method == "POST") {
        std::function<std::string(const std::string&)> fn;
        {
            std::lock_guard<std::mutex> lock(impl->api_mu);
            fn = impl->api.switch_model;
        }
        if (!fn) {
            send_str(client, make_response(503, "Service Unavailable",
                                           "text/plain", "switch_model handler not registered",
                                           cors));
        } else {
            const std::string body_in = read_post_body(client, req);
            const std::string name    = extract_json_string_field(body_in, "name");
            if (name.empty()) {
                send_str(client, make_response(400, "Bad Request",
                                               "text/plain", "missing 'name' field in JSON body",
                                               cors));
            } else {
                const std::string result_json = fn(name);
                send_str(client, make_response(200, "OK", "application/json", result_json, cors));
            }
        }
        ROC_CLOSESOCKET(client);
        return;
    }

    // ---- Phase 8: capture-source enumeration ------------------------------
    if (path == "/api/capture-sources" && method == "GET") {
        std::function<CaptureSourcesSnapshot()> fn;
        {
            std::lock_guard<std::mutex> lock(impl->api_mu);
            fn = impl->api.list_capture_sources;
        }
        if (!fn) {
            send_str(client, make_response(503, "Service Unavailable",
                                           "text/plain",
                                           "list_capture_sources handler not registered",
                                           cors));
        } else {
            send_str(client, make_response(200, "OK", "application/json",
                                           capture_sources_to_json(fn()), cors));
        }
        ROC_CLOSESOCKET(client);
        return;
    }

    // ---- Phase 6: FOV crop ------------------------------------------------
    if (path == "/api/fov" && method == "GET") {
        std::function<FovSettings()> fn;
        {
            std::lock_guard<std::mutex> lock(impl->api_mu);
            fn = impl->api.get_fov;
        }
        if (!fn) {
            send_str(client, make_response(503, "Service Unavailable",
                                           "text/plain", "get_fov handler not registered",
                                           cors));
        } else {
            send_str(client, make_response(200, "OK", "application/json",
                                           fov_to_json(fn()), cors));
        }
        ROC_CLOSESOCKET(client);
        return;
    }

    if (path == "/api/fov" && method == "POST") {
        std::function<void(const FovSettings&)>  setter;
        std::function<FovSettings()>             getter;
        {
            std::lock_guard<std::mutex> lock(impl->api_mu);
            setter = impl->api.set_fov;
            getter = impl->api.get_fov;
        }
        if (!setter || !getter) {
            send_str(client, make_response(503, "Service Unavailable",
                                           "text/plain", "fov handlers not registered",
                                           cors));
        } else {
            const std::string body_in = read_post_body(client, req);
            FovSettings wanted = getter(); // start from current, merge requested fields
            double d = 0.0;
            bool   b = false;
            if (extract_json_bool_field  (body_in, "enabled",        b)) wanted.enabled = b;
            if (extract_json_bool_field  (body_in, "followTarget",   b)) wanted.follow_target = b;
            if (extract_json_number_field(body_in, "cxNorm",         d)) wanted.cx_norm = static_cast<float>(d);
            if (extract_json_number_field(body_in, "cyNorm",         d)) wanted.cy_norm = static_cast<float>(d);
            if (extract_json_number_field(body_in, "radiusPx",       d)) wanted.radius_px = static_cast<float>(d);
            setter(wanted);
            send_str(client, make_response(200, "OK", "application/json",
                                           fov_to_json(getter()), cors));
        }
        ROC_CLOSESOCKET(client);
        return;
    }

    if (path == "/health") {
        const std::string body = "ok";
        std::ostringstream resp;
        resp << "HTTP/1.1 200 OK\r\n"
             << cors
             << "Content-Type: text/plain\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n\r\n"
             << body;
        send_str(client, resp.str());
        ROC_CLOSESOCKET(client);
        return;
    }

    if (path == "/sources") {
        std::string body;
        {
            std::lock_guard<std::mutex> lock(impl->sources_mu);
            body = sources_to_json(impl->sources);
        }
        std::ostringstream resp;
        resp << "HTTP/1.1 200 OK\r\n"
             << cors
             << "Content-Type: application/json\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n\r\n"
             << body;
        send_str(client, resp.str());
        ROC_CLOSESOCKET(client);
        return;
    }

    const int cam_index = parse_camera_index_from_mjpeg_path(path);
    if (cam_index < 0) {
        const std::string body = "404 not found";
        std::ostringstream resp;
        resp << "HTTP/1.1 404 Not Found\r\n"
             << cors
             << "Content-Type: text/plain\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n\r\n"
             << body;
        send_str(client, resp.str());
        ROC_CLOSESOCKET(client);
        return;
    }

    // MJPEG streaming response. multipart/x-mixed-replace is universally
    // understood by browsers as a video-like image source.
    const std::string header =
        "HTTP/1.1 200 OK\r\n"
        + cors +
        "Content-Type: multipart/x-mixed-replace; boundary=rocframe\r\n"
        "Cache-Control: no-cache, private\r\n"
        "Pragma: no-cache\r\n"
        "Connection: close\r\n\r\n"
        "--rocframe\r\n";
    if (!send_str(client, header)) {
        ROC_CLOSESOCKET(client);
        return;
    }

    impl->mjpeg_clients.fetch_add(1, std::memory_order_relaxed);
    auto decrement_on_exit = std::shared_ptr<void>(nullptr, [impl](void*) {
        impl->mjpeg_clients.fetch_sub(1, std::memory_order_relaxed);
    });

    auto& slot = impl->slot_for(cam_index);
    std::uint64_t last_seen_generation = 0;

    while (server->is_running()) {
        std::vector<std::uint8_t> snapshot;
        {
            std::unique_lock<std::mutex> lock(slot.mu);
            slot.cv.wait_for(lock, std::chrono::milliseconds(250), [&]() {
                return !server->is_running() ||
                       slot.generation != last_seen_generation;
            });
            if (!server->is_running()) break;
            if (slot.generation == last_seen_generation) continue;
            last_seen_generation = slot.generation;
            snapshot = slot.jpeg_bytes;
        }

        std::ostringstream part;
        part << "Content-Type: image/jpeg\r\n"
             << "Content-Length: " << snapshot.size() << "\r\n\r\n";
        const std::string part_header = part.str();
        if (!send_all(client, part_header.data(), part_header.size())) break;
        if (!send_all(client, reinterpret_cast<const char*>(snapshot.data()), snapshot.size())) break;
        const char boundary_trailer[] = "\r\n--rocframe\r\n";
        if (!send_all(client, boundary_trailer, sizeof(boundary_trailer) - 1)) break;
    }

    ROC_CLOSESOCKET(client);
}

}  // namespace transport
}  // namespace roc
