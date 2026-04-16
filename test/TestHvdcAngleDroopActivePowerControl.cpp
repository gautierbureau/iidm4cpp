#include <gtest/gtest.h>
#include <iidm/HvdcAngleDroopActivePowerControl.h>
#include <iidm/HvdcLine.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class HvdcAngleDroopActivePowerControlTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle HVDC_HANDLE = 20;

    void SetUp() override {
        backend.bools  [{HVDC_HANDLE, prop::EXT_HADAPC_EXISTS}]  = true;
        backend.doubles[{HVDC_HANDLE, prop::EXT_HADAPC_DROOP}]   = 1.5;
        backend.doubles[{HVDC_HANDLE, prop::EXT_HADAPC_P0}]      = 100.0;
        backend.bools  [{HVDC_HANDLE, prop::EXT_HADAPC_ENABLED}] = true;
    }
};

TEST_F(HvdcAngleDroopActivePowerControlTest, HasExtension) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_TRUE(hvdc.hasHvdcAngleDroopActivePowerControl());
}

TEST_F(HvdcAngleDroopActivePowerControlTest, HasExtensionAbsent) {
    MockBackend empty;
    HvdcLine hvdc(HVDC_HANDLE, &empty);
    EXPECT_FALSE(hvdc.hasHvdcAngleDroopActivePowerControl());
}

TEST_F(HvdcAngleDroopActivePowerControlTest, GetDroop) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(hvdc.getHvdcAngleDroopActivePowerControl().getDroop(), 1.5);
}

TEST_F(HvdcAngleDroopActivePowerControlTest, SetDroop) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    hvdc.getHvdcAngleDroopActivePowerControl().setDroop(2.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{HVDC_HANDLE, prop::EXT_HADAPC_DROOP}]), 2.0);
}

TEST_F(HvdcAngleDroopActivePowerControlTest, GetP0) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(hvdc.getHvdcAngleDroopActivePowerControl().getP0(), 100.0);
}

TEST_F(HvdcAngleDroopActivePowerControlTest, SetP0) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    hvdc.getHvdcAngleDroopActivePowerControl().setP0(200.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{HVDC_HANDLE, prop::EXT_HADAPC_P0}]), 200.0);
}

TEST_F(HvdcAngleDroopActivePowerControlTest, IsEnabled) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_TRUE(hvdc.getHvdcAngleDroopActivePowerControl().isEnabled());
}

TEST_F(HvdcAngleDroopActivePowerControlTest, SetEnabled) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    hvdc.getHvdcAngleDroopActivePowerControl().setEnabled(false);
    EXPECT_FALSE((backend.bools[{HVDC_HANDLE, prop::EXT_HADAPC_ENABLED}]));
}

TEST_F(HvdcAngleDroopActivePowerControlTest, MethodChaining) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    hvdc.getHvdcAngleDroopActivePowerControl().setDroop(3.0).setP0(150.0).setEnabled(false);
    EXPECT_DOUBLE_EQ((backend.doubles[{HVDC_HANDLE, prop::EXT_HADAPC_DROOP}]), 3.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{HVDC_HANDLE, prop::EXT_HADAPC_P0}]), 150.0);
    EXPECT_FALSE((backend.bools[{HVDC_HANDLE, prop::EXT_HADAPC_ENABLED}]));
}

TEST_F(HvdcAngleDroopActivePowerControlTest, IsValid) {
    HvdcAngleDroopActivePowerControl valid(HVDC_HANDLE, &backend);
    HvdcAngleDroopActivePowerControl invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(HvdcAngleDroopActivePowerControlTest, Equality) {
    HvdcAngleDroopActivePowerControl ext1(HVDC_HANDLE, &backend);
    HvdcAngleDroopActivePowerControl ext2(HVDC_HANDLE, &backend);
    HvdcAngleDroopActivePowerControl ext3(99, &backend);
    EXPECT_EQ(ext1, ext2);
    EXPECT_NE(ext1, ext3);
}
