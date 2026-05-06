#include <gtest/gtest.h>
#include <iidm/VscConverterStation.h>
#include <iidm/HvdcLine.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class VscConverterStationTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle VSC_H  = 10;
    static constexpr ObjectHandle TERM_H = 20;
    static constexpr ObjectHandle HVDC_H = 30;

    void SetUp() override {
        backend.strings[{VSC_H, prop::ID}]                     = "VSC1";
        backend.strings[{VSC_H, prop::NAME}]                    = "VSC Station 1";
        backend.doubles[{VSC_H, prop::VSC_LOSS_FACTOR}]         = 0.011f;
        backend.bools  [{VSC_H, prop::VSC_VOLTAGE_REGULATOR_ON}] = true;
        backend.doubles[{VSC_H, prop::VSC_VOLTAGE_SETPOINT}]    = 400.0;
        backend.doubles[{VSC_H, prop::VSC_REACTIVE_POWER_SETPOINT}] = -50.0;
        backend.ints   [{VSC_H, prop::VSC_REACTIVE_LIMITS_KIND}] = 1; // MIN_MAX
        backend.doubles[{VSC_H, prop::VSC_MIN_Q}]               = -200.0;
        backend.doubles[{VSC_H, prop::VSC_MAX_Q}]               = 200.0;
        backend.related[{VSC_H, prop::REL_TERMINAL}]            = TERM_H;
        backend.bools  [{TERM_H, prop::TERMINAL_CONNECTED}]     = true;
        backend.related[{VSC_H, prop::REL_HVDC_LINE}]           = HVDC_H;
        backend.related[{VSC_H, prop::REL_REGULATING_TERMINAL}] = TERM_H;
    }
};

TEST_F(VscConverterStationTest, GetId) {
    VscConverterStation vsc(VSC_H, &backend);
    EXPECT_EQ(vsc.getId(), "VSC1");
}

TEST_F(VscConverterStationTest, GetLossFactor) {
    VscConverterStation vsc(VSC_H, &backend);
    EXPECT_FLOAT_EQ(vsc.getLossFactor(), 0.011f);
}

TEST_F(VscConverterStationTest, VoltageRegulator) {
    VscConverterStation vsc(VSC_H, &backend);
    EXPECT_TRUE(vsc.isVoltageRegulatorOn());
    vsc.setVoltageRegulatorOn(false);
    EXPECT_FALSE((backend.bools[{VSC_H, prop::VSC_VOLTAGE_REGULATOR_ON}]));
}

TEST_F(VscConverterStationTest, GetSetVoltageSetpoint) {
    VscConverterStation vsc(VSC_H, &backend);
    EXPECT_DOUBLE_EQ(vsc.getVoltageSetpoint(), 400.0);
    vsc.setVoltageSetpoint(410.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{VSC_H, prop::VSC_VOLTAGE_SETPOINT}]), 410.0);
}

TEST_F(VscConverterStationTest, GetSetReactivePowerSetpoint) {
    VscConverterStation vsc(VSC_H, &backend);
    EXPECT_DOUBLE_EQ(vsc.getReactivePowerSetpoint(), -50.0);
    vsc.setReactivePowerSetpoint(-60.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{VSC_H, prop::VSC_REACTIVE_POWER_SETPOINT}]), -60.0);
}

TEST_F(VscConverterStationTest, MinMaxReactiveLimits) {
    VscConverterStation vsc(VSC_H, &backend);
    EXPECT_TRUE(vsc.hasMinMaxReactiveLimits());
    EXPECT_FALSE(vsc.hasReactiveCapabilityCurve());
    auto limits = vsc.getMinMaxReactiveLimits();
    EXPECT_DOUBLE_EQ(limits.getMinQ(), -200.0);
    EXPECT_DOUBLE_EQ(limits.getMaxQ(), 200.0);
}

TEST_F(VscConverterStationTest, GetTerminalAndHvdcLine) {
    backend.doubles[{TERM_H, prop::TERMINAL_P}] = 100.0;
    backend.strings[{HVDC_H, prop::ID}] = "HVDC1";
    VscConverterStation vsc(VSC_H, &backend);
    EXPECT_TRUE(vsc.getTerminal().isValid());
    EXPECT_TRUE(vsc.getHvdcLine().isValid());
}

TEST_F(VscConverterStationTest, ConnectDisconnect) {
    VscConverterStation vsc(VSC_H, &backend);
    EXPECT_TRUE(vsc.isConnected());
    vsc.disconnect();
    EXPECT_FALSE((backend.bools[{TERM_H, prop::TERMINAL_CONNECTED}]));
    vsc.connect();
    EXPECT_TRUE((backend.bools[{TERM_H, prop::TERMINAL_CONNECTED}]));
}

TEST_F(VscConverterStationTest, IsValid) {
    VscConverterStation valid(VSC_H, &backend);
    VscConverterStation invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}
