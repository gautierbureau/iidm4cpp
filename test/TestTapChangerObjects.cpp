#include <gtest/gtest.h>
#include <iidm/TwoWindingsTransformer.h>
#include <iidm/RatioTapChanger.h>
#include <iidm/PhaseTapChanger.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

static constexpr ObjectHandle TWT_HANDLE = 20;
static constexpr ObjectHandle S0 = 100, S1 = 101, S2 = 102; // RTC step handles
static constexpr ObjectHandle P0 = 200, P1 = 201;           // PTC step handles

static MockBackend makeTwtBackend() {
    MockBackend b;
    b.strings[{TWT_HANDLE, prop::ID}]  = "TWT1";
    b.strings[{TWT_HANDLE, prop::NAME}] = "Transformer 1";
    // RTC
    b.bools [{TWT_HANDLE, prop::TWO_WT_RTC_EXISTS}]       = true;
    b.ints  [{TWT_HANDLE, prop::TWO_WT_RTC_TAP_POSITION}] = 1;
    b.ints  [{TWT_HANDLE, prop::TWO_WT_RTC_LOW_TAP}]      = 0;
    b.ints  [{TWT_HANDLE, prop::TWO_WT_RTC_HIGH_TAP}]     = 2;
    b.bools [{TWT_HANDLE, prop::TWO_WT_RTC_REGULATING}]   = true;
    b.doubles[{TWT_HANDLE, prop::TWO_WT_RTC_TARGET_V}]    = 225.0;
    // RTC steps (lowTap=0, highTap=2 → 3 steps)
    b.children[{TWT_HANDLE, prop::TWO_WT_RTC_STEP}] = {S0, S1, S2};
    b.doubles[{S0, prop::RTC_STEP_RHO}] = 0.98; b.doubles[{S0, prop::RTC_STEP_R}] = 0.0;
    b.doubles[{S1, prop::RTC_STEP_RHO}] = 1.00; b.doubles[{S1, prop::RTC_STEP_R}] = 0.0;
    b.doubles[{S2, prop::RTC_STEP_RHO}] = 1.02; b.doubles[{S2, prop::RTC_STEP_R}] = 0.0;
    // PTC
    b.bools [{TWT_HANDLE, prop::TWO_WT_PTC_EXISTS}]       = true;
    b.ints  [{TWT_HANDLE, prop::TWO_WT_PTC_TAP_POSITION}] = 0;
    b.ints  [{TWT_HANDLE, prop::TWO_WT_PTC_LOW_TAP}]      = -1;
    b.ints  [{TWT_HANDLE, prop::TWO_WT_PTC_HIGH_TAP}]     = 0;
    b.bools [{TWT_HANDLE, prop::TWO_WT_PTC_REGULATING}]   = false;
    b.ints  [{TWT_HANDLE, prop::TWO_WT_PTC_REG_MODE}]     = 2; // FIXED_TAP
    b.doubles[{TWT_HANDLE, prop::TWO_WT_PTC_REG_VALUE}]   = 0.0;
    // PTC steps
    b.children[{TWT_HANDLE, prop::TWO_WT_PTC_STEP}] = {P0, P1};
    b.doubles[{P0, prop::PTC_STEP_ALPHA}] = -5.0; b.doubles[{P0, prop::PTC_STEP_RHO}] = 1.0;
    b.doubles[{P1, prop::PTC_STEP_ALPHA}] =  0.0; b.doubles[{P1, prop::PTC_STEP_RHO}] = 1.0;
    return b;
}

// ── RatioTapChanger object tests ───────────────────────────────────────────────

TEST(TapChangerObjectTest, RtcPresent) {
    auto b = makeTwtBackend();
    TwoWindingsTransformer twt(TWT_HANDLE, &b);
    auto rtc = twt.getRatioTapChanger();
    ASSERT_TRUE(rtc.has_value());
    EXPECT_TRUE(rtc->isValid());
}

TEST(TapChangerObjectTest, RtcAbsent) {
    MockBackend b;
    b.bools[{TWT_HANDLE, prop::TWO_WT_RTC_EXISTS}] = false;
    TwoWindingsTransformer twt(TWT_HANDLE, &b);
    EXPECT_FALSE(twt.getRatioTapChanger().has_value());
}

TEST(TapChangerObjectTest, RtcTapPosition) {
    auto b = makeTwtBackend();
    TwoWindingsTransformer twt(TWT_HANDLE, &b);
    auto rtc = twt.getRatioTapChanger();
    ASSERT_TRUE(rtc.has_value());
    EXPECT_EQ(rtc->getTapPosition(),  1);
    EXPECT_EQ(rtc->getLowTapPosition(),  0);
    EXPECT_EQ(rtc->getHighTapPosition(), 2);
    rtc->setTapPosition(2);
    int tapPos = b.ints[{TWT_HANDLE, prop::TWO_WT_RTC_TAP_POSITION}];
    EXPECT_EQ(tapPos, 2);
}

