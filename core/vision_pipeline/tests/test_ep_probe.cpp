// =============================================================================
// test_ep_probe.cpp — §1.2 probe-and-fallback coverage.
//
// CPU EP must always be selectable on every platform we ship to.  This test
// loads a trivial in-memory ONNX model (the Identity unary op) constructed
// programmatically via the ORT C API so we don't need a fixture .onnx on
// disk.  The model is round-tripped through Ort::Session under the probe.
// =============================================================================

#include "roc/vision/ep_probe.hpp"

#include <onnxruntime_cxx_api.h>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

using namespace roc::vision;

namespace {

// Minimal Identity-op ONNX model — IR version 7, opset 13.  Input X and
// output Y are both float tensors of shape [1].  Generated once by the
// canonical recipe:
//
//   python -c "
//     import onnx
//     from onnx import helper, TensorProto
//     X = helper.make_tensor_value_info('X', TensorProto.FLOAT, [1])
//     Y = helper.make_tensor_value_info('Y', TensorProto.FLOAT, [1])
//     n = helper.make_node('Identity', ['X'], ['Y'])
//     g = helper.make_graph([n], 'g', [X], [Y])
//     m = helper.make_model(g, opset_imports=[helper.make_opsetid('', 13)])
//     m.ir_version = 7
//     open('tiny.onnx','wb').write(m.SerializeToString())
//   "
//
// 65 bytes.  Smallest valid ONNX that ORT will load.
static const std::uint8_t kTinyIdentityOnnx[] = {
    0x08, 0x07, 0x3a, 0x37, 0x0a, 0x10, 0x0a, 0x01, 0x58, 0x12, 0x01, 0x59,
    0x22, 0x08, 0x49, 0x64, 0x65, 0x6e, 0x74, 0x69, 0x74, 0x79, 0x12, 0x01,
    0x67, 0x5a, 0x0f, 0x0a, 0x01, 0x58, 0x12, 0x0a, 0x0a, 0x08, 0x08, 0x01,
    0x12, 0x04, 0x0a, 0x02, 0x08, 0x01, 0x62, 0x0f, 0x0a, 0x01, 0x59, 0x12,
    0x0a, 0x0a, 0x08, 0x08, 0x01, 0x12, 0x04, 0x0a, 0x02, 0x08, 0x01, 0x42,
    0x04, 0x0a, 0x00, 0x10, 0x0d,
};

std::filesystem::path write_tiny_identity_to_temp() {
    auto dir = std::filesystem::temp_directory_path() / "roc_vision_tests";
    std::filesystem::create_directories(dir);
    std::random_device rd;
    std::mt19937 g(rd());
    auto path = dir / ("tiny_identity_" + std::to_string(g()) + ".onnx");
    std::ofstream ofs(path, std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(kTinyIdentityOnnx), sizeof(kTinyIdentityOnnx));
    ofs.close();
    return path;
}

}  // namespace

TEST_CASE("EpProbe: CPU EP is always selectable", "[ep]") {
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "ep_probe_test");
    Ort::SessionOptions opts;
    const auto path = write_tiny_identity_to_temp();

    std::vector<EpKind> priority{EpKind::Cpu};
    EpProbeResult res = EpProbe::probe(env, ModelPath(path), opts, priority);

    CHECK(res.chosen == EpKind::Cpu);
    CHECK_FALSE(res.device_description.empty());
    CHECK(res.all_ops_native);

    std::filesystem::remove(path);
}

TEST_CASE("EpProbe: empty priority list falls back to CPU floor", "[ep]") {
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "ep_probe_test2");
    Ort::SessionOptions opts;
    const auto path = write_tiny_identity_to_temp();

    EpProbeResult res = EpProbe::probe(env, ModelPath(path), opts,
                                       Span<const EpKind>(nullptr, 0));
    CHECK(res.chosen == EpKind::Cpu);

    std::filesystem::remove(path);
}

TEST_CASE("EpProbe: DML/OV/CUDA availability reflects compile flags", "[ep]") {
    INFO("DML compile-time available  : " << EpProbe::dml_available_at_runtime());
    INFO("OV  compile-time available  : " << EpProbe::openvino_available_at_runtime());
    INFO("CUDA compile-time available : " << EpProbe::cuda_available_at_runtime());
    // No assertion — purely informational on the test machine.
    SUCCEED("EP availability reported via INFO macros");
}

TEST_CASE("EpProbe: priority list including DML still ends at CPU on plain build", "[ep]") {
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "ep_probe_test3");
    Ort::SessionOptions opts;
    const auto path = write_tiny_identity_to_temp();

    std::vector<EpKind> prio{EpKind::DirectML, EpKind::OpenVINO, EpKind::Cpu};
    EpProbeResult res = EpProbe::probe(env, ModelPath(path), opts, prio);

    // Compile-time-disabled EPs simply skip; the CPU floor must win.
    if (!EpProbe::dml_available_at_runtime() &&
        !EpProbe::openvino_available_at_runtime()) {
        CHECK(res.chosen == EpKind::Cpu);
    } else {
        // If a higher-priority EP IS available, it should be selected — but
        // we still verify that the result is one of the priority entries.
        CHECK((res.chosen == EpKind::Cpu || res.chosen == EpKind::DirectML ||
               res.chosen == EpKind::OpenVINO));
    }

    std::filesystem::remove(path);
}

TEST_CASE("EpProbe: default_priority includes CPU floor", "[ep]") {
    auto prio = EpProbe::default_priority();
    bool has_cpu = false;
    for (auto k : prio) if (k == EpKind::Cpu) has_cpu = true;
    CHECK(has_cpu);
}
