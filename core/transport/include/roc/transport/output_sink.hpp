#pragma once

// =============================================================================
// roc::transport::output_sink
//
// Phase 6 part A: §11.4 Output-Sink adapter base class.  Specified in
// `workspace_blueprint/research_notes/dashboard_menu_architecture_2026.md`
// §11.4 ("dual-use OutputSink adapter base class — NoOp / HidMouse /
// DynamixelServo / PwmPanTilt / Modbus / LogfileCsv").
//
// Phase 6 part A ships ONLY the abstract base + the `NoOpSink` concrete.  The
// hardware sinks (HID-mouse over CDC-ACM, Dynamixel Protocol 2.0, Modbus RTU)
// land in Phase 6 part B once the 5080 desktop is back and we can hook the
// host to a serial-attached actuator without risking the laptop's USB stack.
//
// Wire-vocabulary constraint (per §11.4 + §12.4 risk A3): the field names
// here use the sanitised vocabulary — `command_dx_per_tick`, `target_track_id`
// — NEVER `mouse_*`, `aim_*`, `cheat_*` etc.  The forbidden-vocabulary CI
// lint will run against this file in Phase 5c.
//
// REPLAY-mode hard-disable (§11.6 risk A5): every concrete `OutputSink::emit`
// implementation MUST be wrapped at its call site with a state check that
// drops the emission when the dashboard is in REPLAY mode.  The base class
// cannot enforce this on its own — it lives at the orchestrator's
// actuator-thread call site.  The base class only exposes the contract.
// =============================================================================

#include <cstdint>

namespace roc {
namespace transport {

// `OutputSinkKind` — mirrored into the §12.4 actuator-telemetry wire frame's
// `sink_kind` u16 field.  Enumerators are explicit-valued so the wire
// constant stays stable across refactors.
enum class OutputSinkKind : std::uint16_t {
    NoOp           = 0,
    HidMouse       = 1,
    DynamixelServo = 2,
    PwmPanTilt     = 3,
    Modbus         = 4,
    LogfileCsv     = 5,
};

// `ActuatorEmission` — the unit datum every concrete sink consumes.
// Image-plane pixel coordinates referenced to the selector's `(cx, cy)`
// origin (§2.6.4 of `phase_2_ekf_kinematics.md`).
struct ActuatorEmission {
    std::uint64_t target_track_id = 0;     // 0 == no lock; sink parks
    float         command_u_px    = 0.0f;  // absolute u in capture-frame pixels
    float         command_v_px    = 0.0f;  // absolute v in capture-frame pixels
    float         delta_u_per_tick = 0.0f; // u-axis incremental command
    float         delta_v_per_tick = 0.0f; // v-axis incremental command
    std::int64_t  emit_ts_ns      = 0;
    bool          valid           = false; // false ⇒ no target ⇒ skip emission
};

// `OutputSink` — abstract strategy.  Concrete implementations live in their
// own one-file ≤ 250 LOC units (§11.6 risk A4).
class OutputSink {
public:
    virtual ~OutputSink() = default;

    // Apply one actuator emission.  May fail silently (logged); never throws.
    virtual void emit(const ActuatorEmission& e) noexcept = 0;

    // Identification — mirrored into the wire frame and the masthead SINK
    // badge (§11.6 risk A6).
    virtual OutputSinkKind kind() const noexcept = 0;
    virtual const char*    name() const noexcept = 0;
};

// `NoOpSink` — Phase 6 part A demo sink.  Stateful only by emission count so
// the orchestrator can prove the actuator thread is doing work without
// touching any hardware.  Used by the laptop-webcam bring-up + by any
// scenario that wants to record actuator telemetry without driving a sink.
class NoOpSink final : public OutputSink {
public:
    NoOpSink() noexcept = default;

    void emit(const ActuatorEmission& e) noexcept override {
        if (!e.valid) return;
        ++emissions_;
        last_command_u_px_  = e.command_u_px;
        last_command_v_px_  = e.command_v_px;
        last_emit_ts_ns_    = e.emit_ts_ns;
        last_target_track_  = e.target_track_id;
    }

    OutputSinkKind kind() const noexcept override { return OutputSinkKind::NoOp; }
    const char*    name() const noexcept override { return "NoOp"; }

    std::uint64_t emissions()           const noexcept { return emissions_; }
    float         last_command_u_px()   const noexcept { return last_command_u_px_; }
    float         last_command_v_px()   const noexcept { return last_command_v_px_; }
    std::int64_t  last_emit_ts_ns()     const noexcept { return last_emit_ts_ns_; }
    std::uint64_t last_target_track_id() const noexcept { return last_target_track_; }

private:
    std::uint64_t emissions_         = 0;
    float         last_command_u_px_ = 0.0f;
    float         last_command_v_px_ = 0.0f;
    std::int64_t  last_emit_ts_ns_   = 0;
    std::uint64_t last_target_track_ = 0;
};

}  // namespace transport
}  // namespace roc
