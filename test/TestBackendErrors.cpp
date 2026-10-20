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

// Java-side failures must surface as C++ exceptions, with the same type on both backends,
// and leave the backend usable. Runs against the real backends (see RealBackends.h).

using namespace iidm;
using iidm::test::envPath;
using iidm::test::readFile;

namespace {

std::string readTestNetwork() {
    return readFile("data/backend_errors_test.xiidm");
}

// Same scenarios for every backend; the network is test/data/backend_errors_test.xiidm.
void checkBackendErrors(BackendProvider& b) {
    std::string xiidm = readTestNetwork();
    ASSERT_FALSE(xiidm.empty()) << "data/backend_errors_test.xiidm not found";
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");

    ObjectHandle g   = b.findById(prop::GENERATOR, "G");
    ObjectHandle g2  = b.findById(prop::GENERATOR, "G2");
    ObjectHandle vl1 = b.findById(prop::VOLTAGE_LEVEL, "VL1");
    ASSERT_NE(g, INVALID_HANDLE);
    ASSERT_NE(g2, INVALID_HANDLE);
    ASSERT_NE(vl1, INVALID_HANDLE);
    auto l2 = Load(b.findById(prop::LOAD, "L2"), &b);

    // Absent extension: hand-wired JNI path (droop) and dispatcher path (participation factor)
    Generator gen2(g2, &b);
    EXPECT_THROW(gen2.getActivePowerControl().getDroop(), ExtensionNotFoundException);
    EXPECT_THROW(gen2.getActivePowerControl().getParticipationFactor(), ExtensionNotFoundException);
    EXPECT_THROW(l2.getInjectionObservability().isObservable(), ExtensionNotFoundException);

    // Rejected values leave the network unchanged
    Generator gen(g, &b);
    EXPECT_THROW(gen.getActivePowerControl().setMaxTargetP(500.0), ValidationException);
    ASSERT_TRUE(gen.getActivePowerControl().getMaxTargetP().has_value());
    EXPECT_DOUBLE_EQ(*gen.getActivePowerControl().getMaxTargetP(), 150.0);

    SlackTerminal slack = VoltageLevel(vl1, &b).getSlackTerminal();
    EXPECT_THROW(slack.setTerminal(l2.getTerminal()), ValidationException);
    EXPECT_EQ(slack.getTerminal().getConnectableId(), "G");

    // Unknown codes, codes on the wrong type of object, invalid handles
    EXPECT_THROW(b.getDouble(g, 999999), PropertyNotFoundException);
    EXPECT_THROW(b.getDouble(g, prop::MEAS_VALUE), PropertyNotFoundException);
    EXPECT_THROW(b.getChildren(g, 999999), PropertyNotFoundException);
    EXPECT_THROW(b.getDouble(INVALID_HANDLE, prop::GEN_TARGET_P), InvalidHandleException);
    EXPECT_THROW(b.setRelated(INVALID_HANDLE, prop::REL_SLACK_TERMINAL, INVALID_HANDLE), InvalidHandleException);

    // Messages come from Java
    try {
        gen.getActivePowerControl().setMaxTargetP(500.0);
        FAIL() << "no exception";
    } catch (const ValidationException& e) {
        EXPECT_NE(std::string(e.what()).find("maxTargetP"), std::string::npos) << e.what();
    }

    // Still usable after all these failures
    EXPECT_DOUBLE_EQ(gen.getTargetP(), 100.0);
    gen.setTargetP(120.0);
    EXPECT_DOUBLE_EQ(gen.getTargetP(), 120.0);

    // Unreadable network: a C++ exception, not an abort
    const std::string garbage = "<not a network";
    EXPECT_THROW(b.loadNetworkBytes(garbage.data(), garbage.size(), "network.xiidm"), IidmException);
}

// Wrappers created and dropped in a loop must not accumulate Java-side references.
void checkHandlesAreReleased(BackendProvider& b) {
    std::string xiidm = readTestNetwork();
    b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    Generator gen(b.findById(prop::GENERATOR, "G"), &b);
    VoltageLevel vl(b.findById(prop::VOLTAGE_LEVEL, "VL1"), &b);
    const std::size_t baseline = b.liveHandleCount();
    double sum = 0.0;
    for (int i = 0; i < 20000; ++i) {
        sum += gen.getTerminal().getP();                       // new terminal handle each time
        sum += static_cast<double>(vl.getGenerators().size()); // new generator handles
        sum += gen.getActivePowerControl().getDroop();         // shares the generator handle
    }
    EXPECT_EQ(b.liveHandleCount(), baseline);
    (void)sum;
}

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(BackendErrorsTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkBackendErrors(backend);
}

TEST(BackendHandlesTest, GraalVM) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkHandlesAreReleased(backend);
    // The Java-side registry does not grow either (network + a few live wrappers)
    EXPECT_LT(backend.javaHandleCount(), 100);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
using iidm::test::ensureJvm;

TEST(BackendErrorsTest, JNI) {
    if (!ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("backend_errors_test");
    checkBackendErrors(backend);
}

TEST(BackendHandlesTest, JNI) {
    if (!ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    JNIBackend backend("backend_handles_test");
    checkHandlesAreReleased(backend);
}
#endif
