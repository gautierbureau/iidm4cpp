#include <gtest/gtest.h>
#include <iidm/Terminal.h>
#include <iidm/VoltageLevel.h>
#include <iidm/InternalConnection.h>
#include <iidm/Generator.h>
#include <iidm/Load.h>
#include <iidm/Battery.h>
#include <iidm/ShuntCompensator.h>
#include <iidm/StaticVarCompensator.h>
#include <iidm/BoundaryLine.h>
#include <iidm/LccConverterStation.h>
#include <iidm/VscConverterStation.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

static constexpr ObjectHandle VL_HANDLE   = 10;
static constexpr ObjectHandle TERM_HANDLE = 11;
static constexpr ObjectHandle BUS1        = 20;
static constexpr ObjectHandle BUS2        = 21;
static constexpr ObjectHandle SW1         = 30;
static constexpr ObjectHandle BBS1        = 40;
static constexpr ObjectHandle IC1         = 50;
static constexpr ObjectHandle IC2         = 51;

class TopologyViewTest : public ::testing::Test {
protected:
    MockBackend backend;

    void SetUp() override {
        // VoltageLevel
        backend.strings[{VL_HANDLE, prop::ID}]       = "VL1";
        backend.strings[{VL_HANDLE, prop::NAME}]      = "Voltage Level 1";
        backend.doubles[{VL_HANDLE, prop::VL_NOMINAL_V}] = 400.0;
        backend.ints   [{VL_HANDLE, prop::VL_TOPOLOGY_KIND}] = 0; // NODE_BREAKER

        // Switches and busbar sections under the VL
        backend.children[{VL_HANDLE, prop::SWITCH}]        = {SW1};
        backend.children[{VL_HANDLE, prop::REL_NBV_SWITCH}] = {SW1};
        backend.children[{VL_HANDLE, prop::REL_BBV_SWITCH}] = {SW1};
        backend.children[{VL_HANDLE, prop::BUSBAR_SECTION}] = {BBS1};
        backend.strings[{SW1,  prop::ID}]  = "SW1";
        backend.strings[{BBS1, prop::ID}]  = "BBS1";

        // Buses under the VL (bus-breaker view)
        backend.children[{VL_HANDLE, prop::BUS}] = {BUS1, BUS2};
        backend.strings[{BUS1, prop::ID}]   = "BUS1";
        backend.doubles[{BUS1, prop::BUS_V}] = 400.0;
        backend.strings[{BUS2, prop::ID}]   = "BUS2";
        backend.doubles[{BUS2, prop::BUS_V}] = 398.0;

        // Terminal
        backend.doubles[{TERM_HANDLE, prop::TERMINAL_P}]        = 100.0;
        backend.ints   [{TERM_HANDLE, prop::TERMINAL_NODE}]      = 7;
        backend.bools  [{TERM_HANDLE, prop::TERMINAL_CONNECTED}] = true;
        // bus-breaker view: the bus it is connected to, the one it can connect to
        backend.related[{TERM_HANDLE, prop::REL_BUS}]            = BUS1;
        backend.related[{TERM_HANDLE, prop::REL_CONNECTABLE_BUS}] = BUS2;
    }
};

// ── VoltageLevel::NodeBreakerView ─────────────────────────────────────────────

TEST_F(TopologyViewTest, VlNodeBreakerViewIsValid) {
    VoltageLevel vl(VL_HANDLE, &backend);
    EXPECT_TRUE(vl.getNodeBreakerView().isValid());
}

TEST_F(TopologyViewTest, VlNodeBreakerViewGetSwitches) {
    VoltageLevel vl(VL_HANDLE, &backend);
    auto sw = vl.getNodeBreakerView().getSwitches();
    ASSERT_EQ(sw.size(), 1u);
    EXPECT_EQ(sw[0].getId(), "SW1");
}

TEST_F(TopologyViewTest, VlNodeBreakerViewGetBusbarSections) {
    VoltageLevel vl(VL_HANDLE, &backend);
    auto bbs = vl.getNodeBreakerView().getBusbarSections();
    ASSERT_EQ(bbs.size(), 1u);
    EXPECT_EQ(bbs[0].getId(), "BBS1");
}

