#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#  include "graalvm/GraalVMBackend.h"
#endif
#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include "jni/JNIBackend.h"
#  include <jni.h>
#endif

// Java-side failures must surface as C++ exceptions, with the same type on both backends,
// and leave the backend usable. Runs against the real backends:
//   - GraalVM: IIDM_BRIDGE_GRAALVM_LIB = path to libpowsybl-iidm-native
//   - JNI:     IIDM_BRIDGE_JAR         = path to iidm-bridge-java-*-fat.jar (a JVM is created)
// Each backend is skipped when its variable is not set.

using namespace iidm;

namespace {

std::string readTestNetwork() {
    std::ifstream in("data/backend_errors_test.xiidm", std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
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

} // namespace

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
TEST(BackendErrorsTest, GraalVM) {
    const char* lib = std::getenv("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib || !lib[0]) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB not set";
    GraalVMBackend backend(lib);
    checkBackendErrors(backend);
}
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
TEST(BackendErrorsTest, JNI) {
    const char* jar = std::getenv("IIDM_BRIDGE_JAR");
    if (!jar || !jar[0]) GTEST_SKIP() << "IIDM_BRIDGE_JAR not set";

    JavaVM* jvm = nullptr;
    jsize count = 0;
    if (JNI_GetCreatedJavaVMs(&jvm, 1, &count) != JNI_OK || count == 0) {
        std::string classPath = std::string("-Djava.class.path=") + jar;
        std::vector<JavaVMOption> options(1);
        options[0].optionString = const_cast<char*>(classPath.c_str());
        options[0].extraInfo = nullptr;
        JavaVMInitArgs args;
        args.version = JNI_VERSION_1_8;
        args.nOptions = static_cast<jint>(options.size());
        args.options = options.data();
        args.ignoreUnrecognized = JNI_FALSE;
        JNIEnv* env = nullptr;
        ASSERT_EQ(JNI_CreateJavaVM(&jvm, reinterpret_cast<void**>(&env), &args), JNI_OK);
    }
    JNIBackend backend("backend_errors_test");
    checkBackendErrors(backend);
}
#endif
