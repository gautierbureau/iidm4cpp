#include <gtest/gtest.h>
#include <iidm/SlackTerminal.h>
#include <iidm/VoltageLevel.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class SlackTerminalTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle VL_HANDLE   = 50;
    static constexpr ObjectHandle TERM_HANDLE = 51;

    void SetUp() override {
        backend.bools  [{VL_HANDLE, prop::EXT_ST_EXISTS}]         = true;
        backend.related[{VL_HANDLE, prop::REL_SLACK_TERMINAL}]    = TERM_HANDLE;
    }
};

TEST_F(SlackTerminalTest, HasSlackTerminal) {
    VoltageLevel vl(VL_HANDLE, &backend);
    EXPECT_TRUE(vl.hasSlackTerminal());
}

TEST_F(SlackTerminalTest, HasSlackTerminalAbsent) {
    MockBackend empty;
    VoltageLevel vl(VL_HANDLE, &empty);
    EXPECT_FALSE(vl.hasSlackTerminal());
}

TEST_F(SlackTerminalTest, GetTerminal) {
    VoltageLevel vl(VL_HANDLE, &backend);
    Terminal t = vl.getSlackTerminal().getTerminal();
    EXPECT_TRUE(t.isValid());
}

TEST_F(SlackTerminalTest, GetTerminalHandle) {
    VoltageLevel vl(VL_HANDLE, &backend);
    SlackTerminal st = vl.getSlackTerminal();
    Terminal t = st.getTerminal();
    // The terminal handle should match the registered related handle
    EXPECT_TRUE(t.isValid());
}

TEST_F(SlackTerminalTest, IsValid) {
    SlackTerminal valid(VL_HANDLE, &backend);
    SlackTerminal invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}

TEST_F(SlackTerminalTest, Equality) {
    SlackTerminal st1(VL_HANDLE, &backend);
    SlackTerminal st2(VL_HANDLE, &backend);
    SlackTerminal st3(99, &backend);
    EXPECT_EQ(st1, st2);
    EXPECT_NE(st1, st3);
}

TEST_F(SlackTerminalTest, SetTerminal) {
    constexpr ObjectHandle OTHER_TERM = 52;
    VoltageLevel vl(VL_HANDLE, &backend);
    SlackTerminal st = vl.getSlackTerminal();
    st.setTerminal(Terminal(OTHER_TERM, &backend));
    EXPECT_EQ((backend.related[{VL_HANDLE, prop::REL_SLACK_TERMINAL}]), OTHER_TERM);
    EXPECT_EQ(st.getTerminal(), Terminal(OTHER_TERM, &backend));
}

TEST_F(SlackTerminalTest, SetInvalidTerminalUnsetsIt) {
    SlackTerminal st = VoltageLevel(VL_HANDLE, &backend).getSlackTerminal();
    st.setTerminal(Terminal());
    EXPECT_EQ((backend.related[{VL_HANDLE, prop::REL_SLACK_TERMINAL}]), INVALID_HANDLE);
    EXPECT_FALSE(st.getTerminal().isValid());
}

TEST_F(SlackTerminalTest, IsEmpty) {
    SlackTerminal st = VoltageLevel(VL_HANDLE, &backend).getSlackTerminal();
    EXPECT_FALSE(st.isEmpty());
    backend.bools[{VL_HANDLE, prop::EXT_ST_EMPTY}] = true;
    EXPECT_TRUE(st.isEmpty());
}