// ── VoltageLevel::BusBreakerView ──────────────────────────────────────────────

TEST_F(TopologyViewTest, VlBusBreakerViewIsValid) {
    VoltageLevel vl(VL_HANDLE, &backend);
    EXPECT_TRUE(vl.getBusBreakerView().isValid());
}

TEST_F(TopologyViewTest, VlBusBreakerViewGetBuses) {
    VoltageLevel vl(VL_HANDLE, &backend);
    auto buses = vl.getBusBreakerView().getBuses();
    ASSERT_EQ(buses.size(), 2u);
    EXPECT_EQ(buses[0].getId(), "BUS1");
    EXPECT_EQ(buses[1].getId(), "BUS2");
}

TEST_F(TopologyViewTest, VlBusBreakerViewGetSwitches) {
    VoltageLevel vl(VL_HANDLE, &backend);
    auto sw = vl.getBusBreakerView().getSwitches();
    ASSERT_EQ(sw.size(), 1u);
    EXPECT_EQ(sw[0].getId(), "SW1");
}

TEST_F(TopologyViewTest, VlBusBreakerViewEmptyWhenNoBuses) {
    MockBackend b2;
    b2.strings[{VL_HANDLE, prop::ID}] = "VL2";
    VoltageLevel vl(VL_HANDLE, &b2);
    EXPECT_EQ(vl.getBusBreakerView().getBuses().size(), 0u);
}

TEST_F(TopologyViewTest, VlBusBreakerViewGetBus1BySwitchId) {
    backend.relatedByString[{VL_HANDLE, prop::REL_VL_BBV_BUS1, "SW1"}] = BUS1;
    VoltageLevel vl(VL_HANDLE, &backend);
    Bus b = vl.getBusBreakerView().getBus1("SW1");
    EXPECT_TRUE(b.isValid());
    EXPECT_EQ(b.getId(), "BUS1");
}

TEST_F(TopologyViewTest, VlBusBreakerViewGetBus2BySwitchId) {
    backend.relatedByString[{VL_HANDLE, prop::REL_VL_BBV_BUS2, "SW1"}] = BUS2;
    VoltageLevel vl(VL_HANDLE, &backend);
    Bus b = vl.getBusBreakerView().getBus2("SW1");
    EXPECT_TRUE(b.isValid());
    EXPECT_EQ(b.getId(), "BUS2");
}

TEST_F(TopologyViewTest, VlBusBreakerViewGetBus1MissingSwitchId) {
    VoltageLevel vl(VL_HANDLE, &backend);
    Bus b = vl.getBusBreakerView().getBus1("NONEXISTENT");
    EXPECT_FALSE(b.isValid());
}

// ── Terminal::NodeBreakerView ─────────────────────────────────────────────────

TEST_F(TopologyViewTest, TerminalNodeBreakerViewIsValid) {
    Terminal t(TERM_HANDLE, &backend);
    EXPECT_TRUE(t.getNodeBreakerView().isValid());
}

TEST_F(TopologyViewTest, TerminalNodeBreakerViewGetNode) {
    Terminal t(TERM_HANDLE, &backend);
    EXPECT_EQ(t.getNodeBreakerView().getNode(), 7);
}

// ── Terminal::BusBreakerView ──────────────────────────────────────────────────

TEST_F(TopologyViewTest, TerminalBusBreakerViewIsValid) {
    Terminal t(TERM_HANDLE, &backend);
    EXPECT_TRUE(t.getBusBreakerView().isValid());
}

TEST_F(TopologyViewTest, TerminalBusBreakerViewGetBus) {
    Terminal t(TERM_HANDLE, &backend);
    iidm::optional<Bus> b = t.getBusBreakerView().getBus();
    ASSERT_TRUE(b);
    EXPECT_EQ(b->getId(), "BUS1");
}

TEST_F(TopologyViewTest, TerminalBusBreakerViewGetConnectableBus) {
    Terminal t(TERM_HANDLE, &backend);
    iidm::optional<Bus> b = t.getBusBreakerView().getConnectableBus();
    ASSERT_TRUE(b);
    EXPECT_EQ(b->getId(), "BUS2");
}

