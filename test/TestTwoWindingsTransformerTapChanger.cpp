#include <gtest/gtest.h>
#include <iidm/TwoWindingsTransformer.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class TapChangerTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle TWT_HANDLE = 20;

    void SetUp() override {
        backend.strings[{TWT_HANDLE, prop::ID}]   = "TWT1";
        backend.strings[{TWT_HANDLE, prop::NAME}]  = "Transformer 1";
        // RTC
        backend.bools  [{TWT_HANDLE, prop::TWO_WT_RTC_EXISTS}]       = true;
        backend.ints   [{TWT_HANDLE, prop::TWO_WT_RTC_TAP_POSITION}] = 3;
        backend.ints   [{TWT_HANDLE, prop::TWO_WT_RTC_LOW_TAP}]      = 0;
        backend.ints   [{TWT_HANDLE, prop::TWO_WT_RTC_HIGH_TAP}]     = 5;
        backend.bools  [{TWT_HANDLE, prop::TWO_WT_RTC_REGULATING}]   = true;
        backend.doubles[{TWT_HANDLE, prop::TWO_WT_RTC_TARGET_V}]     = 225.0;
        // PTC
        backend.bools  [{TWT_HANDLE, prop::TWO_WT_PTC_EXISTS}]       = true;
        backend.ints   [{TWT_HANDLE, prop::TWO_WT_PTC_TAP_POSITION}] = 1;
        backend.ints   [{TWT_HANDLE, prop::TWO_WT_PTC_LOW_TAP}]      = -5;
        backend.ints   [{TWT_HANDLE, prop::TWO_WT_PTC_HIGH_TAP}]     = 5;
        backend.bools  [{TWT_HANDLE, prop::TWO_WT_PTC_REGULATING}]   = false;
        backend.ints   [{TWT_HANDLE, prop::TWO_WT_PTC_REG_MODE}]     = 1; // ACTIVE_POWER_CONTROL
        backend.doubles[{TWT_HANDLE, prop::TWO_WT_PTC_REG_VALUE}]    = 50.0;
    }
};

// ── RatioTapChanger ───────────────────────────────────────────────────────────

TEST_F(TapChangerTest, RtcExists) {
    TwoWindingsTransformer twt(TWT_HANDLE, &backend);
    EXPECT_TRUE(twt.hasRatioTapChanger());
}

TEST_F(TapChangerTest, RtcTapPositionGetSet) {
    TwoWindingsTransformer twt(TWT_HANDLE, &backend);
    EXPECT_EQ(twt.getRatioTapPosition(), 3);
    twt.setRatioTapPosition(4);
    int pos = backend.ints[{TWT_HANDLE, prop::TWO_WT_RTC_TAP_POSITION}];
    EXPECT_EQ(pos, 4);
}

TEST_F(TapChangerTest, RtcTapRange) {
    TwoWindingsTransformer twt(TWT_HANDLE, &backend);
    EXPECT_EQ(twt.getRatioTapLow(),  0);
    EXPECT_EQ(twt.getRatioTapHigh(), 5);
}

TEST_F(TapChangerTest, RtcRegulatingGetSet) {
    TwoWindingsTransformer twt(TWT_HANDLE, &backend);
    EXPECT_TRUE(twt.isRatioTapRegulating());
    twt.setRatioTapRegulating(false);
    bool reg = backend.bools[{TWT_HANDLE, prop::TWO_WT_RTC_REGULATING}];
    EXPECT_FALSE(reg);
}

TEST_F(TapChangerTest, RtcTargetVGetSet) {
    TwoWindingsTransformer twt(TWT_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(twt.getRatioTapTargetV(), 225.0);
    twt.setRatioTapTargetV(230.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{TWT_HANDLE, prop::TWO_WT_RTC_TARGET_V}]), 230.0);
}

TEST_F(TapChangerTest, RtcFluentChain) {
    TwoWindingsTransformer twt(TWT_HANDLE, &backend);
    twt.setRatioTapPosition(2).setRatioTapRegulating(false).setRatioTapTargetV(220.0);
    int chainPos = backend.ints[{TWT_HANDLE, prop::TWO_WT_RTC_TAP_POSITION}];
    EXPECT_EQ(chainPos, 2);
    EXPECT_DOUBLE_EQ((backend.doubles[{TWT_HANDLE, prop::TWO_WT_RTC_TARGET_V}]), 220.0);
}

// ── PhaseTapChanger ───────────────────────────────────────────────────────────

TEST_F(TapChangerTest, PtcExists) {
    TwoWindingsTransformer twt(TWT_HANDLE, &backend);
    EXPECT_TRUE(twt.hasPhaseTapChanger());
}

TEST_F(TapChangerTest, PtcTapPositionGetSet) {
    TwoWindingsTransformer twt(TWT_HANDLE, &backend);
    EXPECT_EQ(twt.getPhaseTapPosition(), 1);
    twt.setPhaseTapPosition(-2);
    int pos = backend.ints[{TWT_HANDLE, prop::TWO_WT_PTC_TAP_POSITION}];
    EXPECT_EQ(pos, -2);
}

TEST_F(TapChangerTest, PtcTapRange) {
    TwoWindingsTransformer twt(TWT_HANDLE, &backend);
    EXPECT_EQ(twt.getPhaseTapLow(),  -5);
    EXPECT_EQ(twt.getPhaseTapHigh(),  5);
}

TEST_F(TapChangerTest, PtcRegulatingGetSet) {
    TwoWindingsTransformer twt(TWT_HANDLE, &backend);
    EXPECT_FALSE(twt.isPhaseTapRegulating());
    twt.setPhaseTapRegulating(true);
    bool reg = backend.bools[{TWT_HANDLE, prop::TWO_WT_PTC_REGULATING}];
    EXPECT_TRUE(reg);
}

TEST_F(TapChangerTest, PtcRegulationMode) {
    TwoWindingsTransformer twt(TWT_HANDLE, &backend);
    EXPECT_EQ(twt.getPhaseTapRegulationMode(),
              PhaseTapChangerRegulationMode::ACTIVE_POWER_CONTROL);
    twt.setPhaseTapRegulationMode(PhaseTapChangerRegulationMode::FIXED_TAP);
    int mode = backend.ints[{TWT_HANDLE, prop::TWO_WT_PTC_REG_MODE}];
    EXPECT_EQ(mode, 2);
}

TEST_F(TapChangerTest, PtcRegulationValue) {
    TwoWindingsTransformer twt(TWT_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(twt.getPhaseTapRegulationValue(), 50.0);
    twt.setPhaseTapRegulationValue(75.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{TWT_HANDLE, prop::TWO_WT_PTC_REG_VALUE}]), 75.0);
}

TEST_F(TapChangerTest, NoRtcExists) {
    MockBackend b2;
    b2.bools[{TWT_HANDLE, prop::TWO_WT_RTC_EXISTS}] = false;
    TwoWindingsTransformer twt(TWT_HANDLE, &b2);
    EXPECT_FALSE(twt.hasRatioTapChanger());
}
