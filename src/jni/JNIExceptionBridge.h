#pragma once

#include <iidm/IidmException.h>
#include <jni.h>
#include <string>

namespace iidm {

// Checks if a Java exception is pending. If so, clears it and throws
// a C++ IidmException containing the Java exception's message.
inline void checkJNIException(JNIEnv* env) {
    if (env->ExceptionCheck()) {
        jthrowable exc = env->ExceptionOccurred();
        env->ExceptionClear();

        // Try to get the message from the exception
        jclass throwableClass = env->FindClass("java/lang/Throwable");
        jmethodID getMessage  = env->GetMethodID(throwableClass, "getMessage",
                                                  "()Ljava/lang/String;");
        jstring msg = static_cast<jstring>(env->CallObjectMethod(exc, getMessage));

        std::string cppMsg = "Java exception";
        if (msg) {
            const char* chars = env->GetStringUTFChars(msg, nullptr);
            if (chars) {
                cppMsg = chars;
                env->ReleaseStringUTFChars(msg, chars);
            }
            env->DeleteLocalRef(msg);
        }
        env->DeleteLocalRef(exc);
        env->DeleteLocalRef(throwableClass);

        throw IidmException(cppMsg);
    }
}

} // namespace iidm
