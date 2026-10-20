#pragma once

// Helpers for the tests that run against the real backends instead of MockBackend:
//   - GraalVM: IIDM_BRIDGE_GRAALVM_LIB = path to libpowsybl-iidm-native
//   - JNI:     IIDM_BRIDGE_JAR         = path to iidm-bridge-java-*-fat.jar (a JVM is created)
// Each backend is skipped when its variable is not set.

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifdef IIDM_BRIDGE_JNI_ENABLED
#  include <jni.h>
#endif

namespace iidm {
namespace test {

// Value of an environment variable, nullptr when unset or empty.
inline const char* envPath(const char* name) {
    const char* value = std::getenv(name);
    return value && value[0] ? value : nullptr;
}

// Whole file content; empty when the file cannot be read. Relative paths start from test/.
inline std::string readFile(const std::string& path) {
    std::ifstream in(path.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

#ifdef IIDM_BRIDGE_JNI_ENABLED
// Creates the JVM once per process (class path from IIDM_BRIDGE_JAR); false when not configured.
inline bool ensureJvm() {
    const char* jar = envPath("IIDM_BRIDGE_JAR");
    if (!jar) return false;
    JavaVM* jvm = nullptr;
    jsize count = 0;
    if (JNI_GetCreatedJavaVMs(&jvm, 1, &count) == JNI_OK && count > 0) return true;
    std::string classPath = std::string("-Djava.class.path=") + jar;
    std::vector<JavaVMOption> options(2);
    options[0].optionString = const_cast<char*>(classPath.c_str());
    options[0].extraInfo = nullptr;
    options[1].optionString = const_cast<char*>("-Xcheck:jni"); // catches invalid JNI reference use
    options[1].extraInfo = nullptr;
    JavaVMInitArgs args;
    args.version = JNI_VERSION_1_8;
    args.nOptions = static_cast<jint>(options.size());
    args.options = options.data();
    args.ignoreUnrecognized = JNI_FALSE;
    JNIEnv* env = nullptr;
    return JNI_CreateJavaVM(&jvm, reinterpret_cast<void**>(&env), &args) == JNI_OK;
}
#endif

} // namespace test
} // namespace iidm
