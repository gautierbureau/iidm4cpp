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
    static constexpr ObjectHandle GEN_H      = 40;
    static constexpr ObjectHandle LINE_H     = 41;
    static constexpr ObjectHandle INJ_OBS_H  = 42; // the extension objects
    static constexpr ObjectHandle BR_OBS_H   = 43;
    static constexpr ObjectHandle QUALITY_P  = 44; // their qualities
    static constexpr ObjectHandle QUALITY_Q  = 45;
    static constexpr ObjectHandle QUALITY_Q2 = 46;

    void SetUp() override {
        backend.related[{GEN_H, prop::INJECTION_OBSERVABILITY}] = INJ_OBS_H;
        backend.bools  [{INJ_OBS_H, prop::EXT_OBS_OBSERVABLE}]  = true;
        backend.related[{INJ_OBS_H, prop::OBS_QUALITY_P}]       = QUALITY_P;
        backend.doubles[{QUALITY_P, prop::OBS_QUALITY_STANDARD_DEVIATION}] = 1.5;
        backend.ints   [{QUALITY_P, prop::OBS_QUALITY_REDUNDANT}]          = 1;
        backend.related[{INJ_OBS_H, prop::OBS_QUALITY_Q}]       = QUALITY_Q;
        backend.doubles[{QUALITY_Q, prop::OBS_QUALITY_STANDARD_DEVIATION}] = 2.0;
        backend.ints   [{QUALITY_Q, prop::OBS_QUALITY_REDUNDANT}]          = -1;

        backend.related[{LINE_H, prop::BRANCH_OBSERVABILITY}] = BR_OBS_H;
        backend.bools  [{BR_OBS_H, prop::EXT_OBS_OBSERVABLE}] = false;
        backend.related[{BR_OBS_H, prop::OBS_QUALITY_Q2}]     = QUALITY_Q2;
        backend.doubles[{QUALITY_Q2, prop::OBS_QUALITY_STANDARD_DEVIATION}] = 0.2;
        backend.ints   [{QUALITY_Q2, prop::OBS_QUALITY_REDUNDANT}]          = 0;
    }
};

TEST_F(ObservabilityTest, InjectionHosts) {
    EXPECT_TRUE(Generator(GEN_H, &backend).hasInjectionObservability());
    EXPECT_FALSE(Load(GEN_H + 100, &backend).hasInjectionObservability());
    EXPECT_THROW(Load(GEN_H + 100, &backend).getInjectionObservability(), ExtensionNotFoundException);
    backend.related[{GEN_H + 101, prop::INJECTION_OBSERVABILITY}] = INJ_OBS_H;
    EXPECT_TRUE(BusbarSection(GEN_H + 101, &backend).hasInjectionObservability());
}

TEST_F(ObservabilityTest, InjectionObservable) {
    auto obs = Generator(GEN_H, &backend).getInjectionObservability();
    EXPECT_EQ(obs, InjectionObservability(INJ_OBS_H, &backend));
    EXPECT_TRUE(obs.isObservable());
    obs.setObservable(false);
    EXPECT_FALSE((backend.bools[{INJ_OBS_H, prop::EXT_OBS_OBSERVABLE}]));
}

TEST_F(ObservabilityTest, InjectionQualities) {
    auto obs = Generator(GEN_H, &backend).getInjectionObservability();
    auto p = obs.getQualityP();
    ASSERT_TRUE(p.has_value());
    EXPECT_EQ(*p, ObservabilityQuality(QUALITY_P, &backend));
    EXPECT_DOUBLE_EQ(p->getStandardDeviation(), 1.5);
    ASSERT_TRUE(p->isRedundant().has_value());
    EXPECT_TRUE(*p->isRedundant());

    auto q = obs.getQualityQ();
    ASSERT_TRUE(q.has_value());
    EXPECT_DOUBLE_EQ(q->getStandardDeviation(), 2.0);
    EXPECT_FALSE(q->isRedundant().has_value());

    EXPECT_FALSE(obs.getQualityV().has_value());
}

