#include <gtest/gtest.h>
#include <iidm/BusbarSection.h>
#include <iidm/Generator.h>
#include <iidm/Line.h>
#include <iidm/Load.h>
#include <iidm/TwoWindingsTransformer.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class ObservabilityTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle GEN_H  = 40;
    static constexpr ObjectHandle LINE_H = 41;

    static int code(int base, int offset) { return base + offset; }

    void SetUp() override {
        backend.bools  [{GEN_H, prop::EXT_INJ_OBS_EXISTS}]     = true;
        backend.bools  [{GEN_H, prop::EXT_INJ_OBS_OBSERVABLE}] = true;
        backend.bools  [{GEN_H, code(prop::EXT_INJ_OBS_QUALITY_P, prop::OBS_QUALITY_EXISTS_OFF)}]    = true;
        backend.doubles[{GEN_H, code(prop::EXT_INJ_OBS_QUALITY_P, prop::OBS_QUALITY_STD_DEV_OFF)}]   = 1.5;
        backend.ints   [{GEN_H, code(prop::EXT_INJ_OBS_QUALITY_P, prop::OBS_QUALITY_REDUNDANT_OFF)}] = 1;
        backend.bools  [{GEN_H, code(prop::EXT_INJ_OBS_QUALITY_Q, prop::OBS_QUALITY_EXISTS_OFF)}]    = true;
        backend.doubles[{GEN_H, code(prop::EXT_INJ_OBS_QUALITY_Q, prop::OBS_QUALITY_STD_DEV_OFF)}]   = 2.0;
        backend.ints   [{GEN_H, code(prop::EXT_INJ_OBS_QUALITY_Q, prop::OBS_QUALITY_REDUNDANT_OFF)}] = -1;

        backend.bools  [{LINE_H, prop::EXT_BRANCH_OBS_EXISTS}]     = true;
        backend.bools  [{LINE_H, prop::EXT_BRANCH_OBS_OBSERVABLE}] = false;
        backend.bools  [{LINE_H, code(prop::EXT_BRANCH_OBS_QUALITY_Q2, prop::OBS_QUALITY_EXISTS_OFF)}]    = true;
        backend.doubles[{LINE_H, code(prop::EXT_BRANCH_OBS_QUALITY_Q2, prop::OBS_QUALITY_STD_DEV_OFF)}]   = 0.2;
        backend.ints   [{LINE_H, code(prop::EXT_BRANCH_OBS_QUALITY_Q2, prop::OBS_QUALITY_REDUNDANT_OFF)}] = 0;
    }
};

TEST_F(ObservabilityTest, InjectionHosts) {
    EXPECT_TRUE(Generator(GEN_H, &backend).hasInjectionObservability());
    EXPECT_FALSE(Load(GEN_H + 100, &backend).hasInjectionObservability());
    backend.bools[{GEN_H + 101, prop::EXT_INJ_OBS_EXISTS}] = true;
    EXPECT_TRUE(BusbarSection(GEN_H + 101, &backend).hasInjectionObservability());
}

TEST_F(ObservabilityTest, InjectionObservable) {
    auto obs = Generator(GEN_H, &backend).getInjectionObservability();
    EXPECT_TRUE(obs.isObservable());
    obs.setObservable(false);
    EXPECT_FALSE((backend.bools[{GEN_H, prop::EXT_INJ_OBS_OBSERVABLE}]));
}

TEST_F(ObservabilityTest, InjectionQualities) {
    auto obs = Generator(GEN_H, &backend).getInjectionObservability();
    auto p = obs.getQualityP();
    ASSERT_TRUE(p.has_value());
    EXPECT_DOUBLE_EQ(p->standardDeviation, 1.5);
    ASSERT_TRUE(p->redundant.has_value());
    EXPECT_TRUE(*p->redundant);

    auto q = obs.getQualityQ();
    ASSERT_TRUE(q.has_value());
    EXPECT_DOUBLE_EQ(q->standardDeviation, 2.0);
    EXPECT_FALSE(q->redundant.has_value());

    EXPECT_FALSE(obs.getQualityV().has_value());
}

