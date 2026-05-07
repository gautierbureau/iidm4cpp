#include <gtest/gtest.h>
#include <iidm/ShuntCompensator.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class ShuntCompensatorTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle SC_H   = 10;
    static constexpr ObjectHandle TERM_H = 20;
    static constexpr ObjectHandle RT_H   = 21;

    void SetUp() override {
        backend.strings[{SC_H, prop::ID}]                    = "SC1";
        backend.strings[{SC_H, prop::NAME}]                   = "Shunt 1";
        backend.ints   [{SC_H, prop::SHUNT_SECTION_COUNT}]    = 2;
        backend.ints   [{SC_H, prop::SHUNT_MAX_SECTION_COUNT}] = 4;
        backend.ints   [{SC_H, prop::SHUNT_MODEL_KIND}]        = 0; // LINEAR
        backend.doubles[{SC_H, prop::SHUNT_B_PER_SECTION}]    = 0.01;
        backend.doubles[{SC_H, prop::SHUNT_G_PER_SECTION}]    = 0.0;
        backend.doubles[{SC_H, prop::SHUNT_B}]                = 0.02;
        backend.bools  [{SC_H, prop::SHUNT_VOLTAGE_REGULATOR_ON}] = true;
        backend.doubles[{SC_H, prop::SHUNT_TARGET_V}]         = 225.0;
        backend.strings[{SC_H, prop::SHUNT_REGULATING_TERMINAL_ID}] = "GEN1";
        backend.related[{SC_H, prop::REL_TERMINAL}]           = TERM_H;
        backend.bools  [{TERM_H, prop::TERMINAL_CONNECTED}]   = true;
        backend.related[{SC_H, prop::REL_REGULATING_TERMINAL}] = RT_H;
    }
};

TEST_F(ShuntCompensatorTest, GetId) {
    ShuntCompensator sc(SC_H, &backend);
    EXPECT_EQ(sc.getId(), "SC1");
}

TEST_F(ShuntCompensatorTest, GetSectionCount) {
    ShuntCompensator sc(SC_H, &backend);
    EXPECT_EQ(sc.getSectionCount(), 2);
    EXPECT_EQ(sc.getMaximumSectionCount(), 4);
}

TEST_F(ShuntCompensatorTest, SetSectionCount) {
    ShuntCompensator sc(SC_H, &backend);
    sc.setSectionCount(3);
    EXPECT_EQ((backend.ints[{SC_H, prop::SHUNT_SECTION_COUNT}]), 3);
}

TEST_F(ShuntCompensatorTest, LinearModelGetters) {
    ShuntCompensator sc(SC_H, &backend);
    EXPECT_TRUE(sc.hasLinearModel());
    EXPECT_FALSE(sc.hasNonLinearModel());
    EXPECT_DOUBLE_EQ(sc.getBPerSection(), 0.01);
    EXPECT_DOUBLE_EQ(sc.getGPerSection(), 0.0);
    EXPECT_DOUBLE_EQ(sc.getB(), 0.02);
}

// Regression: getBPerSection() and getGPerSection() must return the
// per-section value of the linear model, NOT the total B/G at the current
// section count. Dynawo's getB(int section) wrapper computes
// "bPerSection * section"; if getBPerSection() returned total B
// (sectionCount * bPerSection) the result would scale incorrectly.
TEST_F(ShuntCompensatorTest, LinearPerSectionIsNotTotal) {
    backend.ints   [{SC_H, prop::SHUNT_SECTION_COUNT}] = 2;
    backend.doubles[{SC_H, prop::SHUNT_B_PER_SECTION}] = 12.0;
    backend.doubles[{SC_H, prop::SHUNT_G_PER_SECTION}] = 0.5;
    backend.doubles[{SC_H, prop::SHUNT_B}]             = 24.0;
    ShuntCompensator sc(SC_H, &backend);
    EXPECT_DOUBLE_EQ(sc.getBPerSection(), 12.0);
    EXPECT_DOUBLE_EQ(sc.getGPerSection(), 0.5);
    EXPECT_DOUBLE_EQ(sc.getB(),           24.0);
}

TEST_F(ShuntCompensatorTest, VoltageRegulator) {
    ShuntCompensator sc(SC_H, &backend);
    EXPECT_TRUE(sc.isVoltageRegulatorOn());
    EXPECT_DOUBLE_EQ(sc.getTargetV(), 225.0);
    EXPECT_EQ(sc.getRegulatingTerminalId(), "GEN1");
}

TEST_F(ShuntCompensatorTest, GetTerminal) {
    backend.doubles[{TERM_H, prop::TERMINAL_P}] = 5.0;
    ShuntCompensator sc(SC_H, &backend);
    EXPECT_TRUE(sc.getTerminal().isValid());
}

TEST_F(ShuntCompensatorTest, ConnectDisconnect) {
    ShuntCompensator sc(SC_H, &backend);
    EXPECT_TRUE(sc.isConnected());
    sc.disconnect();
    EXPECT_FALSE((backend.bools[{TERM_H, prop::TERMINAL_CONNECTED}]));
    sc.connect();
    EXPECT_TRUE((backend.bools[{TERM_H, prop::TERMINAL_CONNECTED}]));
}

TEST_F(ShuntCompensatorTest, IsValid) {
    ShuntCompensator valid(SC_H, &backend);
    ShuntCompensator invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}
