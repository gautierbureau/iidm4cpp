#include <gtest/gtest.h>
#include <iidm/Line.h>
#include <iidm/LineCouplings.h>
#include <iidm/Network.h>
#include <iidm/NetworkFactory.h>
#include <iidm/PropertyCodes.h>
#include <iidm/SecondaryVoltageControl.h>
#include "MockBackend.h"
#include <memory>
#include <string>
#include <tuple>
#include <vector>

using namespace iidm;
using namespace iidm::test;

// The secondary voltage control and the line couplings of a network as the backend sees them: zones,
// pilot points and control units by relation, their adders nested; segments read as values and set
// at once; the queries of the line couplings as pending changes whose ACT_ADD gives the result.

static constexpr ObjectHandle NETWORK_HANDLE = 1; // the mock's network
static constexpr ObjectHandle EXT_HANDLE = 20;
static constexpr ObjectHandle ZONE_HANDLE = 30;
static constexpr ObjectHandle PILOT_POINT_HANDLE = 31;
static constexpr ObjectHandle UNIT_HANDLE = 32;
static constexpr ObjectHandle COUPLING_HANDLE = 40;
static constexpr ObjectHandle SEGMENT_HANDLE = 41;
static constexpr ObjectHandle LINE1_HANDLE = 50;
static constexpr ObjectHandle LINE2_HANDLE = 51;
static constexpr ObjectHandle ADDER_HANDLE = 500; // the next "adder" of the mock

class NetworkExtensionsTest : public ::testing::Test {
protected:
    MockBackend backend;

    void SetUp() override {
        backend.nextAdder = ADDER_HANDLE;
    }
};

TEST_F(NetworkExtensionsTest, SecondaryVoltageControl) {
    backend.children[{EXT_HANDLE, prop::CONTROL_ZONE}] = {ZONE_HANDLE};
    backend.relatedByString[std::make_tuple(ObjectHandle(EXT_HANDLE), prop::CONTROL_ZONE, std::string("z1"))] = ZONE_HANDLE;
    backend.strings[{ZONE_HANDLE, prop::EXT_SVC_ZONE_NAME}] = "z1";
    backend.related[{ZONE_HANDLE, prop::PILOT_POINT}] = PILOT_POINT_HANDLE;
    backend.stringLists[{PILOT_POINT_HANDLE, prop::EXT_SVC_PILOT_POINT_IDS}] = {"B2", "B1"};
    backend.doubles[{PILOT_POINT_HANDLE, prop::EXT_SVC_TARGET_V}] = 405;
    backend.children[{ZONE_HANDLE, prop::CONTROL_UNIT}] = {UNIT_HANDLE};
    backend.relatedByString[std::make_tuple(ObjectHandle(ZONE_HANDLE), prop::CONTROL_UNIT, std::string("G1"))] = UNIT_HANDLE;
    backend.strings[{UNIT_HANDLE, prop::EXT_SVC_UNIT_ID}] = "G1";
    backend.bools[{UNIT_HANDLE, prop::EXT_SVC_UNIT_PARTICIPATE}] = true;

    SecondaryVoltageControl svc(EXT_HANDLE, &backend);
    std::vector<ControlZone> zones = svc.getControlZones();
    ASSERT_EQ(zones.size(), 1u);
    EXPECT_EQ(zones[0].getName(), "z1");
    PilotPoint pilotPoint = zones[0].getPilotPoint();
    EXPECT_EQ(pilotPoint.getBusbarSectionsOrBusesIds(), (std::vector<std::string>{"B2", "B1"}));
    EXPECT_DOUBLE_EQ(pilotPoint.getTargetV(), 405);
    EXPECT_EQ(zones[0].getControlUnits()[0].getId(), "G1");
    EXPECT_TRUE(svc.getControlZone("z1")->getControlUnit("G1")->isParticipate());
    EXPECT_FALSE(svc.getControlZone("z2"));
    EXPECT_FALSE(zones[0].getControlUnit("G2"));

    pilotPoint.setTargetV(410);
    EXPECT_DOUBLE_EQ((backend.doubles[{PILOT_POINT_HANDLE, prop::EXT_SVC_TARGET_V}]), 410);
    zones[0].getControlUnits()[0].setParticipate(false);
    EXPECT_FALSE((backend.bools[{UNIT_HANDLE, prop::EXT_SVC_UNIT_PARTICIPATE}]));
}

