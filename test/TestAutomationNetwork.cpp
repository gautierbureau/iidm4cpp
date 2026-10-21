#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "RealBackends.h"
#include <string>
#include <vector>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif
#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include "jni/JNIBackend.h"
#endif

// Overload management systems on the real backends (see RealBackends.h): the same scenario on
// GraalVM and on JNI, with the network test/data/automation_test.xiidm: OMS1 (enabled, L12 side 1
// monitored; trippings SW of the breaker BR, BRANCH of T2 side 2, T3 of T3 side 3) and OMS2
// (disabled), in the substation S.

using namespace iidm;
using iidm::test::envPath;

namespace {

using OMS = OverloadManagementSystem;

void checkSystems(BackendProvider& b) {
    Substation s(b.findById(prop::SUBSTATION, "S"), &b);
    Network network = s.getNetwork();
    EXPECT_EQ(s.getOverloadManagementSystemCount(), 2);
    EXPECT_EQ(network.getOverloadManagementSystemCount(), 2);
    ASSERT_EQ(network.getOverloadManagementSystems().size(), 2u);
    OMS oms = *network.getOverloadManagementSystem("OMS1");
    EXPECT_EQ(s.getOverloadManagementSystems()[0], oms);
    EXPECT_EQ(oms.getType(), IdentifiableType::OVERLOAD_MANAGEMENT_SYSTEM);
    EXPECT_EQ(oms.getNameOrId(), "Line protection");
    EXPECT_TRUE(oms.isEnabled());
    EXPECT_EQ(oms.getSubstation(), s);
    EXPECT_EQ(oms.getMonitoredElementId(), "L12");
    EXPECT_EQ(oms.getMonitoredSide(), ThreeSides::ONE);
    EXPECT_FALSE(network.getOverloadManagementSystem("OMS2")->isEnabled());

    // Its trippings, in order: one of each kind
    std::vector<OMS::Tripping> trippings = oms.getTrippings();
    ASSERT_EQ(trippings.size(), 3u);
    EXPECT_EQ(trippings[0].getType(), OMS::Tripping::Type::SWITCH_TRIPPING);
    EXPECT_EQ(trippings[0].getKey(), "SW");
    EXPECT_EQ(trippings[0].getNameOrKey(), "Open breaker");
    EXPECT_DOUBLE_EQ(trippings[0].getCurrentLimit(), 1000.0);
    EXPECT_TRUE(trippings[0].isOpenAction());
    EXPECT_EQ(trippings[0].as<OMS::SwitchTripping>().getSwitchToOperateId(), "BR");
    EXPECT_FALSE(trippings[0].is<OMS::BranchTripping>());
    OMS::BranchTripping branch = trippings[1].as<OMS::BranchTripping>();
    EXPECT_EQ(branch.getNameOrKey(), "BRANCH");
    EXPECT_EQ(branch.getBranchToOperateId(), "T2");
    EXPECT_EQ(branch.getSideToOperate(), TwoSides::TWO);
    OMS::ThreeWindingsTransformerTripping leg = trippings[2].as<OMS::ThreeWindingsTransformerTripping>();
    EXPECT_EQ(leg.getThreeWindingsTransformerToOperateId(), "T3");
    EXPECT_EQ(leg.getSideToOperate(), ThreeSides::THREE);
    EXPECT_FALSE(leg.isOpenAction());

    // Changes
    oms.setEnabled(false);
    EXPECT_FALSE(oms.isEnabled());
    branch.setName("Trip T2").setCurrentLimit(1100).setOpenAction(false);
    branch.setSideToOperate(TwoSides::ONE).setBranchToOperateId("L12");
    EXPECT_EQ(oms.getTrippings()[1].getNameOrKey(), "Trip T2");
    EXPECT_EQ(oms.getTrippings()[1].as<OMS::BranchTripping>().getSideToOperate(), TwoSides::ONE);
    leg.setProperty("source", "test");
    EXPECT_EQ(*oms.getTrippings()[2].getProperty("source"), "test");

    // A new system, then removed
    OMS created = s.newOverloadManagementSystem().setId("OMS3").setEnabled(true)
        .setMonitoredElementId("T2").setMonitoredElementSide(ThreeSides::TWO)
        .newSwitchTripping().setKey("K1").setCurrentLimit(800).setOpenAction(true).setSwitchToOperateId("BR_T2")
            .setProperty("p", "v").add()
        .newBranchTripping().setKey("K2").setName("line").setCurrentLimit(900).setOpenAction(true)
            .setBranchToOperateId("L12").setSideToOperate(TwoSides::ONE).add()
        .newThreeWindingsTransformerTripping().setKey("K3").setCurrentLimit(950).setOpenAction(false)
            .setThreeWindingsTransformerToOperateId("T3").setSideToOperate(ThreeSides::TWO).add()
        .add();
    ASSERT_EQ(created.getTrippings().size(), 3u);
    EXPECT_EQ(*created.getTrippings()[0].getProperty("p"), "v");
    EXPECT_EQ(created.getTrippings()[1].getNameOrKey(), "line");
    EXPECT_THROW(s.newOverloadManagementSystem().setId("OMS4").setMonitoredElementId("L12")
                     .setMonitoredElementSide(ThreeSides::ONE)
                     .newSwitchTripping().setKey("K").setCurrentLimit(1).setOpenAction(true).setSwitchToOperateId("NONE").add()
                     .add(),
                 ValidationException); // no such switch
    created.addTripping(trippings[0]);
    EXPECT_EQ(created.getTrippings().size(), 4u);
    created.remove();
    EXPECT_FALSE(network.getOverloadManagementSystem("OMS3"));
    EXPECT_EQ(s.getOverloadManagementSystemCount(), 2);
}

void checkNetwork(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/automation_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/automation_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    checkSystems(b);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(AutomationNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkNetwork(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(AutomationNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("automation_test");
    checkNetwork(backend);
}
#endif
