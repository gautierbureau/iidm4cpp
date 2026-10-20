#include "JNIBackend.h"
#include "JNIExceptionBridge.h"
#include <iidm/IidmException.h>
#include <cmath>
#include <fstream>
#include <iterator>
#include <string>

namespace iidm {

// ── Lifecycle ─────────────────────────────────────────────────────────────────

JNIBackend::JNIBackend(const std::string& networkId)
    : networkId_(networkId.empty() ? "default" : networkId) {
    attachToJVM();
    cacheMethodIds();
    bindRegisteredNetwork();
}

JNIBackend::~JNIBackend() {
    close();
}

void JNIBackend::attachToJVM() {
    JavaVM* jvms[1];
    jsize   count = 0;
    jint rc = JNI_GetCreatedJavaVMs(jvms, 1, &count);
    if (rc != JNI_OK || count == 0) {
        throw BackendNotAvailableException("No JVM found; cannot use JNI backend");
    }
    jvm_ = jvms[0];

    JNIEnv* env = nullptr;
    rc = jvm_->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr);
    if (rc != JNI_OK) {
        throw IidmException("Failed to attach to JVM thread");
    }
    env_ = env;
}

void JNIBackend::cacheClass(jclass& out, const char* name) {
    jclass local = env_->FindClass(name);
    checkJNIException(env_);
    if (!local) throw IidmException(std::string("Cannot find class: ") + name);
    out = static_cast<jclass>(env_->NewGlobalRef(local));
    env_->DeleteLocalRef(local);
}

void JNIBackend::cacheMethodIds() {
    cacheClass(cache_.iidmRegistryClass, "com/powsybl/iidmbridge/jni/IidmBridgeRegistry");
    cache_.registry_get = env_->GetStaticMethodID(cache_.iidmRegistryClass,
        "get", "(Ljava/lang/String;)Lcom/powsybl/iidm/network/Network;");
    cache_.registry_save = env_->GetStaticMethodID(cache_.iidmRegistryClass,
        "save", "(Ljava/lang/String;Ljava/lang/String;)V");
    cache_.registry_loadFromBytes = env_->GetStaticMethodID(cache_.iidmRegistryClass,
        "loadFromBytes", "(Ljava/lang/String;[BLjava/lang/String;)V");
    cache_.registry_saveToBytes = env_->GetStaticMethodID(cache_.iidmRegistryClass,
        "saveToBytes", "(Ljava/lang/String;Ljava/lang/String;)[B");
    cache_.registry_createNetwork = env_->GetStaticMethodID(cache_.iidmRegistryClass,
        "createNetwork", "(Ljava/lang/String;Ljava/lang/String;)V");
    checkJNIException(env_);

    cacheClass(cache_.dispatcherClass, "com/powsybl/iidmbridge/PropertyDispatcher");
    jclass d = cache_.dispatcherClass;
    cache_.disp_getDouble  = env_->GetStaticMethodID(d, "getDouble",  "(Ljava/lang/Object;I)D");
    cache_.disp_setDouble  = env_->GetStaticMethodID(d, "setDouble",  "(Ljava/lang/Object;ID)V");
    cache_.disp_getInt     = env_->GetStaticMethodID(d, "getInt",     "(Ljava/lang/Object;I)I");
    cache_.disp_setInt     = env_->GetStaticMethodID(d, "setInt",     "(Ljava/lang/Object;II)V");
    cache_.disp_getBool    = env_->GetStaticMethodID(d, "getBool",    "(Ljava/lang/Object;I)Z");
    cache_.disp_setBool    = env_->GetStaticMethodID(d, "setBool",    "(Ljava/lang/Object;IZ)V");
    cache_.disp_getString  = env_->GetStaticMethodID(d, "getString",  "(Ljava/lang/Object;I)Ljava/lang/String;");
    cache_.disp_setString  = env_->GetStaticMethodID(d, "setString",  "(Ljava/lang/Object;ILjava/lang/String;)V");
    cache_.disp_getChildObjects = env_->GetStaticMethodID(d, "getChildObjects",
        "(Ljava/lang/Object;I)[Ljava/lang/Object;");
    cache_.disp_getRelatedObject = env_->GetStaticMethodID(d, "getRelatedObject",
        "(Ljava/lang/Object;I)Ljava/lang/Object;");
    cache_.disp_setRelatedObject = env_->GetStaticMethodID(d, "setRelatedObject",
        "(Ljava/lang/Object;ILjava/lang/Object;)V");
    cache_.disp_getRelatedObjectByIndex = env_->GetStaticMethodID(d, "getRelatedObjectByIndex",
        "(Ljava/lang/Object;II)Ljava/lang/Object;");
    cache_.disp_getRelatedObjectByString = env_->GetStaticMethodID(d, "getRelatedObjectByString",
        "(Ljava/lang/Object;ILjava/lang/String;)Ljava/lang/Object;");
    cache_.disp_getIntList = env_->GetStaticMethodID(d, "getIntList", "(Ljava/lang/Object;I)[I");
    cache_.disp_findObjectById = env_->GetStaticMethodID(d, "findObjectById",
        "(Ljava/lang/Object;ILjava/lang/String;)Ljava/lang/Object;");
    cache_.disp_invokeObject = env_->GetStaticMethodID(d, "invokeObject", "(Ljava/lang/Object;I)Ljava/lang/Object;");
    checkJNIException(env_);
}

