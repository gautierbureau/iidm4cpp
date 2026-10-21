#include "JavaVMs.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif

namespace iidm { namespace detail {

namespace {

using GetCreatedJavaVMs = jint(JNICALL*)(JavaVM**, jsize, jsize*);

// JNI_GetCreatedJavaVMs of the libjvm already loaded, or null
GetCreatedJavaVMs findGetCreatedJavaVMs() {
#ifdef _WIN32
    HMODULE jvm = GetModuleHandleA("jvm.dll");
    return jvm ? reinterpret_cast<GetCreatedJavaVMs>(GetProcAddress(jvm, "JNI_GetCreatedJavaVMs")) : nullptr;
#else
    // libjvm linked to the program, or loaded by the java launcher (RTLD_GLOBAL)
    if (void* symbol = dlsym(RTLD_DEFAULT, "JNI_GetCreatedJavaVMs")) {
        return reinterpret_cast<GetCreatedJavaVMs>(symbol);
    }
    // Loaded with RTLD_LOCAL: found by its name, without loading it
#  ifdef __APPLE__
    void* jvm = dlopen("libjvm.dylib", RTLD_LAZY | RTLD_NOLOAD);
#  else
    void* jvm = dlopen("libjvm.so", RTLD_LAZY | RTLD_NOLOAD);
#  endif
    if (!jvm) return nullptr;
    void* symbol = dlsym(jvm, "JNI_GetCreatedJavaVMs");
    dlclose(jvm); // only the reference RTLD_NOLOAD took: the JVM keeps it loaded
    return reinterpret_cast<GetCreatedJavaVMs>(symbol);
#endif
}

} // namespace

JavaVM* createdJavaVM() {
    GetCreatedJavaVMs getCreatedJavaVMs = findGetCreatedJavaVMs();
    JavaVM* jvm = nullptr;
    jsize count = 0;
    if (!getCreatedJavaVMs || getCreatedJavaVMs(&jvm, 1, &count) != JNI_OK || count == 0) return nullptr;
    return jvm;
}

} } // namespace iidm::detail
