#include <gtest/gtest.h>
#include <iidm/OverloadManagementSystem.h>
#include <iidm/OverloadManagementSystemAdder.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Substation.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

// Overload management systems as the backend sees them

static constexpr ObjectHandle OMS_HANDLE = 10;
static constexpr ObjectHandle SUBSTATION_HANDLE = 11;
static constexpr ObjectHandle TRIPPING_HANDLE = 20;
static constexpr ObjectHandle ADDER = 500; // the next "adder" of the mock

using OMS = OverloadManagementSystem;

class OverloadManagementSystemTest : public ::testing::Test {
protected:
    MockBackend backend;

    void SetUp() override {
        backend.nextAdder = ADDER;
        backend.bools[{OMS_HANDLE, prop::AUTOMATION_SYSTEM_ENABLED}] = true;
        backend.strings[{OMS_HANDLE, prop::OMS_MONITORED_ELEMENT_ID}] = "L12";
        backend.ints[{OMS_HANDLE, prop::OMS_MONITORED_SIDE}] = static_cast<int>(ThreeSides::TWO);
        backend.related[{OMS_HANDLE, prop::REL_SUBSTATION}] = SUBSTATION_HANDLE;
        backend.children[{OMS_HANDLE, prop::TRIPPING}] = {TRIPPING_HANDLE};
        backend.ints[{TRIPPING_HANDLE, prop::TRIPPING_TYPE}] = static_cast<int>(OMS::Tripping::Type::BRANCH_TRIPPING);
        backend.bools[{TRIPPING_HANDLE, prop::BRANCH_TRIPPING}] = true;
        backend.strings[{TRIPPING_HANDLE, prop::TRIPPING_ELEMENT_ID}] = "T2";
        backend.ints[{TRIPPING_HANDLE, prop::TRIPPING_SIDE}] = static_cast<int>(TwoSides::TWO);
    }
};

TEST_F(OverloadManagementSystemTest, Values) {
    OMS oms(OMS_HANDLE, &backend);
    EXPECT_TRUE(oms.isEnabled());
    EXPECT_EQ(oms.getMonitoredElementId(), "L12");
    EXPECT_EQ(oms.getMonitoredSide(), ThreeSides::TWO);
    EXPECT_EQ(oms.getSubstation(), Substation(SUBSTATION_HANDLE, &backend));
    oms.setEnabled(false);
    EXPECT_FALSE((backend.bools[{OMS_HANDLE, prop::AUTOMATION_SYSTEM_ENABLED}]));
}

TEST_F(OverloadManagementSystemTest, Trippings) {
    OMS oms(OMS_HANDLE, &backend);
    std::vector<OMS::Tripping> trippings = oms.getTrippings();
    ASSERT_EQ(trippings.size(), 1u);
    EXPECT_EQ(trippings[0].getType(), OMS::Tripping::Type::BRANCH_TRIPPING);
    OMS::BranchTripping branch = trippings[0].as<OMS::BranchTripping>();
    EXPECT_EQ(branch.getBranchToOperateId(), "T2");
    EXPECT_EQ(branch.getSideToOperate(), TwoSides::TWO);
    EXPECT_THROW(trippings[0].as<OMS::SwitchTripping>(), PropertyNotFoundException);
    branch.setSideToOperate(TwoSides::ONE).setName("trip");
    EXPECT_EQ((backend.ints[{TRIPPING_HANDLE, prop::TRIPPING_SIDE}]), static_cast<int>(TwoSides::ONE));
    EXPECT_EQ((backend.strings[{TRIPPING_HANDLE, prop::TRIPPING_NAME}]), "trip");
    oms.addTripping(branch);
    EXPECT_EQ((backend.related[{OMS_HANDLE, prop::ADD_TRIPPING}]), ObjectHandle(TRIPPING_HANDLE));
}

TEST_F(OverloadManagementSystemTest, Adder) {
    // The system's adder, then a nested adder per tripping
    OverloadManagementSystemAdder adder(backend.invoke(SUBSTATION_HANDLE, prop::OVERLOAD_MANAGEMENT_SYSTEM), &backend);
    adder.setMonitoredElementId("L12").setMonitoredElementSide(ThreeSides::ONE)
         .newThreeWindingsTransformerTripping().setKey("K").setCurrentLimit(900).setOpenAction(false)
             .setThreeWindingsTransformerToOperateId("T3").setSideToOperate(ThreeSides::THREE).add()
         .setEnabled(true);
    EXPECT_EQ((backend.strings[{ADDER, prop::OMS_MONITORED_ELEMENT_ID}]), "L12");
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(ADDER), prop::THREE_WINDINGS_TRANSFORMER_TRIPPING));
    EXPECT_EQ((backend.strings[{ADDER + 1, prop::TRIPPING_ELEMENT_ID}]), "T3");
    EXPECT_EQ((backend.ints[{ADDER + 1, prop::TRIPPING_SIDE}]), static_cast<int>(ThreeSides::THREE));
    EXPECT_DOUBLE_EQ((backend.doubles[{ADDER + 1, prop::TRIPPING_CURRENT_LIMIT}]), 900.0);
    EXPECT_EQ(backend.invocations[2], std::make_pair(ObjectHandle(ADDER + 1), prop::ACT_ADD));
    EXPECT_TRUE((backend.bools[{ADDER, prop::AUTOMATION_SYSTEM_ENABLED}]));
}
