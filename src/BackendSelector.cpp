#include "BackendSelector.h"
#include <iidm/IidmException.h>
#include <cstdlib>
#include <string>

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
#include "graalvm/GraalVMBackend.h"
#endif

#ifdef IIDM_BRIDGE_JNI_ENABLED
#include "jni/JNIBackend.h"
#include <jni.h>
#endif

namespace iidm {

std::unique_ptr<BackendProvider> selectBackend(const NetworkLoadOptions& opts) {
    // Explicit override via environment variable
    const char* envMode = std::getenv("IIDM_BRIDGE_MODE");
    BackendMode effectiveMode = opts.mode;
    if (envMode) {
        std::string envStr(envMode);
        if (envStr == "graalvm") effectiveMode = BackendMode::GRAALVM;
        else if (envStr == "jni") effectiveMode = BackendMode::JNI;
    }

#ifdef IIDM_BRIDGE_JNI_ENABLED
    if (effectiveMode == BackendMode::JNI) {
        return std::make_unique<JNIBackend>(opts.jniNetworkId);
    }
#else
    if (effectiveMode == BackendMode::JNI) {
        throw BackendNotAvailableException(
            "JNI backend was not compiled in. Rebuild with IIDM_BRIDGE_ENABLE_JNI=ON");
    }
#endif

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
    if (effectiveMode == BackendMode::GRAALVM) {
        return std::make_unique<GraalVMBackend>(opts.graalvmLibPath);
    }
#else
    if (effectiveMode == BackendMode::GRAALVM) {
        throw BackendNotAvailableException(
            "GraalVM backend was not compiled in. Rebuild with IIDM_BRIDGE_ENABLE_GRAALVM=ON");
    }
#endif

    // AUTO: probe for existing JVM
#ifdef IIDM_BRIDGE_JNI_ENABLED
    {
        JavaVM* jvms[1];
        jsize   count = 0;
        if (JNI_GetCreatedJavaVMs(jvms, 1, &count) == JNI_OK && count > 0) {
            return std::make_unique<JNIBackend>(opts.jniNetworkId);
        }
    }
#endif

#ifdef IIDM_BRIDGE_GRAALVM_ENABLED
    return std::make_unique<GraalVMBackend>(opts.graalvmLibPath);
#else
    throw BackendNotAvailableException(
        "No backend available. Rebuild with IIDM_BRIDGE_ENABLE_GRAALVM=ON or IIDM_BRIDGE_ENABLE_JNI=ON");
#endif
}

} // namespace iidm
