#include <gtest/gtest.h>
#include <iidm/PhaseTapChanger.h>
#include <iidm/PropertyCodes.h>
#include <iidm/RatioTapChanger.h>
#include <iidm/TwoWindingsTransformer.h>
#include <iidm/VoltageRegulation.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

namespace {

const ObjectHandle TWT = 20;
const ObjectHandle BARE_TWT = 21; // no tap changer
const ObjectHandle RTC = 30;
const ObjectHandle R_LOW = 31, R_MID = 32, R_HIGH = 33; // RTC steps, positions -1, 0, 1
const ObjectHandle PTC = 40;
const ObjectHandle P0 = 41, P1 = 42; // PTC steps, positions 0, 1
const ObjectHandle TERM = 50;
const ObjectHandle REGULATION = 60;

class TapChangerTest : public ::testing::Test {
protected:
    MockBackend backend;

    void SetUp() override {
        backend.related[{TWT, prop::RATIO_TAP_CHANGER}] = RTC;
        backend.related[{TWT, prop::PHASE_TAP_CHANGER}] = PTC;

        backend.ints   [{RTC, prop::TC_LOW_TAP_POSITION}]  = -1;
        backend.ints   [{RTC, prop::TC_HIGH_TAP_POSITION}] = 1;
        backend.ints   [{RTC, prop::TC_STEP_COUNT}]        = 3;
        backend.ints   [{RTC, prop::TC_TAP_POSITION}]      = 0;
        backend.optInts[{RTC, prop::TC_TAP_POSITION}]      = 0;
        backend.optInts[{RTC, prop::TC_NEUTRAL_POSITION}]  = 0;
        backend.bools  [{RTC, prop::TC_LOAD_TAP_CHANGING_CAPABILITIES}] = true;
        backend.bools  [{RTC, prop::REGULATING}]           = true;
        backend.children[{RTC, prop::RATIO_TAP_CHANGER_STEP}] = {R_LOW, R_MID, R_HIGH};
        backend.relatedByIndex[std::make_tuple(RTC, prop::RATIO_TAP_CHANGER_STEP, 1)] = R_HIGH;
        backend.related[{RTC, prop::REL_CURRENT_STEP}] = R_MID;
        backend.related[{RTC, prop::REL_NEUTRAL_STEP}] = R_MID;
        backend.doubles[{R_LOW, prop::STEP_RHO}]  = 0.9;
        backend.doubles[{R_MID, prop::STEP_RHO}]  = 1.0;
        backend.doubles[{R_HIGH, prop::STEP_RHO}] = 1.1;
        backend.doubles[{R_HIGH, prop::STEP_X}]   = 2.0;

        backend.ints   [{PTC, prop::TC_LOW_TAP_POSITION}] = 0;
        backend.ints   [{PTC, prop::TC_HIGH_TAP_POSITION}] = 1;
        backend.ints   [{PTC, prop::PTC_REGULATION_MODE}] = 1; // ACTIVE_POWER_CONTROL
        backend.doubles[{PTC, prop::PTC_REGULATION_VALUE}] = 50.0;
        backend.doubles[{PTC, prop::PTC_TARGET_DEADBAND}] = 5.0;
        backend.children[{PTC, prop::PHASE_TAP_CHANGER_STEP}] = {P0, P1};
        backend.doubles[{P0, prop::PTC_STEP_ALPHA}] = -10.0;
        backend.doubles[{P0, prop::STEP_RHO}]       = 1.0;
        backend.doubles[{P1, prop::PTC_STEP_ALPHA}] = 0.0;
    }

    RatioTapChanger rtc() { return *TwoWindingsTransformer(TWT, &backend).getRatioTapChanger(); }
    PhaseTapChanger ptc() { return *TwoWindingsTransformer(TWT, &backend).getPhaseTapChanger(); }
};

} // namespace

