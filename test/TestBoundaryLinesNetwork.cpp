#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "RealBackends.h"
#include <cmath>
#include <string>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif
#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include "jni/JNIBackend.h"
#endif

// Boundary lines on the real backends (see RealBackends.h): the same scenario on GraalVM and on
// JNI, with the network test/data/boundary_lines_test.xiidm: BL_GEN (a generation, a pairing
// key, limits), BL_PLAIN (neither), BL_HALF1 and BL_HALF2 (paired in the tie line TIE); grounds.

using namespace iidm;
using iidm::test::envPath;

namespace {

BoundaryLine boundaryLine(BackendProvider& b, const char* id) {
    return BoundaryLine(b.findById(prop::BOUNDARY_LINE, id), &b);
}

void checkReading(BackendProvider& b) {
    BoundaryLine line = boundaryLine(b, "BL_GEN");
    EXPECT_DOUBLE_EQ(line.getP0(), 10.0);
    EXPECT_DOUBLE_EQ(line.getX(), 10.0);
    EXPECT_DOUBLE_EQ(line.getB(), 1e-5);
    EXPECT_FALSE(line.isPaired());
    EXPECT_EQ(line.getPairingKey(), iidm::optional<std::string>("XNODE_GEN"));
    EXPECT_DOUBLE_EQ(line.getCurrentLimits()->getPermanentLimit(), 500.0);

    ASSERT_TRUE(line.getGeneration());
    BoundaryLine::Generation generation = *line.getGeneration();
    EXPECT_DOUBLE_EQ(generation.getTargetP(), 20.0);
    EXPECT_DOUBLE_EQ(generation.getMinP(), 0.0);
    EXPECT_DOUBLE_EQ(generation.getMaxP(), 100.0);
    EXPECT_DOUBLE_EQ(generation.getTargetQ(), 5.0);
    EXPECT_DOUBLE_EQ(generation.getTargetV(), 401.0);
    EXPECT_TRUE(generation.isVoltageRegulationOn());
    EXPECT_EQ(generation.getReactiveLimits().getKind(), ReactiveLimitsKind::MIN_MAX);
    EXPECT_DOUBLE_EQ(generation.getReactiveLimits<MinMaxReactiveLimits>().getMinQ(), -50.0);
    EXPECT_EQ(generation, *line.getGeneration());

    // The far end: computed from the network side
    Boundary boundary = line.getBoundary();
    EXPECT_EQ(boundary, line.getBoundary());
    EXPECT_EQ(boundary.getBoundaryLine(), line);
    EXPECT_EQ(boundary.getNetworkSideVoltageLevel().getId(), "VL1");
    EXPECT_FALSE(std::isnan(boundary.getV()));
    EXPECT_FALSE(std::isnan(boundary.getP()));
    EXPECT_GT(boundary.getI(), 0.0);
    EXPECT_NE(boundary.getV(), 402.0); // not the voltage at the network side

    BoundaryLine plain = boundaryLine(b, "BL_PLAIN");
    EXPECT_FALSE(plain.getGeneration());
    EXPECT_FALSE(plain.getPairingKey());
    EXPECT_TRUE(boundaryLine(b, "BL_HALF1").isPaired());
    EXPECT_EQ(boundaryLine(b, "BL_HALF2").getPairingKey(), iidm::optional<std::string>("XNODE_TIE"));
}

void checkWriting(BackendProvider& b) {
    BoundaryLine line = boundaryLine(b, "BL_GEN");
    line.setR(2).setX(20).setG(2e-6).setB(2e-5).setP0(12).setQ0(6).setPairingKey("XNODE_NEW");
    EXPECT_DOUBLE_EQ(line.getR(), 2.0);
    EXPECT_DOUBLE_EQ(line.getG(), 2e-6);
    EXPECT_EQ(line.getPairingKey(), iidm::optional<std::string>("XNODE_NEW"));
    EXPECT_THROW(line.setX(NAN), ValidationException);

    BoundaryLine::Generation generation = *line.getGeneration();
    generation.setMinP(-10).setMaxP(90).setTargetP(30).setTargetQ(4).setTargetV(399).setVoltageRegulationOn(false);
    EXPECT_DOUBLE_EQ(line.getGeneration()->getTargetP(), 30.0);
    EXPECT_DOUBLE_EQ(line.getGeneration()->getMinP(), -10.0);
    EXPECT_FALSE(line.getGeneration()->isVoltageRegulationOn());
    EXPECT_THROW(generation.setTargetV(NAN).setVoltageRegulationOn(true), ValidationException); // regulating without target

    // Targets from the solved values: -p, -q at the terminal, the bus voltage
    generation.setTargetV(400).setVoltageRegulationOn(true);
    line.setGenerationTargetPToP();
    EXPECT_DOUBLE_EQ(generation.getTargetP(), 12.0);
    line.setGenerationTargetQToQ();
    EXPECT_DOUBLE_EQ(generation.getTargetQ(), 6.0);
    line.setGenerationTargetVToV();
    EXPECT_DOUBLE_EQ(generation.getTargetV(), 402.0);
    line.getTerminal().setP(-15).setQ(-7);
    line.applySolvedValues();
    EXPECT_DOUBLE_EQ(generation.getTargetP(), 15.0);
    EXPECT_DOUBLE_EQ(generation.getTargetQ(), 7.0);
    line.unsetSolvedValues();
    EXPECT_TRUE(std::isnan(line.getTerminal().getP()));
}

// Reading the generations and boundaries leaves no Java reference behind
void checkHandlesAreReleased(BackendProvider& b) {
    BoundaryLine line = boundaryLine(b, "BL_GEN");
    const std::size_t baseline = b.liveHandleCount();
    double sum = 0;
    for (int i = 0; i < 300; ++i) {
        sum += line.getGeneration()->getTargetP() + line.getBoundary().getP();
        sum += line.getGeneration()->getReactiveLimits().getMaxQ(0);
        sum += line.getBoundary().getNetworkSideVoltageLevel().getNominalV();
    }
    EXPECT_EQ(b.liveHandleCount(), baseline);
    EXPECT_NE(sum, 0.0);
}

// The tie line TIE (r and x: those of its boundary lines, added), then a ground
void checkTieLinesAndGrounds(BackendProvider& b) {
    Network network = boundaryLine(b, "BL_HALF1").getNetwork();
    ASSERT_EQ(network.getTieLineCount(), 1);
    TieLine tie = *network.getTieLine("TIE");
    EXPECT_EQ(network.getTieLines()[0], tie);
    EXPECT_EQ(tie.getPairingKey(), iidm::optional<std::string>("XNODE_TIE"));
    EXPECT_EQ(tie.getBoundaryLine1().getId(), "BL_HALF1");
    EXPECT_EQ(tie.getBoundaryLine(TwoSides::TWO).getId(), "BL_HALF2");
    EXPECT_EQ(tie.getBoundaryLine("VL2")->getId(), "BL_HALF2");
    EXPECT_FALSE(tie.getBoundaryLine("NONE"));
    EXPECT_EQ(*boundaryLine(b, "BL_HALF1").getTieLine(), tie);
    EXPECT_FALSE(boundaryLine(b, "BL_PLAIN").getTieLine());
    EXPECT_NEAR(tie.getR(), 2.0, 1e-9);
    EXPECT_NEAR(tie.getX(), 20.0, 1e-9);

    // A branch: the terminals of its boundary lines
    EXPECT_EQ(tie.getTerminal1(), boundaryLine(b, "BL_HALF1").getTerminal());
    Branch branch = tie;
    EXPECT_EQ(*network.getBranch("TIE"), branch);
    EXPECT_TRUE(branch.is<TieLine>());
    EXPECT_EQ(tie.getType(), IdentifiableType::TIE_LINE);
    EXPECT_FALSE(tie.isOverloaded()); // no limits

    EXPECT_TRUE(tie.disconnectBoundaryLines());
    EXPECT_FALSE(boundaryLine(b, "BL_HALF2").getTerminal().isConnected());
    EXPECT_TRUE(tie.connectBoundaryLines());
    EXPECT_FALSE(tie.connectBoundaryLines()); // already
    // One side (with a switch predicate, that bus-breaker topology has no use for)
    EXPECT_TRUE(tie.disconnectBoundaryLines(SwitchPredicate::NONFICTIONAL_CLOSED_BREAKER, TwoSides::ONE));
    EXPECT_FALSE(boundaryLine(b, "BL_HALF1").getTerminal().isConnected());
    EXPECT_TRUE(boundaryLine(b, "BL_HALF2").getTerminal().isConnected());
    EXPECT_TRUE(tie.connectBoundaryLines(SwitchPredicate::NONFICTIONAL_BREAKER, TwoSides::ONE));
    EXPECT_TRUE(boundaryLine(b, "BL_HALF1").getTerminal().isConnected());

    // Removed, the flows at the boundary becoming the boundary lines' P0; then paired again
    BoundaryLine half1 = boundaryLine(b, "BL_HALF1");
    const double boundaryP = half1.getBoundary().getP();
    ASSERT_FALSE(std::isnan(boundaryP));
    tie.remove(true);
    EXPECT_FALSE(half1.isPaired());
    EXPECT_FALSE(network.getTieLine("TIE"));
    EXPECT_DOUBLE_EQ(half1.getP0(), -boundaryP);
    TieLine paired = network.newTieLine().setId("TIE2").setBoundaryLine1("BL_HALF1").setBoundaryLine2("BL_HALF2").add();
    EXPECT_EQ(*half1.getTieLine(), paired);
    EXPECT_THROW(network.newTieLine().setId("TIE3").setBoundaryLine1("BL_HALF1").setBoundaryLine2("BL_GEN").add(),
                 ValidationException); // already paired
    paired.remove();
    EXPECT_EQ(network.getTieLineCount(), 0);

    // A ground
    VoltageLevel vl1(b.findById(prop::VOLTAGE_LEVEL, "VL1"), &b);
    Ground ground = vl1.newGround().setId("GROUND").setBus("B1").add();
    EXPECT_EQ(vl1.getGroundCount(), 1);
    EXPECT_EQ(vl1.getGrounds()[0], ground);
    EXPECT_EQ(*network.getGround("GROUND"), ground);
    EXPECT_EQ(network.getGrounds().size(), 1u);
    EXPECT_EQ(ground.getType(), IdentifiableType::GROUND);
    EXPECT_TRUE(ground.getTerminal().isConnected());
    ground.remove();
    EXPECT_EQ(network.getGroundCount(), 0);
}

void load(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/boundary_lines_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/boundary_lines_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
}

void checkBoundaryLines(BackendProvider& b) {
    load(b);
    checkReading(b);
    checkHandlesAreReleased(b);
    checkWriting(b);
    load(b);
    checkTieLinesAndGrounds(b);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(BoundaryLinesNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkBoundaryLines(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(BoundaryLinesNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("boundary_lines_test");
    checkBoundaryLines(backend);
}
#endif
