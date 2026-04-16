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
