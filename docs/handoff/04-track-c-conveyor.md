# 04 — Track C: Ender-3 conveyor pick-and-place sorter

## Why

The most defensible non-gaming application of the project. Industrial
pick-and-place sorting is a multi-billion-dollar CV market (Tomra,
AMP Robotics, Recycleye). The operator's setup:

- Ender-3 3D printer frame, repurposed as a Cartesian gantry
- Custom 3D-printed spatula end-effector (replaces the hotend)
- Belt-driven conveyor moves objects under the gantry
- Overhead camera looks down on the belt
- Detector classifies each object → spatula sweeps it into the
  matching bin alongside the belt

Every primitive already built in the project finds a legitimate concrete
home:

| Primitive | Conveyor-sorter use |
|---|---|
| ONNX detector | "What's on the belt right now?" |
| EKF tracker | Stable centroid as the object moves down the belt |
| FOV crop | Inspect only the belt region (not the floor / wall background) |
| Track IDs | "Object 47 is the one the spatula is going to grab" |
| `OutputSink` interface (currently `NoOpSink` only) | **First real concrete sink: `SerialMarlinSink` driving Marlin firmware over USB serial** |
| Engage destination's 4-light interlock | Finally has a real job — gates G-code emission until cam-online + tracker-online + serial-online + operator-armed |

## Concrete deliverable

A new `SorterDestination` in the dashboard (6th destination, scoped
explicitly to robotics), plus a `SerialMarlinSink` C++ class that
implements the existing `OutputSink` interface and speaks Marlin G-code
over USB serial.

### Phase C.1 — `SerialMarlinSink` (the actuator driver)

File targets:

| File | Change |
|---|---|
| `core/transport/include/roc/transport/serial_marlin_sink.hpp` | New header — implements `OutputSink`; ctor takes COM port + baud rate (typical Ender-3: COM3, 115200) |
| `core/transport/src/serial_marlin_sink.cpp` | New impl — Win32 `CreateFile("\\\\.\\COM3")` + `SetCommState` + `WriteFile` |
| `core/transport/CMakeLists.txt` | Add the new files to `_roc_transport_sources` |
| `core/transport/tests/test_serial_marlin_sink.cpp` | Unit test using a mock serial port (or skip-with-message if no COM port available) |

API shape:

```cpp
namespace roc::transport {

struct SerialMarlinConfig {
    std::string com_port      = "COM3";  // or "/dev/ttyUSB0" on Linux
    int         baud          = 115200;  // Ender-3 firmware default
    float       feedrate_xy   = 6000.0f; // mm/min, mapped from velocity profile
    float       max_x_mm      = 235.0f;  // Ender-3 build volume
    float       max_y_mm      = 235.0f;
};

class SerialMarlinSink : public OutputSink {
public:
    explicit SerialMarlinSink(const SerialMarlinConfig& cfg);
    ~SerialMarlinSink() override;

    bool open();    // returns false on port-not-found / busy
    void close() noexcept;

    // From OutputSink:
    bool emit(const ActuatorCommand& cmd) noexcept override;

    // Direct G-code escape hatch (for the "home" / "park" buttons in UI):
    bool send_gcode(std::string_view line) noexcept;

    std::uint64_t emissions() const noexcept override { return emit_count_; }

private:
    SerialMarlinConfig cfg_{};
    void*              h_port_{nullptr};   // Win32 HANDLE
    std::atomic<std::uint64_t> emit_count_{0};
    // ...
};

}  // namespace roc::transport
```

Critical Marlin G-codes used:

```
G28               ; home all axes (run on connect)
G90               ; absolute positioning
G1 X120 Y45 F6000 ; move to (120, 45) at 6000 mm/min
M400              ; wait for moves to complete (use sparingly — blocks)
M112              ; EMERGENCY STOP (kill switch)
```

### Phase C.2 — Pixel → mm extrinsic calibration

The overhead camera sees pixels; the spatula moves in mm. Need a
calibration step that learns the homography between the two.

File targets:

| File | Change |
|---|---|
| `dashboard/src/lib/components/destinations/SorterDestination.svelte` | New destination, 6th nav entry. Lives next to the others. |
| `dashboard/src/lib/components/sorter/CalibrationWizard.svelte` | 4-corner click calibration — operator clicks the corners of the belt's working area; we solve a perspective transform |
| `dashboard/src/lib/stores/sorter_store.svelte.ts` | $state for: calibration matrix, class-to-bin routing table, sorter on/off, last G-code line, serial-link status |

Calibration is just `cv::getPerspectiveTransform()` over the 4 click
points → store the 3×3 matrix in `sorter_store`. At runtime,
`world_xy = M * [pixel_x, pixel_y, 1]^T` gives mm coordinates.

### Phase C.3 — Class-to-bin routing

Each detected class routes to a bin position on the belt's side:

```ts
interface BinRoute {
    classId: number;       // YOLO class id
    className: string;     // human label
    binX_mm: number;       // spatula sweep destination
    binY_mm: number;
    enabled: boolean;
}

// Persisted in localStorage as `sorter.routes.v1`
```

Dashboard UI: a table with one row per class; operator picks which bin
(or "ignore"). Class list comes from the active manifest's `classes:`
field — already exposed via `/api/model/active`.

### Phase C.4 — Sequencer (the actual sort loop)

Each frame:
1. Detector returns boxes
2. EKF tracker assigns persistent IDs
3. For each tracked object: if its centroid crossed the belt's "pick line"
   (a configurable Y coordinate in mm), look up its class in the routing
   table
4. If enabled, queue a G-code sequence:
   - `G1 X{obj_x} Y{obj_y} F{travel_rate}`   (go to object)
   - `G1 Z{down}`                              (lower spatula)
   - `G1 X{bin_x} Y{bin_y} F{sweep_rate}`     (sweep into bin)
   - `G1 Z{up}`                                (raise)
   - `G1 X{home} Y{home}`                      (clear belt)
5. Mark the tracked ID as "handled" so it doesn't double-fire on later frames

## Hard rules

- **Belt must be physically slow** during testing. 50 mm/sec max until
  the trajectory generator (Track D) is wired in. Raw G-code at full
  Ender-3 feedrate (6000+ mm/min) WILL damage the printer if commanded
  to instant position changes.
- **Soft limits are not optional.** Reject any G-code line that would
  move outside `[0, max_x_mm] × [0, max_y_mm]` BEFORE writing to serial.
  Marlin's own M211 soft endstop is a backup, not the primary defence.
- **Emergency stop (M112) must be wired to the Engage destination's
  EMERGENCY STOP button.** Already exists visually; wire it to actually
  send the byte.
- **Always start with `G28` (home) on connect.** Marlin doesn't know
  the print head position until homed; any `G1` before `G28` is "move
  to coordinate relative to wherever the head happens to be" — i.e.
  unpredictable.

## Acceptance criteria

1. `SerialMarlinSink` opens COM3 at 115200, sends `G28`, prints
   the firmware's "ok" response to stdout.
2. Manual G-code from the dashboard ("park" / "home" buttons) moves
   the head safely within build volume.
3. With the camera calibrated and a single object on the belt, the
   spatula moves to the object's mm coordinate within ±5 mm.
4. Class-to-bin routing successfully sorts a 5-object mixed batch
   (e.g. 5 different-colored 3D-printed cubes) into 5 different bins
   with zero hardware faults.
