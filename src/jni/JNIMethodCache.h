#pragma once

#include <jni.h>

namespace iidm {

// JNI classes and method ids resolved once by JNIBackend::cacheMethodIds().
struct JNIMethodCache {
    // ── IidmBridgeRegistry (network registration, load / save) ───────────
    jclass    iidmRegistryClass      = nullptr;
    jmethodID registry_get           = nullptr;
    jmethodID registry_save          = nullptr;
    jmethodID registry_loadFromBytes = nullptr;
    jmethodID registry_saveToBytes   = nullptr;

    // ── PropertyDispatcher (object API): every property, child and relation ──
    // The same dispatch table as the GraalVM backend.
    jclass    dispatcherClass               = nullptr;
    jmethodID disp_getDouble                = nullptr;
    jmethodID disp_setDouble                = nullptr;
    jmethodID disp_getInt                   = nullptr;
    jmethodID disp_setInt                   = nullptr;
    jmethodID disp_getBool                  = nullptr;
    jmethodID disp_setBool                  = nullptr;
    jmethodID disp_getString                = nullptr;
    jmethodID disp_setString                = nullptr;
    jmethodID disp_getChildObjects          = nullptr;
    jmethodID disp_getRelatedObject         = nullptr;
    jmethodID disp_setRelatedObject         = nullptr;
    jmethodID disp_getRelatedObjectByIndex  = nullptr;
    jmethodID disp_getRelatedObjectByString = nullptr;
    jmethodID disp_getIntList               = nullptr;
    jmethodID disp_findObjectById           = nullptr;
};

} // namespace iidm
