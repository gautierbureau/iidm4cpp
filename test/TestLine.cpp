#include <gtest/gtest.h>
#include <iidm/Line.h>
#include <iidm/LoadingLimits.h>
#include <iidm/OperationalLimitsGroup.h>
#include <iidm/PropertyCodes.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

class LineTest : public ::testing::Test {
protected:
    MockBackend backend;
    static constexpr ObjectHandle LINE_HANDLE  = 50;
    static constexpr ObjectHandle TERM1_HANDLE = 51;
    static constexpr ObjectHandle TERM2_HANDLE = 52;

    void SetUp() override {
        backend.strings[{LINE_HANDLE, prop::ID}]   = "LINE1";
        backend.strings[{LINE_HANDLE, prop::NAME}]  = "Line 1";
        backend.doubles[{LINE_HANDLE, prop::LINE_R}]  = 0.01;
        backend.doubles[{LINE_HANDLE, prop::LINE_X}]  = 0.1;
        backend.doubles[{LINE_HANDLE, prop::LINE_G1}] = 0.0;
        backend.doubles[{LINE_HANDLE, prop::LINE_B1}] = 1e-4;
        backend.doubles[{LINE_HANDLE, prop::LINE_G2}] = 0.0;
        backend.doubles[{LINE_HANDLE, prop::LINE_B2}] = 1e-4;
        backend.related[{LINE_HANDLE, prop::REL_TERMINAL1}] = TERM1_HANDLE;
        backend.related[{LINE_HANDLE, prop::REL_TERMINAL2}] = TERM2_HANDLE;
    }
};

TEST_F(LineTest, GetId) {
    Line line(LINE_HANDLE, &backend);
    EXPECT_EQ(line.getId(), "LINE1");
}