TEST_F(ObservabilityTest, InjectionSetQualityKeepsRedundancy) {
    auto obs = Generator(GEN_H, &backend).getInjectionObservability();
    backend.ints[{GEN_H, code(prop::EXT_INJ_OBS_QUALITY_V, prop::OBS_QUALITY_REDUNDANT_OFF)}] = 7; // sentinel
    obs.setQualityV(3.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_H, code(prop::EXT_INJ_OBS_QUALITY_V, prop::OBS_QUALITY_STD_DEV_OFF)}]), 3.0);
    EXPECT_EQ((backend.ints[{GEN_H, code(prop::EXT_INJ_OBS_QUALITY_V, prop::OBS_QUALITY_REDUNDANT_OFF)}]), 7);
}

TEST_F(ObservabilityTest, InjectionSetQualityWithRedundancy) {
    auto obs = Generator(GEN_H, &backend).getInjectionObservability();
    obs.setQualityP(4.0, false).setQualityQ(5.0, true).setQualityV(6.0, iidm::optional<bool>());
    EXPECT_DOUBLE_EQ((backend.doubles[{GEN_H, code(prop::EXT_INJ_OBS_QUALITY_P, prop::OBS_QUALITY_STD_DEV_OFF)}]), 4.0);
    EXPECT_EQ((backend.ints[{GEN_H, code(prop::EXT_INJ_OBS_QUALITY_P, prop::OBS_QUALITY_REDUNDANT_OFF)}]), 0);
    EXPECT_EQ((backend.ints[{GEN_H, code(prop::EXT_INJ_OBS_QUALITY_Q, prop::OBS_QUALITY_REDUNDANT_OFF)}]), 1);
    EXPECT_EQ((backend.ints[{GEN_H, code(prop::EXT_INJ_OBS_QUALITY_V, prop::OBS_QUALITY_REDUNDANT_OFF)}]), -1);
}

TEST_F(ObservabilityTest, BranchHosts) {
    EXPECT_TRUE(Line(LINE_H, &backend).hasBranchObservability());
    EXPECT_FALSE(TwoWindingsTransformer(LINE_H + 100, &backend).hasBranchObservability());
}

TEST_F(ObservabilityTest, BranchQualities) {
    auto obs = Line(LINE_H, &backend).getBranchObservability();
    EXPECT_FALSE(obs.isObservable());
    EXPECT_FALSE(obs.getQualityP1().has_value());
    EXPECT_FALSE(obs.getQualityP2().has_value());
    EXPECT_FALSE(obs.getQualityQ1().has_value());
    auto q2 = obs.getQualityQ2();
    ASSERT_TRUE(q2.has_value());
    EXPECT_DOUBLE_EQ(q2->standardDeviation, 0.2);
    ASSERT_TRUE(q2->redundant.has_value());
    EXPECT_FALSE(*q2->redundant);
}

TEST_F(ObservabilityTest, BranchSetters) {
    auto obs = Line(LINE_H, &backend).getBranchObservability();
    obs.setObservable(true).setQualityP1(0.1).setQualityP2(0.3, true).setQualityQ1(0.4);
    EXPECT_TRUE((backend.bools[{LINE_H, prop::EXT_BRANCH_OBS_OBSERVABLE}]));
    EXPECT_DOUBLE_EQ((backend.doubles[{LINE_H, code(prop::EXT_BRANCH_OBS_QUALITY_P1, prop::OBS_QUALITY_STD_DEV_OFF)}]), 0.1);
    EXPECT_DOUBLE_EQ((backend.doubles[{LINE_H, code(prop::EXT_BRANCH_OBS_QUALITY_P2, prop::OBS_QUALITY_STD_DEV_OFF)}]), 0.3);
    EXPECT_EQ((backend.ints[{LINE_H, code(prop::EXT_BRANCH_OBS_QUALITY_P2, prop::OBS_QUALITY_REDUNDANT_OFF)}]), 1);
    EXPECT_DOUBLE_EQ((backend.doubles[{LINE_H, code(prop::EXT_BRANCH_OBS_QUALITY_Q1, prop::OBS_QUALITY_STD_DEV_OFF)}]), 0.4);
}