TEST_F(TopologyViewTest, TerminalBusBreakerViewGetBusEmptyWhenDisconnected) {
    MockBackend b2;
    // No REL_BUS set → INVALID_HANDLE → no bus
    Terminal t(TERM_HANDLE, &b2);
    EXPECT_FALSE(t.getBusBreakerView().getBus());
}

TEST_F(TopologyViewTest, TerminalBusBreakerViewConnectableBusAlwaysPresent) {
    // The connectable bus should exist independently of connection state.
    MockBackend b2;
    b2.related[{TERM_HANDLE, prop::REL_CONNECTABLE_BUS}] = BUS1;
    b2.strings[{BUS1, prop::ID}] = "BUS1";
    b2.bools[{TERM_HANDLE, prop::TERMINAL_CONNECTED}] = false;
    Terminal t(TERM_HANDLE, &b2);
    EXPECT_FALSE(t.isConnected());
    iidm::optional<Bus> connectable = t.getBusBreakerView().getConnectableBus();
    ASSERT_TRUE(connectable);
    EXPECT_EQ(connectable->getId(), "BUS1");
}

// ── Consistency with existing VoltageLevel API ────────────────────────────────

TEST_F(TopologyViewTest, NodeBreakerViewMatchesDirectAccessors) {
    VoltageLevel vl(VL_HANDLE, &backend);
    // NodeBreakerView::getSwitches() must return the same set as getSwitches()
    EXPECT_EQ(vl.getSwitches().size(),
              vl.getNodeBreakerView().getSwitches().size());
    EXPECT_EQ(vl.getBusbarSections().size(),
              vl.getNodeBreakerView().getBusbarSections().size());
}

// ── VoltageLevel::NodeBreakerView::getInternalConnections ────────────────────

TEST_F(TopologyViewTest, VlNodeBreakerViewGetInternalConnectionsEmpty) {
    VoltageLevel vl(VL_HANDLE, &backend);
    // No internal connections registered → empty vector
    EXPECT_EQ(vl.getNodeBreakerView().getInternalConnections().size(), 0u);
}

TEST_F(TopologyViewTest, VlNodeBreakerViewGetInternalConnections) {
    backend.children[{VL_HANDLE, prop::INTERNAL_CONNECTION}] = {IC1, IC2};
    backend.ints[{IC1, prop::IC_NODE1}] = 0;
    backend.ints[{IC1, prop::IC_NODE2}] = 1;
    backend.ints[{IC2, prop::IC_NODE1}] = 2;
    backend.ints[{IC2, prop::IC_NODE2}] = 3;

    VoltageLevel vl(VL_HANDLE, &backend);
    auto ics = vl.getNodeBreakerView().getInternalConnections();
    ASSERT_EQ(ics.size(), 2u);
    EXPECT_TRUE(ics[0].isValid());
    EXPECT_EQ(ics[0].getNode1(), 0);
    EXPECT_EQ(ics[0].getNode2(), 1);
    EXPECT_EQ(ics[1].getNode1(), 2);
    EXPECT_EQ(ics[1].getNode2(), 3);
}

// ── Terminal::getVoltageLevel ─────────────────────────────────────────────────

TEST_F(TopologyViewTest, TerminalGetVoltageLevel) {
    backend.related[{TERM_HANDLE, prop::REL_VOLTAGE_LEVEL}] = VL_HANDLE;
    backend.strings[{VL_HANDLE, prop::ID}] = "VL1";

    Terminal t(TERM_HANDLE, &backend);
    VoltageLevel vl = t.getVoltageLevel();
    EXPECT_TRUE(vl.isValid());
    EXPECT_EQ(vl.getId(), "VL1");
}

TEST_F(TopologyViewTest, TerminalGetVoltageLevelInvalidWhenMissing) {
    MockBackend b2;
    Terminal t(TERM_HANDLE, &b2);
    VoltageLevel vl = t.getVoltageLevel();
    EXPECT_FALSE(vl.isValid());
}

