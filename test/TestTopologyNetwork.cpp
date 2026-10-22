#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "RealBackends.h"
#include <algorithm>
#include <cmath>
#include <set>
#include <string>
#include <vector>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif
#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include "jni/JNIBackend.h"
#endif

// Terminals, connection through switches, buses and the topology views of voltage levels on the
// real backends (see RealBackends.h): the same scenario on GraalVM and on JNI, with the network
// test/data/topology_test.xiidm (VL1 bus-breaker, VL2 node-breaker, in S1; VL3 in S2).

using namespace iidm;
using iidm::test::envPath;

namespace {

template <class T>
std::set<std::string> idsOf(const std::vector<T>& objects) {
    std::set<std::string> ids;
    for (const T& object : objects) ids.insert(object.getId());
    return ids;
}

void checkTerminals(BackendProvider& b) {
    VoltageLevel vl1(b.findById(prop::VOLTAGE_LEVEL, "VL1"), &b);
    VoltageLevel vl2(b.findById(prop::VOLTAGE_LEVEL, "VL2"), &b);
    Load l1(b.findById(prop::LOAD, "L1"), &b);
    Terminal t1 = l1.getTerminal();

    // Bus-breaker view: B1b; bus view: the bus that B1 and B1b make (the switch is closed)
    EXPECT_EQ(t1.getBusBreakerView().getBus()->getId(), "B1b");
    EXPECT_EQ(t1.getBusBreakerView().getConnectableBus()->getId(), "B1b");
    Bus merged = vl1.getBusView().getBuses()[0];
    EXPECT_EQ(*t1.getBusView().getBus(), merged);
    EXPECT_EQ(*t1.getBusView().getConnectableBus(), merged);
    // From its flows (60 MW, 10 Mvar) and the voltage of its bus (402 kV)
    EXPECT_NEAR(t1.getI(), std::hypot(60.0, 10.0) / (std::sqrt(3.0) * 402.0 / 1000), 1e-9);

    // Node-breaker topology: its buses are made of nodes
    Load l2(b.findById(prop::LOAD, "L2"), &b);
    Terminal t2 = l2.getTerminal();
    EXPECT_TRUE(t2.getBusBreakerView().getBus());
    EXPECT_EQ(*t2.getBusView().getBus(), *vl2.getBusView().getMergedBus("BBS1"));
    EXPECT_THROW(t2.getBusBreakerView().setConnectableBus("BBS1"), ValidationException);

    // Disconnected: no bus, still a connectable one
    ASSERT_TRUE(t1.disconnect());
    EXPECT_FALSE(t1.getBusBreakerView().getBus());
    EXPECT_FALSE(t1.getBusView().getBus());
    EXPECT_EQ(t1.getBusBreakerView().getConnectableBus()->getId(), "B1b");
    ASSERT_TRUE(t1.connect());

    t1.unsetSolvedValues();
    EXPECT_TRUE(std::isnan(t1.getP()));
    EXPECT_TRUE(std::isnan(t1.getQ()));
}

void checkMoves(BackendProvider& b) {
    Load l1(b.findById(prop::LOAD, "L1"), &b);
    l1.getTerminal().getBusBreakerView().setConnectableBus("B1");
    EXPECT_EQ(l1.getTerminal().getBusBreakerView().getBus()->getId(), "B1"); // still connected

    // To a bus of another voltage level: the load gets a new terminal there
    l1.getTerminal().getBusBreakerView().moveConnectable("B3", false);
    EXPECT_EQ(l1.getTerminal().getVoltageLevel().getId(), "VL3");
    EXPECT_FALSE(l1.getTerminal().isConnected());
    EXPECT_EQ(l1.getTerminal().getBusBreakerView().getConnectableBus()->getId(), "B3");

    // To a node, of the same voltage level or of another one
    Load l2(b.findById(prop::LOAD, "L2"), &b);
    l2.getTerminal().getNodeBreakerView().moveConnectable(10, "VL2");
    EXPECT_EQ(l2.getTerminal().getNodeBreakerView().getNode(), 10);
    Load l3(b.findById(prop::LOAD, "L3"), &b);
    l3.getTerminal().getNodeBreakerView().moveConnectable(11, "VL2");
    EXPECT_EQ(l3.getTerminal().getVoltageLevel().getId(), "VL2");
    EXPECT_EQ(l3.getTerminal().getNodeBreakerView().getNode(), 11);
    EXPECT_THROW(l3.getTerminal().getNodeBreakerView().moveConnectable(12, "NONE"), ValidationException);
}

// Through the switches a predicate accepts: L2 is behind D_L2 (a disconnector) and BR_L2 (a
// breaker), T2's side TWO behind D_T2 and BR_T2
void checkSwitchPredicates(BackendProvider& b) {
    VoltageLevel::NodeBreakerView view = VoltageLevel(b.findById(prop::VOLTAGE_LEVEL, "VL2"), &b).getNodeBreakerView();
    Switch disconnector = *view.getSwitch("D_L2");
    Switch breaker = *view.getSwitch("BR_L2");
    Load l2(b.findById(prop::LOAD, "L2"), &b);

    // By default, breakers only: not through an open disconnector
    EXPECT_TRUE(l2.disconnect());
    EXPECT_TRUE(breaker.isOpen());
    EXPECT_FALSE(disconnector.isOpen());
    disconnector.setOpen(true);
    EXPECT_FALSE(l2.connect());
    EXPECT_TRUE(l2.connect(SwitchPredicate::BREAKER_OR_DISCONNECTOR));
    EXPECT_FALSE(disconnector.isOpen());
    EXPECT_FALSE(breaker.isOpen());
    EXPECT_TRUE(l2.getTerminal().isConnected());

    // A fictitious breaker: only by the predicates that accept one
    breaker.setFictitious(true);
    EXPECT_FALSE(l2.getTerminal().disconnect());
    EXPECT_TRUE(l2.getTerminal().disconnect(SwitchPredicate::CLOSED_BREAKER));
    EXPECT_TRUE(breaker.isOpen());
    EXPECT_FALSE(l2.getTerminal().connect(SwitchPredicate::NONFICTIONAL_BREAKER));
    EXPECT_TRUE(l2.getTerminal().connect(SwitchPredicate::BREAKER));
    EXPECT_FALSE(l2.getTerminal().connect(SwitchPredicate::BREAKER)); // already

    // One side of a branch: side ONE, in the bus-breaker VL1, stays connected
    TwoWindingsTransformer t2(b.findById(prop::TWO_WINDINGS_TRANSFORMER, "T2"), &b);
    EXPECT_TRUE(t2.disconnect(SwitchPredicate::NONFICTIONAL_CLOSED_BREAKER, ThreeSides::TWO));
    EXPECT_TRUE(view.getSwitch("BR_T2")->isOpen());
    EXPECT_TRUE(t2.getTerminal1().isConnected());
    EXPECT_FALSE(t2.getTerminal2().isConnected());
    EXPECT_TRUE(t2.connect(SwitchPredicate::NONFICTIONAL_BREAKER, ThreeSides::TWO));
    EXPECT_TRUE(t2.getTerminal2().isConnected());
}

void checkBuses(BackendProvider& b) {
    VoltageLevel vl1(b.findById(prop::VOLTAGE_LEVEL, "VL1"), &b);
    Bus b1 = *vl1.getBusBreakerView().getBus("B1");
    Bus b1b = *vl1.getBusBreakerView().getBus("B1b");

    // What is connected to them
    EXPECT_EQ(idsOf(b1b.getLoads()), std::set<std::string>{"L1"});
    EXPECT_TRUE(b1b.getGenerators().empty());
    EXPECT_EQ(b1b.getConnectedTerminalCount(), 1);
    ASSERT_EQ(b1b.getConnectedTerminals().size(), 1u);
    EXPECT_EQ(b1b.getConnectedTerminals()[0].getConnectable().getId(), "L1");
    EXPECT_EQ(idsOf(b1.getGenerators()), std::set<std::string>{"G1"});
    EXPECT_EQ(idsOf(b1.getLines()), std::set<std::string>{"LINE13"});
    EXPECT_EQ(idsOf(b1.getTwoWindingsTransformers()), std::set<std::string>{"T2"});
    EXPECT_EQ(b1.getConnectedTerminalCount(), 3);
    EXPECT_TRUE(b1.getBoundaryLines().empty());
    EXPECT_TRUE(b1.getBoundaryLines(BoundaryLineFilter::PAIRED).empty());

    // Powers of the injections (load sign convention): G1 makes 100 MW, L1 takes 60 MW
    EXPECT_DOUBLE_EQ(b1.getP(), -100.0);
    EXPECT_DOUBLE_EQ(b1b.getP(), 60.0);
    EXPECT_DOUBLE_EQ(b1b.getQ(), 10.0);
    Bus merged = vl1.getBusView().getBuses()[0];
    EXPECT_DOUBLE_EQ(merged.getP(), -40.0);
    EXPECT_EQ(merged.getConnectedTerminalCount(), 4);
    EXPECT_TRUE(merged.isInMainConnectedComponent());
    EXPECT_TRUE(merged.isInMainSynchronousComponent());

    // Values
    Bus b3 = *VoltageLevel(b.findById(prop::VOLTAGE_LEVEL, "VL3"), &b).getBusBreakerView().getBus("B3");
    EXPECT_DOUBLE_EQ(b3.getFictitiousP0(), 0.0);
    b3.setFictitiousP0(2.5).setFictitiousQ0(0.5);
    EXPECT_DOUBLE_EQ(b3.getFictitiousP0(), 2.5);
    EXPECT_DOUBLE_EQ(b3.getFictitiousQ0(), 0.5);
    b3.unsetSolvedValues();
    EXPECT_TRUE(std::isnan(b3.getV()));
    EXPECT_TRUE(std::isnan(b3.getAngle()));
}

void checkNodeBreakerView(BackendProvider& b) {
    VoltageLevel vl2(b.findById(prop::VOLTAGE_LEVEL, "VL2"), &b);
    VoltageLevel::NodeBreakerView view = vl2.getNodeBreakerView();
    EXPECT_GE(view.getMaximumNodeIndex(), 9);
    EXPECT_EQ(view.getTerminal(5)->getConnectable().getId(), "L2");
    EXPECT_EQ(view.getTerminal(0)->getConnectable().getId(), "BBS1");
    EXPECT_FALSE(view.getTerminal(4));
    EXPECT_TRUE(view.hasAttachedEquipment(8));  // an internal connection
    EXPECT_TRUE(view.hasAttachedEquipment(4));  // switches
    EXPECT_FALSE(view.hasAttachedEquipment(20));

    // The fictitious injection at a node
    EXPECT_DOUBLE_EQ(view.getFictitiousP0(5), 1.5);
    EXPECT_DOUBLE_EQ(view.getFictitiousQ0(5), 0.5);
    view.setFictitiousP0(5, 3.0).setFictitiousQ0(4, -1.0);
    EXPECT_DOUBLE_EQ(view.getFictitiousP0(5), 3.0);
    EXPECT_DOUBLE_EQ(view.getFictitiousQ0(4), -1.0);

    // Switches
    EXPECT_EQ(view.getSwitchCount(), 7);
    EXPECT_EQ(idsOf(view.getSwitches(0)), (std::set<std::string>{"D_C1", "D_L2"}));
    EXPECT_EQ(view.getNode1("BR_C"), 2);
    EXPECT_EQ(view.getNode2("BR_C"), 3);
    EXPECT_THROW(view.getNode1("NONE"), ValidationException);
    EXPECT_EQ(view.getTerminal1("D_L2")->getConnectable().getId(), "BBS1");
    EXPECT_FALSE(view.getTerminal2("D_L2"));
    view.removeSwitch("D_T2");
    EXPECT_EQ(view.getSwitchCount(), 6);
    EXPECT_FALSE(view.getSwitch("D_T2"));
    EXPECT_THROW(view.removeSwitch("D_T2"), ValidationException);

    // Busbar sections and internal connections
    EXPECT_EQ(view.getBusbarSectionCount(), 2);
    EXPECT_EQ(view.getBusbarSection("BBS2")->getId(), "BBS2");
    EXPECT_FALSE(view.getBusbarSection("NONE"));
    EXPECT_EQ(view.getInternalConnectionCount(), 1);
    EXPECT_EQ(view.getNodesInternalConnectedTo(8), std::vector<int>{9});
    view.removeInternalConnections(9, 8);
    EXPECT_EQ(view.getInternalConnectionCount(), 0);
    // Nothing left at these nodes: they are gone
    EXPECT_FALSE(view.hasAttachedEquipment(8));
    EXPECT_THROW(view.getNodesInternalConnectedTo(8), ValidationException);
}

void checkBusBreakerView(BackendProvider& b) {
    VoltageLevel vl1(b.findById(prop::VOLTAGE_LEVEL, "VL1"), &b);
    VoltageLevel::BusBreakerView view = vl1.getBusBreakerView();
    EXPECT_EQ(view.getBusCount(), 2);
    EXPECT_EQ(view.getSwitchCount(), 1);
    EXPECT_EQ(view.getSwitch("SW_B1")->getId(), "SW_B1");
    EXPECT_FALSE(view.getSwitch("NONE"));
    std::vector<Bus> buses = view.getBusesFromBusViewBusId(vl1.getBusView().getBuses()[0].getId());
    ASSERT_EQ(buses.size(), 2u);
    EXPECT_EQ(buses[0].getId(), "B1");
    EXPECT_EQ(buses[1].getId(), "B1b");

    // A switch is left: the buses cannot go
    EXPECT_THROW(view.removeAllBuses(), ValidationException);
    view.removeSwitch("SW_B1");
    EXPECT_EQ(view.getSwitchCount(), 0);
    EXPECT_EQ(vl1.getBusView().getBuses().size(), 2u); // no longer merged
    EXPECT_THROW(view.removeSwitch("SW_B1"), ValidationException);
    view.removeAllSwitches();

    // Without equipment, they can
    Network network = vl1.getNetwork();
    VoltageLevel vl4 = network.newVoltageLevel().setId("VL4").setNominalV(63).setTopologyKind(TopologyKind::BUS_BREAKER).add();
    vl4.getBusBreakerView().newBus().setId("B4").add();
    vl4.getBusBreakerView().newBus().setId("B4b").add();
    EXPECT_EQ(vl4.getBusBreakerView().getBusCount(), 2);
    vl4.getBusBreakerView().removeAllBuses();
    EXPECT_EQ(vl4.getBusBreakerView().getBusCount(), 0);
}

// Lists (the switches at a node...) leave no Java reference behind
void checkHandlesAreReleased(BackendProvider& b) {
    VoltageLevel vl2(b.findById(prop::VOLTAGE_LEVEL, "VL2"), &b);
    VoltageLevel::NodeBreakerView view = vl2.getNodeBreakerView();
    const std::size_t baseline = b.liveHandleCount();
    std::size_t sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += view.getSwitches(0).size() + view.getNodesInternalConnectedTo(8).size();
        sum += view.getTerminal1("D_L2") ? 1 : 0;
    }
    EXPECT_EQ(b.liveHandleCount(), baseline);
    EXPECT_EQ(sum, 400u);
}

void load(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/topology_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/topology_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
}

void checkTopology(BackendProvider& b) {
    load(b);
    checkHandlesAreReleased(b);
    checkBuses(b);
    checkTerminals(b);
    checkNodeBreakerView(b);
    checkBusBreakerView(b);
    load(b);
    checkMoves(b);
    load(b);
    checkSwitchPredicates(b);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(TopologyNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkTopology(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(TopologyNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("topology_test");
    checkTopology(backend);
}
#endif
