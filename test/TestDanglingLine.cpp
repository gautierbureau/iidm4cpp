#include <gtest/gtest.h>
#include <iidm/DanglingLine.h>
#include <iidm/CurrentLimits.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class DanglingLineTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle DL_HANDLE = 10;
    static constexpr ObjectHandle CL_HANDLE = 11;

    void SetUp() override {
        backend.strings[{DL_HANDLE, prop::ID}]    = "DL1";
        backend.strings[{DL_HANDLE, prop::NAME}]   = "Dangling Line 1";
        backend.doubles[{DL_HANDLE, prop::DL_P0}]  = 50.0;
        backend.doubles[{DL_HANDLE, prop::DL_Q0}]  = 10.0;
        backend.doubles[{DL_HANDLE, prop::DL_R}]   = 1.0;
    }
};

TEST_F(DanglingLineTest, GetId) {
    DanglingLine dl(DL_HANDLE, &backend);
    EXPECT_EQ(dl.getId(), "DL1");
}

// ── getCurrentLimits ──────────────────────────────────────────────────────────

TEST_F(DanglingLineTest, GetCurrentLimitsAbsent) {
    DanglingLine dl(DL_HANDLE, &backend);
    EXPECT_FALSE(dl.getCurrentLimits().has_value());
}

TEST_F(DanglingLineTest, GetCurrentLimitsPresent) {
    backend.related[{DL_HANDLE, prop::CURRENT_LIMITS}]     = CL_HANDLE;
    backend.doubles[{CL_HANDLE, prop::CL_PERMANENT_LIMIT}] = 300.0;
    DanglingLine dl(DL_HANDLE, &backend);
    auto cl = dl.getCurrentLimits();
    ASSERT_TRUE(cl.has_value());
    EXPECT_DOUBLE_EQ(cl->getPermanentLimit(), 300.0);
}

// Its limits are those of a flows limits holder: one set of groups
TEST_F(DanglingLineTest, OperationalLimitsGroups) {
    static constexpr ObjectHandle OLG_HANDLE = 12;
    backend.children[{DL_HANDLE, prop::OPERATIONAL_LIMITS_GROUP}] = {OLG_HANDLE};
    backend.related [{DL_HANDLE, prop::REL_SELECTED_OLG}]         = OLG_HANDLE;
    backend.strings [{OLG_HANDLE, prop::OLG_ID}]                  = "DEFAULT";
    DanglingLine dl(DL_HANDLE, &backend);
    auto groups = dl.getOperationalLimitsGroups();
    ASSERT_EQ(groups.size(), 1u);
    EXPECT_EQ(groups[0].getId(), "DEFAULT");
    auto selected = dl.getSelectedOperationalLimitsGroup();
    ASSERT_TRUE(selected.has_value());
    EXPECT_EQ(*selected, groups[0]);
}

TEST_F(DanglingLineTest, GetName) {
    DanglingLine dl(DL_HANDLE, &backend);
    EXPECT_EQ(dl.getNameOrId(), "Dangling Line 1");
}

TEST_F(DanglingLineTest, GetP0Q0) {
    DanglingLine dl(DL_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(dl.getP0(), 50.0);
    EXPECT_DOUBLE_EQ(dl.getQ0(), 10.0);
}

TEST_F(DanglingLineTest, SetP0Q0) {
    DanglingLine dl(DL_HANDLE, &backend);
    dl.setP0(55.0).setQ0(15.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{DL_HANDLE, prop::DL_P0}]), 55.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{DL_HANDLE, prop::DL_Q0}]), 15.0);
}

TEST_F(DanglingLineTest, GetRXGB) {
    backend.doubles[{DL_HANDLE, prop::DL_X}] = 2.0;
    backend.doubles[{DL_HANDLE, prop::DL_G}] = 0.001;
    backend.doubles[{DL_HANDLE, prop::DL_B}] = 0.002;
    DanglingLine dl(DL_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(dl.getR(), 1.0);
    EXPECT_DOUBLE_EQ(dl.getX(), 2.0);
    EXPECT_DOUBLE_EQ(dl.getG(), 0.001);
    EXPECT_DOUBLE_EQ(dl.getB(), 0.002);
}

TEST_F(DanglingLineTest, GetTerminal) {
    static constexpr ObjectHandle TERM_H = 50;
    backend.related[{DL_HANDLE, prop::REL_TERMINAL}] = TERM_H;
    backend.doubles[{TERM_H, prop::TERMINAL_P}]      = 50.0;
    DanglingLine dl(DL_HANDLE, &backend);
    EXPECT_TRUE(dl.getTerminal().isValid());
}

TEST_F(DanglingLineTest, IsValid) {
    DanglingLine valid(DL_HANDLE, &backend);
    DanglingLine invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}
