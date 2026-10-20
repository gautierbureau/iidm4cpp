#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "MockBackend.h"
#include <utility>

// The voltage regulation of every holder (powsybl 7.4 regulation.VoltageRegulationHolder):
// generators, batteries, static var compensators, shunt compensators, VSC converter stations.

using namespace iidm;
using namespace iidm::test;

namespace {

constexpr ObjectHandle HOLDER = 10;
constexpr ObjectHandle REGULATION = 11;
constexpr ObjectHandle TERMINAL = 12;

std::pair<ObjectHandle, int> call(ObjectHandle h, int code) {
    return std::make_pair(h, code);
}

template <class T>
void checkHolder() {
    MockBackend b;
    T holder(HOLDER, &b);

    // Local targets, chained with the holder's own setters
    T& same = holder.setLocalTargetV(400).setLocalTargetQ(-5);
    EXPECT_EQ(&same, &holder);
    EXPECT_DOUBLE_EQ(holder.getLocalTargetV(), 400);
    EXPECT_DOUBLE_EQ(holder.getLocalTargetQ(), -5);

    // The regulation object
    EXPECT_FALSE(holder.getVoltageRegulation());
    b.related[{HOLDER, prop::VOLTAGE_REGULATION}] = REGULATION;
    ASSERT_TRUE(holder.getVoltageRegulation());
    EXPECT_EQ(*holder.getVoltageRegulation(), VoltageRegulation(REGULATION, &b));
    holder.newVoltageRegulation();
    EXPECT_EQ(b.invocations.back(), call(HOLDER, prop::VOLTAGE_REGULATION));
    holder.removeVoltageRegulation();
    EXPECT_EQ((b.related[{HOLDER, prop::VOLTAGE_REGULATION}]), INVALID_HANDLE);

    // Computed by powsybl
    b.bools[{HOLDER, prop::REGULATING}] = true;
    b.intLists[{HOLDER, prop::REGULATING_MODES}] = {static_cast<int>(RegulationMode::VOLTAGE_PER_REACTIVE_POWER)};
    b.intLists[{HOLDER, prop::WITH_MODES}] = {static_cast<int>(RegulationMode::VOLTAGE_PER_REACTIVE_POWER)};
    b.doubles[{HOLDER, prop::REGULATING_TARGET_V}] = 398;
    b.doubles[{HOLDER, prop::REGULATING_TARGET_Q}] = 7;
    b.related[{HOLDER, prop::REL_REGULATING_TERMINAL}] = TERMINAL;
    b.bools[{HOLDER, prop::HAS_REGULATING_TERMINAL}] = true;
    EXPECT_TRUE(holder.isRegulating());
    EXPECT_TRUE(holder.isRegulatingWithMode(RegulationMode::VOLTAGE_PER_REACTIVE_POWER));
    EXPECT_FALSE(holder.isRegulatingWithMode(RegulationMode::VOLTAGE));
    EXPECT_TRUE(holder.isWithMode(RegulationMode::VOLTAGE_PER_REACTIVE_POWER));
    EXPECT_FALSE(holder.isWithMode(RegulationMode::REACTIVE_POWER));
    EXPECT_DOUBLE_EQ(holder.getRegulatingTargetV(), 398);
    EXPECT_DOUBLE_EQ(holder.getRegulatingTargetQ(), 7);
    EXPECT_EQ(holder.getRegulatingTerminal(), Terminal(TERMINAL, &b));
    EXPECT_TRUE(holder.hasRegulatingTerminal());

    // Actions
    holder.setTargetQToQ();
    EXPECT_EQ(b.invocations.back(), call(HOLDER, prop::ACT_SET_TARGET_Q_TO_Q));
    holder.setTargetVToV();
    EXPECT_EQ(b.invocations.back(), call(HOLDER, prop::ACT_SET_TARGET_V_TO_V));
}

} // namespace

TEST(VoltageRegulationHolderTest, EveryHolder) {
    checkHolder<Generator>();
    checkHolder<Battery>();
    checkHolder<StaticVarCompensator>();
    checkHolder<ShuntCompensator>();
    checkHolder<VscConverterStation>();
}

TEST(VoltageRegulationHolderTest, NotBoundToANetwork) {
    EXPECT_THROW(StaticVarCompensator().getVoltageRegulation(), InvalidHandleException);
    EXPECT_THROW(ShuntCompensator().isRegulatingWithMode(RegulationMode::VOLTAGE), InvalidHandleException);
}
