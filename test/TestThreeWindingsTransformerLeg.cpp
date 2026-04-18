#include <gtest/gtest.h>
#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/CurrentLimits.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

static constexpr ObjectHandle TWT_HANDLE = 70;
static constexpr ObjectHandle T1 = 71, T2 = 72, T3 = 73;
static constexpr ObjectHandle STEP0 = 80, STEP1 = 81;

class ThreeWindingsTransformerLegTest : public ::testing::Test {
protected:
    MockBackend backend;

    void SetUp() override {
        backend.strings[{TWT_HANDLE, prop::ID}]   = "3WT1";
        backend.strings[{TWT_HANDLE, prop::NAME}]  = "Three Windings T1";
        // Terminals
        backend.related[{TWT_HANDLE, prop::REL_TERMINAL1}] = T1;
        backend.related[{TWT_HANDLE, prop::REL_TERMINAL2}] = T2;
        backend.related[{TWT_HANDLE, prop::REL_TERMINAL}]  = T3;
        backend.bools[{T1, prop::TERMINAL_CONNECTED}] = true;
        backend.bools[{T2, prop::TERMINAL_CONNECTED}] = true;
        backend.bools[{T3, prop::TERMINAL_CONNECTED}] = true;
        // Leg 1 electrical parameters
        backend.doubles[{TWT_HANDLE, prop::THREE_WT_LEG1_BASE + prop::THREE_WT_LEG_R_OFF}]       = 1.5;
        backend.doubles[{TWT_HANDLE, prop::THREE_WT_LEG1_BASE + prop::THREE_WT_LEG_X_OFF}]       = 10.0;
        backend.doubles[{TWT_HANDLE, prop::THREE_WT_LEG1_BASE + prop::THREE_WT_LEG_G_OFF}]       = 0.001;
        backend.doubles[{TWT_HANDLE, prop::THREE_WT_LEG1_BASE + prop::THREE_WT_LEG_B_OFF}]       = 0.002;
        backend.doubles[{TWT_HANDLE, prop::THREE_WT_LEG1_BASE + prop::THREE_WT_LEG_RATED_U_OFF}] = 400.0;
        // Leg 2 electrical parameters
        backend.doubles[{TWT_HANDLE, prop::THREE_WT_LEG2_BASE + prop::THREE_WT_LEG_R_OFF}]       = 0.5;
        backend.doubles[{TWT_HANDLE, prop::THREE_WT_LEG2_BASE + prop::THREE_WT_LEG_X_OFF}]       = 5.0;
        backend.doubles[{TWT_HANDLE, prop::THREE_WT_LEG2_BASE + prop::THREE_WT_LEG_RATED_U_OFF}] = 225.0;
        // Leg 3 electrical parameters
        backend.doubles[{TWT_HANDLE, prop::THREE_WT_LEG3_BASE + prop::THREE_WT_LEG_R_OFF}]       = 0.1;
        backend.doubles[{TWT_HANDLE, prop::THREE_WT_LEG3_BASE + prop::THREE_WT_LEG_X_OFF}]       = 1.0;
        backend.doubles[{TWT_HANDLE, prop::THREE_WT_LEG3_BASE + prop::THREE_WT_LEG_RATED_U_OFF}] = 63.0;
        // Leg 1 with RTC
        backend.bools[{TWT_HANDLE, prop::THREE_WT_LEG1_BASE + prop::THREE_WT_LEG_RTC_EXISTS_OFF}]     = true;
        backend.ints [{TWT_HANDLE, prop::THREE_WT_LEG1_BASE + prop::THREE_WT_LEG_RTC_TAP_POS_OFF}]    = 0;
        backend.ints [{TWT_HANDLE, prop::THREE_WT_LEG1_BASE + prop::THREE_WT_LEG_RTC_LOW_TAP_OFF}]    = -1;
        backend.ints [{TWT_HANDLE, prop::THREE_WT_LEG1_BASE + prop::THREE_WT_LEG_RTC_HIGH_TAP_OFF}]   = 1;
        backend.bools[{TWT_HANDLE, prop::THREE_WT_LEG1_BASE + prop::THREE_WT_LEG_RTC_REGULATING_OFF}] = true;
        backend.doubles[{TWT_HANDLE, prop::THREE_WT_LEG1_BASE + prop::THREE_WT_LEG_RTC_TARGET_V_OFF}] = 400.0;
        // RTC steps for leg 1
        backend.children[{TWT_HANDLE, prop::THREE_WT_LEG1_RTC_STEP}] = {STEP0, STEP1};
        backend.doubles[{STEP0, prop::RTC_STEP_RHO}] = 0.99;
        backend.doubles[{STEP1, prop::RTC_STEP_RHO}] = 1.00;
    }
};

TEST_F(ThreeWindingsTransformerLegTest, GetId) {
    ThreeWindingsTransformer twt(TWT_HANDLE, &backend);
    EXPECT_EQ(twt.getId(), "3WT1");
}

TEST_F(ThreeWindingsTransformerLegTest, Leg1ElectricalParameters) {
    ThreeWindingsTransformer twt(TWT_HANDLE, &backend);
    auto leg = twt.getLeg1();
    EXPECT_TRUE(leg.isValid());
    EXPECT_DOUBLE_EQ(leg.getR(),      1.5);
    EXPECT_DOUBLE_EQ(leg.getX(),     10.0);
    EXPECT_DOUBLE_EQ(leg.getG(),    0.001);
    EXPECT_DOUBLE_EQ(leg.getB(),    0.002);
    EXPECT_DOUBLE_EQ(leg.getRatedU(), 400.0);
}

