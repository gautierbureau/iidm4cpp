#include <gtest/gtest.h>
#include <iidm/StaticVarCompensator.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class StaticVarCompensatorTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle SVC_H  = 10;
    static constexpr ObjectHandle TERM_H = 20;

    void SetUp() override {
        backend.strings[{SVC_H, prop::ID}]                    = "SVC1";
        backend.strings[{SVC_H, prop::NAME}]                   = "Static Var 1";
        backend.doubles[{SVC_H, prop::SVC_B_MIN}]              = -0.1;
        backend.doubles[{SVC_H, prop::SVC_B_MAX}]              = 0.1;
        backend.doubles[{SVC_H, prop::SVC_VOLTAGE_SETPOINT}]   = 225.0;
        backend.doubles[{SVC_H, prop::SVC_REACTIVE_POWER_SETPOINT}] = 50.0;
        backend.ints   [{SVC_H, prop::SVC_REGULATION_MODE}]    = 0; // VOLTAGE
        backend.related[{SVC_H, prop::REL_TERMINAL}]           = TERM_H;
        backend.bools  [{TERM_H, prop::TERMINAL_CONNECTED}]    = true;
        backend.related[{SVC_H, prop::REL_REGULATING_TERMINAL}] = TERM_H;
    }
};

TEST_F(StaticVarCompensatorTest, GetId) {
    StaticVarCompensator svc(SVC_H, &backend);
    EXPECT_EQ(svc.getId(), "SVC1");
}

TEST_F(StaticVarCompensatorTest, GetBLimits) {
    StaticVarCompensator svc(SVC_H, &backend);
    EXPECT_DOUBLE_EQ(svc.getBMin(), -0.1);
    EXPECT_DOUBLE_EQ(svc.getBMax(),  0.1);
}

TEST_F(StaticVarCompensatorTest, GetSetVoltageSetpoint) {
    StaticVarCompensator svc(SVC_H, &backend);
    EXPECT_DOUBLE_EQ(svc.getVoltageSetpoint(), 225.0);
    svc.setVoltageSetpoint(230.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{SVC_H, prop::SVC_VOLTAGE_SETPOINT}]), 230.0);
}

TEST_F(StaticVarCompensatorTest, GetSetReactivePowerSetpoint) {
    StaticVarCompensator svc(SVC_H, &backend);
    EXPECT_DOUBLE_EQ(svc.getReactivePowerSetpoint(), 50.0);
    svc.setReactivePowerSetpoint(60.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{SVC_H, prop::SVC_REACTIVE_POWER_SETPOINT}]), 60.0);
}

TEST_F(StaticVarCompensatorTest, GetSetRegulationMode) {
    StaticVarCompensator svc(SVC_H, &backend);
    EXPECT_EQ(svc.getRegulationMode(), StaticVarCompensatorRegulationMode::VOLTAGE);
    svc.setRegulationMode(StaticVarCompensatorRegulationMode::REACTIVE_POWER);
    EXPECT_EQ((backend.ints[{SVC_H, prop::SVC_REGULATION_MODE}]), 1);
}

// Regression: SVC_REGULATION_MODE must round-trip OFF (= 2). powsybl-core 7.x dropped
// OFF from its enum and represents it via isRegulating(); the dispatcher (Java) and
// JNI backend translate, so at the C++ wrapper layer OFF is just integer 2.
TEST_F(StaticVarCompensatorTest, RegulationModeOffRoundTrip) {
    backend.ints[{SVC_H, prop::SVC_REGULATION_MODE}] = 2; // OFF
    StaticVarCompensator svc(SVC_H, &backend);
    EXPECT_EQ(svc.getRegulationMode(), StaticVarCompensatorRegulationMode::OFF);
    svc.setRegulationMode(StaticVarCompensatorRegulationMode::OFF);
    EXPECT_EQ((backend.ints[{SVC_H, prop::SVC_REGULATION_MODE}]), 2);
}

TEST_F(StaticVarCompensatorTest, GetTerminal) {
    backend.doubles[{TERM_H, prop::TERMINAL_P}] = 10.0;
    StaticVarCompensator svc(SVC_H, &backend);
    EXPECT_TRUE(svc.getTerminal().isValid());
    EXPECT_TRUE(svc.getRegulatingTerminal().isValid());
}

TEST_F(StaticVarCompensatorTest, ConnectDisconnect) {
    StaticVarCompensator svc(SVC_H, &backend);
    EXPECT_TRUE(svc.isConnected());
    svc.disconnect();
    EXPECT_FALSE((backend.bools[{TERM_H, prop::TERMINAL_CONNECTED}]));
    svc.connect();
    EXPECT_TRUE((backend.bools[{TERM_H, prop::TERMINAL_CONNECTED}]));
}

TEST_F(StaticVarCompensatorTest, IsValid) {
    StaticVarCompensator valid(SVC_H, &backend);
    StaticVarCompensator invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}
