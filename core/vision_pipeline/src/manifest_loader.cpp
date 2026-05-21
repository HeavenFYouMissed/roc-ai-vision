// =============================================================================
// roc::vision::ManifestLoader — YAML manifest parser (§3.3 schema, §3.4 rules).
//
// Implementation notes:
//   * yaml-cpp is the parser.  We catch ALL yaml-cpp exceptions and turn them
//     into nullopt + error string (no exceptions ever leave ManifestLoader).
//   * We accept the canonical strings only — typos are rejected with an
//     informative error rather than silently defaulting (matches the binding
//     spec's explicit-configuration mandate).
//   * Numeric ranges:
//       - schema_version must equal 1 (forward-compat hook for future
//         schema bumps; §3.3 declares "v1" explicitly).
//       - input.expected_shape must be a 4-vector of positive ints with C ≥ 1.
//       - output.expected_shape must be a 3-vector; dynamic axes encoded as -1
//         are accepted (the YAML schema example uses -1 for the anchor count).
//       - num_classes >= 0; classes vector size may exceed num_classes only
//         when the YAML deliberately overloads (rejected at §3.4 #3).
//       - thresholds.score in [0, 1] inclusive of 0 (no detection floor) and
//         exclusive of values > 1.
// =============================================================================

#include "roc/vision/manifest_loader.hpp"

#include <yaml-cpp/yaml.h>

#include <array>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>

