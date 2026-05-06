#include <gtest/gtest.h>
#include <iidm/TwoWindingsTransformer.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class TwoWindingsTransformerTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle TWT_H  = 10;
    static constexpr ObjectHandle T1_H   = 20;
    static constexpr ObjectHandle T2_H   = 21;
    static constexpr ObjectHandle CL1_H  = 30;
    static constexpr ObjectHandle CL2_H  = 31;

    void SetUp() override {
        backend.strings[{TWT_H, prop::ID}]           = "TWT1";
        backend.strings[{TWT_H, prop::NAME}]          = "Two Windings T1";
        backend.doubles[{TWT_H, prop::TWO_WT_R}]      = 1.5;
        backend.doubles[{TWT_H, prop::TWO_WT_X}]      = 10.0;
        backend.doubles[{TWT_H, prop::TWO_WT_G}]      = 0.001;
        backend.doubles[{TWT_H, prop::TWO_WT_B}]      = 0.002;
        backend.doubles[{TWT_H, prop::TWO_WT_RATED_U1}] = 400.0;
        backend.doubles[{TWT_H, prop::TWO_WT_RATED_U2}] = 225.0;
        backend.doubles[{TWT_H, prop::TWO_WT_RATED_S}]  = 300.0;
        backend.related[{TWT_H, prop::REL_TERMINAL1}] = T1_H;
        backend.related[{TWT_H, prop::REL_TERMINAL2}] = T2_H;
        backend.bools  [{TWT_H, prop::TERMINAL_CONNECTED}] = true;
    }
};

TEST_F(TwoWindingsTransformerTest, GetId) {
    TwoWindingsTransformer twt(TWT_H, &backend);
    EXPECT_EQ(twt.getId(), "TWT1");
}

TEST_F(TwoWindingsTransformerTest, GetElectricalParameters) {
    TwoWindingsTransformer twt(TWT_H, &backend);
    EXPECT_DOUBLE_EQ(twt.getR(), 1.5);
    EXPECT_DOUBLE_EQ(twt.getX(), 10.0);
    EXPECT_DOUBLE_EQ(twt.getG(), 0.001);
    EXPECT_DOUBLE_EQ(twt.getB(), 0.002);
    EXPECT_DOUBLE_EQ(twt.getRatedU1(), 400.0);
    EXPECT_DOUBLE_EQ(twt.getRatedU2(), 225.0);
    ASSERT_TRUE(twt.getRatedS().has_value());
    EXPECT_DOUBLE_EQ(*twt.getRatedS(), 300.0);
}

TEST_F(TwoWindingsTransformerTest, GetRatedSAbsent) {
    // NaN → nullopt
    TwoWindingsTransformer twt(TWT_H, &backend);
    backend.doubles.erase({TWT_H, prop::TWO_WT_RATED_S});
    EXPECT_FALSE(twt.getRatedS().has_value());
}

TEST_F(TwoWindingsTransformerTest, GetTerminals) {
    backend.doubles[{T1_H, prop::TERMINAL_P}] = 100.0;
    backend.doubles[{T2_H, prop::TERMINAL_P}] = -98.0;
    TwoWindingsTransformer twt(TWT_H, &backend);
    EXPECT_TRUE(twt.getTerminal1().isValid());
    EXPECT_TRUE(twt.getTerminal2().isValid());
    EXPECT_DOUBLE_EQ(twt.getTerminal1().getP(), 100.0);
    EXPECT_DOUBLE_EQ(twt.getTerminal2().getP(), -98.0);
}

TEST_F(TwoWindingsTransformerTest, GetCurrentLimitsAbsent) {
    TwoWindingsTransformer twt(TWT_H, &backend);
    EXPECT_FALSE(twt.getCurrentLimits1().has_value());
    EXPECT_FALSE(twt.getCurrentLimits2().has_value());
}

TEST_F(TwoWindingsTransformerTest, GetCurrentLimitsPresent) {
    backend.related[{TWT_H, prop::REL_CURRENT_LIMITS1}] = CL1_H;
    backend.related[{TWT_H, prop::REL_CURRENT_LIMITS2}] = CL2_H;
    backend.doubles[{CL1_H, prop::CL_PERMANENT_LIMIT}]  = 500.0;
    backend.doubles[{CL2_H, prop::CL_PERMANENT_LIMIT}]  = 600.0;
    TwoWindingsTransformer twt(TWT_H, &backend);
    ASSERT_TRUE(twt.getCurrentLimits1().has_value());
    EXPECT_DOUBLE_EQ(twt.getCurrentLimits1()->getPermanentLimit(), 500.0);
    ASSERT_TRUE(twt.getCurrentLimits2().has_value());
    EXPECT_DOUBLE_EQ(twt.getCurrentLimits2()->getPermanentLimit(), 600.0);
}

