#include <gtest/gtest.h>
#include <iidm/Terminal.h>
#include <iidm/VoltageLevel.h>
#include <iidm/InternalConnection.h>
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
