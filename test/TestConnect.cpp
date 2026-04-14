#include <gtest/gtest.h>
#include <iidm/Generator.h>
#include <iidm/Load.h>
#include <iidm/DanglingLine.h>
#include <iidm/ShuntCompensator.h>
#include <iidm/StaticVarCompensator.h>
#include <iidm/VscConverterStation.h>
#include <iidm/LccConverterStation.h>
#include <iidm/Line.h>
#include <iidm/TwoWindingsTransformer.h>
#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

// ── Helpers ───────────────────────────────────────────────────────────────────

static constexpr ObjectHandle COMP_HANDLE  = 10;
static constexpr ObjectHandle TERM_HANDLE  = 20;
static constexpr ObjectHandle TERM2_HANDLE = 21;
static constexpr ObjectHandle TERM3_HANDLE = 22;

// Seed a backend so that COMP_HANDLE → single terminal TERM_HANDLE (connected).
static void setupSingleTerminal(MockBackend& b, bool connected = true) {
    b.related[{COMP_HANDLE, prop::REL_TERMINAL}] = TERM_HANDLE;
    b.bools[{TERM_HANDLE, prop::TERMINAL_CONNECTED}] = connected;
}

// Seed a backend so that COMP_HANDLE → two terminals (both connected).
static void setupTwoTerminals(MockBackend& b, bool t1 = true, bool t2 = true) {
    b.related[{COMP_HANDLE, prop::REL_TERMINAL1}] = TERM_HANDLE;
    b.related[{COMP_HANDLE, prop::REL_TERMINAL2}] = TERM2_HANDLE;
    b.bools[{TERM_HANDLE,  prop::TERMINAL_CONNECTED}] = t1;
    b.bools[{TERM2_HANDLE, prop::TERMINAL_CONNECTED}] = t2;
}

// Seed a backend so that COMP_HANDLE → three terminals (all connected).
static void setupThreeTerminals(MockBackend& b, bool t1 = true, bool t2 = true, bool t3 = true) {
    b.related[{COMP_HANDLE, prop::REL_TERMINAL1}] = TERM_HANDLE;
    b.related[{COMP_HANDLE, prop::REL_TERMINAL2}] = TERM2_HANDLE;
    b.related[{COMP_HANDLE, prop::REL_TERMINAL}]  = TERM3_HANDLE;  // terminal3 uses REL_TERMINAL
    b.bools[{TERM_HANDLE,  prop::TERMINAL_CONNECTED}] = t1;
    b.bools[{TERM2_HANDLE, prop::TERMINAL_CONNECTED}] = t2;
    b.bools[{TERM3_HANDLE, prop::TERMINAL_CONNECTED}] = t3;
}

// ── Generator ─────────────────────────────────────────────────────────────────

TEST(ConnectTest, GeneratorIsConnected) {
    MockBackend b;
    setupSingleTerminal(b, /*connected=*/true);
    Generator gen(COMP_HANDLE, &b);
    EXPECT_TRUE(gen.isConnected());
}

TEST(ConnectTest, GeneratorDisconnect) {
    MockBackend b;
    setupSingleTerminal(b, /*connected=*/true);
    Generator gen(COMP_HANDLE, &b);
    gen.disconnect();
    EXPECT_FALSE((b.bools[{TERM_HANDLE, prop::TERMINAL_CONNECTED}]));
}

TEST(ConnectTest, GeneratorConnect) {
    MockBackend b;
    setupSingleTerminal(b, /*connected=*/false);
    Generator gen(COMP_HANDLE, &b);
    gen.connect();
    EXPECT_TRUE((b.bools[{TERM_HANDLE, prop::TERMINAL_CONNECTED}]));
}

// ── Load ──────────────────────────────────────────────────────────────────────

TEST(ConnectTest, LoadIsConnected) {
    MockBackend b;
    setupSingleTerminal(b, true);
    Load load(COMP_HANDLE, &b);
    EXPECT_TRUE(load.isConnected());
}

