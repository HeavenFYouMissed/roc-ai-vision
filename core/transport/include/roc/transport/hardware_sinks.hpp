#pragma once

// Phase 6 part B — hardware OutputSink stubs (COM3 HID, Dynamixel, Modbus).
// Full serial I/O lands when the 5080 desktop is available for bench testing.

#include "roc/transport/output_sink.hpp"

namespace roc {
namespace transport {

class HidMouseSink final : public OutputSink {
public:
    void emit(const ActuatorEmission& e) noexcept override {
        if (!e.valid) return;
        ++emissions_;
    }
    OutputSinkKind kind() const noexcept override { return OutputSinkKind::HidMouse; }
    const char* name() const noexcept override { return "HidMouse"; }
    std::uint64_t emissions() const noexcept { return emissions_; }

private:
    std::uint64_t emissions_ = 0;
};

class DynamixelServoSink final : public OutputSink {
public:
    void emit(const ActuatorEmission& e) noexcept override {
        if (!e.valid) return;
        ++emissions_;
    }
    OutputSinkKind kind() const noexcept override {
        return OutputSinkKind::DynamixelServo;
    }
    const char* name() const noexcept override { return "DynamixelServo"; }
    std::uint64_t emissions() const noexcept { return emissions_; }

private:
    std::uint64_t emissions_ = 0;
};

class ModbusSink final : public OutputSink {
public:
    void emit(const ActuatorEmission& e) noexcept override {
        if (!e.valid) return;
        ++emissions_;
    }
    OutputSinkKind kind() const noexcept override { return OutputSinkKind::Modbus; }
    const char* name() const noexcept override { return "Modbus"; }
    std::uint64_t emissions() const noexcept { return emissions_; }

private:
    std::uint64_t emissions_ = 0;
};

}  // namespace transport
}  // namespace roc
