#include <gtest/gtest.h>
#include <iidm/BusbarSection.h>
#include <iidm/BusbarSectionPosition.h>
#include <iidm/ConnectablePosition.h>
#include <iidm/ConnectablePositionAdder.h>
#include <iidm/Line.h>
#include <iidm/LinePosition.h>
#include <iidm/LinePositionAdder.h>
#include <iidm/Load.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Substation.h>
#include <iidm/SubstationPosition.h>
#include "MockBackend.h"
#include <string>
#include <tuple>
#include <vector>

using namespace iidm;
using namespace iidm::test;

// The position extensions as the backend sees them: coordinates read from coordinate objects and
// added as nested adders; feeders by slot, added as nested adders that carry their slot.

static constexpr ObjectHandle HOST_HANDLE = 10;
static constexpr ObjectHandle EXT_HANDLE = 20;
static constexpr ObjectHandle C1_HANDLE = 30;
static constexpr ObjectHandle C2_HANDLE = 31;
static constexpr ObjectHandle FEEDER_HANDLE = 40;
static constexpr ObjectHandle ADDER_HANDLE = 500; // the next "adder" of the mock

class PositionExtensionsTest : public ::testing::Test {
protected:
    MockBackend backend;

    void SetUp() override {
        backend.nextAdder = ADDER_HANDLE;
        backend.doubles[{C1_HANDLE, prop::COORD_LATITUDE}] = 48.85;
        backend.doubles[{C1_HANDLE, prop::COORD_LONGITUDE}] = 2.35;
        backend.doubles[{C2_HANDLE, prop::COORD_LATITUDE}] = 50.85;
        backend.doubles[{C2_HANDLE, prop::COORD_LONGITUDE}] = 4.35;
    }

    void attach(int type) {
        backend.related[{HOST_HANDLE, type}] = EXT_HANDLE;
    }
};

TEST_F(PositionExtensionsTest, Coordinates) {
    attach(prop::SUBSTATION_POSITION);
    backend.related[{EXT_HANDLE, prop::COORDINATE}] = C1_HANDLE;
    Substation s(HOST_HANDLE, &backend);
    EXPECT_EQ(s.getExtension<SubstationPosition>().getCoordinate(), Coordinate(48.85, 2.35));

    attach(prop::LINE_POSITION);
    backend.children[{EXT_HANDLE, prop::COORDINATE}] = {C1_HANDLE, C2_HANDLE};
    Line line(HOST_HANDLE, &backend);
    EXPECT_EQ(line.getExtension<LinePosition>().getCoordinates(), (std::vector<Coordinate>{Coordinate(48.85, 2.35), Coordinate(50.85, 4.35)}));

    // New: a nested coordinate for each, in order
    line.newExtension<LinePositionAdder>().withCoordinates({Coordinate(1, 2), Coordinate(3, 4)}).add();
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(HOST_HANDLE), prop::LINE_POSITION));
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(ADDER_HANDLE), prop::COORDINATE));
    EXPECT_DOUBLE_EQ((backend.doubles[{ADDER_HANDLE + 1, prop::COORD_LATITUDE}]), 1.0);
    EXPECT_EQ(backend.invocations[2], std::make_pair(ObjectHandle(ADDER_HANDLE + 1), prop::ACT_ADD));
    EXPECT_DOUBLE_EQ((backend.doubles[{ADDER_HANDLE + 2, prop::COORD_LONGITUDE}]), 4.0);
    EXPECT_EQ(backend.invocations[5], std::make_pair(ObjectHandle(ADDER_HANDLE), prop::ACT_ADD));
}

TEST_F(PositionExtensionsTest, Feeders) {
    attach(prop::CONNECTABLE_POSITION);
    backend.relatedByIndex[std::make_tuple(ObjectHandle(EXT_HANDLE), prop::FEEDER, 2)] = FEEDER_HANDLE;
    backend.optStrings[{FEEDER_HANDLE, prop::EXT_FEEDER_NAME}] = "L12 side 2";
    backend.optInts[{FEEDER_HANDLE, prop::EXT_FEEDER_ORDER}] = 3;
    backend.ints[{FEEDER_HANDLE, prop::EXT_FEEDER_DIRECTION}] = static_cast<int>(ConnectablePosition::Direction::BOTTOM);
    ConnectablePosition position = Line(HOST_HANDLE, &backend).getExtension<ConnectablePosition>();
    EXPECT_FALSE(position.getFeeder());
    EXPECT_FALSE(position.getFeeder1());
    ConnectablePosition::Feeder feeder = *position.getFeeder2();
    EXPECT_EQ(*feeder.getName(), "L12 side 2");
    EXPECT_EQ(*feeder.getOrder(), 3);
    EXPECT_EQ(feeder.getDirection(), ConnectablePosition::Direction::BOTTOM);
    feeder.setName("side 2").setOrder(4).setDirection(ConnectablePosition::Direction::TOP).removeOrder();
    EXPECT_EQ((backend.strings[{FEEDER_HANDLE, prop::EXT_FEEDER_NAME}]), "side 2");
    EXPECT_EQ((backend.ints[{FEEDER_HANDLE, prop::EXT_FEEDER_ORDER}]), 4);
    EXPECT_EQ((backend.ints[{FEEDER_HANDLE, prop::EXT_FEEDER_DIRECTION}]), 0);
    EXPECT_EQ(backend.invocations.back(), std::make_pair(ObjectHandle(FEEDER_HANDLE), prop::ACT_REMOVE_ORDER));

    // New: a nested feeder per end, with its slot
    backend.invocations.clear();
    backend.nextAdder = ADDER_HANDLE;
    Load(HOST_HANDLE, &backend).newExtension<ConnectablePositionAdder>()
        .newFeeder().withName("LD feeder").withOrder(10).withDirection(ConnectablePosition::Direction::BOTTOM).add()
        .newFeeder3().add()
        .add();
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(ADDER_HANDLE), prop::FEEDER));
    EXPECT_EQ((backend.ints[{ADDER_HANDLE + 1, prop::EXT_FEEDER_SLOT}]), 0);
    EXPECT_EQ((backend.strings[{ADDER_HANDLE + 1, prop::EXT_FEEDER_NAME}]), "LD feeder");
    EXPECT_EQ((backend.ints[{ADDER_HANDLE + 1, prop::EXT_FEEDER_ORDER}]), 10);
    EXPECT_EQ(backend.invocations[2], std::make_pair(ObjectHandle(ADDER_HANDLE + 1), prop::ACT_ADD));
    EXPECT_EQ((backend.ints[{ADDER_HANDLE + 2, prop::EXT_FEEDER_SLOT}]), 3);
    EXPECT_EQ(backend.invocations[5], std::make_pair(ObjectHandle(ADDER_HANDLE), prop::ACT_ADD));
}
