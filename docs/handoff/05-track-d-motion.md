# 05 — Track D: Jerk-limited motion profiles (Ruckig)

## Why

Any physical actuator (the Ender-3 spatula in Track C, any future robotic
arm, any servo-driven gimbal) **must** receive jerk-limited motion
commands. Sending raw step-to-target moves causes:

- Stepper motor step-loss (the motor physically can't keep up with the
  commanded acceleration and silently skips)
- Mechanical resonance, belt slap, audible bangs on rail endstops
- Premature wear on belts, bearings, lead screws, gears
- Marlin firmware thermal-runaway false positives (current spikes look
  like a short circuit)
- On a servo: controller overcurrent fault

This is standard industrial control-loop math: convert a "target
position" into a smooth time-sequenced trajectory of `(position,
velocity, acceleration)` waypoints that respect the actuator's
mechanical limits.

## Library: Ruckig (BSD-3)

https://github.com/pantor/ruckig — open-source, BSD-3 licensed,
industrial-grade jerk-limited online trajectory generator. Used in
real robotics products (Franka Emika, etc.). Header-only C++17.

The community edition (BSD-3) covers our use case completely: single-DOF
and multi-DOF jerk-limited profiles, runtime parameter changes, online
re-planning when the target moves mid-trajectory. The commercial Pro
edition adds features we don't need.

## Concrete deliverable

A `MotionProfileGenerator` class that wraps Ruckig and converts
detector-driven target positions into a stream of `(t, pos, vel, acc)`
waypoints suitable for whichever actuator sink consumes them.

### File targets

| File | Change |
|---|---|
| `core/kinematics_engine/CMakeLists.txt` | `FetchContent_Declare` Ruckig v0.14+ (BSD-3 community) |
| `core/kinematics_engine/include/roc/kinematics/motion_profile.hpp` | New header — `MotionProfileGenerator` class |
| `core/kinematics_engine/src/motion_profile.cpp` | Implementation — thin wrapper around `ruckig::Ruckig<NDOF>` |
| `core/kinematics_engine/tests/test_motion_profile.cpp` | Catch2 tests — verify jerk-limit respected, monotone time, settles to target |
| `core/transport/src/serial_marlin_sink.cpp` (Track C) | Consumes waypoints from `MotionProfileGenerator` → emits one `G1` per waypoint with `F{feedrate}` mapped from velocity |

### API shape

```cpp
namespace roc::kinematics {

struct MotionLimits {
    double max_velocity_mm_s    = 100.0;
    double max_acceleration_mm_s2 = 500.0;
    double max_jerk_mm_s3       = 5000.0;
};

struct MotionWaypoint {
    double t_s   = 0.0;   // seconds from trajectory start
    double pos_x_mm = 0.0;
    double pos_y_mm = 0.0;
    double vel_x_mm_s = 0.0;
    double vel_y_mm_s = 0.0;
    // (acc is computed by the sink if needed for feedrate mapping)
};

class MotionProfileGenerator {
public:
    explicit MotionProfileGenerator(const MotionLimits& limits);
    ~MotionProfileGenerator();

    // Plan a new trajectory from (current_pos, current_vel) to target_pos
    // with target_vel = 0 (settled stop). Returns the full waypoint list
    // at the given control_rate_hz (e.g. 50 Hz for Marlin).
    std::vector<MotionWaypoint> plan(
        double current_x_mm, double current_y_mm,
        double current_vx_mm_s, double current_vy_mm_s,
        double target_x_mm, double target_y_mm,
        double control_rate_hz);

    // Online re-plan: feed the latest tracker EKF position as the
    // target; Ruckig handles the smooth transition without violating
    // any limit. Returns the NEXT waypoint to emit, or nullopt if the
    // trajectory has settled at the current target.
    std::optional<MotionWaypoint> step(double dt_s,
                                       double live_target_x_mm,
                                       double live_target_y_mm);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    MotionLimits limits_;
};

}  // namespace roc::kinematics
```

### Wiring into Track C

`SerialMarlinSink::emit(const ActuatorCommand& cmd)` becomes:

```cpp
auto waypoints = motion_gen_.plan(
    last_emitted_x_, last_emitted_y_, 0.0, 0.0,
    cmd.target_x_mm, cmd.target_y_mm,
    /*control_rate_hz=*/50.0);
for (const auto& wp : waypoints) {
    const double speed_mm_min = std::hypot(wp.vel_x_mm_s, wp.vel_y_mm_s) * 60.0;
    char line[64];
    std::snprintf(line, sizeof(line), "G1 X%.3f Y%.3f F%.1f\n",
                  wp.pos_x_mm, wp.pos_y_mm, speed_mm_min);
    send_gcode(line);
}
last_emitted_x_ = waypoints.back().pos_x_mm;
last_emitted_y_ = waypoints.back().pos_y_mm;
```

## Default limits (safe Ender-3 starting point)

| Limit | Value | Justification |
|---|---|---|
| `max_velocity_mm_s` | 100 | Half of Marlin's default 200 — leaves headroom for transients |
| `max_acceleration_mm_s2` | 500 | Half of Marlin's default 1000 |
| `max_jerk_mm_s3` | 5000 | Marlin's "Junction Deviation" jerk default mapped to true jerk units |

Operator can raise these per-machine in `sorter_store.svelte.ts` after
they've characterized their specific rig (different print beds, belt
tensions, end-effector weights).

## Acceptance criteria

1. `test_motion_profile.cpp`: 10-meter target with limits above produces
   a trajectory where `max(|d³pos/dt³|) ≤ max_jerk`. Time-monotone.
2. `MotionProfileGenerator::step()` called at 50 Hz with a target that
   jumps mid-trajectory smoothly re-plans without exceeding limits.
3. On the actual Ender-3: silent operation at the default limits. Any
   audible "bang" on direction reversal = limits set too high; reduce.

## Reference

- Ruckig paper: Berscheid & Kröger 2021, "Jerk-limited Real-time
  Trajectory Generation with Arbitrary Target States"
  https://arxiv.org/abs/2105.04830
- Ruckig docs: https://docs.ruckig.com/