TEST_F(ObservabilityTest, QualitySetters) {
    ObservabilityQuality p = *Generator(GEN_H, &backend).getInjectionObservability().getQualityP();
    EXPECT_EQ(&p.setStandardDeviation(2.5), &p);
    EXPECT_DOUBLE_EQ((backend.doubles[{QUALITY_P, prop::OBS_QUALITY_STANDARD_DEVIATION}]), 2.5);
    p.setRedundant(false);
    EXPECT_EQ((backend.ints[{QUALITY_P, prop::OBS_QUALITY_REDUNDANT}]), 0);
    p.setRedundant(iidm::optional<bool>());
    EXPECT_EQ((backend.ints[{QUALITY_P, prop::OBS_QUALITY_REDUNDANT}]), -1);
}

TEST_F(ObservabilityTest, InjectionSetQualityKeepsRedundancy) {
    auto obs = Generator(GEN_H, &backend).getInjectionObservability();
    const auto redundancies = backend.ints;
    obs.setQualityV(3.0).setQualityP(3.5);
    // The standard deviation is set on the extension, which creates the quality if needed
    EXPECT_DOUBLE_EQ((backend.doubles[{INJ_OBS_H, prop::OBS_QUALITY_V}]), 3.0);
    EXPECT_DOUBLE_EQ((backend.doubles[{INJ_OBS_H, prop::OBS_QUALITY_P}]), 3.5);
    EXPECT_EQ(backend.ints, redundancies);
}

TEST_F(ObservabilityTest, InjectionSetQualityWithRedundancy) {
    auto obs = Generator(GEN_H, &backend).getInjectionObservability();
    obs.setQualityP(4.0, false).setQualityQ(5.0, true);
    EXPECT_DOUBLE_EQ((backend.doubles[{INJ_OBS_H, prop::OBS_QUALITY_P}]), 4.0);
    EXPECT_EQ((backend.ints[{QUALITY_P, prop::OBS_QUALITY_REDUNDANT}]), 0);
    EXPECT_DOUBLE_EQ((backend.doubles[{INJ_OBS_H, prop::OBS_QUALITY_Q}]), 5.0);
    EXPECT_EQ((backend.ints[{QUALITY_Q, prop::OBS_QUALITY_REDUNDANT}]), 1);
    obs.setQualityP(4.5, iidm::optional<bool>());
    EXPECT_EQ((backend.ints[{QUALITY_P, prop::OBS_QUALITY_REDUNDANT}]), -1);
}

TEST_F(ObservabilityTest, InjectionNewAndRemove) {
    MockBackend b;
    Load load(GEN_H, &b);
    InjectionObservabilityAdder adder = load.newInjectionObservability();
    EXPECT_EQ(b.invocations.back(), std::make_pair(static_cast<ObjectHandle>(GEN_H), prop::INJECTION_OBSERVABILITY));
    const ObjectHandle adderHandle = b.lastAdder;
    b.addResults[adderHandle] = INJ_OBS_H;
    auto obs = adder.withObservable(true)
                    .withStandardDeviationP(0.1).withRedundantP(true)
                    .withStandardDeviationQ(0.2)
                    .withStandardDeviationV(0.3).withRedundantV(false)
                    .add();
    EXPECT_TRUE((b.bools[{adderHandle, prop::EXT_OBS_OBSERVABLE}]));
    EXPECT_DOUBLE_EQ((b.doubles[{adderHandle, prop::OBS_QUALITY_P}]), 0.1);
    EXPECT_EQ((b.ints[{adderHandle, prop::OBS_QUALITY_P}]), 1);
    EXPECT_DOUBLE_EQ((b.doubles[{adderHandle, prop::OBS_QUALITY_Q}]), 0.2);
    EXPECT_FALSE(b.ints.count({adderHandle, prop::OBS_QUALITY_Q}));
    EXPECT_DOUBLE_EQ((b.doubles[{adderHandle, prop::OBS_QUALITY_V}]), 0.3);
    EXPECT_EQ((b.ints[{adderHandle, prop::OBS_QUALITY_V}]), 0);
    EXPECT_EQ(obs, InjectionObservability(INJ_OBS_H, &b));

    load.removeInjectionObservability();
    EXPECT_EQ((b.related[{GEN_H, prop::INJECTION_OBSERVABILITY}]), INVALID_HANDLE);
}

