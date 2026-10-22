#include <gtest/gtest.h>
#include <iidm/TwoWindingsTransformer.h>
#include <iidm/Substation.h>
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

TEST_F(TwoWindingsTransformerTest, Substation) {
    static constexpr ObjectHandle SUB_H = 40;
    TwoWindingsTransformer twt(TWT_H, &backend);
    EXPECT_FALSE(twt.getSubstation());
    backend.related[{TWT_H, prop::REL_SUBSTATION}] = SUB_H;
    backend.strings[{SUB_H, prop::ID}] = "S";
    EXPECT_EQ(twt.getSubstation()->getId(), "S");
}

TEST_F(TwoWindingsTransformerTest, SolvedValues) {
    TwoWindingsTransformer twt(TWT_H, &backend);
    twt.applySolvedValues();
    twt.setRatioTapPositionToSolvedTapPosition();
    twt.setPhaseTapPositionToSolvedTapPosition();
    twt.unsetSolvedValues();
    ASSERT_EQ(backend.invocations.size(), 4u);
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(TWT_H), prop::ACT_APPLY_SOLVED_VALUES));
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(TWT_H), prop::ACT_SET_RATIO_TAP_POSITION_TO_SOLVED_TAP_POSITION));
    EXPECT_EQ(backend.invocations[2], std::make_pair(ObjectHandle(TWT_H), prop::ACT_SET_PHASE_TAP_POSITION_TO_SOLVED_TAP_POSITION));
    EXPECT_EQ(backend.invocations[3], std::make_pair(ObjectHandle(TWT_H), prop::ACT_UNSET_SOLVED_VALUES));
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

// The limits of each side are those of its holder (see TestLine.cpp)
TEST_F(TwoWindingsTransformerTest, GetCurrentLimits) {
    static constexpr ObjectHandle SIDE1_H = 32;
    static constexpr ObjectHandle SIDE2_H = 33;
    backend.relatedByIndex[std::make_tuple(ObjectHandle(TWT_H), prop::FLOWS_LIMITS_HOLDER, 0)] = SIDE1_H;
    backend.relatedByIndex[std::make_tuple(ObjectHandle(TWT_H), prop::FLOWS_LIMITS_HOLDER, 1)] = SIDE2_H;
    TwoWindingsTransformer twt(TWT_H, &backend);
    EXPECT_FALSE(twt.getCurrentLimits1());
    backend.related[{SIDE1_H, prop::CURRENT_LIMITS}]       = CL1_H;
    backend.related[{SIDE2_H, prop::CURRENT_LIMITS}]       = CL2_H;
    backend.doubles[{CL1_H, prop::LL_PERMANENT_LIMIT}] = 500.0;
    backend.doubles[{CL2_H, prop::LL_PERMANENT_LIMIT}] = 600.0;
    ASSERT_TRUE(twt.getCurrentLimits1());
    EXPECT_DOUBLE_EQ(twt.getCurrentLimits1()->getPermanentLimit(), 500.0);
    ASSERT_TRUE(twt.getCurrentLimits2());
    EXPECT_DOUBLE_EQ(twt.getCurrentLimits2()->getPermanentLimit(), 600.0);
}