TEST(TapChangerObjectTest, RtcRegulatingAndTargetV) {
    auto b = makeTwtBackend();
    TwoWindingsTransformer twt(TWT_HANDLE, &b);
    auto rtc = twt.getRatioTapChanger();
    ASSERT_TRUE(rtc.has_value());
    EXPECT_TRUE(rtc->isRegulating());
    EXPECT_DOUBLE_EQ(rtc->getTargetV(), 225.0);
    rtc->setTargetV(230.0);
    double tv = b.doubles[{TWT_HANDLE, prop::TWO_WT_RTC_TARGET_V}];
    EXPECT_DOUBLE_EQ(tv, 230.0);
}

TEST(TapChangerObjectTest, RtcAllSteps) {
    auto b = makeTwtBackend();
    TwoWindingsTransformer twt(TWT_HANDLE, &b);
    auto rtc = twt.getRatioTapChanger();
    ASSERT_TRUE(rtc.has_value());
    auto steps = rtc->getAllSteps();
    ASSERT_EQ(steps.size(), 3u);
    EXPECT_DOUBLE_EQ(steps[0].getRho(), 0.98);
    EXPECT_DOUBLE_EQ(steps[1].getRho(), 1.00);
    EXPECT_DOUBLE_EQ(steps[2].getRho(), 1.02);
}

TEST(TapChangerObjectTest, RtcCurrentStep) {
    auto b = makeTwtBackend(); // tapPosition=1, lowTap=0 → index 1 → rho=1.00
    TwoWindingsTransformer twt(TWT_HANDLE, &b);
    auto rtc = twt.getRatioTapChanger();
    ASSERT_TRUE(rtc.has_value());
    EXPECT_DOUBLE_EQ(rtc->getCurrentStep().getRho(), 1.00);
}

// ── PhaseTapChanger object tests ──────────────────────────────────────────────

TEST(TapChangerObjectTest, PtcPresent) {
    auto b = makeTwtBackend();
    TwoWindingsTransformer twt(TWT_HANDLE, &b);
    auto ptc = twt.getPhaseTapChanger();
    ASSERT_TRUE(ptc.has_value());
    EXPECT_TRUE(ptc->isValid());
}

TEST(TapChangerObjectTest, PtcTapPosition) {
    auto b = makeTwtBackend();
    TwoWindingsTransformer twt(TWT_HANDLE, &b);
    auto ptc = twt.getPhaseTapChanger();
    ASSERT_TRUE(ptc.has_value());
    EXPECT_EQ(ptc->getTapPosition(),   0);
    EXPECT_EQ(ptc->getLowTapPosition(), -1);
    EXPECT_EQ(ptc->getHighTapPosition(), 0);
}

TEST(TapChangerObjectTest, PtcRegulationMode) {
    auto b = makeTwtBackend();
    TwoWindingsTransformer twt(TWT_HANDLE, &b);
    auto ptc = twt.getPhaseTapChanger();
    ASSERT_TRUE(ptc.has_value());
    EXPECT_EQ(ptc->getRegulationMode(), PhaseTapChangerRegulationMode::FIXED_TAP);
    ptc->setRegulationMode(PhaseTapChangerRegulationMode::CURRENT_LIMITER);
    int regMode = b.ints[{TWT_HANDLE, prop::TWO_WT_PTC_REG_MODE}];
    EXPECT_EQ(regMode, 0);
}

TEST(TapChangerObjectTest, PtcAllSteps) {
    auto b = makeTwtBackend();
    TwoWindingsTransformer twt(TWT_HANDLE, &b);
    auto ptc = twt.getPhaseTapChanger();
    ASSERT_TRUE(ptc.has_value());
    auto steps = ptc->getAllSteps();
    ASSERT_EQ(steps.size(), 2u);
    EXPECT_DOUBLE_EQ(steps[0].getAlpha(), -5.0);
    EXPECT_DOUBLE_EQ(steps[1].getAlpha(),  0.0);
}

TEST(TapChangerObjectTest, PtcCurrentStep) {
    auto b = makeTwtBackend(); // tapPosition=0, lowTap=-1 → index 1 → alpha=0.0
    TwoWindingsTransformer twt(TWT_HANDLE, &b);
    auto ptc = twt.getPhaseTapChanger();
    ASSERT_TRUE(ptc.has_value());
    EXPECT_DOUBLE_EQ(ptc->getCurrentStep().getAlpha(), 0.0);
}