// ── Bus::getVoltageLevel ──────────────────────────────────────────────────────

TEST_F(TopologyViewTest, BusGetVoltageLevel) {
    backend.related[{BUS1, prop::REL_VOLTAGE_LEVEL}] = VL_HANDLE;
    Bus bus(BUS1, &backend);
    VoltageLevel vl = bus.getVoltageLevel();
    EXPECT_TRUE(vl.isValid());
    EXPECT_EQ(vl.getId(), "VL1");
}

TEST_F(TopologyViewTest, BusGetVoltageLevelInvalidWhenMissing) {
    MockBackend b2;
    Bus bus(BUS1, &b2);
    VoltageLevel vl = bus.getVoltageLevel();
    EXPECT_FALSE(vl.isValid());
}

// ── NodeBreakerView::getNodes / getSwitch(id) / getTerminal(node) ─────────────

TEST_F(TopologyViewTest, VlNodeBreakerViewGetNodes) {
    backend.intLists[{VL_HANDLE, prop::VL_NBV_NODES}] = {0, 1, 2, 5};
    VoltageLevel vl(VL_HANDLE, &backend);
    auto nodes = vl.getNodeBreakerView().getNodes();
    ASSERT_EQ(nodes.size(), 4u);
    EXPECT_EQ(nodes[0], 0);
    EXPECT_EQ(nodes[1], 1);
    EXPECT_EQ(nodes[2], 2);
    EXPECT_EQ(nodes[3], 5);
}

TEST_F(TopologyViewTest, VlNodeBreakerViewGetNodesEmpty) {
    VoltageLevel vl(VL_HANDLE, &backend);
    auto nodes = vl.getNodeBreakerView().getNodes();
    EXPECT_TRUE(nodes.empty());
}

TEST_F(TopologyViewTest, VlNodeBreakerViewGetSwitchById) {
    backend.relatedByString[std::make_tuple(ObjectHandle(VL_HANDLE), prop::REL_NBV_SWITCH, std::string("SW1"))] = SW1;
    backend.strings[{SW1, prop::ID}] = "SW1";
    VoltageLevel vl(VL_HANDLE, &backend);
    auto sw = vl.getNodeBreakerView().getSwitch("SW1");
    ASSERT_TRUE(sw.has_value());
    EXPECT_EQ(sw->getId(), "SW1");
}

TEST_F(TopologyViewTest, VlNodeBreakerViewGetSwitchByIdMissing) {
    VoltageLevel vl(VL_HANDLE, &backend);
    auto sw = vl.getNodeBreakerView().getSwitch("NONEXISTENT");
    EXPECT_FALSE(sw.has_value());
}

TEST_F(TopologyViewTest, VlNodeBreakerViewGetTerminalAtNode) {
    backend.relatedByIndex[{VL_HANDLE, prop::REL_NBV_TERMINAL_AT_NODE, 7}] = TERM_HANDLE;
    backend.doubles[{TERM_HANDLE, prop::TERMINAL_P}] = 100.0;
    VoltageLevel vl(VL_HANDLE, &backend);
    iidm::optional<Terminal> t = vl.getNodeBreakerView().getTerminal(7);
    ASSERT_TRUE(t);
    EXPECT_DOUBLE_EQ(t->getP(), 100.0);
    EXPECT_FALSE(vl.getNodeBreakerView().getTerminal(8)); // nothing there
}

// ── VoltageLevel per-injector getters ─────────────────────────────────────────

static constexpr ObjectHandle GEN1 = 60, LOAD1 = 61, BAT1 = 62;
static constexpr ObjectHandle SHUNT1 = 63, SVC1 = 64, DL1 = 65;
static constexpr ObjectHandle LCC1 = 66, VSC1 = 67;

TEST_F(TopologyViewTest, VlGetGenerators) {
    backend.children[{VL_HANDLE, prop::GENERATOR}] = {GEN1};
    backend.strings[{GEN1, prop::ID}] = "GEN1";
    VoltageLevel vl(VL_HANDLE, &backend);
    auto gens = vl.getGenerators();
    ASSERT_EQ(gens.size(), 1u);
    EXPECT_EQ(gens[0].getId(), "GEN1");
}