TEST_F(NetworkExtensionsTest, SecondaryVoltageControlAdder) {
    std::unique_ptr<MockBackend> mock(new MockBackend());
    MockBackend& b = *mock;
    b.nextAdder = ADDER_HANDLE;
    Network network = NetworkFactory::createFromBackend(std::move(mock));
    network.newSecondaryVoltageControl()
        .newControlZone().withName("z1")
            .newPilotPoint().withBusbarSectionsOrBusesIds({"B2", "B1"}).withTargetV(405).add()
            .newControlUnit().withId("G1").withParticipate(false).add()
            .add()
        .add();
    EXPECT_EQ(b.invocations[0], std::make_pair(ObjectHandle(NETWORK_HANDLE), prop::SECONDARY_VOLTAGE_CONTROL));
    EXPECT_EQ(b.invocations[1], std::make_pair(ObjectHandle(ADDER_HANDLE), prop::CONTROL_ZONE));
    EXPECT_EQ((b.strings[{ADDER_HANDLE + 1, prop::EXT_SVC_ZONE_NAME}]), "z1");
    // The pilot point: its ids in order
    EXPECT_EQ(b.invocations[2], std::make_pair(ObjectHandle(ADDER_HANDLE + 1), prop::PILOT_POINT));
    EXPECT_EQ(b.stringSets[1], std::make_tuple(ObjectHandle(ADDER_HANDLE + 2), prop::EXT_SVC_PILOT_POINT_IDS, std::string("B2")));
    EXPECT_EQ(b.stringSets[2], std::make_tuple(ObjectHandle(ADDER_HANDLE + 2), prop::EXT_SVC_PILOT_POINT_IDS, std::string("B1")));
    EXPECT_DOUBLE_EQ((b.doubles[{ADDER_HANDLE + 2, prop::EXT_SVC_TARGET_V}]), 405);
    EXPECT_EQ(b.invocations[3], std::make_pair(ObjectHandle(ADDER_HANDLE + 2), prop::ACT_ADD));
    // The control unit, then the zone, then the extension
    EXPECT_EQ(b.invocations[4], std::make_pair(ObjectHandle(ADDER_HANDLE + 1), prop::CONTROL_UNIT));
    EXPECT_EQ((b.strings[{ADDER_HANDLE + 3, prop::EXT_SVC_UNIT_ID}]), "G1");
    EXPECT_FALSE((b.bools[{ADDER_HANDLE + 3, prop::EXT_SVC_UNIT_PARTICIPATE}]));
    EXPECT_EQ(b.invocations[5], std::make_pair(ObjectHandle(ADDER_HANDLE + 3), prop::ACT_ADD));
    EXPECT_EQ(b.invocations[6], std::make_pair(ObjectHandle(ADDER_HANDLE + 1), prop::ACT_ADD));
    EXPECT_EQ(b.invocations[7], std::make_pair(ObjectHandle(ADDER_HANDLE), prop::ACT_ADD));

    EXPECT_FALSE(network.hasSecondaryVoltageControl());
    EXPECT_THROW(network.getSecondaryVoltageControl(), ExtensionNotFoundException);
    b.related[{NETWORK_HANDLE, prop::SECONDARY_VOLTAGE_CONTROL}] = EXT_HANDLE;
    EXPECT_TRUE(network.hasSecondaryVoltageControl());
    EXPECT_FALSE(network.hasLineCouplings());
    network.removeSecondaryVoltageControl();
    EXPECT_EQ((b.related[{NETWORK_HANDLE, prop::SECONDARY_VOLTAGE_CONTROL}]), INVALID_HANDLE);
}

TEST_F(NetworkExtensionsTest, MutualCoupling) {
    backend.children[{EXT_HANDLE, prop::MUTUAL_COUPLING}] = {COUPLING_HANDLE};
    backend.related[{COUPLING_HANDLE, prop::EXT_MC_LINE1}] = LINE1_HANDLE;
    backend.related[{COUPLING_HANDLE, prop::EXT_MC_LINE2}] = LINE2_HANDLE;
    backend.doubles[{COUPLING_HANDLE, prop::EXT_MC_R}] = 0.2;
    backend.related[{COUPLING_HANDLE, prop::EXT_MC_LINE1_SEGMENT}] = SEGMENT_HANDLE;
    backend.doubles[{SEGMENT_HANDLE, prop::EXT_MC_SEGMENT_START}] = 0.1;
    backend.doubles[{SEGMENT_HANDLE, prop::EXT_MC_SEGMENT_END}] = 0.6;
    std::vector<MutualCoupling> couplings = LineCouplings(EXT_HANDLE, &backend).getMutualCouplings();
    ASSERT_EQ(couplings.size(), 1u);
    MutualCoupling& coupling = couplings[0];
    EXPECT_EQ(detail::Access::handle(coupling.getLine1()), ObjectHandle(LINE1_HANDLE));
    EXPECT_EQ(detail::Access::handle(coupling.getLine2()), ObjectHandle(LINE2_HANDLE));
    EXPECT_DOUBLE_EQ(coupling.getR(), 0.2);
    EXPECT_EQ(coupling.getLine1Segment(), LineSegment(0.1, 0.6));

    // A segment set at once
    coupling.setLine2Segment(LineSegment(0.25, 1)).setX(0.9);
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(COUPLING_HANDLE), prop::EXT_MC_LINE2_SEGMENT));
    EXPECT_DOUBLE_EQ((backend.doubles[{ADDER_HANDLE, prop::EXT_MC_SEGMENT_START}]), 0.25);
    EXPECT_DOUBLE_EQ((backend.doubles[{ADDER_HANDLE, prop::EXT_MC_SEGMENT_END}]), 1);
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(ADDER_HANDLE), prop::ACT_ADD));
    EXPECT_DOUBLE_EQ((backend.doubles[{COUPLING_HANDLE, prop::EXT_MC_X}]), 0.9);
    EXPECT_EQ(LineSegment(), LineSegment::FULL_LINE);
}

