#include <gtest/gtest.h>
#include <cmath>
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
    ActivePowerControl valid(GEN_HANDLE, &backend, prop::EXT_APC_DROOP, prop::EXT_APC_PARTICIPATE);
    ActivePowerControl invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(ActivePowerControlTest, Equality) {
    ActivePowerControl apc1(GEN_HANDLE, &backend, prop::EXT_APC_DROOP, prop::EXT_APC_PARTICIPATE);
    ActivePowerControl apc2(GEN_HANDLE, &backend, prop::EXT_APC_DROOP, prop::EXT_APC_PARTICIPATE);
    ActivePowerControl apc3(99, &backend, prop::EXT_APC_DROOP, prop::EXT_APC_PARTICIPATE);
    EXPECT_EQ(apc1, apc2);
    EXPECT_NE(apc1, apc3);
}

TEST_F(ActivePowerControlTest, ParticipationFactor) {
    backend.doubles[{GEN_HANDLE, prop::EXT_APC_PARTICIPATION_FACTOR}] = 0.25;
    auto apc = Generator(GEN_HANDLE, &backend).getActivePowerControl();
    EXPECT_DOUBLE_EQ(apc.getParticipationFactor(), 0.25);
    apc.setParticipationFactor(0.5);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_HANDLE, prop::EXT_APC_PARTICIPATION_FACTOR}]), 0.5);
}

TEST_F(ActivePowerControlTest, TargetPLimits) {
    auto apc = Generator(GEN_HANDLE, &backend).getActivePowerControl();
    // Undefined limits come back as NaN from the backend → empty optional
    EXPECT_FALSE(apc.getMinTargetP().has_value());
    EXPECT_FALSE(apc.getMaxTargetP().has_value());

    apc.setMinTargetP(10.0).setMaxTargetP(90.0);
    ASSERT_TRUE(apc.getMinTargetP().has_value());
    EXPECT_DOUBLE_EQ(*apc.getMinTargetP(), 10.0);
    ASSERT_TRUE(apc.getMaxTargetP().has_value());
    EXPECT_DOUBLE_EQ(*apc.getMaxTargetP(), 90.0);

    apc.unsetMinTargetP().unsetMaxTargetP();
    EXPECT_TRUE(std::isnan((backend.doubles[{GEN_HANDLE, prop::EXT_APC_MIN_TARGET_P}])));
    EXPECT_TRUE(std::isnan((backend.doubles[{GEN_HANDLE, prop::EXT_APC_MAX_TARGET_P}])));
    EXPECT_FALSE(apc.getMinTargetP().has_value());
}

// The former (droopCode, participateCode) constructor maps the new fields onto the same block.
TEST_F(ActivePowerControlTest, LegacyConstructorUsesSameCodeBlock) {
    ActivePowerControl apc(GEN_HANDLE, &backend, prop::EXT_BAT_APC_DROOP, prop::EXT_BAT_APC_PARTICIPATE);
    apc.setParticipationFactor(0.7).setMinTargetP(1.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_HANDLE, prop::EXT_BAT_APC_PARTICIPATION_FACTOR}]), 0.7);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_HANDLE, prop::EXT_BAT_APC_MIN_TARGET_P}]), 1.0);
}