TEST_F(TapChangerTest, OfItsTransformer) {
    TwoWindingsTransformer twt(TWT, &backend);
    EXPECT_TRUE(twt.hasRatioTapChanger());
    EXPECT_TRUE(twt.hasPhaseTapChanger());
    EXPECT_EQ(*twt.getRatioTapChanger(), RatioTapChanger(RTC, &backend));
    TwoWindingsTransformer bare(BARE_TWT, &backend);
    EXPECT_FALSE(bare.hasRatioTapChanger());
    EXPECT_FALSE(bare.getPhaseTapChanger().has_value());
}

TEST_F(TapChangerTest, Positions) {
    RatioTapChanger tc = rtc();
    EXPECT_EQ(tc.getLowTapPosition(), -1);
    EXPECT_EQ(tc.getHighTapPosition(), 1);
    EXPECT_EQ(tc.getStepCount(), 3);
    EXPECT_EQ(tc.getTapPosition(), 0);
    ASSERT_TRUE(tc.findTapPosition().has_value());
    EXPECT_EQ(*tc.findTapPosition(), 0);
    EXPECT_EQ(*tc.getNeutralPosition(), 0);
    EXPECT_FALSE(tc.getSolvedTapPosition().has_value());

    // Setters chain with the other setters of the tap changer
    tc.setTapPosition(1).setLowTapPosition(-2).setSolvedTapPosition(1).setLoadTapChangingCapabilities(false);
    EXPECT_EQ((backend.ints[{RTC, prop::TC_TAP_POSITION}]), 1);
    EXPECT_EQ((backend.ints[{RTC, prop::TC_LOW_TAP_POSITION}]), -2);
    EXPECT_EQ((backend.ints[{RTC, prop::TC_SOLVED_TAP_POSITION}]), 1);
    EXPECT_FALSE((backend.bools[{RTC, prop::TC_LOAD_TAP_CHANGING_CAPABILITIES}]));
}

TEST_F(TapChangerTest, Actions) {
    RatioTapChanger tc = rtc();
    tc.unsetTapPosition().unsetSolvedTapPosition();
    tc.applySolvedValues();
    tc.setTapPositionToSolvedTapPosition();
    tc.remove();
    ASSERT_EQ(backend.invocations.size(), 5u);
    EXPECT_EQ(backend.invocations[0], std::make_pair(RTC, prop::ACT_UNSET_TAP_POSITION));
    EXPECT_EQ(backend.invocations[1], std::make_pair(RTC, prop::ACT_UNSET_SOLVED_TAP_POSITION));
    EXPECT_EQ(backend.invocations[2], std::make_pair(RTC, prop::ACT_APPLY_SOLVED_VALUES));
    EXPECT_EQ(backend.invocations[3], std::make_pair(RTC, prop::ACT_SET_TAP_POSITION_TO_SOLVED_TAP_POSITION));
    EXPECT_EQ(backend.invocations[4], std::make_pair(RTC, prop::ACT_REMOVE));
}

TEST_F(TapChangerTest, StepsByTapPosition) {
    RatioTapChanger tc = rtc();
    std::map<int, RatioTapChangerStep> steps = tc.getAllSteps();
    ASSERT_EQ(steps.size(), 3u);
    EXPECT_DOUBLE_EQ(steps.at(-1).getRho(), 0.9);
    EXPECT_DOUBLE_EQ(steps.at(0).getRho(), 1.0);
    EXPECT_DOUBLE_EQ(steps.at(1).getRho(), 1.1);
    EXPECT_EQ(tc.getStep(1), steps.at(1));
    EXPECT_DOUBLE_EQ(tc.getStep(1).getX(), 2.0);
    ASSERT_TRUE(tc.getCurrentStep().has_value());
    EXPECT_EQ(*tc.getCurrentStep(), steps.at(0));
    EXPECT_EQ(*tc.getNeutralStep(), steps.at(0));
    EXPECT_FALSE(tc.getSolvedCurrentStep().has_value());

    tc.getStep(1).setRho(1.2).setR(0.5);
    EXPECT_DOUBLE_EQ((backend.doubles[{R_HIGH, prop::STEP_RHO}]), 1.2);
    EXPECT_DOUBLE_EQ((backend.doubles[{R_HIGH, prop::STEP_R}]), 0.5);
}

