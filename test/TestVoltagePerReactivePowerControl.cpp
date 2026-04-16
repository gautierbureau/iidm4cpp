#include <gtest/gtest.h>
#include <iidm/VoltagePerReactivePowerControl.h>
#include <iidm/StaticVarCompensator.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class VoltagePerReactivePowerControlTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle SVC_HANDLE = 40;

    void SetUp() override {
        backend.bools  [{SVC_HANDLE, prop::EXT_VPRC_EXISTS}] = true;
        backend.doubles[{SVC_HANDLE, prop::EXT_VPRC_SLOPE}]  = 0.5;
    }
};

TEST_F(VoltagePerReactivePowerControlTest, HasExtension) {
    StaticVarCompensator svc(SVC_HANDLE, &backend);
    EXPECT_TRUE(svc.hasVoltagePerReactivePowerControl());
}

TEST_F(VoltagePerReactivePowerControlTest, HasExtensionAbsent) {
    MockBackend empty;
    StaticVarCompensator svc(SVC_HANDLE, &empty);
    EXPECT_FALSE(svc.hasVoltagePerReactivePowerControl());
}

TEST_F(VoltagePerReactivePowerControlTest, GetSlope) {
    StaticVarCompensator svc(SVC_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(svc.getVoltagePerReactivePowerControl().getSlope(), 0.5);
}

TEST_F(VoltagePerReactivePowerControlTest, SetSlope) {
    StaticVarCompensator svc(SVC_HANDLE, &backend);
    svc.getVoltagePerReactivePowerControl().setSlope(1.2);
    EXPECT_DOUBLE_EQ((backend.doubles[{SVC_HANDLE, prop::EXT_VPRC_SLOPE}]), 1.2);
}

TEST_F(VoltagePerReactivePowerControlTest, MethodChaining) {
    StaticVarCompensator svc(SVC_HANDLE, &backend);
    auto& vprc = svc.getVoltagePerReactivePowerControl().setSlope(0.8);
    EXPECT_DOUBLE_EQ((backend.doubles[{SVC_HANDLE, prop::EXT_VPRC_SLOPE}]), 0.8);
    (void)vprc;
}

TEST_F(VoltagePerReactivePowerControlTest, IsValid) {
    VoltagePerReactivePowerControl valid(SVC_HANDLE, &backend);
    VoltagePerReactivePowerControl invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(VoltagePerReactivePowerControlTest, Equality) {
    VoltagePerReactivePowerControl ext1(SVC_HANDLE, &backend);
    VoltagePerReactivePowerControl ext2(SVC_HANDLE, &backend);
    VoltagePerReactivePowerControl ext3(99, &backend);
    EXPECT_EQ(ext1, ext2);
    EXPECT_NE(ext1, ext3);
}
