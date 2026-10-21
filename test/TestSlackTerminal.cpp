#include <gtest/gtest.h>
#include <iidm/SlackTerminal.h>
#include <iidm/SlackTerminalAdder.h>
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
    static constexpr ObjectHandle ST_HANDLE   = 53; // the extension object

    void SetUp() override {
        backend.related[{VL_HANDLE, prop::SLACK_TERMINAL}] = ST_HANDLE;
        backend.related[{ST_HANDLE, prop::REL_TERMINAL}]   = TERM_HANDLE;
    }
};

TEST_F(SlackTerminalTest, GetTerminal) {
    VoltageLevel vl(VL_HANDLE, &backend);
    SlackTerminal st = vl.getExtension<SlackTerminal>();
    EXPECT_EQ(st, SlackTerminal(ST_HANDLE, &backend));
    ASSERT_TRUE(st.getTerminal());
    EXPECT_EQ(*st.getTerminal(), Terminal(TERM_HANDLE, &backend));
}

TEST_F(SlackTerminalTest, SetTerminal) {
    constexpr ObjectHandle OTHER_TERM = 52;
    SlackTerminal st = VoltageLevel(VL_HANDLE, &backend).getExtension<SlackTerminal>();
    EXPECT_EQ(&st.setTerminal(Terminal(OTHER_TERM, &backend)), &st);
    EXPECT_EQ((backend.related[{ST_HANDLE, prop::REL_TERMINAL}]), OTHER_TERM);
    EXPECT_EQ(*st.getTerminal(), Terminal(OTHER_TERM, &backend));
    // Without cleanIfEmpty, nothing else is done
    EXPECT_TRUE(backend.invocations.empty());
}

TEST_F(SlackTerminalTest, SetInvalidTerminalUnsetsIt) {
    SlackTerminal st = VoltageLevel(VL_HANDLE, &backend).getExtension<SlackTerminal>();
    st.setTerminal(Terminal());
    EXPECT_EQ((backend.related[{ST_HANDLE, prop::REL_TERMINAL}]), INVALID_HANDLE);
    EXPECT_FALSE(st.getTerminal());
}

TEST_F(SlackTerminalTest, SetTerminalAndCleanIfEmpty) {
    SlackTerminal st = VoltageLevel(VL_HANDLE, &backend).getExtension<SlackTerminal>();
    st.setTerminal(Terminal(), true);
    EXPECT_EQ((backend.related[{ST_HANDLE, prop::REL_TERMINAL}]), INVALID_HANDLE);
    ASSERT_EQ(backend.invocations.size(), 1u);
    EXPECT_EQ(backend.invocations[0], std::make_pair(static_cast<ObjectHandle>(ST_HANDLE), prop::ACT_CLEAN_IF_EMPTY));
}

