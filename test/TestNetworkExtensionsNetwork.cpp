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

// The secondary voltage control and the line couplings on the real backends (see RealBackends.h): the
// same scenario on GraalVM and on JNI, with the network test/data/network_extensions_test.xiidm. Zone z1:
// pilot point B2 then B1, target 405, control units G1 (participating) and G2 (not); zone z2: pilot
// point B3, target 230.5, control unit G3. Couplings L12-L13 (r 0.1, x 0.5, full lines) and L23-L12
// (r 0.2, x 0.7, segments 0.1-0.6 of L23 and 0.25-1 of L12).

using namespace iidm;
using iidm::test::envPath;

namespace {

using Ids = std::vector<std::string>;

Ids unitsOf(const ControlZone& zone) {
    Ids units;
    for (const ControlUnit& unit : zone.getControlUnits()) {
        units.push_back(unit.getId() + (unit.isParticipate() ? "=true" : "=false"));
    }
    return units;
}

Ids couplingsOf(const LineCouplings& lineCouplings) {
    Ids couplings;
    for (const MutualCoupling& coupling : lineCouplings.getMutualCouplings()) {
        couplings.push_back(coupling.getLine1().getId() + "-" + coupling.getLine2().getId());
    }
    return couplings;
}

void checkSecondaryVoltageControl(Network& network) {
    SecondaryVoltageControl svc = network.getExtension<SecondaryVoltageControl>();
    std::vector<ControlZone> zones = svc.getControlZones();
    ASSERT_EQ(zones.size(), 2u);
    EXPECT_EQ(zones[0].getName(), "z1");
    PilotPoint pilotPoint = zones[0].getPilotPoint();
    EXPECT_EQ(pilotPoint.getBusbarSectionsOrBusesIds(), (Ids{"B2", "B1"}));
    EXPECT_EQ(pilotPoint.getTargetV(), 405);
    EXPECT_EQ(unitsOf(zones[0]), (Ids{"G1=true", "G2=false"}));
    EXPECT_EQ(svc.getControlZone("z2")->getPilotPoint().getTargetV(), 230.5);
    EXPECT_FALSE(svc.getControlZone("z3"));
    EXPECT_FALSE(zones[0].getControlUnit("G3"));

    // Changes, by variant
    pilotPoint.setTargetV(410);
    zones[0].getControlUnit("G2")->setParticipate(true);
    EXPECT_EQ(unitsOf(zones[0]), (Ids{"G1=true", "G2=true"}));
    EXPECT_THROW(pilotPoint.setTargetV(std::nan("")), ValidationException);
    VariantManager variants = network.getVariantManager();
    variants.cloneVariant(VariantManager::INITIAL_VARIANT_ID, "V2");
    variants.setWorkingVariant("V2");
    pilotPoint.setTargetV(420);
    zones[0].getControlUnit("G1")->setParticipate(false);
    variants.setWorkingVariant(VariantManager::INITIAL_VARIANT_ID);
    EXPECT_EQ(pilotPoint.getTargetV(), 410);
    EXPECT_EQ(unitsOf(zones[0]), (Ids{"G1=true", "G2=true"}));
    variants.setWorkingVariant("V2");
    EXPECT_EQ(pilotPoint.getTargetV(), 420);
    EXPECT_EQ(unitsOf(zones[0]), (Ids{"G1=false", "G2=true"}));
    variants.setWorkingVariant(VariantManager::INITIAL_VARIANT_ID);
    variants.removeVariant("V2");

    // A new one; powsybl's checks
    network.removeExtension<SecondaryVoltageControl>();
    EXPECT_FALSE(network.hasExtension<SecondaryVoltageControl>());
    SecondaryVoltageControl added = network.newExtension<SecondaryVoltageControlAdder>()
        .newControlZone().withName("z")
            .newPilotPoint().withBusbarSectionsOrBusesIds({"B3", "B1"}).withTargetV(400).add()
            .newControlUnit().withId("G3").withParticipate(false).add()
            .newControlUnit().withId("G1").add()
            .add()
        .add();
    ControlZone zone = *added.getControlZone("z");
    EXPECT_EQ(zone.getPilotPoint().getBusbarSectionsOrBusesIds(), (Ids{"B3", "B1"}));
    EXPECT_EQ(unitsOf(zone), (Ids{"G3=false", "G1=true"}));
    network.removeExtension<SecondaryVoltageControl>();
    EXPECT_THROW(network.newExtension<SecondaryVoltageControlAdder>().add(), ValidationException);
    SecondaryVoltageControlAdder adder = network.newExtension<SecondaryVoltageControlAdder>();
    ControlZoneAdder noUnit = adder.newControlZone();
    noUnit.withName("z").newPilotPoint().withBusbarSectionsOrBusesIds({"B1"}).withTargetV(400).add();
    EXPECT_THROW(noUnit.add(), ValidationException);
    EXPECT_THROW(noUnit.newPilotPoint().withTargetV(400).add(), ValidationException); // no bus
}

void checkLineCouplings(Network& network) {
    LineCouplings lineCouplings = network.getExtension<LineCouplings>();
    EXPECT_EQ(couplingsOf(lineCouplings), (Ids{"L12-L13", "L23-L12"}));
    std::vector<MutualCoupling> couplings = lineCouplings.getMutualCouplings();
    EXPECT_EQ(couplings[0].getR(), 0.1);
    EXPECT_EQ(couplings[0].getX(), 0.5);
    EXPECT_EQ(couplings[0].getLine1Segment(), LineSegment::FULL_LINE);
    EXPECT_EQ(couplings[1].getLine1Segment(), LineSegment(0.1, 0.6));
    EXPECT_EQ(couplings[1].getLine2Segment(), LineSegment(0.25, 1));

    // Found in either order
    Line l12 = *network.getLine("L12");
    Line l13 = *network.getLine("L13");
    Line l23 = *network.getLine("L23");
    EXPECT_EQ(*lineCouplings.findMutualCoupling(l13, l12), couplings[0]);
    EXPECT_FALSE(lineCouplings.findMutualCoupling(l13, l23));

    // Changes: a segment at once (invalid: rejected)
    couplings[0].setR(0.15).setX(0.55).setLine1Segment(LineSegment(0.2, 0.8));
    MutualCoupling changed = *lineCouplings.findMutualCoupling(l12, l13);
    EXPECT_EQ(changed.getR(), 0.15);
    EXPECT_EQ(changed.getX(), 0.55);
    EXPECT_EQ(changed.getLine1Segment(), LineSegment(0.2, 0.8));
    EXPECT_THROW(changed.setLine2Segment(LineSegment(0.8, 0.2)), ValidationException);
    EXPECT_EQ(changed.getLine2Segment(), LineSegment::FULL_LINE);

    // Removed, added back
    EXPECT_TRUE(lineCouplings.removeMutualCoupling(couplings[0]));
    EXPECT_FALSE(lineCouplings.removeMutualCoupling(couplings[0]));
    EXPECT_FALSE(lineCouplings.removeMutualCoupling(l12, l13));
    EXPECT_TRUE(lineCouplings.removeMutualCoupling(l12, l23));
    EXPECT_TRUE(lineCouplings.getMutualCouplings().empty());
    EXPECT_EQ(couplingsOf(lineCouplings.add(couplings[0])), (Ids{"L12-L13"}));
    EXPECT_THROW(lineCouplings.add(couplings[0]), ValidationException);

    // New ones: two different lines, not coupled yet
    MutualCoupling added = lineCouplings.newMutualCoupling()
        .withLine1(l13).withLine2(l23).withR(0.3).withX(0.9).withLine2Segment(LineSegment(0.5, 1))
        .add();
    EXPECT_EQ(added.getLine1Segment(), LineSegment::FULL_LINE);
    EXPECT_EQ(added.getLine2Segment(), LineSegment(0.5, 1));
    EXPECT_EQ(couplingsOf(lineCouplings), (Ids{"L12-L13", "L13-L23"}));
    EXPECT_THROW(lineCouplings.newMutualCoupling().withLine1(l23).withLine2(l13).add(), ValidationException);
    EXPECT_THROW(lineCouplings.newMutualCoupling().withLine1(l23).withLine2(l23).add(), ValidationException);
    EXPECT_THROW(lineCouplings.newMutualCoupling().withLine1(l12).withLine2(l23).withLine1Segment(LineSegment(-1, 1)),
                 ValidationException);

    // A line removed: its couplings too
    l13.remove();
    EXPECT_TRUE(lineCouplings.getMutualCouplings().empty());

    // Removed (powsybl then fails, see Network::removeLineCouplings); a new, empty one
    EXPECT_THROW(network.removeExtension<LineCouplings>(), PropertyNotFoundException);
    EXPECT_FALSE(network.hasExtension<LineCouplings>());
    EXPECT_TRUE(network.newExtension<LineCouplingsAdder>().add().getMutualCouplings().empty());
}

void checkNetwork(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/network_extensions_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/network_extensions_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    Network network = Identifiable(b.getNetworkHandle(), &b).getNetwork();
    checkSecondaryVoltageControl(network);
    checkLineCouplings(network);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(NetworkExtensionsNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkNetwork(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(NetworkExtensionsNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("network_extensions_test");
    checkNetwork(backend);
}
#endif
