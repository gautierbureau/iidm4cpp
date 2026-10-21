#pragma once

#include <jni.h>

namespace iidm { namespace detail {

// The JVM running in this process, or null. The library does not link libjvm: a JVM that runs in
// the process has loaded it, and its JNI_GetCreatedJavaVMs is looked up there.
JavaVM* createdJavaVM();

} } // namespace iidm::detail
