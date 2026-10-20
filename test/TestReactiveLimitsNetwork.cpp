#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "RealBackends.h"
#include <string>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif
#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include "jni/JNIBackend.h"
#endif

// Reactive limits on the real backends (see RealBackends.h): the same scenario on GraalVM and on
// JNI, with the network test/data/reactive_limits_test.xiidm.

using namespace iidm;
using iidm::test::envPath;

namespace {

void checkCurve(BackendProvider& b) {
    Generator g(b.findById(prop::GENERATOR, "G_CURVE"), &b);
    ReactiveLimits limits = g.getReactiveLimits();
    EXPECT_EQ(limits.getKind(), ReactiveLimitsKind::CURVE);
    EXPECT_TRUE(limits.is<ReactiveCapabilityCurve>());
    EXPECT_FALSE(limits.is<MinMaxReactiveLimits>());
    EXPECT_EQ(*limits.getProperty("source"), "datasheet");
    EXPECT_THROW(g.getReactiveLimits<MinMaxReactiveLimits>(), ValidationException);
    EXPECT_THROW(limits.as<MinMaxReactiveLimits>(), PropertyNotFoundException);

    ReactiveCapabilityCurve curve = g.getReactiveLimits<ReactiveCapabilityCurve>();
    EXPECT_EQ(curve, limits);
    EXPECT_EQ(curve.getPointCount(), 3);
    EXPECT_DOUBLE_EQ(curve.getMinP(), 0.0);
    EXPECT_DOUBLE_EQ(curve.getMaxP(), 200.0);
    std::vector<ReactiveCapabilityCurve::Point> points = curve.getPoints();
    ASSERT_EQ(points.size(), 3u);
    EXPECT_DOUBLE_EQ(points[1].getP(), 100.0);
    EXPECT_DOUBLE_EQ(points[1].getMinQ(), -50.0);
    EXPECT_DOUBLE_EQ(points[1].getMaxQ(), 80.0);
    EXPECT_EQ(*points[0].getProperty("note"), "p0");

    // Interpolated between points; beyond the curve, the last point or its slope
    EXPECT_DOUBLE_EQ(limits.getMinQ(50.0), -55.0);
    EXPECT_DOUBLE_EQ(curve.getMaxQ(150.0), 60.0);
    EXPECT_DOUBLE_EQ(curve.getMaxQ(300.0, false), 40.0);
    EXPECT_NE(curve.getMaxQ(300.0, true), 40.0);

    Battery bat(b.findById(prop::BATTERY, "BAT"), &b);
    EXPECT_DOUBLE_EQ(bat.getReactiveLimits<ReactiveCapabilityCurve>().getMinQ(0.0), -22.5);
}

void checkMinMax(BackendProvider& b) {
    Generator g(b.findById(prop::GENERATOR, "G_MINMAX"), &b);
    MinMaxReactiveLimits limits = g.getReactiveLimits().as<MinMaxReactiveLimits>();
    EXPECT_EQ(limits.getKind(), ReactiveLimitsKind::MIN_MAX);
    EXPECT_DOUBLE_EQ(limits.getMinQ(), -30.0);
    EXPECT_DOUBLE_EQ(limits.getMaxQ(), 35.0);
    EXPECT_DOUBLE_EQ(limits.getMinQ(1000.0), -30.0); // whatever the active power

    VscConverterStation vsc(b.findById(prop::VSC_CONVERTER_STATION, "VSC"), &b);
    EXPECT_DOUBLE_EQ(vsc.getReactiveLimits<MinMaxReactiveLimits>().getMaxQ(), 150.0);
}

// Reading reactive limits leaves no Java reference behind
void checkHandlesAreReleased(BackendProvider& b) {
    Generator g(b.findById(prop::GENERATOR, "G_CURVE"), &b);
    const std::size_t baseline = b.liveHandleCount();
    double sum = 0;
    for (int i = 0; i < 500; ++i) {
        ReactiveCapabilityCurve curve = g.getReactiveLimits().as<ReactiveCapabilityCurve>();
        for (const ReactiveCapabilityCurve::Point& point : curve.getPoints()) sum += point.getMaxQ();
        sum += curve.getMinQ(10.0 * i, true);
    }
    EXPECT_EQ(b.liveHandleCount(), baseline);
    EXPECT_NE(sum, 0.0);
}

void checkReactiveLimits(BackendProvider& b) {
    std::string xiidm = iidm::test::readFile("data/reactive_limits_test.xiidm");
    ASSERT_FALSE(xiidm.empty()) << "data/reactive_limits_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    checkCurve(b);
    checkMinMax(b);
    checkHandlesAreReleased(b);
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(ReactiveLimitsNetworkTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkReactiveLimits(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(ReactiveLimitsNetworkTest, JNI) {
    if (!iidm::test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("reactive_limits_test");
    checkReactiveLimits(backend);
}
#endif
