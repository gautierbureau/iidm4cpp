#include <gtest/gtest.h>
#include <iidm/Terminal.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Connectable.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class TerminalTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle TERM_HANDLE = 20;
    static constexpr ObjectHandle BUS_HANDLE  = 21;

    void SetUp() override {
        backend.doubles[{TERM_HANDLE, prop::TERMINAL_P}] = 55.0;
        backend.doubles[{TERM_HANDLE, prop::TERMINAL_Q}] = 10.0;
        backend.bools  [{TERM_HANDLE, prop::TERMINAL_CONNECTED}] = true;
        backend.related[{TERM_HANDLE, prop::REL_BV_BUS}] = BUS_HANDLE;
        backend.doubles[{BUS_HANDLE,  prop::BUS_V}]     = 1.02;
        backend.doubles[{BUS_HANDLE,  prop::BUS_ANGLE}]  = 0.05;
    }
};

TEST_F(TerminalTest, GetPQ) {
    Terminal term(TERM_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(term.getP(), 55.0);
    EXPECT_DOUBLE_EQ(term.getQ(), 10.0);
}

TEST_F(TerminalTest, SetPQ) {
    Terminal term(TERM_HANDLE, &backend);
    term.setP(60.0).setQ(12.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{TERM_HANDLE, prop::TERMINAL_P}]), 60.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{TERM_HANDLE, prop::TERMINAL_Q}]), 12.0);
}

TEST_F(TerminalTest, IsConnected) {
    Terminal term(TERM_HANDLE, &backend);
    EXPECT_TRUE(term.isConnected());
}

TEST_F(TerminalTest, ConnectAndDisconnect) {
    Terminal term(TERM_HANDLE, &backend);
    EXPECT_TRUE(term.disconnect());
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(TERM_HANDLE), prop::ACT_DISCONNECT));
    EXPECT_TRUE(term.connect());
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(TERM_HANDLE), prop::ACT_CONNECT));
    backend.unchanged.insert(ObjectHandle(TERM_HANDLE)); // could not, or already was
    EXPECT_FALSE(term.connect());
    EXPECT_EQ(backend.liveHandleCount(), 1u); // the result handles are released
}

TEST_F(TerminalTest, GetBusView) {
    Terminal term(TERM_HANDLE, &backend);
    iidm::optional<Bus> bus = term.getBusView().getBus();
    ASSERT_TRUE(bus);
    EXPECT_DOUBLE_EQ(bus->getV(), 1.02);
    EXPECT_DOUBLE_EQ(bus->getAngle(), 0.05);
    EXPECT_FALSE(term.getBusView().getConnectableBus()); // none set
    backend.related[{TERM_HANDLE, prop::REL_BV_CONNECTABLE_BUS}] = BUS_HANDLE;
    EXPECT_EQ(*term.getBusView().getConnectableBus(), *bus);
}

TEST_F(TerminalTest, CurrentAndSolvedValues) {
    backend.doubles[{TERM_HANDLE, prop::TERMINAL_I}] = 80.5;
    Terminal term(TERM_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(term.getI(), 80.5);
    term.unsetSolvedValues();
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(TERM_HANDLE), prop::ACT_UNSET_SOLVED_VALUES));
}

TEST_F(TerminalTest, ConnectableBusAndMoves) {
    Terminal term(TERM_HANDLE, &backend);
    term.getBusBreakerView().setConnectableBus("B2");
    EXPECT_EQ((backend.strings[{TERM_HANDLE, prop::TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID}]), "B2");

    // A move: a change on the terminal, with its fields, then applied
    term.getBusBreakerView().moveConnectable("B3", true);
    ObjectHandle move = backend.lastAdder;
    EXPECT_EQ(backend.invocations[backend.invocations.size() - 2],
              std::make_pair(ObjectHandle(TERM_HANDLE), prop::MOVE_CONNECTABLE));
    EXPECT_EQ((backend.strings[{move, prop::TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID}]), "B3");
    EXPECT_TRUE((backend.bools[{move, prop::TERMINAL_CONNECTED}]));
    EXPECT_EQ(backend.invocations.back(), std::make_pair(move, prop::ACT_ADD));

    term.getNodeBreakerView().moveConnectable(12, "VL2");
    move = backend.lastAdder;
    EXPECT_EQ((backend.ints[{move, prop::TERMINAL_NODE}]), 12);
    EXPECT_EQ((backend.strings[{move, prop::TERMINAL_VOLTAGE_LEVEL_ID}]), "VL2");
    EXPECT_EQ(backend.invocations.back(), std::make_pair(move, prop::ACT_ADD));
    EXPECT_EQ(backend.liveHandleCount(), 1u); // the changes are released
}

TEST_F(TerminalTest, ConnectableAndSide) {
    static constexpr ObjectHandle GEN = 26;
    backend.related[{TERM_HANDLE, prop::REL_CONNECTABLE}] = GEN;
    backend.strings[{GEN, prop::ID}] = "GEN1";
    backend.ints[{TERM_HANDLE, prop::TERMINAL_SIDE}] = -1;
    Terminal term(TERM_HANDLE, &backend);
    EXPECT_EQ(term.getConnectable().getId(), "GEN1");
    EXPECT_FALSE(term.getSide());
    backend.ints[{TERM_HANDLE, prop::TERMINAL_SIDE}] = static_cast<int>(ThreeSides::TWO);
    EXPECT_EQ(*term.getSide(), ThreeSides::TWO);
}

TEST_F(TerminalTest, ConnectableTerminals) {
    static constexpr ObjectHandle GEN = 26;
    static constexpr ObjectHandle OTHER_TERM = 25;
    backend.related[{TERM_HANDLE, prop::REL_CONNECTABLE}] = GEN;
    backend.children[{GEN, prop::TERMINAL}] = {TERM_HANDLE, OTHER_TERM};
    backend.doubles[{OTHER_TERM, prop::TERMINAL_P}] = 30.0;
    Terminal term(TERM_HANDLE, &backend);
    auto terms = term.getConnectable().getTerminals();
    ASSERT_EQ(terms.size(), 2u);
    EXPECT_EQ(terms[0], term);
    EXPECT_DOUBLE_EQ(terms[0].getP(), 55.0);
    EXPECT_DOUBLE_EQ(terms[1].getP(), 30.0);
}