TEST_F(ObservabilityTest, BranchHosts) {
    EXPECT_TRUE(Line(LINE_H, &backend).hasBranchObservability());
    EXPECT_FALSE(TwoWindingsTransformer(LINE_H + 100, &backend).hasBranchObservability());
    EXPECT_THROW(TwoWindingsTransformer(LINE_H + 100, &backend).getBranchObservability(), ExtensionNotFoundException);
}

TEST_F(ObservabilityTest, BranchQualities) {
    auto obs = Line(LINE_H, &backend).getBranchObservability();
    EXPECT_FALSE(obs.isObservable());
    EXPECT_FALSE(obs.getQualityP1().has_value());
    EXPECT_FALSE(obs.getQualityP2().has_value());
    EXPECT_FALSE(obs.getQualityQ1().has_value());
    auto q2 = obs.getQualityQ2();
    ASSERT_TRUE(q2.has_value());
    EXPECT_DOUBLE_EQ(q2->getStandardDeviation(), 0.2);
    ASSERT_TRUE(q2->isRedundant().has_value());
    EXPECT_FALSE(*q2->isRedundant());
}

TEST_F(ObservabilityTest, BranchSetters) {
    auto obs = Line(LINE_H, &backend).getBranchObservability();
    obs.setObservable(true).setQualityP1(0.1).setQualityQ2(0.3, true).setQualityQ1(0.4);
    EXPECT_TRUE((backend.bools[{BR_OBS_H, prop::EXT_OBS_OBSERVABLE}]));
    EXPECT_DOUBLE_EQ((backend.doubles[{BR_OBS_H, prop::OBS_QUALITY_P1}]), 0.1);
    EXPECT_DOUBLE_EQ((backend.doubles[{BR_OBS_H, prop::OBS_QUALITY_Q2}]), 0.3);
    EXPECT_EQ((backend.ints[{QUALITY_Q2, prop::OBS_QUALITY_REDUNDANT}]), 1);
    EXPECT_DOUBLE_EQ((backend.doubles[{BR_OBS_H, prop::OBS_QUALITY_Q1}]), 0.4);
}

TEST_F(ObservabilityTest, BranchNewAndRemove) {
    MockBackend b;
    TwoWindingsTransformer twt(LINE_H, &b);
    BranchObservabilityAdder adder = twt.newBranchObservability();
    EXPECT_EQ(b.invocations.back(), std::make_pair(static_cast<ObjectHandle>(LINE_H), prop::BRANCH_OBSERVABILITY));
    const ObjectHandle adderHandle = b.lastAdder;
    adder.withObservable(false)
         .withStandardDeviationP1(1.0).withRedundantP1(false)
         .withStandardDeviationP2(2.0)
         .withStandardDeviationQ1(3.0).withRedundantQ1(true)
         .withStandardDeviationQ2(4.0).withRedundantQ2(false);
    EXPECT_FALSE((b.bools[{adderHandle, prop::EXT_OBS_OBSERVABLE}]));
    EXPECT_DOUBLE_EQ((b.doubles[{adderHandle, prop::OBS_QUALITY_P1}]), 1.0);
    EXPECT_EQ((b.ints[{adderHandle, prop::OBS_QUALITY_P1}]), 0);
    EXPECT_DOUBLE_EQ((b.doubles[{adderHandle, prop::OBS_QUALITY_P2}]), 2.0);
    EXPECT_EQ((b.ints[{adderHandle, prop::OBS_QUALITY_Q1}]), 1);
    EXPECT_DOUBLE_EQ((b.doubles[{adderHandle, prop::OBS_QUALITY_Q2}]), 4.0);

    twt.removeBranchObservability();
    EXPECT_EQ((b.related[{LINE_H, prop::BRANCH_OBSERVABILITY}]), INVALID_HANDLE);
}
