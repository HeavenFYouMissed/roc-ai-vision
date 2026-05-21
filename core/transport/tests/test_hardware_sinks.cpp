#include <catch2/catch_test_macros.hpp>

#include "roc/transport/hardware_sinks.hpp"

TEST_CASE("hardware sink stubs accept valid emissions", "[hardware_sinks]") {
    roc::transport::HidMouseSink hid;
    roc::transport::DynamixelServoSink dxl;
    roc::transport::ModbusSink modbus;

    roc::transport::ActuatorEmission e{};
    e.valid = true;
    e.command_u_px = 100.0f;
    e.command_v_px = 200.0f;

    hid.emit(e);
    dxl.emit(e);
    modbus.emit(e);

    REQUIRE(hid.emissions() == 1);
    REQUIRE(dxl.emissions() == 1);
    REQUIRE(modbus.emissions() == 1);
}