TEST_F(TopologyViewTest, VlGetLoads) {
    backend.children[{VL_HANDLE, prop::LOAD}] = {LOAD1};
    backend.strings[{LOAD1, prop::ID}] = "LOAD1";
    VoltageLevel vl(VL_HANDLE, &backend);
    auto loads = vl.getLoads();
    ASSERT_EQ(loads.size(), 1u);
    EXPECT_EQ(loads[0].getId(), "LOAD1");
}

TEST_F(TopologyViewTest, VlGetBatteries) {
    backend.children[{VL_HANDLE, prop::BATTERY}] = {BAT1};
    backend.strings[{BAT1, prop::ID}] = "BAT1";
    VoltageLevel vl(VL_HANDLE, &backend);
    auto bats = vl.getBatteries();
    ASSERT_EQ(bats.size(), 1u);
    EXPECT_EQ(bats[0].getId(), "BAT1");
}

TEST_F(TopologyViewTest, VlGetShuntCompensators) {
    backend.children[{VL_HANDLE, prop::SHUNT_COMPENSATOR}] = {SHUNT1};
    backend.strings[{SHUNT1, prop::ID}] = "SHUNT1";
    VoltageLevel vl(VL_HANDLE, &backend);
    auto shunts = vl.getShuntCompensators();
    ASSERT_EQ(shunts.size(), 1u);
    EXPECT_EQ(shunts[0].getId(), "SHUNT1");
}

TEST_F(TopologyViewTest, VlGetStaticVarCompensators) {
    backend.children[{VL_HANDLE, prop::STATIC_VAR_COMPENSATOR}] = {SVC1};
    backend.strings[{SVC1, prop::ID}] = "SVC1";
    VoltageLevel vl(VL_HANDLE, &backend);
    auto svcs = vl.getStaticVarCompensators();
    ASSERT_EQ(svcs.size(), 1u);
    EXPECT_EQ(svcs[0].getId(), "SVC1");
}

TEST_F(TopologyViewTest, VlGetBoundaryLines) {
    backend.children[{VL_HANDLE, prop::BOUNDARY_LINE}] = {DL1};
    backend.strings[{DL1, prop::ID}] = "DL1";
    VoltageLevel vl(VL_HANDLE, &backend);
    auto dls = vl.getBoundaryLines();
    ASSERT_EQ(dls.size(), 1u);
    EXPECT_EQ(dls[0].getId(), "DL1");
}

TEST_F(TopologyViewTest, VlGetLccConverterStations) {
    backend.children[{VL_HANDLE, prop::LCC_CONVERTER_STATION}] = {LCC1};
    backend.strings[{LCC1, prop::ID}] = "LCC1";
    VoltageLevel vl(VL_HANDLE, &backend);
    auto lccs = vl.getLccConverterStations();
    ASSERT_EQ(lccs.size(), 1u);
    EXPECT_EQ(lccs[0].getId(), "LCC1");
}

TEST_F(TopologyViewTest, VlGetVscConverterStations) {
    backend.children[{VL_HANDLE, prop::VSC_CONVERTER_STATION}] = {VSC1};
    backend.strings[{VSC1, prop::ID}] = "VSC1";
    VoltageLevel vl(VL_HANDLE, &backend);
    auto vscs = vl.getVscConverterStations();
    ASSERT_EQ(vscs.size(), 1u);
    EXPECT_EQ(vscs[0].getId(), "VSC1");
}

TEST_F(TopologyViewTest, VlGetBoundaryLinesFiltered) {
    static constexpr ObjectHandle DL2 = 68;
    backend.children[{VL_HANDLE, prop::BOUNDARY_LINE}] = {DL1, DL2};
    backend.strings[{DL1, prop::ID}] = "DL1";
    backend.strings[{DL2, prop::ID}] = "DL2";
    backend.bools[{DL2, prop::BL_PAIRED}] = true;
    VoltageLevel vl(VL_HANDLE, &backend);
    ASSERT_EQ(vl.getBoundaryLines(BoundaryLineFilter::PAIRED).size(), 1u);
    EXPECT_EQ(vl.getBoundaryLines(BoundaryLineFilter::PAIRED)[0].getId(), "DL2");
    EXPECT_EQ(vl.getBoundaryLines(BoundaryLineFilter::UNPAIRED)[0].getId(), "DL1");
    EXPECT_EQ(vl.getBoundaryLines(BoundaryLineFilter::ALL).size(), 2u);
}

