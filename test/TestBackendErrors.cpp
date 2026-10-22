#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include "RealBackends.h"
#include <memory>
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

    // Absent extension: the C++ getter throws; the extension's codes do not apply to its host
    Generator gen2(g2, &b);
    EXPECT_FALSE(gen2.hasExtension<ActivePowerControl>());
    EXPECT_THROW(gen2.getExtension<ActivePowerControl>(), ExtensionNotFoundException);
    EXPECT_THROW(l2.getExtension<InjectionObservability>(), ExtensionNotFoundException);
    EXPECT_THROW(b.getDouble(g2, prop::EXT_APC_DROOP), PropertyNotFoundException);

    // Rejected values leave the network unchanged
    Generator gen(g, &b);
    EXPECT_THROW(gen.getExtension<ActivePowerControl>().setMaxTargetP(500.0), ValidationException);
    ASSERT_TRUE(gen.getExtension<ActivePowerControl>().getMaxTargetP());
    EXPECT_DOUBLE_EQ(*gen.getExtension<ActivePowerControl>().getMaxTargetP(), 150.0);

    SlackTerminal slack = VoltageLevel(vl1, &b).getExtension<SlackTerminal>();
    EXPECT_THROW(slack.setTerminal(l2.getTerminal()), ValidationException);
    EXPECT_EQ(slack.getTerminal()->getConnectable().getId(), "G");

    // An extension by its class: empty when absent, named in the message when it must be there
    EXPECT_FALSE(gen2.findExtension<ActivePowerControl>());
    ASSERT_TRUE(gen.findExtension<ActivePowerControl>());
    EXPECT_DOUBLE_EQ(*gen.findExtension<ActivePowerControl>()->getMaxTargetP(), 150.0);
    try {
        gen2.getExtension<ActivePowerControl>();
        FAIL() << "no exception";
    } catch (const ExtensionNotFoundException& e) {
        EXPECT_EQ(std::string(e.what()), "ActivePowerControl extension not present on G2");
    }
    // A host that powsybl does not allow for the extension never has it, and cannot take one
    // (powsybl's adder casts the host: a ClassCastException)
    EXPECT_FALSE(l2.hasExtension<GeneratorShortCircuit>());
    EXPECT_THROW(l2.newExtension<GeneratorShortCircuitAdder>(), PropertyNotFoundException);

    // Unknown codes, codes on the wrong type of object, invalid handles
    EXPECT_THROW(b.getDouble(g, 999999), PropertyNotFoundException);
    EXPECT_THROW(b.getDouble(g, prop::MEAS_VALUE), PropertyNotFoundException);
    EXPECT_THROW(b.getChildren(g, 999999), PropertyNotFoundException);
    EXPECT_THROW(b.getDouble(INVALID_HANDLE, prop::GEN_TARGET_P), InvalidHandleException);
    EXPECT_THROW(b.setRelated(INVALID_HANDLE, prop::REL_TERMINAL, INVALID_HANDLE), InvalidHandleException);

    // Messages come from Java
    try {
        gen.getExtension<ActivePowerControl>().setMaxTargetP(500.0);
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
        sum += gen.getExtension<ActivePowerControl>().getDroop();         // new extension handle
    }
    EXPECT_EQ(b.liveHandleCount(), baseline);
    (void)sum;
}

// Objects used after their network is destroyed throw, rather than reach a freed backend
void checkObjectsOutliveTheirNetwork(std::unique_ptr<BackendProvider> backend) {
    std::string xiidm = readTestNetwork();
    backend->loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm");
    Generator gen;
    {
        Network network = NetworkFactory::createFromBackend(std::move(backend));
        gen = *network.getGenerator("G");
        EXPECT_EQ(gen.getId(), "G");
    }
    EXPECT_TRUE(gen.isValid());
    EXPECT_THROW(gen.getTargetP(), InvalidHandleException);
    EXPECT_THROW(gen.getTerminal(), InvalidHandleException);
    EXPECT_THROW(gen.setTargetP(1), InvalidHandleException);
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

TEST(BackendHandlesTest, GraalVMObjectsOutliveTheirNetwork) {
    const char* lib = envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    checkObjectsOutliveTheirNetwork(std::unique_ptr<BackendProvider>(new GraalVMBackend(lib)));
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

TEST(BackendHandlesTest, JNIObjectsOutliveTheirNetwork) {
    if (!ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";
    checkObjectsOutliveTheirNetwork(std::unique_ptr<BackendProvider>(new JNIBackend("backend_outlived_test")));
}
#endif