TEST_F(NetworkExtensionsTest, MutualCouplingAdder) {
    Line line1(LINE1_HANDLE, &backend);
    Line line2(LINE2_HANDLE, &backend);
    LineCouplings(EXT_HANDLE, &backend).newMutualCoupling()
        .withLine1(line1).withLine2(line2).withR(0.3).withX(0.9).withLine1Segment(LineSegment(0, 0.4))
        .add();
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(EXT_HANDLE), prop::MUTUAL_COUPLING));
    EXPECT_EQ((backend.related[{ADDER_HANDLE, prop::EXT_MC_LINE1}]), ObjectHandle(LINE1_HANDLE));
    EXPECT_EQ((backend.related[{ADDER_HANDLE, prop::EXT_MC_LINE2}]), ObjectHandle(LINE2_HANDLE));
    EXPECT_DOUBLE_EQ((backend.doubles[{ADDER_HANDLE, prop::EXT_MC_R}]), 0.3);
    EXPECT_DOUBLE_EQ((backend.doubles[{ADDER_HANDLE, prop::EXT_MC_X}]), 0.9);
    // The segment: a nested change
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(ADDER_HANDLE), prop::EXT_MC_LINE1_SEGMENT));
    EXPECT_DOUBLE_EQ((backend.doubles[{ADDER_HANDLE + 1, prop::EXT_MC_SEGMENT_END}]), 0.4);
    EXPECT_EQ(backend.invocations[2], std::make_pair(ObjectHandle(ADDER_HANDLE + 1), prop::ACT_ADD));
    EXPECT_EQ(backend.invocations[3], std::make_pair(ObjectHandle(ADDER_HANDLE), prop::ACT_ADD));
}

TEST_F(NetworkExtensionsTest, MutualCouplingQueries) {
    LineCouplings couplings(EXT_HANDLE, &backend);
    Line line1(LINE1_HANDLE, &backend);
    Line line2(LINE2_HANDLE, &backend);
    backend.addResults[ADDER_HANDLE] = COUPLING_HANDLE;
    iidm::optional<MutualCoupling> found = couplings.findMutualCoupling(line1, line2);
    ASSERT_TRUE(found);
    EXPECT_EQ(detail::Access::handle(*found), ObjectHandle(COUPLING_HANDLE));
    EXPECT_EQ(backend.invocations[0], std::make_pair(ObjectHandle(EXT_HANDLE), prop::FIND_MUTUAL_COUPLING));
    EXPECT_EQ((backend.related[{ADDER_HANDLE, prop::EXT_MC_LINE1}]), ObjectHandle(LINE1_HANDLE));
    EXPECT_EQ((backend.related[{ADDER_HANDLE, prop::EXT_MC_LINE2}]), ObjectHandle(LINE2_HANDLE));
    EXPECT_EQ(backend.invocations[1], std::make_pair(ObjectHandle(ADDER_HANDLE), prop::ACT_ADD));
    EXPECT_FALSE(couplings.findMutualCoupling(line2, line1)); // no result

    // Removed when ACT_ADD gives something (the extension)
    backend.addResults[ADDER_HANDLE + 2] = EXT_HANDLE;
    EXPECT_TRUE(couplings.removeMutualCoupling(line1, line2));
    EXPECT_EQ(backend.invocations[4], std::make_pair(ObjectHandle(EXT_HANDLE), prop::REMOVE_MUTUAL_COUPLING));
    EXPECT_FALSE(couplings.removeMutualCoupling(*found));
    EXPECT_EQ(backend.invocations[6], std::make_pair(ObjectHandle(EXT_HANDLE), prop::REMOVE_MUTUAL_COUPLING));
    EXPECT_EQ((backend.related[{ADDER_HANDLE + 3, prop::MUTUAL_COUPLING}]), ObjectHandle(COUPLING_HANDLE));

    couplings.add(*found);
    EXPECT_EQ((backend.related[{EXT_HANDLE, prop::ADD_MUTUAL_COUPLING}]), ObjectHandle(COUPLING_HANDLE));
}