TEST_F(ThreeWindingsTransformerLegTest, Leg2ElectricalParameters) {
    ThreeWindingsTransformer twt(TWT_HANDLE, &backend);
    auto leg = twt.getLeg2();
    EXPECT_DOUBLE_EQ(leg.getR(),      0.5);
    EXPECT_DOUBLE_EQ(leg.getX(),      5.0);
    EXPECT_DOUBLE_EQ(leg.getRatedU(), 225.0);
}

TEST_F(ThreeWindingsTransformerLegTest, Leg3ElectricalParameters) {
    ThreeWindingsTransformer twt(TWT_HANDLE, &backend);
    auto leg = twt.getLeg3();
    EXPECT_DOUBLE_EQ(leg.getR(),      0.1);
    EXPECT_DOUBLE_EQ(leg.getX(),      1.0);
    EXPECT_DOUBLE_EQ(leg.getRatedU(), 63.0);
}

TEST_F(ThreeWindingsTransformerLegTest, Leg1HasRatioTapChanger) {
    ThreeWindingsTransformer twt(TWT_HANDLE, &backend);
    auto leg = twt.getLeg1();
    EXPECT_TRUE(leg.hasRatioTapChanger());
    EXPECT_FALSE(leg.hasPhaseTapChanger());
}

TEST_F(ThreeWindingsTransformerLegTest, Leg1RatioTapChangerValues) {
    ThreeWindingsTransformer twt(TWT_HANDLE, &backend);
    auto rtc = twt.getLeg1().getRatioTapChanger();
    EXPECT_TRUE(rtc.isValid());
    EXPECT_EQ(rtc.getTapPosition(),   0);
    EXPECT_EQ(rtc.getLowTapPosition(), -1);
    EXPECT_EQ(rtc.getHighTapPosition(), 1);
    EXPECT_TRUE(rtc.isRegulating());
    EXPECT_DOUBLE_EQ(rtc.getTargetV(), 400.0);
}

TEST_F(ThreeWindingsTransformerLegTest, Leg1RtcSteps) {
    ThreeWindingsTransformer twt(TWT_HANDLE, &backend);
    auto steps = twt.getLeg1().getRatioTapChanger().getAllSteps();
    ASSERT_EQ(steps.size(), 2u);
    EXPECT_DOUBLE_EQ(steps[0].getRho(), 0.99);
    EXPECT_DOUBLE_EQ(steps[1].getRho(), 1.00);
}

TEST_F(ThreeWindingsTransformerLegTest, Leg2NoTapChanger) {
    ThreeWindingsTransformer twt(TWT_HANDLE, &backend);
    auto leg = twt.getLeg2();
    EXPECT_FALSE(leg.hasRatioTapChanger());
    EXPECT_FALSE(leg.hasPhaseTapChanger());
}

TEST_F(ThreeWindingsTransformerLegTest, LegTerminals) {
    ThreeWindingsTransformer twt(TWT_HANDLE, &backend);
    EXPECT_TRUE(twt.getLeg1().getTerminal().isValid());
    EXPECT_TRUE(twt.getLeg2().getTerminal().isValid());
    EXPECT_TRUE(twt.getLeg3().getTerminal().isValid());
}

// ── Leg::getCurrentLimits ─────────────────────────────────────────────────────

static constexpr ObjectHandle CL1_H = 80;
static constexpr ObjectHandle CL3_H = 81;

TEST_F(ThreeWindingsTransformerLegTest, Leg1NoCurrentLimits) {
    ThreeWindingsTransformer twt(TWT_HANDLE, &backend);
    EXPECT_FALSE(twt.getLeg1().getCurrentLimits().has_value());
}

TEST_F(ThreeWindingsTransformerLegTest, Leg1CurrentLimits) {
    // Leg1 uses REL_CURRENT_LIMITS1 (2009)
    backend.related[{TWT_HANDLE, prop::REL_CURRENT_LIMITS1}] = CL1_H;
    backend.doubles[{CL1_H, prop::CL_PERMANENT_LIMIT}] = 600.0;
    ThreeWindingsTransformer twt(TWT_HANDLE, &backend);
    auto cl = twt.getLeg1().getCurrentLimits();
    ASSERT_TRUE(cl.has_value());
    EXPECT_DOUBLE_EQ(cl->getPermanentLimit(), 600.0);
}

TEST_F(ThreeWindingsTransformerLegTest, Leg3CurrentLimits) {
    // Leg3 uses REL_CURRENT_LIMITS3 (2011)
    backend.related[{TWT_HANDLE, prop::REL_CURRENT_LIMITS3}] = CL3_H;
    backend.doubles[{CL3_H, prop::CL_PERMANENT_LIMIT}] = 300.0;
    ThreeWindingsTransformer twt(TWT_HANDLE, &backend);
    auto cl = twt.getLeg3().getCurrentLimits();
    ASSERT_TRUE(cl.has_value());
    EXPECT_DOUBLE_EQ(cl->getPermanentLimit(), 300.0);
}