TEST(ConnectTest, LoadDisconnect) {
    MockBackend b;
    setupSingleTerminal(b, true);
    Load load(COMP_HANDLE, &b);
    load.disconnect();
    EXPECT_FALSE((b.bools[{TERM_HANDLE, prop::TERMINAL_CONNECTED}]));
}

TEST(ConnectTest, LoadConnect) {
    MockBackend b;
    setupSingleTerminal(b, false);
    Load load(COMP_HANDLE, &b);
    load.connect();
    EXPECT_TRUE((b.bools[{TERM_HANDLE, prop::TERMINAL_CONNECTED}]));
}

// ── DanglingLine ──────────────────────────────────────────────────────────────

TEST(ConnectTest, DanglingLineIsConnected) {
    MockBackend b;
    setupSingleTerminal(b, true);
    DanglingLine dl(COMP_HANDLE, &b);
    EXPECT_TRUE(dl.isConnected());
}

TEST(ConnectTest, DanglingLineDisconnect) {
    MockBackend b;
    setupSingleTerminal(b, true);
    DanglingLine dl(COMP_HANDLE, &b);
    dl.disconnect();
    EXPECT_FALSE((b.bools[{TERM_HANDLE, prop::TERMINAL_CONNECTED}]));
}

TEST(ConnectTest, DanglingLineConnect) {
    MockBackend b;
    setupSingleTerminal(b, false);
    DanglingLine dl(COMP_HANDLE, &b);
    dl.connect();
    EXPECT_TRUE((b.bools[{TERM_HANDLE, prop::TERMINAL_CONNECTED}]));
}

// ── ShuntCompensator ──────────────────────────────────────────────────────────

TEST(ConnectTest, ShuntCompensatorDisconnectAndConnect) {
    MockBackend b;
    setupSingleTerminal(b, true);
    ShuntCompensator shunt(COMP_HANDLE, &b);
    EXPECT_TRUE(shunt.isConnected());
    shunt.disconnect();
    EXPECT_FALSE(shunt.isConnected());
    shunt.connect();
    EXPECT_TRUE(shunt.isConnected());
}

// ── StaticVarCompensator ──────────────────────────────────────────────────────

TEST(ConnectTest, StaticVarCompensatorDisconnectAndConnect) {
    MockBackend b;
    setupSingleTerminal(b, true);
    StaticVarCompensator svc(COMP_HANDLE, &b);
    EXPECT_TRUE(svc.isConnected());
    svc.disconnect();
    EXPECT_FALSE(svc.isConnected());
    svc.connect();
    EXPECT_TRUE(svc.isConnected());
}

// ── VscConverterStation ───────────────────────────────────────────────────────

TEST(ConnectTest, VscConverterStationDisconnectAndConnect) {
    MockBackend b;
    setupSingleTerminal(b, true);
    VscConverterStation vsc(COMP_HANDLE, &b);
    EXPECT_TRUE(vsc.isConnected());
    vsc.disconnect();
    EXPECT_FALSE(vsc.isConnected());
    vsc.connect();
    EXPECT_TRUE(vsc.isConnected());
}

// ── LccConverterStation ───────────────────────────────────────────────────────

TEST(ConnectTest, LccConverterStationDisconnectAndConnect) {
    MockBackend b;
    setupSingleTerminal(b, true);
    LccConverterStation lcc(COMP_HANDLE, &b);
    EXPECT_TRUE(lcc.isConnected());
    lcc.disconnect();
    EXPECT_FALSE(lcc.isConnected());
    lcc.connect();
    EXPECT_TRUE(lcc.isConnected());
}

// ── Line (two terminals) ──────────────────────────────────────────────────────

TEST(ConnectTest, LineIsConnectedBothTerminals) {
    MockBackend b;
    setupTwoTerminals(b, /*t1=*/true, /*t2=*/true);
    Line line(COMP_HANDLE, &b);
    EXPECT_TRUE(line.isConnected());
}

TEST(ConnectTest, LineIsConnectedOneTerminalDown) {
    MockBackend b;
    setupTwoTerminals(b, /*t1=*/true, /*t2=*/false);
    Line line(COMP_HANDLE, &b);
    EXPECT_FALSE(line.isConnected());
}