// ── NodeBreakerView members ───────────────────────────────────────────────────

static constexpr ObjectHandle LIST = 90;
static constexpr ObjectHandle SW2 = 31;

TEST_F(TopologyViewTest, VlNodeBreakerViewNodes) {
    backend.ints[{VL_HANDLE, prop::VL_NBV_MAXIMUM_NODE_INDEX}] = 15;
    backend.evaluations[std::make_tuple(ObjectHandle(VL_HANDLE), prop::VL_NBV_ATTACHED_EQUIPMENT, std::vector<double>{3})] = 1;
    backend.evaluations[std::make_tuple(ObjectHandle(VL_HANDLE), prop::VL_NBV_ATTACHED_EQUIPMENT, std::vector<double>{4})] = 0;
    VoltageLevel::NodeBreakerView view = VoltageLevel(VL_HANDLE, &backend).getNodeBreakerView();
    EXPECT_EQ(view.getMaximumNodeIndex(), 15);
    EXPECT_TRUE(view.hasAttachedEquipment(3));
    EXPECT_FALSE(view.hasAttachedEquipment(4));
}

TEST_F(TopologyViewTest, VlNodeBreakerViewFictitiousInjections) {
    backend.evaluations[std::make_tuple(ObjectHandle(VL_HANDLE), prop::FICTITIOUS_P0, std::vector<double>{5})] = 1.5;
    backend.evaluations[std::make_tuple(ObjectHandle(VL_HANDLE), prop::FICTITIOUS_Q0, std::vector<double>{5})] = 0.5;
    VoltageLevel::NodeBreakerView view = VoltageLevel(VL_HANDLE, &backend).getNodeBreakerView();
    EXPECT_DOUBLE_EQ(view.getFictitiousP0(5), 1.5);
    EXPECT_DOUBLE_EQ(view.getFictitiousQ0(5), 0.5);

    // Two values at once: a change on the voltage level
    view.setFictitiousP0(6, 2.0).setFictitiousQ0(6, 1.0);
    ObjectHandle change = backend.lastAdder;
    EXPECT_EQ(backend.invocations[backend.invocations.size() - 2],
              std::make_pair(ObjectHandle(VL_HANDLE), prop::FICTITIOUS_Q0));
    EXPECT_EQ((backend.ints[{change, prop::TERMINAL_NODE}]), 6);
    EXPECT_DOUBLE_EQ((backend.doubles[{change, prop::FICTITIOUS_Q0}]), 1.0);
    EXPECT_EQ(backend.invocations.back(), std::make_pair(change, prop::ACT_ADD));
}

TEST_F(TopologyViewTest, VlNodeBreakerViewSwitches) {
    backend.evaluations[std::make_tuple(ObjectHandle(VL_HANDLE), prop::CHILD_COUNT,
                                        std::vector<double>{prop::REL_NBV_SWITCH})] = 1;
    backend.relatedByString[std::make_tuple(ObjectHandle(VL_HANDLE), prop::REL_NBV_SWITCH, std::string("SW1"))] = SW1;
    backend.ints[{SW1, prop::SW_NODE1}] = 2;
    backend.ints[{SW1, prop::SW_NODE2}] = 3;
    backend.relatedByIndex[{VL_HANDLE, prop::REL_NBV_TERMINAL_AT_NODE, 3}] = TERM_HANDLE;
    // The switches at a node: a list
    backend.relatedByIndex[{VL_HANDLE, prop::REL_NBV_SWITCH, 2}] = LIST;
    backend.children[{LIST, prop::ELEMENTS}] = {SW1, SW2};
    backend.strings[{SW2, prop::ID}] = "SW2";

    VoltageLevel::NodeBreakerView view = VoltageLevel(VL_HANDLE, &backend).getNodeBreakerView();
    EXPECT_EQ(view.getSwitchCount(), 1);
    EXPECT_EQ(view.getNode1("SW1"), 2);
    EXPECT_EQ(view.getNode2("SW1"), 3);
    EXPECT_THROW(view.getNode1("NONE"), ValidationException);
    EXPECT_FALSE(view.getTerminal1("SW1")); // nothing at node 2
    EXPECT_DOUBLE_EQ(view.getTerminal2("SW1")->getP(), 100.0);
    std::vector<Switch> atNode = view.getSwitches(2);
    ASSERT_EQ(atNode.size(), 2u);
    EXPECT_EQ(atNode[1].getId(), "SW2");

    view.removeSwitch("SW1");
    EXPECT_EQ((backend.strings[{VL_HANDLE, prop::VL_NBV_REMOVE_SWITCH}]), "SW1");
}

