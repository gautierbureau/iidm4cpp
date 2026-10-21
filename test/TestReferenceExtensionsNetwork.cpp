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

// Reference priorities, reference terminals and observability areas on the real backends (see
// RealBackends.h): the same scenario on GraalVM and on JNI, with the network
// test/data/references_test.xiidm. Priorities: G1 2, G2 1, L12 side 2 3, LD 0; reference terminals:
// G1's; observability areas of VL1 (bus-breaker: area 1 of B1a and B1b) and VL3 (node-breaker: area 1
// of nodes 0 and 1, area 2 of node 2).

using namespace iidm;
using iidm::test::envPath;

namespace {

using Ids = std::vector<std::string>;

Ids prioritiesOf(const std::vector<ReferencePriority>& priorities) {
    Ids ids;
    for (const ReferencePriority& priority : priorities) {
        ids.push_back(priority.getTerminal().getConnectable().getId() + "=" + std::to_string(priority.getPriority()));
    }
    return ids;
}

void checkPriorities(Network& network) {
    EXPECT_EQ(prioritiesOf(ReferencePriorities::get(network)), (Ids{"G2=1", "G1=2", "L12=3"}));
    Generator g1 = *network.getGenerator("G1");
    EXPECT_EQ(prioritiesOf(g1.getExtension<ReferencePriorities>().getReferencePriorities()), (Ids{"G1=2"}));
    EXPECT_EQ(ReferencePriority::get(g1), 2);
    Line line = *network.getLine("L12");
    EXPECT_EQ(ReferencePriority::get(line, TwoSides::TWO), 3);
    EXPECT_EQ(ReferencePriority::get(line, TwoSides::ONE), 0);
    EXPECT_EQ(ReferencePriority::get(*network.getLoad("LD")), 0);

    // Set: the extension created when absent, the priority of a terminal replaced
    Load ld2 = *network.getLoad("LD2");
    EXPECT_FALSE(ld2.hasExtension<ReferencePriorities>());
    ReferencePriority::set(ld2, 4);
    EXPECT_EQ(ReferencePriority::get(ld2), 4);
    ReferencePriority::set(line, TwoSides::ONE, 5);
    EXPECT_EQ(ReferencePriority::get(line, TwoSides::ONE), 5);
    EXPECT_EQ(line.getExtension<ReferencePriorities>().getReferencePriorities().size(), 2u);
    EXPECT_THROW(ReferencePriority::set(g1, -1), ValidationException);
    ReferencePriority replaced = g1.getExtension<ReferencePriorities>().newReferencePriority().setTerminal(g1.getTerminal()).setPriority(6).add();
    EXPECT_EQ(replaced.getPriority(), 6);
    EXPECT_EQ(prioritiesOf(g1.getExtension<ReferencePriorities>().getReferencePriorities()), (Ids{"G1=6"}));
    // Not a terminal of the generator
    EXPECT_THROW(g1.getExtension<ReferencePriorities>().newReferencePriority().setTerminal(ld2.getTerminal()).setPriority(1).add(),
                 ValidationException);

    g1.getExtension<ReferencePriorities>().deleteReferencePriorities();
    EXPECT_TRUE(g1.getExtension<ReferencePriorities>().getReferencePriorities().empty());
    ReferencePriorities::deleteReferencePriorities(network);
    EXPECT_TRUE(ReferencePriorities::get(network).empty());
    Generator g2 = *network.getGenerator("G2");
    g2.removeExtension<ReferencePriorities>();
    EXPECT_TRUE(g2.newExtension<ReferencePrioritiesAdder>().add().getReferencePriorities().empty());
}

void checkTerminals(Network& network) {
    Terminal g1 = network.getGenerator("G1")->getTerminal();
    Terminal g2 = network.getGenerator("G2")->getTerminal();
    ReferenceTerminals terminals = network.getExtension<ReferenceTerminals>();
    std::vector<Terminal> listed = terminals.getReferenceTerminals();
    ASSERT_EQ(listed.size(), 1u);
    EXPECT_EQ(listed[0], g1);
    EXPECT_EQ(terminals.addReferenceTerminal(g2).getReferenceTerminals().size(), 2u);
    terminals.setReferenceTerminals({g2});
    EXPECT_EQ(ReferenceTerminals::getTerminals(network).size(), 1u);
    EXPECT_EQ(ReferenceTerminals::getTerminals(network)[0], g2);
    EXPECT_TRUE(terminals.reset().getReferenceTerminals().empty());
    ReferenceTerminals::addTerminal(g1);
    EXPECT_EQ(ReferenceTerminals::getTerminals(network)[0], g1);
    ReferenceTerminals::reset(network);
    EXPECT_TRUE(ReferenceTerminals::getTerminals(network).empty());

    network.removeExtension<ReferenceTerminals>();
    EXPECT_TRUE(ReferenceTerminals::getTerminals(network).empty());
    EXPECT_EQ(network.newExtension<ReferenceTerminalsAdder>().withTerminals({g1, g2}).add().getReferenceTerminals().size(), 2u);
}

void checkObservabilityAreas(Network& network) {
    VoltageLevel vl3 = *network.getVoltageLevel("VL3");
    std::vector<ObservabilityArea::AreaCharacteristics> areas = vl3.getExtension<ObservabilityArea>().getObservabilityAreas();
    ASSERT_EQ(areas.size(), 2u);
    EXPECT_EQ(areas[1].getNodes(), (std::vector<int>{2}));
    EXPECT_EQ(areas[1].getStatus(), ObservabilityStatus::NON_OBSERVABLE);

    // New ones: by nodes, by buses of the bus-breaker view, by a bus of the bus view
    vl3.removeExtension<ObservabilityArea>();
    ObservabilityArea byNodes = vl3.newExtension<ObservabilityAreaAdder>()
        .withObservabilityAreaByNodes({0, 1, 2}, 5, ObservabilityStatus::OBSERVABLE)
        .add();
    ASSERT_EQ(byNodes.getObservabilityAreas().size(), 1u);
    EXPECT_EQ(byNodes.getObservabilityAreas()[0].getAreaNumber(), 5);
    EXPECT_EQ(byNodes.getObservabilityAreas()[0].getNodes(), (std::vector<int>{0, 1, 2}));

    VoltageLevel vl1 = *network.getVoltageLevel("VL1");
    vl1.removeExtension<ObservabilityArea>();
    vl1.newExtension<ObservabilityAreaAdder>().withObservabilityAreaByBusBreakerViewBuses({"B1a", "B1b"}, 6, ObservabilityStatus::BORDER).add();
    EXPECT_EQ(vl1.getExtension<ObservabilityArea>().getObservabilityAreas()[0].getBuses().size(), 2u);
    vl1.removeExtension<ObservabilityArea>();
    std::string busId = vl1.getBusView().getBuses()[0].getId();
    vl1.newExtension<ObservabilityAreaAdder>().withObservabilityAreaByBusViewBus(busId, 7, ObservabilityStatus::OBSERVABLE).add();
    EXPECT_EQ(vl1.getExtension<ObservabilityArea>().getObservabilityAreaByBus(busId)->getAreaNumber(), 7);
}

void checkNetwork(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/references_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/references_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    Network network = Identifiable(b.getNetworkHandle(), &b).getNetwork();
    checkPriorities(network);
    checkTerminals(network);
    checkObservabilityAreas(network);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(ReferenceExtensionsNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkNetwork(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(ReferenceExtensionsNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("references_test");
    checkNetwork(backend);
}
#endif
