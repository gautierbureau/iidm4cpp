#include <gtest/gtest.h>
#include <iidm/ShuntCompensator.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class ShuntCompensatorTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle SC_H    = 10;
    static constexpr ObjectHandle TERM_H  = 20;
    static constexpr ObjectHandle RT_H    = 21;
    static constexpr ObjectHandle MODEL_H = 30;

    void SetUp() override {
        backend.strings[{SC_H, prop::ID}]                     = "SC1";
        backend.strings[{SC_H, prop::NAME}]                   = "Shunt 1";
        backend.ints   [{SC_H, prop::SHUNT_SECTION_COUNT}]    = 2;
        backend.optInts[{SC_H, prop::SHUNT_SECTION_COUNT}]    = 2;
        backend.ints   [{SC_H, prop::SHUNT_MAX_SECTION_COUNT}] = 4;
        backend.ints   [{SC_H, prop::SHUNT_MODEL_TYPE}]       = static_cast<int>(ShuntCompensatorModelType::LINEAR);
        backend.doubles[{SC_H, prop::SHUNT_B}]                = 0.02;
        backend.doubles[{SC_H, prop::SHUNT_G}]                = 0.0;
        backend.related[{SC_H, prop::REL_TERMINAL}]           = TERM_H;
        backend.related[{SC_H, prop::REL_REGULATING_TERMINAL}] = RT_H;
        backend.related[{SC_H, prop::SHUNT_COMPENSATOR_MODEL}] = MODEL_H;
        backend.related[{SC_H, prop::SHUNT_COMPENSATOR_LINEAR_MODEL}] = MODEL_H;
        backend.bools  [{MODEL_H, prop::SHUNT_COMPENSATOR_LINEAR_MODEL}] = true;
        backend.doubles[{MODEL_H, prop::SHUNT_B_PER_SECTION}] = 0.01;
        backend.doubles[{MODEL_H, prop::SHUNT_G_PER_SECTION}] = 0.0;
    }
};

TEST_F(ShuntCompensatorTest, SectionCount) {
    ShuntCompensator sc(SC_H, &backend);
    EXPECT_EQ(sc.getSectionCount(), 2);
    EXPECT_EQ(sc.findSectionCount(), iidm::optional<int>(2));
    EXPECT_EQ(sc.getMaximumSectionCount(), 4);
    EXPECT_EQ(&sc.setSectionCount(3), &sc);
    EXPECT_EQ((backend.ints[{SC_H, prop::SHUNT_SECTION_COUNT}]), 3);
    EXPECT_EQ(&sc.unsetSectionCount(), &sc);
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(SC_H), prop::ACT_UNSET_SECTION_COUNT));
    backend.optInts.erase({SC_H, prop::SHUNT_SECTION_COUNT});
    EXPECT_FALSE(sc.findSectionCount());
}

TEST_F(ShuntCompensatorTest, SolvedSectionCount) {
    ShuntCompensator sc(SC_H, &backend);
    EXPECT_FALSE(sc.getSolvedSectionCount());
    EXPECT_FALSE(sc.findSolvedSectionCount());
    backend.optInts[{SC_H, prop::SHUNT_SOLVED_SECTION_COUNT}] = 3;
    EXPECT_EQ(sc.getSolvedSectionCount(), iidm::optional<int>(3));
    EXPECT_EQ(sc.findSolvedSectionCount(), iidm::optional<int>(3));
    EXPECT_EQ(&sc.setSolvedSectionCount(1), &sc);
    EXPECT_EQ((backend.ints[{SC_H, prop::SHUNT_SOLVED_SECTION_COUNT}]), 1);
    EXPECT_EQ(&sc.unsetSolvedSectionCount(), &sc);
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(SC_H), prop::ACT_UNSET_SOLVED_SECTION_COUNT));
}

TEST_F(ShuntCompensatorTest, SolvedValues) {
    ShuntCompensator sc(SC_H, &backend);
    sc.applySolvedValues();
    sc.setSectionCountToSolvedSectionCount();
    sc.unsetSolvedValues();
    ASSERT_EQ(backend.invocations.size(), 3u);
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(SC_H), prop::ACT_APPLY_SOLVED_VALUES));
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(SC_H), prop::ACT_SET_SECTION_COUNT_TO_SOLVED_SECTION_COUNT));
    EXPECT_EQ(backend.invocations[2], std::make_pair(ObjectHandle(SC_H), prop::ACT_UNSET_SOLVED_VALUES));
}

// At the section count, and at any other
TEST_F(ShuntCompensatorTest, SusceptanceAndConductance) {
    backend.evaluations[std::make_tuple(ObjectHandle(SC_H), prop::SHUNT_B, std::vector<double>{3.0})] = 0.03;
    backend.evaluations[std::make_tuple(ObjectHandle(SC_H), prop::SHUNT_G, std::vector<double>{0.0})] = 0.0;
    ShuntCompensator sc(SC_H, &backend);
    EXPECT_DOUBLE_EQ(sc.getB(), 0.02);
    EXPECT_DOUBLE_EQ(sc.getG(), 0.0);
    EXPECT_DOUBLE_EQ(sc.getB(3), 0.03);
    EXPECT_DOUBLE_EQ(sc.getG(0), 0.0);
}

TEST_F(ShuntCompensatorTest, LinearModel) {
    ShuntCompensator sc(SC_H, &backend);
    EXPECT_EQ(sc.getModelType(), ShuntCompensatorModelType::LINEAR);
    ShuntCompensatorModel model = sc.getModel();
    EXPECT_TRUE(model.is<ShuntCompensatorLinearModel>());
    EXPECT_FALSE(model.is<ShuntCompensatorNonLinearModel>());
    ShuntCompensatorLinearModel linear = sc.getModel<ShuntCompensatorLinearModel>();
    EXPECT_EQ(linear, model.as<ShuntCompensatorLinearModel>());
    EXPECT_DOUBLE_EQ(linear.getBPerSection(), 0.01);
    EXPECT_DOUBLE_EQ(linear.getGPerSection(), 0.0);
    EXPECT_EQ(&linear.setBPerSection(0.02).setGPerSection(0.001).setMaximumSectionCount(5), &linear);
    EXPECT_DOUBLE_EQ((backend.doubles[{MODEL_H, prop::SHUNT_B_PER_SECTION}]), 0.02);
    EXPECT_DOUBLE_EQ((backend.doubles[{MODEL_H, prop::SHUNT_G_PER_SECTION}]), 0.001);
    EXPECT_EQ((backend.ints[{MODEL_H, prop::SHUNT_MAX_SECTION_COUNT}]), 5);
    model.setProperty("maker", "abb");
    EXPECT_EQ(linear.getProperty("maker"), iidm::optional<std::string>("abb"));
}

TEST_F(ShuntCompensatorTest, RegulatingTerminal) {
    ShuntCompensator sc(SC_H, &backend);
    EXPECT_EQ(sc.getRegulatingTerminal(), Terminal(RT_H, &backend));
}

TEST_F(ShuntCompensatorTest, GetTerminal) {
    backend.doubles[{TERM_H, prop::TERMINAL_P}] = 5.0;
    ShuntCompensator sc(SC_H, &backend);
    EXPECT_TRUE(sc.getTerminal().isValid());
}

