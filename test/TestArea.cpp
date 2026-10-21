#include <gtest/gtest.h>
#include <iidm/Area.h>
#include <iidm/AreaAdder.h>
#include <iidm/BoundaryLine.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Terminal.h>
#include <iidm/VoltageAngleLimit.h>
#include <iidm/VoltageLevel.h>
#include "MockBackend.h"

using namespace iidm;
using namespace iidm::test;

// Areas and voltage angle limits as the backend sees them

static constexpr ObjectHandle AREA_HANDLE = 10;
static constexpr ObjectHandle VL_HANDLE = 11;
static constexpr ObjectHandle TERM_HANDLE = 12;
static constexpr ObjectHandle OTHER_TERM = 13;
static constexpr ObjectHandle BOUNDARY_HANDLE = 14;
static constexpr ObjectHandle AB1 = 20;
static constexpr ObjectHandle AB2 = 21;
static constexpr ObjectHandle ADDER = 500; // the next "adder" of the mock

class AreaTest : public ::testing::Test {
protected:
    MockBackend backend;

    void SetUp() override {
        backend.nextAdder = ADDER;
        backend.strings[{AREA_HANDLE, prop::AREA_TYPE}] = "ControlArea";
        backend.children[{AREA_HANDLE, prop::AREA_BOUNDARY}] = {AB1, AB2};
        backend.related[{AB1, prop::REL_TERMINAL}] = TERM_HANDLE;
        backend.related[{AB2, prop::BOUNDARY}] = BOUNDARY_HANDLE;
        backend.bools[{AB1, prop::AREA_BOUNDARY_AC}] = true;
    }
};

TEST_F(AreaTest, Values) {
    backend.doubles[{AREA_HANDLE, prop::AREA_AC_INTERCHANGE}] = 99.5;
    Area area(AREA_HANDLE, &backend);
    EXPECT_EQ(area.getAreaType(), "ControlArea");
    EXPECT_FALSE(area.getInterchangeTarget()); // NaN: none
    area.setInterchangeTarget(-100);
    EXPECT_EQ(area.getInterchangeTarget(), iidm::optional<double>(-100.0));
    EXPECT_DOUBLE_EQ(area.getAcInterchange(), 99.5);
    area.remove();
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(AREA_HANDLE), prop::ACT_REMOVE));
}

TEST_F(AreaTest, Boundaries) {
    Area area(AREA_HANDLE, &backend);
    std::vector<AreaBoundary> boundaries = area.getAreaBoundaries();
    ASSERT_EQ(boundaries.size(), 2u);
    EXPECT_TRUE(boundaries[0].isAc());
    EXPECT_FALSE(boundaries[1].isAc());
    // Found by what they are
    EXPECT_EQ(*area.getAreaBoundary(Terminal(TERM_HANDLE, &backend)), boundaries[0]);
    EXPECT_EQ(*area.getAreaBoundary(Boundary(BOUNDARY_HANDLE, &backend)), boundaries[1]);
    EXPECT_FALSE(area.getAreaBoundary(Terminal(OTHER_TERM, &backend)));

    area.removeAreaBoundary(Terminal(TERM_HANDLE, &backend));
    EXPECT_EQ((backend.related[{AREA_HANDLE, prop::REMOVE_AREA_BOUNDARY}]), ObjectHandle(TERM_HANDLE));
    backend.addResults[ADDER] = AREA_HANDLE;
    Area same = area.newAreaBoundary().setTerminal(Terminal(OTHER_TERM, &backend)).setAc(true).add();
    EXPECT_EQ(same, area);
    EXPECT_EQ((backend.related[{ADDER, prop::REL_TERMINAL}]), ObjectHandle(OTHER_TERM));
    EXPECT_TRUE((backend.bools[{ADDER, prop::AREA_BOUNDARY_AC}]));
}

TEST_F(AreaTest, VoltageLevels) {
    backend.relatedByString[std::make_tuple(ObjectHandle(VL_HANDLE), prop::AREA, std::string("ControlArea"))] = AREA_HANDLE;
    VoltageLevel vl(VL_HANDLE, &backend);
    Area area(AREA_HANDLE, &backend);
    EXPECT_EQ(*vl.getArea("ControlArea"), area);
    EXPECT_FALSE(vl.getArea("BiddingZone"));
    vl.addArea(area);
    EXPECT_EQ((backend.related[{VL_HANDLE, prop::ADD_AREA}]), ObjectHandle(AREA_HANDLE));
    area.removeVoltageLevel(vl);
    EXPECT_EQ((backend.related[{AREA_HANDLE, prop::REMOVE_VOLTAGE_LEVEL}]), ObjectHandle(VL_HANDLE));
}

TEST_F(AreaTest, Adder) {
    // The area adder, then a nested adder per boundary
    AreaAdder adder(backend.invoke(1, prop::AREA), &backend);
    adder.setAreaType("Region").addVoltageLevel(VoltageLevel(VL_HANDLE, &backend))
         .addAreaBoundary(Boundary(BOUNDARY_HANDLE, &backend), false);
    EXPECT_EQ((backend.strings[{ADDER, prop::AREA_TYPE}]), "Region");
    EXPECT_EQ((backend.related[{ADDER, prop::ADD_VOLTAGE_LEVEL}]), ObjectHandle(VL_HANDLE));
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(ADDER), prop::AREA_BOUNDARY));
    EXPECT_EQ((backend.related[{ADDER + 1, prop::BOUNDARY}]), ObjectHandle(BOUNDARY_HANDLE));
    EXPECT_FALSE((backend.bools[{ADDER + 1, prop::AREA_BOUNDARY_AC}]));
    EXPECT_EQ(backend.invocations[2], std::make_pair(ObjectHandle(ADDER + 1), prop::ACT_ADD));
}

TEST_F(AreaTest, VoltageAngleLimit) {
    backend.strings[{30, prop::VAL_ID}] = "VAL";
    backend.ints[{30, prop::LL_LIMIT_TYPE}] = static_cast<int>(LimitType::VOLTAGE_ANGLE);
    backend.doubles[{30, prop::VAL_HIGH_LIMIT}] = 15.0;
    backend.related[{30, prop::REL_TERMINAL_FROM}] = TERM_HANDLE;
    VoltageAngleLimit limit(30, &backend);
    EXPECT_EQ(limit.getId(), "VAL");
    EXPECT_EQ(limit.getLimitType(), LimitType::VOLTAGE_ANGLE);
    EXPECT_FALSE(limit.getLowLimit());
    EXPECT_EQ(limit.getHighLimit(), iidm::optional<double>(15.0));
    EXPECT_EQ(limit.getTerminalFrom(), Terminal(TERM_HANDLE, &backend));

    VoltageAngleLimitAdder adder(backend.invoke(1, prop::VOLTAGE_ANGLE_LIMIT), &backend);
    adder.setId("NEW").from(Terminal(TERM_HANDLE, &backend)).to(Terminal(OTHER_TERM, &backend)).setLowLimit(-5);
    EXPECT_EQ((backend.strings[{ADDER, prop::VAL_ID}]), "NEW");
    EXPECT_EQ((backend.related[{ADDER, prop::REL_TERMINAL_TO}]), ObjectHandle(OTHER_TERM));
    EXPECT_DOUBLE_EQ((backend.doubles[{ADDER, prop::VAL_LOW_LIMIT}]), -5.0);
}
