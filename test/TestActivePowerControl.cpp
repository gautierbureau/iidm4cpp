#include <gtest/gtest.h>
#include <iidm/ActivePowerControl.h>
#include <iidm/Generator.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class ActivePowerControlTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle GEN_HANDLE = 10;

    void SetUp() override {
        backend.bools  [{GEN_HANDLE, prop::EXT_APC_EXISTS}]      = true;
        backend.doubles[{GEN_HANDLE, prop::EXT_APC_DROOP}]       = 4.0;
        backend.bools  [{GEN_HANDLE, prop::EXT_APC_PARTICIPATE}]  = true;
    }
};

TEST_F(ActivePowerControlTest, HasActivePowerControl) {
    Generator gen(GEN_HANDLE, &backend);
    EXPECT_TRUE(gen.hasActivePowerControl());
}

TEST_F(ActivePowerControlTest, HasActivePowerControlAbsent) {
    MockBackend empty;
    Generator gen(GEN_HANDLE, &empty);
    EXPECT_FALSE(gen.hasActivePowerControl());
}

TEST_F(ActivePowerControlTest, GetDroop) {
    Generator gen(GEN_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(gen.getActivePowerControl().getDroop(), 4.0);
}

TEST_F(ActivePowerControlTest, SetDroop) {
    Generator gen(GEN_HANDLE, &backend);
    gen.getActivePowerControl().setDroop(6.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_HANDLE, prop::EXT_APC_DROOP}]), 6.0);
}

TEST_F(ActivePowerControlTest, IsParticipate) {
    Generator gen(GEN_HANDLE, &backend);
    EXPECT_TRUE(gen.getActivePowerControl().isParticipate());
}

TEST_F(ActivePowerControlTest, SetParticipate) {
    Generator gen(GEN_HANDLE, &backend);
    gen.getActivePowerControl().setParticipate(false);
    EXPECT_FALSE((backend.bools[{GEN_HANDLE, prop::EXT_APC_PARTICIPATE}]));
}

TEST_F(ActivePowerControlTest, MethodChaining) {
    Generator gen(GEN_HANDLE, &backend);
    gen.getActivePowerControl().setDroop(8.0).setParticipate(false);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_HANDLE, prop::EXT_APC_DROOP}]), 8.0);
    EXPECT_FALSE((backend.bools[{GEN_HANDLE, prop::EXT_APC_PARTICIPATE}]));
}

TEST_F(ActivePowerControlTest, IsValid) {
    ActivePowerControl valid(GEN_HANDLE, &backend);
    ActivePowerControl invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(ActivePowerControlTest, Equality) {
    ActivePowerControl apc1(GEN_HANDLE, &backend);
    ActivePowerControl apc2(GEN_HANDLE, &backend);
    ActivePowerControl apc3(99, &backend);
    EXPECT_EQ(apc1, apc2);
    EXPECT_NE(apc1, apc3);
}