TEST_F(TopologyViewTest, VlNodeBreakerViewBusbarSectionsAndInternalConnections) {
    backend.relatedByString[std::make_tuple(ObjectHandle(VL_HANDLE), prop::BUSBAR_SECTION, std::string("BBS1"))] = BBS1;
    backend.evaluations[std::make_tuple(ObjectHandle(VL_HANDLE), prop::CHILD_COUNT,
                                        std::vector<double>{prop::BUSBAR_SECTION})] = 1;
    backend.evaluations[std::make_tuple(ObjectHandle(VL_HANDLE), prop::CHILD_COUNT,
                                        std::vector<double>{prop::INTERNAL_CONNECTION})] = 2;
    backend.relatedByIndex[{VL_HANDLE, prop::INTERNAL_CONNECTION, 3}] = LIST;
    backend.intLists[{LIST, prop::ELEMENTS}] = {0, 4};

    VoltageLevel::NodeBreakerView view = VoltageLevel(VL_HANDLE, &backend).getNodeBreakerView();
    EXPECT_EQ(view.getBusbarSection("BBS1")->getId(), "BBS1");
    EXPECT_FALSE(view.getBusbarSection("NONE"));
    EXPECT_EQ(view.getBusbarSectionCount(), 1);
    EXPECT_EQ(view.getInternalConnectionCount(), 2);
    EXPECT_EQ(view.getNodesInternalConnectedTo(3), (std::vector<int>{0, 4}));

    view.removeInternalConnections(3, 4);
    ObjectHandle removal = backend.lastAdder;
    EXPECT_EQ(backend.invocations[backend.invocations.size() - 2],
              std::make_pair(ObjectHandle(VL_HANDLE), prop::VL_NBV_REMOVE_INTERNAL_CONNECTIONS));
    EXPECT_EQ((backend.ints[{removal, prop::IC_NODE1}]), 3);
    EXPECT_EQ((backend.ints[{removal, prop::IC_NODE2}]), 4);
    EXPECT_EQ(backend.invocations.back(), std::make_pair(removal, prop::ACT_ADD));
}

// ── BusBreakerView members ────────────────────────────────────────────────────

TEST_F(TopologyViewTest, VlBusBreakerViewMembers) {
    backend.evaluations[std::make_tuple(ObjectHandle(VL_HANDLE), prop::CHILD_COUNT, std::vector<double>{prop::BUS})] = 2;
    backend.evaluations[std::make_tuple(ObjectHandle(VL_HANDLE), prop::CHILD_COUNT,
                                        std::vector<double>{prop::REL_BBV_SWITCH})] = 1;
    backend.relatedByString[std::make_tuple(ObjectHandle(VL_HANDLE), prop::REL_BBV_SWITCH, std::string("SW1"))] = SW1;
    backend.relatedByString[std::make_tuple(ObjectHandle(VL_HANDLE), prop::REL_BBV_BUSES_OF_BV_BUS, std::string("VL1_0"))] = LIST;
    backend.children[{LIST, prop::ELEMENTS}] = {BUS1, BUS2};

    VoltageLevel::BusBreakerView view = VoltageLevel(VL_HANDLE, &backend).getBusBreakerView();
    EXPECT_EQ(view.getBusCount(), 2);
    EXPECT_EQ(view.getSwitchCount(), 1);
    EXPECT_EQ(view.getSwitch("SW1")->getId(), "SW1");
    EXPECT_FALSE(view.getSwitch("NONE"));
    std::vector<Bus> buses = view.getBusesFromBusViewBusId("VL1_0");
    ASSERT_EQ(buses.size(), 2u);
    EXPECT_EQ(buses[1].getId(), "BUS2");

    view.removeSwitch("SW1");
    EXPECT_EQ((backend.strings[{VL_HANDLE, prop::VL_BBV_REMOVE_SWITCH}]), "SW1");
    view.removeAllSwitches();
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(VL_HANDLE), prop::ACT_REMOVE_ALL_SWITCHES));
    view.removeAllBuses();
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(VL_HANDLE), prop::ACT_REMOVE_ALL_BUSES));
}

