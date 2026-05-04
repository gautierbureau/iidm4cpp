#include <gtest/gtest.h>
#include <iidm/Terminal.h>
#include <iidm/VoltageLevel.h>
#include <iidm/InternalConnection.h>
#include <iidm/Generator.h>
#include <iidm/Load.h>
#include <iidm/Battery.h>
#include <iidm/ShuntCompensator.h>
#include <iidm/StaticVarCompensator.h>
#include <iidm/DanglingLine.h>
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
        // bus view (merged/calculated bus)
        backend.related[{TERM_HANDLE, prop::REL_BUS}]            = BUS1;
        // bus-breaker connectable bus
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
    Bus b = t.getBusBreakerView().getBus();
    EXPECT_TRUE(b.isValid());
    EXPECT_EQ(b.getId(), "BUS1");
}

TEST_F(TopologyViewTest, TerminalBusBreakerViewGetConnectableBus) {
    Terminal t(TERM_HANDLE, &backend);
    Bus b = t.getBusBreakerView().getConnectableBus();
    EXPECT_TRUE(b.isValid());
    EXPECT_EQ(b.getId(), "BUS2");
}

TEST_F(TopologyViewTest, TerminalBusBreakerViewGetBusInvalidWhenDisconnected) {
    MockBackend b2;
    // No REL_BUS set → INVALID_HANDLE → Bus.isValid() == false
    Terminal t(TERM_HANDLE, &b2);
    Bus bus = t.getBusBreakerView().getBus();
    EXPECT_FALSE(bus.isValid());
}

TEST_F(TopologyViewTest, TerminalBusBreakerViewConnectableBusAlwaysPresent) {
    // The connectable bus should exist independently of connection state.
    MockBackend b2;
    b2.related[{TERM_HANDLE, prop::REL_CONNECTABLE_BUS}] = BUS1;
    b2.strings[{BUS1, prop::ID}] = "BUS1";
    b2.bools[{TERM_HANDLE, prop::TERMINAL_CONNECTED}] = false;
    Terminal t(TERM_HANDLE, &b2);
    EXPECT_FALSE(t.isConnected());
    Bus connectable = t.getBusBreakerView().getConnectableBus();
    EXPECT_TRUE(connectable.isValid());
    EXPECT_EQ(connectable.getId(), "BUS1");
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
    backend.byId[{prop::SWITCH, "SW1"}] = SW1;
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
    Terminal t = vl.getNodeBreakerView().getTerminal(7);
    EXPECT_TRUE(t.isValid());
    EXPECT_DOUBLE_EQ(t.getP(), 100.0);
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

TEST_F(TopologyViewTest, VlGetDanglingLines) {
    backend.children[{VL_HANDLE, prop::DANGLING_LINE}] = {DL1};
    backend.strings[{DL1, prop::ID}] = "DL1";
    VoltageLevel vl(VL_HANDLE, &backend);
    auto dls = vl.getDanglingLines();
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
