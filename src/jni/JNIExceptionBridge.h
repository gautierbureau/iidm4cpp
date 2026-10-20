#pragma once

#include "../BridgeErrors.h"
#include "JNIStrings.h"
#include <iidm/IidmException.h>
#include <jni.h>
#include <string>

namespace iidm {

// Checks if a Java exception is pending. If so, clears it and throws the C++ exception
// matching its kind (com.powsybl.iidmbridge.BridgeErrors — the classification shared with
// the GraalVM backend), with the Java exception's message.
inline void checkJNIException(JNIEnv* env) {
    if (!env->ExceptionCheck()) return;

    jthrowable exc = env->ExceptionOccurred();
    env->ExceptionClear();

    int kind = detail::BRIDGE_GENERIC;
    std::string message = "Java exception";
    jclass errorsClass = env->FindClass("com/powsybl/iidmbridge/BridgeErrors");
    if (errorsClass) {
        jmethodID kindOf  = env->GetStaticMethodID(errorsClass, "kindOf", "(Ljava/lang/Throwable;)I");
        jmethodID messageOf = env->GetStaticMethodID(errorsClass, "message",
                                                     "(Ljava/lang/Throwable;)Ljava/lang/String;");
        if (kindOf && messageOf) {
            kind = env->CallStaticIntMethod(errorsClass, kindOf, exc);
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                kind = detail::BRIDGE_GENERIC;
            }
            jstring msg = static_cast<jstring>(env->CallStaticObjectMethod(errorsClass, messageOf, exc));
            if (env->ExceptionCheck()) {
                env->ExceptionClear();
                msg = nullptr;
            }
            if (msg) {
                message = detail::javaString(env, msg);
                env->DeleteLocalRef(msg);
            }
        }
        env->DeleteLocalRef(errorsClass);
    }
    // Never leave an exception pending (e.g. BridgeErrors missing from the class path)
    if (env->ExceptionCheck()) env->ExceptionClear();
    env->DeleteLocalRef(exc);

    detail::throwBridgeError(kind, message);
}

} // namespace iidm
