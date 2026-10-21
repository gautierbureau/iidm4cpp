#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "RealBackends.h"
#include <string>
#include <vector>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif
#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include "jni/JNIBackend.h"
#endif

// The position extensions on the real backends (see RealBackends.h): the same scenario on GraalVM
// and on JNI, with the network test/data/positions_test.xiidm. S1: substation position; BBS:
// busbar section position; LD (feeder "LD feeder", order 10, BOTTOM) and L12 (feeder1 TOP, feeder2
// BOTTOM order 3): connectable positions; L12 and BL: line positions. S2 and G have none.

using namespace iidm;
using iidm::test::envPath;

namespace {

void checkCoordinates(Network& network) {
    Substation s1 = *network.getSubstation("S1");
    EXPECT_EQ(s1.getSubstationPosition().getCoordinate(), Coordinate(48.85, 2.35));
    Substation s2 = *network.getSubstation("S2");
    EXPECT_FALSE(s2.hasSubstationPosition());
    EXPECT_EQ(s2.newSubstationPosition().withCoordinate(Coordinate(50.85, 4.35)).add().getCoordinate(),
              Coordinate(50.85, 4.35));

    Line line = *network.getLine("L12");
    EXPECT_EQ(line.getLinePosition().getCoordinates(),
              (std::vector<Coordinate>{Coordinate(48.85, 2.35), Coordinate(49.5, 3.1), Coordinate(50.85, 4.35)}));
    BoundaryLine bl = *network.getBoundaryLine("BL");
    EXPECT_EQ(bl.getLinePosition().getCoordinates().size(), 2u);
    bl.removeLinePosition();
    EXPECT_FALSE(bl.hasLinePosition());
    std::vector<Coordinate> path{Coordinate(1, 2), Coordinate(3, 4), Coordinate(5, 6)};
    EXPECT_EQ(bl.newLinePosition().withCoordinates(path).add().getCoordinates(), path);

    BusbarSectionPosition bsp = network.getBusbarSection("BBS")->getBusbarSectionPosition();
    EXPECT_EQ(bsp.getBusbarIndex(), 1);
    EXPECT_EQ(bsp.getSectionIndex(), 2);
    bsp.setBusbarIndex(2);
    EXPECT_EQ(network.getBusbarSection("BBS")->getBusbarSectionPosition().getBusbarIndex(), 2);
}

void checkFeeders(Network& network) {
    Load load = *network.getLoad("LD");
    ConnectablePosition position = load.getConnectablePosition();
    ASSERT_TRUE(position.getFeeder());
    ConnectablePosition::Feeder feeder = *position.getFeeder();
    EXPECT_EQ(*feeder.getName(), "LD feeder");
    EXPECT_EQ(*feeder.getOrder(), 10);
    EXPECT_EQ(feeder.getDirection(), ConnectablePosition::Direction::BOTTOM);
    EXPECT_FALSE(position.getFeeder1());
    feeder.removeOrder();
    EXPECT_FALSE(load.getConnectablePosition().getFeeder()->getOrder());

    ConnectablePosition linePosition = network.getLine("L12")->getConnectablePosition();
    EXPECT_FALSE(linePosition.getFeeder());
    ConnectablePosition::Feeder feeder1 = *linePosition.getFeeder1();
    EXPECT_FALSE(feeder1.getName());
    EXPECT_FALSE(feeder1.getOrder());
    EXPECT_EQ(feeder1.getDirection(), ConnectablePosition::Direction::TOP);
    EXPECT_EQ(*linePosition.getFeeder2()->getOrder(), 3);
    EXPECT_FALSE(linePosition.getFeeder3());
    feeder1.setName("L12 side 1").setOrder(7).setDirection(ConnectablePosition::Direction::UNDEFINED);
    EXPECT_EQ(*network.getLine("L12")->getConnectablePosition().getFeeder1()->getName(), "L12 side 1");
    EXPECT_EQ(*network.getLine("L12")->getConnectablePosition().getFeeder1()->getOrder(), 7);

    // New ones
    Generator g = *network.getGenerator("G");
    EXPECT_FALSE(g.hasConnectablePosition());
    ConnectablePosition added = g.newConnectablePosition()
        .newFeeder().withName("G feeder").withOrder(20).withDirection(ConnectablePosition::Direction::TOP).add()
        .add();
    EXPECT_EQ(*added.getFeeder()->getName(), "G feeder");
    g.removeConnectablePosition();
    // An injection's feeder and a side's: rejected
    EXPECT_THROW(g.newConnectablePosition().newFeeder().add().newFeeder1().add().add(), ValidationException);
    EXPECT_THROW(g.newConnectablePosition().add(), ValidationException);
}

void checkNetwork(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/positions_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/positions_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    Network network = Identifiable(b.getNetworkHandle(), &b).getNetwork();
    checkCoordinates(network);
    checkFeeders(network);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(PositionExtensionsNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkNetwork(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(PositionExtensionsNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("positions_test");
    checkNetwork(backend);
}
#endif