// ── Bus members ───────────────────────────────────────────────────────────────

TEST_F(TopologyViewTest, BusValues) {
    backend.doubles[{BUS1, prop::BUS_P}] = 10.0;
    backend.doubles[{BUS1, prop::BUS_Q}] = 2.0;
    backend.bools[{BUS1, prop::BUS_IN_MAIN_CONNECTED_COMPONENT}] = true;
    Bus bus(BUS1, &backend);
    EXPECT_DOUBLE_EQ(bus.getP(), 10.0);
    EXPECT_DOUBLE_EQ(bus.getQ(), 2.0);
    EXPECT_TRUE(bus.isInMainConnectedComponent());
    EXPECT_FALSE(bus.isInMainSynchronousComponent());
    bus.setFictitiousP0(1.5).setFictitiousQ0(0.5);
    EXPECT_DOUBLE_EQ(bus.getFictitiousP0(), 1.5);
    EXPECT_DOUBLE_EQ(bus.getFictitiousQ0(), 0.5);
    bus.unsetSolvedValues();
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(BUS1), prop::ACT_UNSET_SOLVED_VALUES));
}

TEST_F(TopologyViewTest, BusConnectedEquipment) {
    backend.children[{BUS1, prop::TERMINAL}] = {TERM_HANDLE};
    backend.evaluations[std::make_tuple(ObjectHandle(BUS1), prop::CHILD_COUNT, std::vector<double>{prop::TERMINAL})] = 1;
    backend.children[{BUS1, prop::GENERATOR}] = {GEN1};
    backend.strings[{GEN1, prop::ID}] = "GEN1";
    backend.children[{BUS1, prop::BOUNDARY_LINE}] = {DL1};
    backend.bools[{DL1, prop::BL_PAIRED}] = true;
    Bus bus(BUS1, &backend);
    ASSERT_EQ(bus.getConnectedTerminals().size(), 1u);
    EXPECT_DOUBLE_EQ(bus.getConnectedTerminals()[0].getP(), 100.0);
    EXPECT_EQ(bus.getConnectedTerminalCount(), 1);
    EXPECT_EQ(bus.getGenerators()[0].getId(), "GEN1");
    EXPECT_TRUE(bus.getLoads().empty());
    EXPECT_EQ(bus.getBoundaryLines().size(), 1u);
    EXPECT_TRUE(bus.getBoundaryLines(BoundaryLineFilter::UNPAIRED).empty());
}

// ── Terminal::BusView ─────────────────────────────────────────────────────────

TEST_F(TopologyViewTest, TerminalBusView) {
    backend.related[{TERM_HANDLE, prop::REL_BV_BUS}] = BUS2;
    backend.related[{TERM_HANDLE, prop::REL_BV_CONNECTABLE_BUS}] = BUS2;
    Terminal t(TERM_HANDLE, &backend);
    EXPECT_EQ(t.getBusView().getBus()->getId(), "BUS2");
    EXPECT_EQ(t.getBusView().getConnectableBus()->getId(), "BUS2");
    MockBackend disconnected;
    EXPECT_FALSE(Terminal(TERM_HANDLE, &disconnected).getBusView().getBus());
}
