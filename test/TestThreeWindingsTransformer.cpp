#include <gtest/gtest.h>
#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/Substation.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class ThreeWindingsTransformerTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle TWT_H = 10;
    static constexpr ObjectHandle T1_H  = 20;
    static constexpr ObjectHandle T2_H  = 21;
    static constexpr ObjectHandle T3_H  = 22;
    static constexpr ObjectHandle SUB_H = 30;

    void SetUp() override {
        backend.strings[{TWT_H, prop::ID}]            = "3WT1";
        backend.strings[{TWT_H, prop::NAME}]           = "Three Windings T1";
        backend.doubles[{TWT_H, prop::THREE_WT_RATED_U0}] = 110.0;
        backend.related[{TWT_H, prop::REL_TERMINAL1}] = T1_H;
        backend.related[{TWT_H, prop::REL_TERMINAL2}] = T2_H;
        backend.related[{TWT_H, prop::REL_TERMINAL}]  = T3_H;
        backend.related[{TWT_H, prop::REL_SUBSTATION}] = SUB_H;
        backend.strings[{SUB_H, prop::ID}]             = "SUB1";
        backend.bools[{T1_H, prop::TERMINAL_CONNECTED}] = true;
        backend.bools[{T2_H, prop::TERMINAL_CONNECTED}] = true;
        backend.bools[{T3_H, prop::TERMINAL_CONNECTED}] = true;
        // Leg 1 minimal electrical params
        backend.doubles[{TWT_H, prop::THREE_WT_LEG1_BASE + prop::THREE_WT_LEG_R_OFF}] = 1.0;
        backend.doubles[{TWT_H, prop::THREE_WT_LEG1_BASE + prop::THREE_WT_LEG_X_OFF}] = 5.0;
        backend.doubles[{TWT_H, prop::THREE_WT_LEG1_BASE + prop::THREE_WT_LEG_RATED_U_OFF}] = 400.0;
    }
};

TEST_F(ThreeWindingsTransformerTest, GetId) {
    ThreeWindingsTransformer twt(TWT_H, &backend);
    EXPECT_EQ(twt.getId(), "3WT1");
}

TEST_F(ThreeWindingsTransformerTest, GetRatedU0) {
    ThreeWindingsTransformer twt(TWT_H, &backend);
    EXPECT_DOUBLE_EQ(twt.getRatedU0(), 110.0);
}

TEST_F(ThreeWindingsTransformerTest, GetSubstation) {
    ThreeWindingsTransformer twt(TWT_H, &backend);
    Substation sub = twt.getSubstation();
    EXPECT_TRUE(sub.isValid());
    EXPECT_EQ(sub.getId(), "SUB1");
}

TEST_F(ThreeWindingsTransformerTest, GetTerminals) {
    ThreeWindingsTransformer twt(TWT_H, &backend);
    EXPECT_TRUE(twt.getLeg1().getTerminal().isValid());
    EXPECT_TRUE(twt.getLeg2().getTerminal().isValid());
    EXPECT_TRUE(twt.getLeg3().getTerminal().isValid());
}

TEST_F(ThreeWindingsTransformerTest, Leg1ElectricalParams) {
    ThreeWindingsTransformer twt(TWT_H, &backend);
    EXPECT_DOUBLE_EQ(twt.getLeg1().getR(), 1.0);
    EXPECT_DOUBLE_EQ(twt.getLeg1().getX(), 5.0);
    EXPECT_DOUBLE_EQ(twt.getLeg1().getRatedU(), 400.0);
}

TEST_F(ThreeWindingsTransformerTest, GetName) {
    ThreeWindingsTransformer twt(TWT_H, &backend);
    EXPECT_EQ(twt.getName(), "Three Windings T1");
}

TEST_F(ThreeWindingsTransformerTest, ConnectDisconnect) {
    ThreeWindingsTransformer twt(TWT_H, &backend);
    EXPECT_TRUE(twt.isConnected());
    twt.disconnect();
    EXPECT_FALSE((backend.bools[{T1_H, prop::TERMINAL_CONNECTED}]));
    EXPECT_FALSE((backend.bools[{T2_H, prop::TERMINAL_CONNECTED}]));
    EXPECT_FALSE((backend.bools[{T3_H, prop::TERMINAL_CONNECTED}]));
    twt.connect();
    EXPECT_TRUE((backend.bools[{T1_H, prop::TERMINAL_CONNECTED}]));
    EXPECT_TRUE((backend.bools[{T2_H, prop::TERMINAL_CONNECTED}]));
    EXPECT_TRUE((backend.bools[{T3_H, prop::TERMINAL_CONNECTED}]));
}

TEST_F(ThreeWindingsTransformerTest, IsValid) {
    ThreeWindingsTransformer valid(TWT_H, &backend);
    ThreeWindingsTransformer invalid;
    EXPECT_TRUE(valid.isValid());
    EXPECT_FALSE(invalid.isValid());
}
