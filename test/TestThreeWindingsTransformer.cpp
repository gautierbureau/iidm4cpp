#include <gtest/gtest.h>
#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/Substation.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

namespace {

const ObjectHandle TWT = 10;
const ObjectHandle LEG1 = 11, LEG2 = 12, LEG3 = 13;
const ObjectHandle T1 = 20, T2 = 21, T3 = 22;
const ObjectHandle SUB = 30;
const ObjectHandle RTC = 40;
const ObjectHandle OLG = 50;
const ObjectHandle LIMITS = 51;

class ThreeWindingsTransformerTest : public ::testing::Test {
protected:
    MockBackend backend;

    void SetUp() override {
        backend.strings[{TWT, prop::ID}]                = "3WT1";
        backend.doubles[{TWT, prop::THREE_WT_RATED_U0}] = 110.0;
        backend.related[{TWT, prop::REL_SUBSTATION}]    = SUB;
        backend.strings[{SUB, prop::ID}]                = "SUB1";
        backend.children[{TWT, prop::THREE_WINDINGS_TRANSFORMER_LEG}] = {LEG1, LEG2, LEG3};
        const ObjectHandle legs[] = {LEG1, LEG2, LEG3};
        const ObjectHandle terminals[] = {T1, T2, T3};
        for (int side = 0; side < 3; ++side) {
            backend.relatedByIndex[std::make_tuple(TWT, prop::THREE_WINDINGS_TRANSFORMER_LEG, side)] = legs[side];
            backend.relatedByIndex[std::make_tuple(TWT, prop::REL_TERMINAL, side)] = terminals[side];
            backend.related[{legs[side], prop::REL_TERMINAL}] = terminals[side];
            backend.related[{legs[side], prop::THREE_WINDINGS_TRANSFORMER}] = TWT;
            backend.ints   [{legs[side], prop::LEG_SIDE}] = side;
        }
        backend.relatedByString[std::make_tuple(TWT, prop::REL_TERMINAL, std::string("VL2"))] = T2;
        backend.doubles[{LEG1, prop::LEG_R}]       = 1.5;
        backend.doubles[{LEG1, prop::LEG_X}]       = 10.0;
        backend.doubles[{LEG1, prop::LEG_G}]       = 0.001;
        backend.doubles[{LEG1, prop::LEG_B}]       = 0.002;
        backend.doubles[{LEG1, prop::LEG_RATED_U}] = 400.0;
        backend.doubles[{LEG3, prop::LEG_RATED_S}] = 100.0;
        backend.related[{LEG1, prop::RATIO_TAP_CHANGER}] = RTC;
        backend.ints   [{RTC, prop::TC_TAP_POSITION}]    = 2;
        backend.children[{LEG1, prop::OPERATIONAL_LIMITS_GROUP}] = {OLG};
        backend.related [{LEG1, prop::REL_SELECTED_OLG}]         = OLG;
        backend.related [{LEG1, prop::CURRENT_LIMITS}]           = LIMITS;
        backend.related [{OLG, prop::CURRENT_LIMITS}]            = LIMITS;
        backend.strings [{OLG, prop::OLG_ID}]                    = "DEFAULT";
        backend.doubles [{LIMITS, prop::CL_PERMANENT_LIMIT}]     = 600.0;
    }
};

} // namespace

TEST_F(ThreeWindingsTransformerTest, Transformer) {
    ThreeWindingsTransformer twt(TWT, &backend);
    EXPECT_EQ(twt.getId(), "3WT1");
    EXPECT_DOUBLE_EQ(twt.getRatedU0(), 110.0);
    EXPECT_EQ(twt.getSubstation().getId(), "SUB1");
}

TEST_F(ThreeWindingsTransformerTest, Legs) {
    ThreeWindingsTransformer twt(TWT, &backend);
    std::vector<ThreeWindingsTransformer::Leg> legs = twt.getLegs();
    ASSERT_EQ(legs.size(), 3u);
    EXPECT_EQ(twt.getLeg1(), legs[0]);
    EXPECT_EQ(twt.getLeg2(), legs[1]);
    EXPECT_EQ(twt.getLeg3(), legs[2]);
    EXPECT_EQ(twt.getLeg(ThreeSides::TWO), legs[1]);
    EXPECT_EQ(legs[2].getSide(), ThreeSides::THREE);
    EXPECT_EQ(legs[0].getTransformer(), twt);
}