TEST_F(TwoWindingsTransformerTest, GetName) {
    TwoWindingsTransformer twt(TWT_H, &backend);
    EXPECT_EQ(twt.getName(), "Two Windings T1");
}

TEST_F(TwoWindingsTransformerTest, GetOperationalLimitsGroupsEmpty) {
    TwoWindingsTransformer twt(TWT_H, &backend);
    EXPECT_EQ(twt.getOperationalLimitsGroups1().size(), 0u);
    EXPECT_EQ(twt.getOperationalLimitsGroups2().size(), 0u);
    EXPECT_FALSE(twt.getSelectedOperationalLimitsGroup1().has_value());
    EXPECT_FALSE(twt.getSelectedOperationalLimitsGroup2().has_value());
}

TEST_F(TwoWindingsTransformerTest, GetOperationalLimitsGroups) {
    static constexpr ObjectHandle OLG1_H = 40;
    static constexpr ObjectHandle OLG2_H = 41;
    backend.children[{TWT_H, prop::OLG_SIDE1}] = {OLG1_H};
    backend.children[{TWT_H, prop::OLG_SIDE2}] = {OLG2_H};
    backend.strings [{OLG1_H, prop::OLG_ID}]   = "DEFAULT";
    backend.strings [{OLG2_H, prop::OLG_ID}]   = "DEFAULT";
    TwoWindingsTransformer twt(TWT_H, &backend);
    ASSERT_EQ(twt.getOperationalLimitsGroups1().size(), 1u);
    ASSERT_EQ(twt.getOperationalLimitsGroups2().size(), 1u);
    EXPECT_EQ(twt.getOperationalLimitsGroups1()[0].getId(), "DEFAULT");
    EXPECT_EQ(twt.getOperationalLimitsGroups2()[0].getId(), "DEFAULT");
}

TEST_F(TwoWindingsTransformerTest, GetSelectedOperationalLimitsGroups) {
    static constexpr ObjectHandle OLG1_H = 40;
    static constexpr ObjectHandle OLG2_H = 41;
    backend.related[{TWT_H, prop::REL_SELECTED_OLG1}] = OLG1_H;
    backend.related[{TWT_H, prop::REL_SELECTED_OLG2}] = OLG2_H;
    backend.strings[{OLG1_H, prop::OLG_ID}] = "GRP1";
    backend.strings[{OLG2_H, prop::OLG_ID}] = "GRP2";
    TwoWindingsTransformer twt(TWT_H, &backend);
    auto olg1 = twt.getSelectedOperationalLimitsGroup1();
    auto olg2 = twt.getSelectedOperationalLimitsGroup2();
    ASSERT_TRUE(olg1.has_value());
    ASSERT_TRUE(olg2.has_value());
    EXPECT_EQ(olg1->getId(), "GRP1");
    EXPECT_EQ(olg2->getId(), "GRP2");
}

TEST_F(TwoWindingsTransformerTest, ConnectDisconnect) {
    backend.bools[{T1_H, prop::TERMINAL_CONNECTED}] = true;
    backend.bools[{T2_H, prop::TERMINAL_CONNECTED}] = true;
    TwoWindingsTransformer twt(TWT_H, &backend);
    EXPECT_TRUE(twt.isConnected());
    twt.disconnect();
    EXPECT_FALSE((backend.bools[{T1_H, prop::TERMINAL_CONNECTED}]));
    EXPECT_FALSE((backend.bools[{T2_H, prop::TERMINAL_CONNECTED}]));
    twt.connect();
    EXPECT_TRUE((backend.bools[{T1_H, prop::TERMINAL_CONNECTED}]));
    EXPECT_TRUE((backend.bools[{T2_H, prop::TERMINAL_CONNECTED}]));
}

TEST_F(TwoWindingsTransformerTest, IsValid) {
    TwoWindingsTransformer valid(TWT_H, &backend);
    TwoWindingsTransformer invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}
