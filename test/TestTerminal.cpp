#include <gtest/gtest.h>
#include <iidm/Terminal.h>
#include <iidm/PropertyCodes.h>
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
        backend.related[{TERM_HANDLE, prop::REL_BUS}] = BUS_HANDLE;
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

TEST_F(TerminalTest, Disconnect) {
    Terminal term(TERM_HANDLE, &backend);
    term.disconnect();
    EXPECT_FALSE((backend.bools[{TERM_HANDLE, prop::TERMINAL_CONNECTED}]));
}

TEST_F(TerminalTest, Connect) {
    Terminal term(TERM_HANDLE, &backend);
    term.disconnect();
    term.connect();
    EXPECT_TRUE((backend.bools[{TERM_HANDLE, prop::TERMINAL_CONNECTED}]));
}

TEST_F(TerminalTest, GetBusView) {
    Terminal term(TERM_HANDLE, &backend);
    Bus bus = term.getBusView();
    EXPECT_TRUE(bus.isValid());
    EXPECT_DOUBLE_EQ(bus.getV(), 1.02);
    EXPECT_DOUBLE_EQ(bus.getAngle(), 0.05);
}
