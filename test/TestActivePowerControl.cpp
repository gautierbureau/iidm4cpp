#include <gtest/gtest.h>
#include <cmath>
#include <iidm/ActivePowerControl.h>
#include <iidm/ActivePowerControlAdder.h>
#include <iidm/Generator.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class ActivePowerControlTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle GEN_HANDLE = 10;
    static constexpr ObjectHandle APC_HANDLE = 11; // the extension object

    void SetUp() override {
        backend.related[{GEN_HANDLE, prop::ACTIVE_POWER_CONTROL}] = APC_HANDLE;
        backend.doubles[{APC_HANDLE, prop::EXT_APC_DROOP}]       = 4.0;
        backend.bools  [{APC_HANDLE, prop::EXT_APC_PARTICIPATE}]  = true;
    }
};

TEST_F(ActivePowerControlTest, HasActivePowerControl) {
    Generator gen(GEN_HANDLE, &backend);
    EXPECT_TRUE(gen.hasExtension<ActivePowerControl>());
}

TEST_F(ActivePowerControlTest, HasActivePowerControlAbsent) {
    MockBackend empty;
    Generator gen(GEN_HANDLE, &empty);
    EXPECT_FALSE(gen.hasExtension<ActivePowerControl>());
}

TEST_F(ActivePowerControlTest, MethodChaining) {
    Generator gen(GEN_HANDLE, &backend);
    gen.getExtension<ActivePowerControl>().setDroop(8.0).setParticipate(false);
    EXPECT_DOUBLE_EQ((backend.doubles[{APC_HANDLE, prop::EXT_APC_DROOP}]), 8.0);
    EXPECT_FALSE((backend.bools[{APC_HANDLE, prop::EXT_APC_PARTICIPATE}]));
}

TEST_F(ActivePowerControlTest, TargetPLimits) {
    auto apc = Generator(GEN_HANDLE, &backend).getExtension<ActivePowerControl>();
    // Undefined limits come back as NaN from the backend → empty optional
    EXPECT_FALSE(apc.getMinTargetP().has_value());
    EXPECT_FALSE(apc.getMaxTargetP().has_value());

    apc.setMinTargetP(10.0).setMaxTargetP(90.0);
    ASSERT_TRUE(apc.getMinTargetP().has_value());
    EXPECT_DOUBLE_EQ(*apc.getMinTargetP(), 10.0);
    ASSERT_TRUE(apc.getMaxTargetP().has_value());
    EXPECT_DOUBLE_EQ(*apc.getMaxTargetP(), 90.0);

    apc.unsetMinTargetP().unsetMaxTargetP();
    EXPECT_TRUE(std::isnan((backend.doubles[{APC_HANDLE, prop::EXT_APC_MIN_TARGET_P}])));
    EXPECT_TRUE(std::isnan((backend.doubles[{APC_HANDLE, prop::EXT_APC_MAX_TARGET_P}])));
    EXPECT_FALSE(apc.getMinTargetP().has_value());
}

TEST_F(ActivePowerControlTest, AbsentThrows) {
    MockBackend empty;
    Generator gen(GEN_HANDLE, &empty);
    EXPECT_THROW(gen.getExtension<ActivePowerControl>(), ExtensionNotFoundException);
}

TEST_F(ActivePowerControlTest, NewAndRemove) {
    MockBackend b;
    static constexpr ObjectHandle NEW_APC = 12;
    Generator gen(GEN_HANDLE, &b);
    ActivePowerControlAdder adder = gen.newExtension<ActivePowerControlAdder>();
    ASSERT_EQ(b.invocations.size(), 1u);
    EXPECT_EQ(b.invocations[0], std::make_pair(static_cast<ObjectHandle>(GEN_HANDLE), prop::ACTIVE_POWER_CONTROL));
    const ObjectHandle adderHandle = b.lastAdder;
    b.addResults[adderHandle] = NEW_APC;
    ActivePowerControl apc = adder.withDroop(5.0).withParticipate(true).withParticipationFactor(0.4)
                                  .withMinTargetP(1.0).withMaxTargetP(9.0).add();
    EXPECT_DOUBLE_EQ((b.doubles[{adderHandle, prop::EXT_APC_DROOP}]), 5.0);
    EXPECT_TRUE((b.bools[{adderHandle, prop::EXT_APC_PARTICIPATE}]));
    EXPECT_DOUBLE_EQ((b.doubles[{adderHandle, prop::EXT_APC_PARTICIPATION_FACTOR}]), 0.4);
    EXPECT_DOUBLE_EQ((b.doubles[{adderHandle, prop::EXT_APC_MIN_TARGET_P}]), 1.0);
    EXPECT_DOUBLE_EQ((b.doubles[{adderHandle, prop::EXT_APC_MAX_TARGET_P}]), 9.0);
    EXPECT_EQ(b.invocations.back(), std::make_pair(adderHandle, prop::ACT_ADD));
    EXPECT_EQ(apc, ActivePowerControl(NEW_APC, &b));

    gen.removeExtension<ActivePowerControl>();
    EXPECT_EQ((b.related[{GEN_HANDLE, prop::ACTIVE_POWER_CONTROL}]), INVALID_HANDLE);
}