TEST_F(LineTest, GetRX) {
    Line line(LINE_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(line.getR(), 0.01);
    EXPECT_DOUBLE_EQ(line.getX(), 0.1);
}

TEST_F(LineTest, GetG1B1G2B2) {
    Line line(LINE_HANDLE, &backend);
    EXPECT_DOUBLE_EQ(line.getG1(), 0.0);
    EXPECT_DOUBLE_EQ(line.getB1(), 1e-4);
    EXPECT_DOUBLE_EQ(line.getG2(), 0.0);
    EXPECT_DOUBLE_EQ(line.getB2(), 1e-4);
}

TEST_F(LineTest, GetTerminals) {
    Line line(LINE_HANDLE, &backend);
    Terminal t1 = line.getTerminal1();
    Terminal t2 = line.getTerminal2();
    EXPECT_TRUE(t1.isValid());
    EXPECT_TRUE(t2.isValid());
    EXPECT_NE(t1, t2);
}

// ── Limits: through the holder of each side ───────────────────────────────────

static constexpr ObjectHandle SIDE1_HANDLE = 55;
static constexpr ObjectHandle SIDE2_HANDLE = 56;
static constexpr ObjectHandle CL1_HANDLE = 60;
static constexpr ObjectHandle CL2_HANDLE = 61;
static constexpr ObjectHandle TL1_HANDLE = 70;
static constexpr ObjectHandle OLG1_HANDLE = 80;
static constexpr ObjectHandle OLG2_HANDLE = 81;

class LineLimitsTest : public LineTest {
protected:
    void SetUp() override {
        LineTest::SetUp();
        backend.relatedByIndex[std::make_tuple(ObjectHandle(LINE_HANDLE), prop::FLOWS_LIMITS_HOLDER, 0)] = SIDE1_HANDLE;
        backend.relatedByIndex[std::make_tuple(ObjectHandle(LINE_HANDLE), prop::FLOWS_LIMITS_HOLDER, 1)] = SIDE2_HANDLE;
    }
};

TEST_F(LineLimitsTest, GetCurrentLimitsAbsent) {
    Line line(LINE_HANDLE, &backend);
    EXPECT_FALSE(line.getCurrentLimits1().has_value());
    EXPECT_FALSE(line.getCurrentLimits2().has_value());
}

TEST_F(LineLimitsTest, GetCurrentLimitsPerSide) {
    backend.related[{SIDE1_HANDLE, prop::CURRENT_LIMITS}] = CL1_HANDLE;
    backend.doubles[{CL1_HANDLE, prop::LL_PERMANENT_LIMIT}] = 1000.0;
    backend.related[{SIDE2_HANDLE, prop::CURRENT_LIMITS}] = CL2_HANDLE;
    backend.doubles[{CL2_HANDLE, prop::LL_PERMANENT_LIMIT}] = 800.0;
    Line line(LINE_HANDLE, &backend);
    ASSERT_TRUE(line.getCurrentLimits1().has_value());
    EXPECT_DOUBLE_EQ(line.getCurrentLimits1()->getPermanentLimit(), 1000.0);
    EXPECT_DOUBLE_EQ(line.getCurrentLimits(TwoSides::TWO)->getPermanentLimit(), 800.0);
    EXPECT_FALSE(line.getActivePowerLimits1().has_value());
}

TEST_F(LineLimitsTest, TemporaryLimits) {
    backend.related[{SIDE1_HANDLE, prop::CURRENT_LIMITS}] = CL1_HANDLE;
    backend.children[{CL1_HANDLE, prop::TEMPORARY_LIMIT}] = {TL1_HANDLE};
    backend.strings[{TL1_HANDLE, prop::TL_NAME}]                = "10min";
    backend.doubles[{TL1_HANDLE, prop::TL_VALUE}]               = 1200.0;
    backend.ints   [{TL1_HANDLE, prop::TL_ACCEPTABLE_DURATION}] = 600;
    backend.bools  [{TL1_HANDLE, prop::TL_FICTITIOUS}]          = false;

    Line line(LINE_HANDLE, &backend);
    std::vector<TemporaryLimit> tls = line.getCurrentLimits1()->getTemporaryLimits();
    ASSERT_EQ(tls.size(), 1u);
    EXPECT_EQ(tls[0].getName(), "10min");
    EXPECT_DOUBLE_EQ(tls[0].getValue(), 1200.0);
    EXPECT_EQ(tls[0].getAcceptableDuration(), 600);
    EXPECT_FALSE(tls[0].isFictitious());
}

TEST_F(LineLimitsTest, Groups) {
    backend.children[{SIDE1_HANDLE, prop::OPERATIONAL_LIMITS_GROUP}] = {OLG1_HANDLE};
    backend.strings[{OLG1_HANDLE, prop::OLG_ID}]   = "DEFAULT";
    backend.bools  [{OLG1_HANDLE, prop::OLG_EMPTY}] = false;
    backend.related[{SIDE1_HANDLE, prop::REL_SELECTED_OLG}] = OLG1_HANDLE;
    backend.optStrings[{SIDE1_HANDLE, prop::SELECTED_OLG_ID}] = "DEFAULT";
    backend.relatedByString[std::make_tuple(ObjectHandle(SIDE2_HANDLE), prop::OPERATIONAL_LIMITS_GROUP, std::string("EMPTY"))] = OLG2_HANDLE;
    backend.bools  [{OLG2_HANDLE, prop::OLG_EMPTY}] = true;

    Line line(LINE_HANDLE, &backend);
    std::vector<OperationalLimitsGroup> groups = line.getOperationalLimitsGroups1();
    ASSERT_EQ(groups.size(), 1u);
    EXPECT_EQ(groups[0].getId(), "DEFAULT");
    EXPECT_FALSE(groups[0].isEmpty());
    EXPECT_EQ(*line.getSelectedOperationalLimitsGroup1(), groups[0]);
    EXPECT_EQ(line.getSelectedOperationalLimitsGroupId1(), iidm::optional<std::string>("DEFAULT"));
    EXPECT_FALSE(line.getSelectedOperationalLimitsGroupId2());
    EXPECT_TRUE(line.getOperationalLimitsGroups2().empty());
    ASSERT_TRUE(line.getOperationalLimitsGroup2("EMPTY"));
    EXPECT_TRUE(line.getOperationalLimitsGroup2("EMPTY")->isEmpty());
    EXPECT_FALSE(line.getOperationalLimitsGroup2("UNKNOWN"));
}

TEST_F(LineLimitsTest, Selection) {
    Line line(LINE_HANDLE, &backend);
    EXPECT_EQ(&line.setSelectedOperationalLimitsGroup2("WINTER"), &line);
    EXPECT_EQ((backend.strings[{SIDE2_HANDLE, prop::SELECTED_OLG_ID}]), "WINTER");
    line.addSelectedOperationalLimitsGroups(TwoSides::ONE, {"A", "B"});
    EXPECT_EQ((backend.strings[{SIDE1_HANDLE, prop::ADD_SELECTED_OLG}]), "B"); // one call per group
    line.deselectOperationalLimitsGroups(TwoSides::ONE, {"A"});
    EXPECT_EQ((backend.strings[{SIDE1_HANDLE, prop::DESELECT_OLG}]), "A");
    line.removeOperationalLimitsGroup1("A");
    EXPECT_EQ((backend.strings[{SIDE1_HANDLE, prop::REMOVE_OLG}]), "A");
    backend.related[{SIDE1_HANDLE, prop::REL_SELECTED_OLG}] = OLG1_HANDLE;
    line.cancelSelectedOperationalLimitsGroup1();
    EXPECT_EQ((backend.related[{SIDE1_HANDLE, prop::REL_SELECTED_OLG}]), INVALID_HANDLE);

    backend.stringLists[{SIDE1_HANDLE, prop::ALL_SELECTED_OLG_IDS}] = {"A", "B"};
    backend.stringLists[{SIDE1_HANDLE, prop::ALL_SELECTED_OLG_IDS_ORDERED}] = {"B", "A"};
    backend.children[{SIDE1_HANDLE, prop::REL_SELECTED_OLG}] = {OLG1_HANDLE, OLG2_HANDLE};
    backend.children[{SIDE1_HANDLE, prop::CURRENT_LIMITS}] = {CL1_HANDLE, CL2_HANDLE};
    EXPECT_EQ(line.getAllSelectedOperationalLimitsGroupIds(TwoSides::ONE), (std::vector<std::string>{"A", "B"}));
    EXPECT_EQ(line.getAllSelectedOperationalLimitsGroupIdsOrdered(TwoSides::ONE), (std::vector<std::string>{"B", "A"}));
    EXPECT_EQ(line.getAllSelectedOperationalLimitsGroups(TwoSides::ONE).size(), 2u);
    EXPECT_EQ(line.getAllSelectedCurrentLimits(TwoSides::ONE).size(), 2u);
    EXPECT_EQ(line.getAllSelectedLimits(LimitType::CURRENT, TwoSides::ONE).size(), 2u);
    EXPECT_TRUE(line.getAllSelectedApparentPowerLimits(TwoSides::ONE).empty());
    EXPECT_THROW(line.getAllSelectedLimits(LimitType::VOLTAGE, TwoSides::ONE), PropertyNotFoundException);
}

TEST_F(LineLimitsTest, Creation) {
    Line line(LINE_HANDLE, &backend);
    backend.addResults[backend.nextAdder] = OLG1_HANDLE;
    OperationalLimitsGroup group = line.newOperationalLimitsGroup2("NEW");
    const ObjectHandle adder = backend.lastAdder;
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(SIDE2_HANDLE), prop::OPERATIONAL_LIMITS_GROUP));
    EXPECT_EQ((backend.strings[{adder, prop::OLG_ID}]), "NEW");
    EXPECT_EQ(group, OperationalLimitsGroup(OLG1_HANDLE, &backend));

    backend.addResults[backend.nextAdder] = OLG2_HANDLE;
    EXPECT_EQ(line.getOrCreateSelectedOperationalLimitsGroup1("SEL"), OperationalLimitsGroup(OLG2_HANDLE, &backend));
    EXPECT_EQ(backend.invocations[2], std::make_pair(ObjectHandle(SIDE1_HANDLE), prop::REL_SELECTED_OLG));
    EXPECT_EQ((backend.strings[{backend.lastAdder, prop::OLG_ID}]), "SEL");
}

TEST_F(LineLimitsTest, GetSide) {
    Line line(LINE_HANDLE, &backend);
    EXPECT_EQ(line.getSide(Terminal(TERM1_HANDLE, &backend)), TwoSides::ONE);
    EXPECT_EQ(line.getSide(Terminal(TERM2_HANDLE, &backend)), TwoSides::TWO);
    EXPECT_THROW(line.getSide(Terminal(99, &backend)), ValidationException);
}