TEST_F(TapChangerTest, RatioTapChangerRegulatesThroughItsVoltageRegulation) {
    backend.related [{RTC, prop::VOLTAGE_REGULATION}] = REGULATION;
    backend.doubles [{REGULATION, prop::VR_TARGET_VALUE}] = 230.0;
    backend.intLists[{RTC, prop::REGULATING_MODES}] = {static_cast<int>(RegulationMode::VOLTAGE)};
    RatioTapChanger tc = rtc();
    EXPECT_TRUE(tc.isRegulating());
    EXPECT_TRUE(tc.isRegulatingWithMode(RegulationMode::VOLTAGE));
    EXPECT_FALSE(tc.isRegulatingWithMode(RegulationMode::REACTIVE_POWER));
    ASSERT_TRUE(tc.getVoltageRegulation().has_value());
    EXPECT_DOUBLE_EQ(tc.getVoltageRegulation()->getTargetValue(), 230.0);
}

TEST_F(TapChangerTest, PhaseTapChanger) {
    PhaseTapChanger tc = ptc();
    EXPECT_EQ(tc.getRegulationMode(), PhaseTapChangerRegulationMode::ACTIVE_POWER_CONTROL);
    EXPECT_DOUBLE_EQ(tc.getRegulationValue(), 50.0);
    EXPECT_DOUBLE_EQ(tc.getTargetDeadband(), 5.0);
    EXPECT_FALSE(tc.isRegulating());
    EXPECT_FALSE(tc.getRegulationTerminal().has_value());

    tc.setRegulationMode(PhaseTapChangerRegulationMode::CURRENT_LIMITER)
      .setRegulationValue(300.0)
      .setTargetDeadband(10.0)
      .setRegulationTerminal(Terminal(TERM, &backend))
      .setRegulating(true)
      .setTapPosition(1);
    EXPECT_EQ((backend.ints[{PTC, prop::PTC_REGULATION_MODE}]), 0);
    EXPECT_DOUBLE_EQ((backend.doubles[{PTC, prop::PTC_REGULATION_VALUE}]), 300.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{PTC, prop::PTC_TARGET_DEADBAND}]), 10.0);
    EXPECT_EQ((backend.related[{PTC, prop::REL_REGULATION_TERMINAL}]), TERM);
    EXPECT_TRUE((backend.bools[{PTC, prop::REGULATING}]));
    ASSERT_TRUE(tc.getRegulationTerminal().has_value());
    EXPECT_EQ(*tc.getRegulationTerminal(), Terminal(TERM, &backend));
}

TEST_F(TapChangerTest, PhaseTapChangerSteps) {
    std::map<int, PhaseTapChangerStep> steps = ptc().getAllSteps();
    ASSERT_EQ(steps.size(), 2u);
    EXPECT_DOUBLE_EQ(steps.at(0).getAlpha(), -10.0);
    EXPECT_DOUBLE_EQ(steps.at(0).getRho(), 1.0);
    EXPECT_DOUBLE_EQ(steps.at(1).getAlpha(), 0.0);
    steps.at(1).setAlpha(5.0).setX(1.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{P1, prop::PTC_STEP_ALPHA}]), 5.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{P1, prop::STEP_X}]), 1.0);
}

TEST_F(TapChangerTest, Properties) {
    PhaseTapChanger tc = ptc();
    tc.setProperty("k", "v").setRegulating(false);
    EXPECT_EQ(*tc.getProperty("k"), "v");
    PhaseTapChangerStep step = tc.getAllSteps().at(0);
    step.setProperty("s", "w").setAlpha(1.0);
    EXPECT_EQ(*step.getProperty("s"), "w");
}

TEST_F(TapChangerTest, Invalid) {
    RatioTapChanger none;
    EXPECT_FALSE(none.isValid());
    EXPECT_THROW(none.getTapPosition(), InvalidHandleException);
    EXPECT_THROW(PhaseTapChangerStep().getAlpha(), InvalidHandleException);
}
