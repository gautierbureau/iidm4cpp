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

// Lines and HVDC lines on the real backends (see RealBackends.h): the same scenario on GraalVM
// and on JNI, with the networks examples/IEEE14.xiidm (lines) and examples/extensions_test.xiidm
// (HVDC1, between VSC1 and VSC2).

using namespace iidm;
using iidm::test::envPath;

namespace {

bool load(BackendProvider& b, const char* path) {
    std::string xiidm = iidm::test::readFile(path);
    if (xiidm.empty()) return false;
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    return true;
}

void checkLines(BackendProvider& b) {
    ASSERT_TRUE(load(b, "../examples/IEEE14.xiidm"));
    Line line(b.findById(prop::LINE, "_BUS___10-BUS___11-1_AC"), &b);
    EXPECT_DOUBLE_EQ(line.getR(), 0.156256);
    line.setR(0.2).setX(0.4).setG1(1e-6).setB1(2e-5).setG2(3e-6).setB2(4e-5);
    EXPECT_DOUBLE_EQ(line.getR(), 0.2);
    EXPECT_DOUBLE_EQ(line.getX(), 0.4);
    EXPECT_DOUBLE_EQ(line.getG1(), 1e-6);
    EXPECT_DOUBLE_EQ(line.getB1(), 2e-5);
    EXPECT_DOUBLE_EQ(line.getG2(), 3e-6);
    EXPECT_DOUBLE_EQ(line.getB2(), 4e-5);
    EXPECT_THROW(line.setX(NAN), ValidationException);

    TwoWindingsTransformer twt(b.findById(prop::TWO_WINDINGS_TRANSFORMER, "_BUS____4-BUS____9-1_PT"), &b);
    ASSERT_TRUE(twt.getSubstation());
    EXPECT_EQ(twt.getSubstation()->getId(), "_BUS____4_SS");
}

void checkHvdcLine(BackendProvider& b) {
    ASSERT_TRUE(load(b, "../examples/extensions_test.xiidm"));
    HvdcLine hvdc(b.findById(prop::HVDC_LINE, "HVDC1"), &b);
    hvdc.setR(1.5).setNominalV(320).setMaxP(600).setActivePowerSetpoint(550);
    EXPECT_DOUBLE_EQ(hvdc.getR(), 1.5);
    EXPECT_DOUBLE_EQ(hvdc.getNominalV(), 320.0);
    EXPECT_DOUBLE_EQ(hvdc.getMaxP(), 600.0);
    EXPECT_THROW(hvdc.setMaxP(-1), ValidationException);

    HvdcConverterStation station1 = hvdc.getConverterStation(TwoSides::ONE);
    EXPECT_EQ(station1.getId(), "VSC1");
    EXPECT_EQ(station1, hvdc.getConverterStation1());
    EXPECT_EQ(hvdc.getConverterStation(TwoSides::TWO), hvdc.getConverterStation2());

    // Both stations disconnected, then connected again
    EXPECT_TRUE(hvdc.disconnectConverterStations());
    EXPECT_FALSE(hvdc.getConverterStation1().getTerminal().isConnected());
    EXPECT_FALSE(hvdc.getConverterStation2().getTerminal().isConnected());
    EXPECT_FALSE(hvdc.disconnectConverterStations()); // already
    EXPECT_TRUE(hvdc.connectConverterStations());
    EXPECT_TRUE(hvdc.getConverterStation2().getTerminal().isConnected());

    // One side (with a switch predicate, that bus-breaker topology has no use for)
    EXPECT_TRUE(hvdc.disconnectConverterStations(SwitchPredicate::NONFICTIONAL_CLOSED_BREAKER, TwoSides::TWO));
    EXPECT_TRUE(hvdc.getConverterStation1().getTerminal().isConnected());
    EXPECT_FALSE(hvdc.getConverterStation2().getTerminal().isConnected());
    EXPECT_TRUE(hvdc.connectConverterStations(SwitchPredicate::NONFICTIONAL_BREAKER));
    EXPECT_TRUE(hvdc.getConverterStation2().getTerminal().isConnected());
}

void checkLinesAndHvdc(BackendProvider& b) {
    checkLines(b);
    checkHvdcLine(b);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(LinesNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkLinesAndHvdc(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(LinesNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("lines_test");
    checkLinesAndHvdc(backend);
}
#endif