void JNIBackend::bindRegisteredNetwork() {
    // Bind the network already registered under networkId_, if any (embedded mode).
    // Otherwise the network comes later from loadNetwork / loadNetworkBytes.
    jstring jId = env_->NewStringUTF(networkId_.c_str());
    jobject netObj = env_->CallStaticObjectMethod(cache_.iidmRegistryClass, cache_.registry_get, jId);
    env_->DeleteLocalRef(jId);
    if (env_->ExceptionCheck()) {
        env_->ExceptionClear(); // no network registered under this id yet
        return;
    }
    if (netObj) {
        networkRef_ = env_->NewGlobalRef(netObj);
        env_->DeleteLocalRef(netObj);
    }
}

void JNIBackend::loadNetwork(const std::string& filePath) {
    // Read the file in C++ and load it in the JVM, registered under networkId_.
    std::ifstream in(filePath, std::ios::binary);
    if (!in) throw IidmException("Cannot read network file: " + filePath);
    std::string bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    std::string::size_type slash = filePath.find_last_of("/\\");
    loadNetworkBytes(bytes.data(), bytes.size(),
                     slash == std::string::npos ? filePath : filePath.substr(slash + 1));
}

void JNIBackend::saveNetwork(const std::string& filePath) {
    jstring jId   = env_->NewStringUTF(networkId_.c_str());
    jstring jPath = env_->NewStringUTF(filePath.c_str());
    env_->CallStaticVoidMethod(cache_.iidmRegistryClass, cache_.registry_save, jId, jPath);
    env_->DeleteLocalRef(jId);
    env_->DeleteLocalRef(jPath);
    checkJNIException(env_);
}

void JNIBackend::loadNetworkBytes(const char* data, std::size_t length,
                                  const std::string& filenameHint) {
    jbyteArray arr = env_->NewByteArray(static_cast<jsize>(length));
    if (!arr) {
        checkJNIException(env_);
        throw IidmException("Failed to allocate Java byte[] for network load");
    }
    if (length > 0) {
        env_->SetByteArrayRegion(arr, 0, static_cast<jsize>(length),
                                 reinterpret_cast<const jbyte*>(data));
    }
    jstring jId   = env_->NewStringUTF(networkId_.c_str());
    jstring jHint = env_->NewStringUTF(filenameHint.c_str());
    env_->CallStaticVoidMethod(cache_.iidmRegistryClass,
                               cache_.registry_loadFromBytes, jId, arr, jHint);
    env_->DeleteLocalRef(jHint);
    env_->DeleteLocalRef(jId);
    env_->DeleteLocalRef(arr);
    checkJNIException(env_);
    rebindNetwork("loadFromBytes");
}

void JNIBackend::createNetwork(const std::string& id) {
    jstring jId        = env_->NewStringUTF(networkId_.c_str());
    jstring jNetworkId = env_->NewStringUTF(id.c_str());
    env_->CallStaticVoidMethod(cache_.iidmRegistryClass, cache_.registry_createNetwork, jId, jNetworkId);
    env_->DeleteLocalRef(jNetworkId);
    env_->DeleteLocalRef(jId);
    checkJNIException(env_);
    rebindNetwork("createNetwork");
}

void JNIBackend::rebindNetwork(const char* operation) {
    // The newly registered network replaces any previous reference held here.
    if (networkRef_) {
        env_->DeleteGlobalRef(networkRef_);
        networkRef_ = nullptr;
    }
    bindRegisteredNetwork();
    if (!networkRef_) {
        throw NetworkNotFoundException(std::string("Network missing after ") + operation + ": " + networkId_);
    }
}

