#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "RealBackends.h"
#include <cmath>
#include <string>
#include <vector>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif
#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include "jni/JNIBackend.h"
#endif

// Areas and voltage angle limits on the real backends (see RealBackends.h): the same scenario on
// GraalVM and on JNI, with the network test/data/areas_test.xiidm: the control areas CA_FR (VL1, VL2;
// boundaries L23 side 1, AC, and the boundary of BL, DC) and CA_BE (VL3; L23 side 2), the bidding
// zone BZ (all three); the voltage angle limits VAL13 (L12 side 1 to L23 side 2) and VAL_HIGH.

using namespace iidm;
using iidm::test::envPath;

namespace {

using Ids = std::vector<std::string>;

template <class T>
Ids idsOf(const std::vector<T>& objects) {
    Ids ids;
    for (const T& object : objects) ids.push_back(object.getId());
    return ids;
}

void checkAreas(BackendProvider& b) {
    VoltageLevel vl2(b.findById(prop::VOLTAGE_LEVEL, "VL2"), &b);
    VoltageLevel vl3(b.findById(prop::VOLTAGE_LEVEL, "VL3"), &b);
    Network network = vl2.getNetwork();
    EXPECT_EQ(network.getAreaCount(), 3);
    EXPECT_EQ(network.getAreaTypes(), (Ids{"BiddingZone", "ControlArea"}));
    EXPECT_EQ(network.getAreaTypeCount(), 2);
    EXPECT_EQ(idsOf(network.getAreas()), (Ids{"CA_FR", "CA_BE", "BZ"}));

    Area fr = *network.getArea("CA_FR");
    EXPECT_EQ(fr.getType(), IdentifiableType::AREA);
    EXPECT_EQ(fr.getAreaType(), "ControlArea");
    EXPECT_EQ(fr.getNameOrId(), "France");
    EXPECT_EQ(*fr.getProperty("tso"), "RTE");
    EXPECT_EQ(fr.getInterchangeTarget(), iidm::optional<double>(-100.0));
    EXPECT_EQ(idsOf(fr.getVoltageLevels()), (Ids{"VL1", "VL2"}));
    EXPECT_EQ(idsOf(network.getArea("BZ")->getVoltageLevels()), (Ids{"VL1", "VL2", "VL3"})); // sorted
    EXPECT_FALSE(network.getArea("BZ")->getInterchangeTarget());
    EXPECT_EQ(idsOf(vl2.getAreas()), (Ids{"CA_FR", "BZ"}));
    EXPECT_EQ(*vl2.getArea("ControlArea"), fr);
    EXPECT_FALSE(vl2.getArea("Region"));

    // Its boundaries: L23 side 1 (AC), the boundary of BL (DC)
    Line l23(b.findById(prop::LINE, "L23"), &b);
    BoundaryLine bl(b.findById(prop::BOUNDARY_LINE, "BL"), &b);
    std::vector<AreaBoundary> boundaries = fr.getAreaBoundaries();
    ASSERT_EQ(boundaries.size(), 2u);
    EXPECT_EQ(*boundaries[0].getTerminal(), l23.getTerminal1());
    EXPECT_FALSE(boundaries[0].getBoundary());
    EXPECT_TRUE(boundaries[0].isAc());
    EXPECT_DOUBLE_EQ(boundaries[0].getP(), 99.5);
    EXPECT_DOUBLE_EQ(boundaries[0].getQ(), 9.0);
    EXPECT_EQ(boundaries[0].getArea(), fr);
    EXPECT_EQ(*boundaries[1].getBoundary(), bl.getBoundary());
    EXPECT_FALSE(boundaries[1].getTerminal());
    EXPECT_FALSE(boundaries[1].isAc());
    EXPECT_EQ(*fr.getAreaBoundary(l23.getTerminal1()), boundaries[0]);
    EXPECT_EQ(*fr.getAreaBoundary(bl.getBoundary()), boundaries[1]);
    EXPECT_FALSE(fr.getAreaBoundary(l23.getTerminal2()));
    boundaries[0].setProperty("kind", "line");
    EXPECT_EQ(*boundaries[0].getProperty("kind"), "line");

    // Interchanges: the flows out through the boundaries
    EXPECT_DOUBLE_EQ(fr.getAcInterchange(), 99.5);
    EXPECT_NEAR(fr.getDcInterchange(), boundaries[1].getP(), 1e-9);
    EXPECT_NEAR(fr.getInterchange(), fr.getAcInterchange() + fr.getDcInterchange(), 1e-9);

    // Changes
    fr.setInterchangeTarget(NAN);
    EXPECT_FALSE(fr.getInterchangeTarget());
    fr.setInterchangeTarget(-50);
    Area be = *network.getArea("CA_BE");
    be.removeVoltageLevel(vl3);
    EXPECT_TRUE(be.getVoltageLevels().empty());
    vl3.addArea(be);
    EXPECT_EQ(idsOf(be.getVoltageLevels()), (Ids{"VL3"}));
    EXPECT_THROW(fr.addVoltageLevel(vl3), ValidationException); // already in a control area
    fr.removeAreaBoundary(bl.getBoundary());
    EXPECT_EQ(fr.getAreaBoundaries().size(), 1u);
    EXPECT_EQ(fr.newAreaBoundary().setBoundary(bl.getBoundary()).setAc(false).add(), fr);
    EXPECT_EQ(fr.getAreaBoundaries().size(), 2u);

    // A new area, then removed
    Area region = network.newArea().setId("REGION").setAreaType("Region").setInterchangeTarget(10)
        .addVoltageLevel(vl3).addAreaBoundary(l23.getTerminal2(), true).add();
    EXPECT_EQ(idsOf(region.getVoltageLevels()), (Ids{"VL3"}));
    ASSERT_EQ(region.getAreaBoundaries().size(), 1u);
    EXPECT_EQ(*region.getAreaBoundaries()[0].getTerminal(), l23.getTerminal2());
    EXPECT_EQ(network.getAreaTypeCount(), 3);
    region.remove();
    EXPECT_FALSE(network.getArea("REGION"));
    EXPECT_EQ(idsOf(vl3.getAreas()), (Ids{"BZ", "CA_BE"})); // CA_BE added again, last
}

void checkVoltageAngleLimits(BackendProvider& b) {
    Line l12(b.findById(prop::LINE, "L12"), &b);
    Line l23(b.findById(prop::LINE, "L23"), &b);
    Network network = l12.getNetwork();
    EXPECT_EQ(network.getVoltageAngleLimits().size(), 2u);
    VoltageAngleLimit limit = *network.getVoltageAngleLimit("VAL13");
    EXPECT_EQ(limit.getId(), "VAL13");
    EXPECT_EQ(limit.getLimitType(), LimitType::VOLTAGE_ANGLE);
    EXPECT_EQ(limit.getLowLimit(), iidm::optional<double>(-10.0));
    EXPECT_EQ(limit.getHighLimit(), iidm::optional<double>(15.0));
    EXPECT_EQ(limit.getTerminalFrom(), l12.getTerminal1());
    EXPECT_EQ(limit.getTerminalTo(), l23.getTerminal2());
    EXPECT_FALSE(network.getVoltageAngleLimit("VAL_HIGH")->getLowLimit());
    EXPECT_FALSE(network.getVoltageAngleLimit("NONE"));
    limit.setProperty("source", "test");
    EXPECT_EQ(limit.getPropertyNames(), (Ids{"source"}));

    VoltageAngleLimit added = network.newVoltageAngleLimit().setId("VAL_NEW").from(l12.getTerminal2())
        .to(l23.getTerminal1()).setLowLimit(-5).setProperty("k", "v").add();
    EXPECT_EQ(*added.getProperty("k"), "v");
    EXPECT_EQ(added.getLowLimit(), iidm::optional<double>(-5.0));
    EXPECT_EQ(network.getVoltageAngleLimits().size(), 3u);
    added.remove();
    EXPECT_FALSE(network.getVoltageAngleLimit("VAL_NEW"));
}

void checkNetwork(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/areas_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/areas_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    checkAreas(b);
    checkVoltageAngleLimits(b);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(AreasNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkNetwork(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(AreasNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("areas_test");
    checkNetwork(backend);
}
#endif
