#include <gtest/gtest.h>
#include <iidm/HvdcAngleDroopActivePowerControl.h>
#include <iidm/HvdcAngleDroopActivePowerControlAdder.h>
#include <iidm/HvdcLine.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class HvdcAngleDroopActivePowerControlTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle HVDC_HANDLE = 20;
    static constexpr ObjectHandle EXT_HANDLE = 21; // the extension object

    void SetUp() override {
        backend.related[{HVDC_HANDLE, prop::HVDC_ANGLE_DROOP_ACTIVE_POWER_CONTROL}] = EXT_HANDLE;
        backend.doubles[{EXT_HANDLE, prop::EXT_HADAPC_DROOP}]   = 1.5;
        backend.doubles[{EXT_HANDLE, prop::EXT_HADAPC_P0}]      = 100.0;
        backend.bools  [{EXT_HANDLE, prop::EXT_HADAPC_ENABLED}] = true;
    }
};

TEST_F(HvdcAngleDroopActivePowerControlTest, HasExtension) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_TRUE(hvdc.hasExtension<HvdcAngleDroopActivePowerControl>());
}

TEST_F(HvdcAngleDroopActivePowerControlTest, HasExtensionAbsent) {
    MockBackend empty;
    HvdcLine hvdc(HVDC_HANDLE, &empty);
    EXPECT_FALSE(hvdc.hasExtension<HvdcAngleDroopActivePowerControl>());
}

TEST_F(HvdcAngleDroopActivePowerControlTest, GetDroop) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(hvdc.getExtension<HvdcAngleDroopActivePowerControl>().getDroop(), 1.5);
}

TEST_F(HvdcAngleDroopActivePowerControlTest, SetDroop) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    hvdc.getExtension<HvdcAngleDroopActivePowerControl>().setDroop(2.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{EXT_HANDLE, prop::EXT_HADAPC_DROOP}]), 2.0);
}

TEST_F(HvdcAngleDroopActivePowerControlTest, GetP0) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(hvdc.getExtension<HvdcAngleDroopActivePowerControl>().getP0(), 100.0);
}

TEST_F(HvdcAngleDroopActivePowerControlTest, SetP0) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    hvdc.getExtension<HvdcAngleDroopActivePowerControl>().setP0(200.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{EXT_HANDLE, prop::EXT_HADAPC_P0}]), 200.0);
}

TEST_F(HvdcAngleDroopActivePowerControlTest, IsEnabled) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    EXPECT_TRUE(hvdc.getExtension<HvdcAngleDroopActivePowerControl>().isEnabled());
}

TEST_F(HvdcAngleDroopActivePowerControlTest, SetEnabled) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    hvdc.getExtension<HvdcAngleDroopActivePowerControl>().setEnabled(false);
    EXPECT_FALSE((backend.bools[{EXT_HANDLE, prop::EXT_HADAPC_ENABLED}]));
}

TEST_F(HvdcAngleDroopActivePowerControlTest, MethodChaining) {
    HvdcLine hvdc(HVDC_HANDLE, &backend);
    hvdc.getExtension<HvdcAngleDroopActivePowerControl>().setDroop(3.0).setP0(150.0).setEnabled(false);
    EXPECT_DOUBLE_EQ((backend.doubles[{EXT_HANDLE, prop::EXT_HADAPC_DROOP}]), 3.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{EXT_HANDLE, prop::EXT_HADAPC_P0}]), 150.0);
    EXPECT_FALSE((backend.bools[{EXT_HANDLE, prop::EXT_HADAPC_ENABLED}]));
}

TEST_F(HvdcAngleDroopActivePowerControlTest, IsValid) {
    HvdcAngleDroopActivePowerControl valid(EXT_HANDLE, &backend);
    HvdcAngleDroopActivePowerControl invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(HvdcAngleDroopActivePowerControlTest, Equality) {
    HvdcAngleDroopActivePowerControl ext1(EXT_HANDLE, &backend);
    HvdcAngleDroopActivePowerControl ext2(EXT_HANDLE, &backend);
    HvdcAngleDroopActivePowerControl ext3(99, &backend);
    EXPECT_EQ(ext1, ext2);
    EXPECT_NE(ext1, ext3);
}

TEST_F(HvdcAngleDroopActivePowerControlTest, AbsentThrows) {
    MockBackend empty;
    EXPECT_THROW(HvdcLine(HVDC_HANDLE, &empty).getExtension<HvdcAngleDroopActivePowerControl>(), ExtensionNotFoundException);
}

TEST_F(HvdcAngleDroopActivePowerControlTest, NewAndRemove) {
    MockBackend b;
    static constexpr ObjectHandle NEW_EXT = 22;
    HvdcLine hvdc(HVDC_HANDLE, &b);
    HvdcAngleDroopActivePowerControlAdder adder = hvdc.newExtension<HvdcAngleDroopActivePowerControlAdder>();
    EXPECT_EQ(b.invocations.back(), std::make_pair(static_cast<ObjectHandle>(HVDC_HANDLE), prop::HVDC_ANGLE_DROOP_ACTIVE_POWER_CONTROL));
    const ObjectHandle adderHandle = b.lastAdder;
    b.addResults[adderHandle] = NEW_EXT;
    HvdcAngleDroopActivePowerControl ext = adder.withP0(120.0).withDroop(0.5).withEnabled(true).add();
    EXPECT_DOUBLE_EQ((b.doubles[{adderHandle, prop::EXT_HADAPC_P0}]), 120.0);
    EXPECT_DOUBLE_EQ((b.doubles[{adderHandle, prop::EXT_HADAPC_DROOP}]), 0.5);
    EXPECT_TRUE((b.bools[{adderHandle, prop::EXT_HADAPC_ENABLED}]));
    EXPECT_EQ(ext, HvdcAngleDroopActivePowerControl(NEW_EXT, &b));

    hvdc.removeExtension<HvdcAngleDroopActivePowerControl>();
    EXPECT_EQ((b.related[{HVDC_HANDLE, prop::HVDC_ANGLE_DROOP_ACTIVE_POWER_CONTROL}]), INVALID_HANDLE);
}