std::string JNIBackend::saveNetworkBytes(const std::string& filenameHint) const {
    jstring jId   = env_->NewStringUTF(networkId_.c_str());
    jstring jHint = env_->NewStringUTF(filenameHint.c_str());
    jobject result = env_->CallStaticObjectMethod(cache_.iidmRegistryClass,
                                                  cache_.registry_saveToBytes, jId, jHint);
    env_->DeleteLocalRef(jHint);
    env_->DeleteLocalRef(jId);
    checkJNIException(env_);
    if (!result) {
        throw IidmException("Java returned null bytes for network save");
    }
    jbyteArray arr = static_cast<jbyteArray>(result);
    jsize n = env_->GetArrayLength(arr);
    std::string out(static_cast<std::size_t>(n), '\0');
    if (n > 0) {
        env_->GetByteArrayRegion(arr, 0, n, reinterpret_cast<jbyte*>(&out[0]));
    }
    env_->DeleteLocalRef(arr);
    checkJNIException(env_);
    return out;
}

void JNIBackend::close() {
    if (env_) {
        // Wrappers still alive at this point release nothing later on
        releaseAllHandles();
    }
    if (networkRef_ && env_) {
        env_->DeleteGlobalRef(networkRef_);
        networkRef_ = nullptr;
    }
    if (jvm_) {
        jvm_->DetachCurrentThread();
        jvm_ = nullptr;
        env_ = nullptr;
    }
}

void JNIBackend::releaseHandle(ObjectHandle h) {
    jobject obj = toNullableObject(h);
    if (env_ && obj && obj != networkRef_) {
        env_->DeleteGlobalRef(obj);
    }
}

// ── Handle helpers ────────────────────────────────────────────────────────────

ObjectHandle JNIBackend::toHandle(jobject local) const {
    if (!local) return INVALID_HANDLE;
    jobject globalRef = env_->NewGlobalRef(local);
    env_->DeleteLocalRef(local);
    return reinterpret_cast<ObjectHandle>(globalRef);
}

jobject JNIBackend::toObject(ObjectHandle h) const {
    // Calling a Java method on a null receiver would crash the JVM
    if (h == INVALID_HANDLE)
        throw InvalidHandleException("Invalid handle 0 (default-constructed or unset object)");
    return reinterpret_cast<jobject>(h);
}

jobject JNIBackend::toNullableObject(ObjectHandle h) const {
    return h == INVALID_HANDLE ? nullptr : reinterpret_cast<jobject>(h);
}

std::string JNIBackend::toString(jstring local) const {
    std::string result;
    if (local) {
        const char* chars = env_->GetStringUTFChars(local, nullptr);
        if (chars) {
            result = chars;
            env_->ReleaseStringUTFChars(local, chars);
        }
        env_->DeleteLocalRef(local);
    }
    return result;
}

// ── Properties (PropertyDispatcher object API) ────────────────────────────────

ObjectHandle JNIBackend::getNetworkHandle() const {
    return reinterpret_cast<ObjectHandle>(networkRef_);
}

double JNIBackend::getDouble(ObjectHandle h, int property) const {
    double result = env_->CallStaticDoubleMethod(cache_.dispatcherClass, cache_.disp_getDouble,
                                                 toObject(h), static_cast<jint>(property));
    checkJNIException(env_);
    return result;
}

void JNIBackend::setDouble(ObjectHandle h, int property, double value) {
    env_->CallStaticVoidMethod(cache_.dispatcherClass, cache_.disp_setDouble,
                               toObject(h), static_cast<jint>(property), value);
    checkJNIException(env_);
}

int JNIBackend::getInt(ObjectHandle h, int property) const {
    int result = env_->CallStaticIntMethod(cache_.dispatcherClass, cache_.disp_getInt,
                                           toObject(h), static_cast<jint>(property));
    checkJNIException(env_);
    return result;
}

void JNIBackend::setInt(ObjectHandle h, int property, int value) {
    env_->CallStaticVoidMethod(cache_.dispatcherClass, cache_.disp_setInt,
                               toObject(h), static_cast<jint>(property), static_cast<jint>(value));
    checkJNIException(env_);
}

bool JNIBackend::getBool(ObjectHandle h, int property) const {
    jboolean result = env_->CallStaticBooleanMethod(cache_.dispatcherClass, cache_.disp_getBool,
                                                    toObject(h), static_cast<jint>(property));
    checkJNIException(env_);
    return result == JNI_TRUE;
}