namespace roc {
namespace vision {

namespace {

// Local helper — append `msg` to `*err` if non-null, separated by "; ".
void write_err(std::string* err, const std::string& msg) {
    if (!err) return;
    if (err->empty()) {
        *err = msg;
    } else {
        err->append("; ").append(msg);
    }
}

bool require_node(const YAML::Node& parent, const std::string& key, std::string* err) {
    if (!parent[key]) {
        write_err(err, "missing required key '" + key + "'");
        return false;
    }
    return true;
}

bool parse_layout(const std::string& s, TensorLayout& out, std::string* err) {
    if (s == "NCHW") { out = TensorLayout::NCHW; return true; }
    if (s == "NHWC") { out = TensorLayout::NHWC; return true; }
    write_err(err, "unknown input.layout '" + s + "' (expected NCHW or NHWC)");
    return false;
}

bool parse_channel_order(const std::string& s, ChannelOrder& out, std::string* err) {
    if (s == "RGB") { out = ChannelOrder::RGB; return true; }
    if (s == "BGR") { out = ChannelOrder::BGR; return true; }
    write_err(err, "unknown input.channel_order '" + s + "' (expected RGB or BGR)");
    return false;
}

bool parse_dtype(const std::string& s, TensorDType& out, std::string* err) {
    if (s == "float32") { out = TensorDType::Float32; return true; }
    if (s == "float16") { out = TensorDType::Float16; return true; }
    if (s == "uint8")   { out = TensorDType::Uint8;   return true; }
    write_err(err, "unknown input.dtype '" + s + "' (expected float32/float16/uint8)");
    return false;
}

bool parse_pad_to(const std::string& s, PadTo& out, std::string* err) {
    if (s == "model_input") { out = PadTo::ModelInput; return true; }
    if (s == "square")      { out = PadTo::Square;     return true; }
    if (s == "none")        { out = PadTo::None;       return true; }
    write_err(err, "unknown preprocessing.pad_to '" + s + "'");
    return false;
}

bool parse_head(const std::string& s, OutputHead& out, std::string* err) {
    if (s == "yolo26_detect") { out = OutputHead::Yolo26Detect; return true; }
    if (s == "osnet_reid")    { out = OutputHead::OsnetReid;    return true; }
    if (s == "yolov8_detect") { out = OutputHead::Yolov8Detect; return true; }
    write_err(err, "unknown output.head '" + s + "' (no postprocessor registered)");
    return false;
}

bool parse_format(const std::string& s, OutputFormat& out, std::string* err) {
    if (s == "cxcywh_logits")  { out = OutputFormat::CxCyWhLogits;  return true; }
    if (s == "xyxy_sigmoid")   { out = OutputFormat::XyxySigmoid;   return true; }
    if (s == "xyxy_post_nms")  { out = OutputFormat::XyxyPostNms;   return true; }
    if (s == "reid_embedding") { out = OutputFormat::ReidEmbedding; return true; }
    if (s == "cxcywh_sigmoid") { out = OutputFormat::CxCyWhSigmoid; return true; }
    write_err(err, "unknown output.format '" + s + "'");
    return false;
}

bool parse_ep(const std::string& s, EpKind& out, std::string* err) {
    if (s == "DirectML") { out = EpKind::DirectML; return true; }
    if (s == "OpenVINO") { out = EpKind::OpenVINO; return true; }
    if (s == "Cpu")      { out = EpKind::Cpu;      return true; }
    if (s == "Cuda")     { out = EpKind::Cuda;     return true; }
    write_err(err, "unknown runtime_hints.preferred_ep '" + s + "'");
    return false;
}

bool parse_precision(const std::string& s, Precision& out, std::string* err) {
    if (s == "fp32") { out = Precision::Fp32; return true; }
    if (s == "fp16") { out = Precision::Fp16; return true; }
    if (s == "int8") { out = Precision::Int8; return true; }
    write_err(err, "unknown runtime_hints.precision '" + s + "'");
    return false;
}

// Read a fixed-size integer sequence; emits a descriptive error on shape mismatch.
template <std::size_t N>
bool read_int_sequence(const YAML::Node&    node,
                       const std::string&   key_for_errors,
                       std::array<int64_t, N>& out,
                       bool                 allow_negative,
                       std::string*         err) {
    if (!node.IsSequence() || node.size() != N) {
        std::ostringstream oss;
        oss << key_for_errors << " must be a sequence of length " << N;
        write_err(err, oss.str());
        return false;
    }
    for (std::size_t i = 0; i < N; ++i) {
        try {
            int64_t v = node[i].as<int64_t>();
            if (!allow_negative && v <= 0) {
                std::ostringstream oss;
                oss << key_for_errors << "[" << i << "] must be positive (got " << v << ")";
                write_err(err, oss.str());
                return false;
            }
            out[i] = v;
        } catch (const YAML::Exception& e) {
            write_err(err, key_for_errors + ": " + e.what());
            return false;
        }
    }
    return true;
}

template <std::size_t N>
bool read_double_sequence(const YAML::Node&    node,
                          const std::string&   key_for_errors,
                          std::array<double, N>& out,
                          std::string*         err) {
    if (!node.IsSequence() || node.size() != N) {
        std::ostringstream oss;
        oss << key_for_errors << " must be a sequence of length " << N;
        write_err(err, oss.str());
        return false;
    }
    for (std::size_t i = 0; i < N; ++i) {
        try {
            out[i] = node[i].as<double>();
        } catch (const YAML::Exception& e) {
            write_err(err, key_for_errors + ": " + e.what());
            return false;
        }
    }
    return true;
}

}  // namespace

std::optional<ManifestSpec>
ManifestLoader::load(const std::filesystem::path& yaml_path, std::string* err_out) {
    if (err_out) err_out->clear();

    std::ifstream ifs(yaml_path);
    if (!ifs.is_open()) {
        write_err(err_out, "cannot open manifest file: " + yaml_path.string());
        return std::nullopt;
    }
    std::ostringstream buf;
    buf << ifs.rdbuf();
    return load_from_string(buf.str(), err_out);
}

std::optional<ManifestSpec>
ManifestLoader::load_from_string(const std::string& yaml_text, std::string* err_out) {
    if (err_out) err_out->clear();
    ManifestSpec spec;

    YAML::Node root;
    try {
        root = YAML::Load(yaml_text);
    } catch (const YAML::Exception& e) {
        write_err(err_out, std::string("YAML parse error: ") + e.what());
        return std::nullopt;
    }

    if (!root || !root.IsMap()) {
        write_err(err_out, "manifest root must be a mapping");
        return std::nullopt;
    }

    // §3.3 schema_version ----------------------------------------------------
    if (!require_node(root, "schema_version", err_out)) return std::nullopt;
    try {
        spec.schema_version = root["schema_version"].as<int>();
    } catch (const YAML::Exception& e) {
        write_err(err_out, std::string("schema_version: ") + e.what());
        return std::nullopt;
    }
    if (spec.schema_version != 1) {
        std::ostringstream oss;
        oss << "schema_version " << spec.schema_version << " unsupported (this loader speaks v1)";
        write_err(err_out, oss.str());
        return std::nullopt;
    }

    // Identification ---------------------------------------------------------
    if (!require_node(root, "name",    err_out)) return std::nullopt;
    if (!require_node(root, "purpose", err_out)) return std::nullopt;
    try {
        spec.name       = root["name"].as<std::string>();
        spec.purpose    = root["purpose"].as<std::string>();
        if (root["trained_on"]) spec.trained_on = root["trained_on"].as<std::string>();
        if (root["notes"])      spec.notes      = root["notes"].as<std::string>();
    } catch (const YAML::Exception& e) {
        write_err(err_out, std::string("identification fields: ") + e.what());
        return std::nullopt;
    }

    // Input contract ---------------------------------------------------------
    if (!require_node(root, "input", err_out)) return std::nullopt;
    {
        const YAML::Node in = root["input"];
        if (!in.IsMap()) { write_err(err_out, "input must be a mapping"); return std::nullopt; }
        for (const char* k : { "layout", "channel_order", "expected_shape", "dtype", "normalization" }) {
            if (!require_node(in, k, err_out)) return std::nullopt;
        }
        try {
            if (!parse_layout(in["layout"].as<std::string>(), spec.input.layout, err_out)) return std::nullopt;
            if (!parse_channel_order(in["channel_order"].as<std::string>(), spec.input.channel_order, err_out)) return std::nullopt;
            if (!read_int_sequence<4>(in["expected_shape"], "input.expected_shape",
                                       spec.input.expected_shape, /*allow_negative=*/true, err_out)) return std::nullopt;
            // Shape semantic checks: B must be >= 1; C, H, W must be positive (no dynamic
            // axes on input — the engine cross-checks introspection against this).
            if (spec.input.expected_shape[0] < 1 ||
                spec.input.expected_shape[1] <= 0 ||
                spec.input.expected_shape[2] <= 0 ||
                spec.input.expected_shape[3] <= 0) {
                write_err(err_out, "input.expected_shape must have B>=1 and C/H/W>0");
                return std::nullopt;
            }
            if (!parse_dtype(in["dtype"].as<std::string>(), spec.input.dtype, err_out)) return std::nullopt;

            const YAML::Node nrm = in["normalization"];
            if (!nrm.IsMap()) { write_err(err_out, "input.normalization must be a mapping"); return std::nullopt; }
            for (const char* k : { "scale", "mean", "std" }) {
                if (!require_node(nrm, k, err_out)) return std::nullopt;
            }
            spec.input.normalization.scale = nrm["scale"].as<double>();
            if (!read_double_sequence<3>(nrm["mean"], "input.normalization.mean",
                                          spec.input.normalization.mean, err_out)) return std::nullopt;
            if (!read_double_sequence<3>(nrm["std"], "input.normalization.std",
                                          spec.input.normalization.std,  err_out)) return std::nullopt;
            for (double s : spec.input.normalization.std) {
                if (s == 0.0) { write_err(err_out, "input.normalization.std contains 0"); return std::nullopt; }
            }

            if (in["preprocessing"]) {
                const YAML::Node pp = in["preprocessing"];
                if (!pp.IsMap()) { write_err(err_out, "input.preprocessing must be a mapping"); return std::nullopt; }
                if (pp["letterbox"]) spec.input.preprocessing.letterbox = pp["letterbox"].as<bool>();
                if (pp["pad_value"]) {
                    std::array<int64_t, 3> tmp{};
                    if (!read_int_sequence<3>(pp["pad_value"], "input.preprocessing.pad_value",
                                               tmp, /*allow_negative=*/false, err_out)) return std::nullopt;
                    spec.input.preprocessing.pad_value = {
                        static_cast<int>(tmp[0]), static_cast<int>(tmp[1]), static_cast<int>(tmp[2])
                    };
                }
                if (pp["pad_to"]) {
                    if (!parse_pad_to(pp["pad_to"].as<std::string>(), spec.input.preprocessing.pad_to, err_out)) return std::nullopt;
                }
            }
        } catch (const YAML::Exception& e) {
            write_err(err_out, std::string("input section: ") + e.what());
            return std::nullopt;
        }
    }

    // Output contract --------------------------------------------------------
    if (!require_node(root, "output", err_out)) return std::nullopt;
    {
        const YAML::Node out = root["output"];
        if (!out.IsMap()) { write_err(err_out, "output must be a mapping"); return std::nullopt; }
        for (const char* k : { "head", "format", "expected_shape" }) {
            if (!require_node(out, k, err_out)) return std::nullopt;
        }
        try {
            spec.head_string = out["head"].as<std::string>();
            if (!parse_head(spec.head_string,                spec.output.head,   err_out)) return std::nullopt;
            if (!parse_format(out["format"].as<std::string>(), spec.output.format, err_out)) return std::nullopt;
            if (!read_int_sequence<3>(out["expected_shape"], "output.expected_shape",
                                       spec.output.expected_shape, /*allow_negative=*/true, err_out)) return std::nullopt;
            if (out["num_classes"]) spec.output.num_classes = out["num_classes"].as<int>();
            if (out["nms_in_graph"]) spec.output.nms_in_graph = out["nms_in_graph"].as<bool>();
            if (out["thresholds"]) {
                const YAML::Node th = out["thresholds"];
                if (th["score"]) {
                    spec.output.thresholds.score = th["score"].as<double>();
                    if (spec.output.thresholds.score < 0.0 || spec.output.thresholds.score > 1.0) {
                        write_err(err_out, "output.thresholds.score must lie in [0, 1]");
                        return std::nullopt;
                    }
                }
                if (th["iou"]) {
                    double v = th["iou"].as<double>();
                    if (v < 0.0 || v > 1.0) {
                        write_err(err_out, "output.thresholds.iou must lie in [0, 1]");
                        return std::nullopt;
                    }
                    spec.output.thresholds.iou = v;
                }
            }
        } catch (const YAML::Exception& e) {
            write_err(err_out, std::string("output section: ") + e.what());
            return std::nullopt;
        }
    }

    // Runtime hints ----------------------------------------------------------
    if (root["runtime_hints"]) {
        const YAML::Node rh = root["runtime_hints"];
        if (!rh.IsMap()) { write_err(err_out, "runtime_hints must be a mapping"); return std::nullopt; }
        try {
            if (rh["preferred_ep"]) {
                if (!parse_ep(rh["preferred_ep"].as<std::string>(),
                              spec.runtime_hints.preferred_ep, err_out)) return std::nullopt;
            }
            if (rh["precision"]) {
                if (!parse_precision(rh["precision"].as<std::string>(),
                                     spec.runtime_hints.precision, err_out)) return std::nullopt;
            }
            if (rh["cache_compiled"]) spec.runtime_hints.cache_compiled = rh["cache_compiled"].as<bool>();
            if (rh["batchable"])      spec.runtime_hints.batchable      = rh["batchable"].as<bool>();
        } catch (const YAML::Exception& e) {
            write_err(err_out, std::string("runtime_hints: ") + e.what());
            return std::nullopt;
        }
    }

    // Classes table ----------------------------------------------------------
    if (root["classes"]) {
        const YAML::Node cls = root["classes"];
        if (!cls.IsSequence()) { write_err(err_out, "classes must be a sequence"); return std::nullopt; }
        spec.classes.reserve(cls.size());
        try {
            for (std::size_t i = 0; i < cls.size(); ++i) {
                const YAML::Node& c = cls[i];
                ClassMapping cm;
                cm.id        = c["id"].as<int>();
                cm.name      = c["name"].as<std::string>();
                const std::string lbl = c["roc_label"].as<std::string>();
                cm.roc_label = label_from_string(lbl);
                // §3.4 #4 — roc_label MUST resolve to a known Label.  Map both
                // "Unknown" (sentinel) and any typo to a rejection here.
                if (cm.roc_label == Label::Unknown && lbl != "Unknown") {
                    write_err(err_out, "classes[" + std::to_string(i) +
                                       "].roc_label='" + lbl + "' is not a roc::kinematics::Label");
                    return std::nullopt;
                }
                spec.classes.push_back(cm);
            }
        } catch (const YAML::Exception& e) {
            write_err(err_out, std::string("classes: ") + e.what());
            return std::nullopt;
        }
    }

    // ----------------- Post-parse cross-validation (§3.4 #3, #5) ------------
    // §3.4 #3 — class ids in [0, num_classes) for every detection head.
    const bool is_detection_head =
        spec.output.head == OutputHead::Yolo26Detect ||
        spec.output.head == OutputHead::Yolov8Detect;
    if (is_detection_head) {
        if (spec.output.num_classes <= 0) {
            write_err(err_out, "detection head requires output.num_classes > 0");
            return std::nullopt;
        }
        for (const auto& c : spec.classes) {
            if (c.id < 0 || c.id >= spec.output.num_classes) {
                std::ostringstream oss;
                oss << "class id " << c.id << " out of range [0, " << spec.output.num_classes << ")";
                write_err(err_out, oss.str());
                return std::nullopt;
            }
        }
    }

    // §3.4 #5 — INT8 + non-OpenVINO warns (we don't fail; we just record).
    // (Logging is deferred to the engine which has access to its logger.)

    return spec;
}

}  // namespace vision
}  // namespace roc