TEST_F(ThreeWindingsTransformerTest, Terminals) {
    ThreeWindingsTransformer twt(TWT, &backend);
    EXPECT_EQ(twt.getTerminal(ThreeSides::ONE), Terminal(T1, &backend));
    EXPECT_EQ(twt.getTerminal(ThreeSides::THREE), Terminal(T3, &backend));
    EXPECT_EQ(twt.getTerminal("VL2"), Terminal(T2, &backend));
    EXPECT_EQ(twt.getLeg2().getTerminal(), Terminal(T2, &backend));
}

TEST_F(ThreeWindingsTransformerTest, LegParameters) {
    ThreeWindingsTransformer::Leg leg = ThreeWindingsTransformer(TWT, &backend).getLeg1();
    EXPECT_DOUBLE_EQ(leg.getR(), 1.5);
    EXPECT_DOUBLE_EQ(leg.getX(), 10.0);
    EXPECT_DOUBLE_EQ(leg.getG(), 0.001);
    EXPECT_DOUBLE_EQ(leg.getB(), 0.002);
    EXPECT_DOUBLE_EQ(leg.getRatedU(), 400.0);
    EXPECT_FALSE(leg.getRatedS().has_value()); // NaN
    leg.setR(2.0).setX(20.0).setG(0.0).setB(0.0).setRatedU(410.0).setRatedS(50.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{LEG1, prop::LEG_R}]), 2.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{LEG1, prop::LEG_RATED_U}]), 410.0);
    EXPECT_DOUBLE_EQ(*leg.getRatedS(), 50.0);
    EXPECT_DOUBLE_EQ(*ThreeWindingsTransformer(TWT, &backend).getLeg3().getRatedS(), 100.0);
}

TEST_F(ThreeWindingsTransformerTest, LegTapChangers) {
    ThreeWindingsTransformer twt(TWT, &backend);
    ThreeWindingsTransformer::Leg leg1 = twt.getLeg1();
    EXPECT_TRUE(leg1.hasRatioTapChanger());
    EXPECT_FALSE(leg1.hasPhaseTapChanger());
    ASSERT_TRUE(leg1.getRatioTapChanger().has_value());
    EXPECT_EQ(leg1.getRatioTapChanger()->getTapPosition(), 2);
    EXPECT_FALSE(twt.getLeg2().getRatioTapChanger().has_value());
}

TEST_F(ThreeWindingsTransformerTest, LegLimits) {
    ThreeWindingsTransformer twt(TWT, &backend);
    ThreeWindingsTransformer::Leg leg1 = twt.getLeg1();
    std::vector<OperationalLimitsGroup> groups = leg1.getOperationalLimitsGroups();
    ASSERT_EQ(groups.size(), 1u);
    EXPECT_EQ(groups[0].getId(), "DEFAULT");
    ASSERT_TRUE(leg1.getSelectedOperationalLimitsGroup().has_value());
    EXPECT_DOUBLE_EQ(leg1.getSelectedOperationalLimitsGroup()->getCurrentLimits()->getPermanentLimit(), 600.0);
    ASSERT_TRUE(leg1.getCurrentLimits().has_value());
    EXPECT_DOUBLE_EQ(leg1.getCurrentLimits()->getPermanentLimit(), 600.0);

    ThreeWindingsTransformer::Leg leg2 = twt.getLeg2();
    EXPECT_TRUE(leg2.getOperationalLimitsGroups().empty());
    EXPECT_FALSE(leg2.getSelectedOperationalLimitsGroup().has_value());
    EXPECT_FALSE(leg2.getCurrentLimits().has_value());
}

TEST_F(ThreeWindingsTransformerTest, Invalid) {
    ThreeWindingsTransformer none;
    EXPECT_FALSE(none.isValid());
    EXPECT_FALSE(ThreeWindingsTransformer::Leg().isValid());
    EXPECT_THROW(none.getLeg1(), InvalidHandleException);
}