void JNIBackend::setBool(ObjectHandle h, int property, bool value) {
    env_->CallStaticVoidMethod(cache_.dispatcherClass, cache_.disp_setBool,
                               toObject(h), static_cast<jint>(property), static_cast<jboolean>(value));
    checkJNIException(env_);
}

std::string JNIBackend::getString(ObjectHandle h, int property) const {
    jobject result = env_->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_getString,
                                                  toObject(h), static_cast<jint>(property));
    checkJNIException(env_);
    return toString(static_cast<jstring>(result));
}

void JNIBackend::setString(ObjectHandle h, int property, const std::string& value) {
    jstring jval = env_->NewStringUTF(value.c_str());
    env_->CallStaticVoidMethod(cache_.dispatcherClass, cache_.disp_setString,
                               toObject(h), static_cast<jint>(property), jval);
    env_->DeleteLocalRef(jval);
    checkJNIException(env_);
}

iidm::optional<double> JNIBackend::getOptDouble(ObjectHandle h, int property) const {
    double val = getDouble(h, property);
    if (std::isnan(val)) return {};
    return val;
}

// ── Navigation ────────────────────────────────────────────────────────────────

std::vector<ObjectHandle> JNIBackend::getChildren(ObjectHandle h, int childType) const {
    jobject result = env_->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_getChildObjects,
                                                  toObject(h), static_cast<jint>(childType));
    checkJNIException(env_);
    std::vector<ObjectHandle> handles;
    if (!result) return handles;
    jobjectArray arr = static_cast<jobjectArray>(result);
    jsize len = env_->GetArrayLength(arr);
    handles.reserve(static_cast<std::size_t>(len));
    for (jsize i = 0; i < len; ++i) {
        handles.push_back(toHandle(env_->GetObjectArrayElement(arr, i)));
    }
    env_->DeleteLocalRef(arr);
    return handles;
}

ObjectHandle JNIBackend::getRelated(ObjectHandle h, int relation) const {
    jobject related = env_->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_getRelatedObject,
                                                   toObject(h), static_cast<jint>(relation));
    checkJNIException(env_);
    return toHandle(related);
}

ObjectHandle JNIBackend::invoke(ObjectHandle h, int code) {
    jobject result = env_->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_invokeObject,
                                                  toObject(h), static_cast<jint>(code));
    checkJNIException(env_);
    return toHandle(result);
}

void JNIBackend::setRelated(ObjectHandle h, int relation, ObjectHandle value) {
    env_->CallStaticVoidMethod(cache_.dispatcherClass, cache_.disp_setRelatedObject,
                               toObject(h), static_cast<jint>(relation), toNullableObject(value));
    checkJNIException(env_);
}

std::vector<int> JNIBackend::getIntList(ObjectHandle h, int listCode) const {
    jobject result = env_->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_getIntList,
                                                  toObject(h), static_cast<jint>(listCode));
    checkJNIException(env_);
    std::vector<int> values;
    if (!result) return values;
    jintArray arr = static_cast<jintArray>(result);
    jsize len = env_->GetArrayLength(arr);
    values.resize(static_cast<std::size_t>(len));
    if (len > 0) env_->GetIntArrayRegion(arr, 0, len, reinterpret_cast<jint*>(values.data()));
    env_->DeleteLocalRef(arr);
    return values;
}

ObjectHandle JNIBackend::getRelatedByIndex(ObjectHandle h, int relation, int index) const {
    jobject related = env_->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_getRelatedObjectByIndex,
                                                   toObject(h), static_cast<jint>(relation),
                                                   static_cast<jint>(index));
    checkJNIException(env_);
    return toHandle(related);
}

ObjectHandle JNIBackend::getRelatedByString(ObjectHandle h, int relation, const std::string& key) const {
    jobject obj = toObject(h);
    jstring jKey = env_->NewStringUTF(key.c_str());
    jobject related = env_->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_getRelatedObjectByString,
                                                   obj, static_cast<jint>(relation), jKey);
    env_->DeleteLocalRef(jKey);
    checkJNIException(env_);
    return toHandle(related);
}

ObjectHandle JNIBackend::findById(int objectType, const std::string& id) const {
    if (!networkRef_) throw NetworkNotFoundException("No network loaded or registered with id: " + networkId_);
    jstring jId = env_->NewStringUTF(id.c_str());
    jobject found = env_->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_findObjectById,
                                                 networkRef_, static_cast<jint>(objectType), jId);
    env_->DeleteLocalRef(jId);
    checkJNIException(env_);
    return toHandle(found);
}

} // namespace iidm