TEST(ConnectTest, LineDisconnect) {
    MockBackend b;
    setupTwoTerminals(b, true, true);
    Line line(COMP_HANDLE, &b);
    line.disconnect();
    EXPECT_FALSE((b.bools[{TERM_HANDLE,  prop::TERMINAL_CONNECTED}]));
    EXPECT_FALSE((b.bools[{TERM2_HANDLE, prop::TERMINAL_CONNECTED}]));
}

TEST(ConnectTest, LineConnect) {
    MockBackend b;
    setupTwoTerminals(b, false, false);
    Line line(COMP_HANDLE, &b);
    line.connect();
    EXPECT_TRUE((b.bools[{TERM_HANDLE,  prop::TERMINAL_CONNECTED}]));
    EXPECT_TRUE((b.bools[{TERM2_HANDLE, prop::TERMINAL_CONNECTED}]));
}

// ── TwoWindingsTransformer (two terminals) ────────────────────────────────────

TEST(ConnectTest, TwoWindingsTransformerIsConnected) {
    MockBackend b;
    setupTwoTerminals(b, true, true);
    TwoWindingsTransformer twt(COMP_HANDLE, &b);
    EXPECT_TRUE(twt.isConnected());
}

TEST(ConnectTest, TwoWindingsTransformerDisconnect) {
    MockBackend b;
    setupTwoTerminals(b, true, true);
    TwoWindingsTransformer twt(COMP_HANDLE, &b);
    twt.disconnect();
    EXPECT_FALSE((b.bools[{TERM_HANDLE,  prop::TERMINAL_CONNECTED}]));
    EXPECT_FALSE((b.bools[{TERM2_HANDLE, prop::TERMINAL_CONNECTED}]));
}

TEST(ConnectTest, TwoWindingsTransformerConnect) {
    MockBackend b;
    setupTwoTerminals(b, false, false);
    TwoWindingsTransformer twt(COMP_HANDLE, &b);
    twt.connect();
    EXPECT_TRUE((b.bools[{TERM_HANDLE,  prop::TERMINAL_CONNECTED}]));
    EXPECT_TRUE((b.bools[{TERM2_HANDLE, prop::TERMINAL_CONNECTED}]));
}

// ── ThreeWindingsTransformer (three terminals) ────────────────────────────────

TEST(ConnectTest, ThreeWindingsTransformerIsConnected) {
    MockBackend b;
    setupThreeTerminals(b, true, true, true);
    ThreeWindingsTransformer twt(COMP_HANDLE, &b);
    EXPECT_TRUE(twt.isConnected());
}

TEST(ConnectTest, ThreeWindingsTransformerIsConnectedOneDown) {
    MockBackend b;
    setupThreeTerminals(b, true, true, false);
    ThreeWindingsTransformer twt(COMP_HANDLE, &b);
    EXPECT_FALSE(twt.isConnected());
}

TEST(ConnectTest, ThreeWindingsTransformerDisconnect) {
    MockBackend b;
    setupThreeTerminals(b, true, true, true);
    ThreeWindingsTransformer twt(COMP_HANDLE, &b);
    twt.disconnect();
    EXPECT_FALSE((b.bools[{TERM_HANDLE,  prop::TERMINAL_CONNECTED}]));
    EXPECT_FALSE((b.bools[{TERM2_HANDLE, prop::TERMINAL_CONNECTED}]));
    EXPECT_FALSE((b.bools[{TERM3_HANDLE, prop::TERMINAL_CONNECTED}]));
}

TEST(ConnectTest, ThreeWindingsTransformerConnect) {
    MockBackend b;
    setupThreeTerminals(b, false, false, false);
    ThreeWindingsTransformer twt(COMP_HANDLE, &b);
    twt.connect();
    EXPECT_TRUE((b.bools[{TERM_HANDLE,  prop::TERMINAL_CONNECTED}]));
    EXPECT_TRUE((b.bools[{TERM2_HANDLE, prop::TERMINAL_CONNECTED}]));
    EXPECT_TRUE((b.bools[{TERM3_HANDLE, prop::TERMINAL_CONNECTED}]));
}
