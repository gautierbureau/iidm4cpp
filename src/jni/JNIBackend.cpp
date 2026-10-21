#include "JNIBackend.h"
#include "JNIExceptionBridge.h"
#include "JNIStrings.h"
#include "JavaVMs.h"
#include "../PropertyCodesCheck.h"
#include <iidm/IidmException.h>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>
#include <iterator>
#include <string>

namespace iidm {

namespace {

// A plain thread-local value: still readable once the thread's destructors have run (static
// objects closing a network at exit)
thread_local bool threadEnded = false;

// Detaches the calling thread from the JVM when it ends, if the backend attached it: the threads
// attached by others (the one that created the JVM, Java threads calling C++) stay as they are.
struct JvmAttachment {
    JavaVM* jvm = nullptr;
    ~JvmAttachment() {
        threadEnded = true;
        // Not after the JVM is destroyed
        if (jvm && detail::createdJavaVM() == jvm) jvm->DetachCurrentThread();
    }
};
thread_local JvmAttachment jvmAttachment;

} // namespace

// ── Lifecycle ─────────────────────────────────────────────────────────────────

JNIBackend::JNIBackend(const std::string& networkId)
    : networkId_(networkId.empty() ? "default" : networkId) {
    findJVM();
    JNIEnv* env = this->env();
    cacheMethodIds(env);
    checkPropertyCodes(env);
    bindRegisteredNetwork(env);
}

JNIBackend::~JNIBackend() {
    close();
}

void JNIBackend::findJVM() {
    jvm_ = detail::createdJavaVM();
    if (!jvm_) throw BackendNotAvailableException("No JVM found; cannot use JNI backend");
}

JNIEnv* JNIBackend::env() const {
    if (!jvm_) throw InvalidHandleException("The network of this object is closed");
    JNIEnv* env = nullptr;
    const jint status = jvm_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_8);
    if (status == JNI_OK) return env;
    // A daemon, so that it never holds the JVM's shutdown up
    if (status != JNI_EDETACHED || jvm_->AttachCurrentThreadAsDaemon(reinterpret_cast<void**>(&env), nullptr) != JNI_OK) {
        throw IidmException("Failed to attach the thread to the JVM");
    }
    if (!threadEnded) jvmAttachment.jvm = jvm_;
    return env;
}

void JNIBackend::checkPropertyCodes(JNIEnv* env) {
    // Classes older than the check have no codesHash
    jmethodID codesHash = env->GetStaticMethodID(cache_.dispatcherClass, "codesHash", "()J");
    jlong javaHash = 0;
    if (codesHash) {
        javaHash = env->CallStaticLongMethod(cache_.dispatcherClass, codesHash);
        checkJNIException(env);
    } else {
        env->ExceptionClear();
    }
    detail::checkPropertyCodes(static_cast<std::uint64_t>(javaHash), "The Java bridge on the class path of the JVM");
}

void JNIBackend::cacheClass(JNIEnv* env, jclass& out, const char* name) {
    jclass local = env->FindClass(name);
    checkJNIException(env);
    if (!local) throw IidmException(std::string("Cannot find class: ") + name);
    out = static_cast<jclass>(env->NewGlobalRef(local));
    env->DeleteLocalRef(local);
}

void JNIBackend::cacheMethodIds(JNIEnv* env) {
    cacheClass(env, cache_.iidmRegistryClass, "com/powsybl/iidmbridge/jni/IidmBridgeRegistry");
    cache_.registry_get = env->GetStaticMethodID(cache_.iidmRegistryClass,
        "get", "(Ljava/lang/String;)Lcom/powsybl/iidm/network/Network;");
    cache_.registry_save = env->GetStaticMethodID(cache_.iidmRegistryClass,
        "save", "(Lcom/powsybl/iidm/network/Network;Ljava/lang/String;)V");
    cache_.registry_loadFromBytes = env->GetStaticMethodID(cache_.iidmRegistryClass,
        "loadFromBytes", "(Ljava/lang/String;[BLjava/lang/String;)V");
    cache_.registry_saveToBytes = env->GetStaticMethodID(cache_.iidmRegistryClass,
        "saveToBytes", "(Lcom/powsybl/iidm/network/Network;Ljava/lang/String;)[B");
    cache_.registry_createNetwork = env->GetStaticMethodID(cache_.iidmRegistryClass,
        "createNetwork", "(Ljava/lang/String;Ljava/lang/String;)V");
    cache_.registry_fromUtf8 = env->GetStaticMethodID(cache_.iidmRegistryClass,
        "fromUtf8", "([B)Ljava/lang/String;");
    checkJNIException(env);

    cacheClass(env, cache_.dispatcherClass, "com/powsybl/iidmbridge/PropertyDispatcher");
    jclass d = cache_.dispatcherClass;
    cache_.disp_getDouble  = env->GetStaticMethodID(d, "getDouble",  "(Ljava/lang/Object;I)D");
    cache_.disp_setDouble  = env->GetStaticMethodID(d, "setDouble",  "(Ljava/lang/Object;ID)V");
    cache_.disp_getInt     = env->GetStaticMethodID(d, "getInt",     "(Ljava/lang/Object;I)I");
    cache_.disp_setInt     = env->GetStaticMethodID(d, "setInt",     "(Ljava/lang/Object;II)V");
    cache_.disp_getBool    = env->GetStaticMethodID(d, "getBool",    "(Ljava/lang/Object;I)Z");
    cache_.disp_setBool    = env->GetStaticMethodID(d, "setBool",    "(Ljava/lang/Object;IZ)V");
    cache_.disp_getString  = env->GetStaticMethodID(d, "getString",  "(Ljava/lang/Object;I)Ljava/lang/String;");
    cache_.disp_setString  = env->GetStaticMethodID(d, "setString",  "(Ljava/lang/Object;ILjava/lang/String;)V");
    cache_.disp_getChildObjects = env->GetStaticMethodID(d, "getChildObjects",
        "(Ljava/lang/Object;I)[Ljava/lang/Object;");
    cache_.disp_getRelatedObject = env->GetStaticMethodID(d, "getRelatedObject",
        "(Ljava/lang/Object;I)Ljava/lang/Object;");
    cache_.disp_setRelatedObject = env->GetStaticMethodID(d, "setRelatedObject",
        "(Ljava/lang/Object;ILjava/lang/Object;)V");
    cache_.disp_getRelatedObjectByIndex = env->GetStaticMethodID(d, "getRelatedObjectByIndex",
        "(Ljava/lang/Object;II)Ljava/lang/Object;");
    cache_.disp_getRelatedObjectByString = env->GetStaticMethodID(d, "getRelatedObjectByString",
        "(Ljava/lang/Object;ILjava/lang/String;)Ljava/lang/Object;");
    cache_.disp_getIntList = env->GetStaticMethodID(d, "getIntList", "(Ljava/lang/Object;I)[I");
    cache_.disp_findObjectById = env->GetStaticMethodID(d, "findObjectById",
        "(Ljava/lang/Object;ILjava/lang/String;)Ljava/lang/Object;");
    cache_.disp_invokeObject = env->GetStaticMethodID(d, "invokeObject", "(Ljava/lang/Object;I)Ljava/lang/Object;");
    cache_.disp_getOptInt    = env->GetStaticMethodID(d, "getOptInt", "(Ljava/lang/Object;I)J");
    cache_.disp_getOptString = env->GetStaticMethodID(d, "getOptString", "(Ljava/lang/Object;I)Ljava/lang/String;");
    cache_.disp_getStringList = env->GetStaticMethodID(d, "getStringList", "(Ljava/lang/Object;I)[Ljava/lang/String;");
    cache_.disp_getKeyedString = env->GetStaticMethodID(d, "getKeyedString",
        "(Ljava/lang/Object;ILjava/lang/String;)Ljava/lang/String;");
    cache_.disp_setKeyedString = env->GetStaticMethodID(d, "setKeyedString",
        "(Ljava/lang/Object;ILjava/lang/String;Ljava/lang/String;)V");
    cache_.disp_isSameObject = env->GetStaticMethodID(d, "isSameObject", "(Ljava/lang/Object;Ljava/lang/Object;)Z");
    cache_.disp_evaluate = env->GetStaticMethodID(d, "evaluate", "(Ljava/lang/Object;I[D)D");
    checkJNIException(env);
}

void JNIBackend::bindRegisteredNetwork(JNIEnv* env) {
    // Bind the network already registered under networkId_, if any (embedded mode).
    // Otherwise the network comes later from loadNetwork / loadNetworkBytes.
    jstring jId = newString(env, networkId_);
    jobject netObj = env->CallStaticObjectMethod(cache_.iidmRegistryClass, cache_.registry_get, jId);
    env->DeleteLocalRef(jId);
    if (env->ExceptionCheck()) {
        env->ExceptionClear(); // no network registered under this id yet
        return;
    }
    if (netObj) {
        networkRef_ = env->NewGlobalRef(netObj);
        env->DeleteLocalRef(netObj);
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

void JNIBackend::saveNetwork(ObjectHandle network, const std::string& filePath) {
    JNIEnv* env = this->env();
    jobject obj = toObject(network);
    jstring jPath = newString(env, filePath);
    env->CallStaticVoidMethod(cache_.iidmRegistryClass, cache_.registry_save, obj, jPath);
    env->DeleteLocalRef(jPath);
    checkJNIException(env);
}

void JNIBackend::loadNetworkBytes(const char* data, std::size_t length,
                                  const std::string& filenameHint) {
    JNIEnv* env = this->env();
    jbyteArray arr = env->NewByteArray(static_cast<jsize>(length));
    if (!arr) {
        checkJNIException(env);
        throw IidmException("Failed to allocate Java byte[] for network load");
    }
    if (length > 0) {
        env->SetByteArrayRegion(arr, 0, static_cast<jsize>(length),
                                 reinterpret_cast<const jbyte*>(data));
    }
    jstring jId   = newString(env, networkId_);
    jstring jHint = newString(env, filenameHint);
    env->CallStaticVoidMethod(cache_.iidmRegistryClass,
                               cache_.registry_loadFromBytes, jId, arr, jHint);
    env->DeleteLocalRef(jHint);
    env->DeleteLocalRef(jId);
    env->DeleteLocalRef(arr);
    checkJNIException(env);
    rebindNetwork(env, "loadFromBytes");
}

void JNIBackend::createNetwork(const std::string& id) {
    JNIEnv* env = this->env();
    jstring jId        = newString(env, networkId_);
    jstring jNetworkId = newString(env, id);
    env->CallStaticVoidMethod(cache_.iidmRegistryClass, cache_.registry_createNetwork, jId, jNetworkId);
    env->DeleteLocalRef(jNetworkId);
    env->DeleteLocalRef(jId);
    checkJNIException(env);
    rebindNetwork(env, "createNetwork");
}

void JNIBackend::rebindNetwork(JNIEnv* env, const char* operation) {
    // The newly registered network replaces any previous reference held here.
    if (networkRef_) {
        env->DeleteGlobalRef(networkRef_);
        networkRef_ = nullptr;
    }
    bindRegisteredNetwork(env);
    if (!networkRef_) {
        throw NetworkNotFoundException(std::string("Network missing after ") + operation + ": " + networkId_);
    }
}

std::string JNIBackend::saveNetworkBytes(ObjectHandle network, const std::string& filenameHint) const {
    JNIEnv* env = this->env();
    jobject obj = toObject(network);
    jstring jHint = newString(env, filenameHint);
    jobject result = env->CallStaticObjectMethod(cache_.iidmRegistryClass,
                                                  cache_.registry_saveToBytes, obj, jHint);
    env->DeleteLocalRef(jHint);
    checkJNIException(env);
    if (!result) {
        throw IidmException("Java returned null bytes for network save");
    }
    jbyteArray arr = static_cast<jbyteArray>(result);
    jsize n = env->GetArrayLength(arr);
    std::string out(static_cast<std::size_t>(n), '\0');
    if (n > 0) {
        env->GetByteArrayRegion(arr, 0, n, reinterpret_cast<jbyte*>(&out[0]));
    }
    env->DeleteLocalRef(arr);
    checkJNIException(env);
    return out;
}

void JNIBackend::close() {
    if (!jvm_) return;
    // Wrappers still alive at this point release nothing later on
    releaseAllHandles();
    if (networkRef_) {
        try {
            env()->DeleteGlobalRef(networkRef_);
        } catch (const IidmException&) {
            // The JVM was destroyed before the network: nothing left to release
        }
        networkRef_ = nullptr;
    }
    jvm_ = nullptr;
}

void JNIBackend::releaseHandle(ObjectHandle h) {
    jobject obj = toNullableObject(h);
    if (jvm_ && obj && obj != networkRef_) {
        env()->DeleteGlobalRef(obj);
    }
}

// ── Handle helpers ────────────────────────────────────────────────────────────

ObjectHandle JNIBackend::toHandle(JNIEnv* env, jobject local) const {
    if (!local) return INVALID_HANDLE;
    jobject globalRef = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
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

std::string JNIBackend::toString(JNIEnv* env, jstring local) const {
    std::string result;
    if (local) {
        result = detail::javaString(env, local);
        env->DeleteLocalRef(local);
    }
    return result;
}

jstring JNIBackend::newString(JNIEnv* env, const std::string& value) const {
    std::u16string utf16;
    if (detail::utf8ToUtf16(value.c_str(), utf16)) {
        return env->NewString(reinterpret_cast<const jchar*>(utf16.data()), static_cast<jsize>(utf16.size()));
    }
    // Not well-formed UTF-8: decoded by Java, as by the GraalVM backend
    const jsize length = static_cast<jsize>(std::strlen(value.c_str()));
    jbyteArray bytes = env->NewByteArray(length);
    env->SetByteArrayRegion(bytes, 0, length, reinterpret_cast<const jbyte*>(value.c_str()));
    jobject text = env->CallStaticObjectMethod(cache_.iidmRegistryClass, cache_.registry_fromUtf8, bytes);
    env->DeleteLocalRef(bytes);
    checkJNIException(env);
    return static_cast<jstring>(text);
}

// ── Properties (PropertyDispatcher object API) ────────────────────────────────

ObjectHandle JNIBackend::getNetworkHandle() const {
    return reinterpret_cast<ObjectHandle>(networkRef_);
}

double JNIBackend::getDouble(ObjectHandle h, int property) const {
    JNIEnv* env = this->env();
    double result = env->CallStaticDoubleMethod(cache_.dispatcherClass, cache_.disp_getDouble,
                                                 toObject(h), static_cast<jint>(property));
    checkJNIException(env);
    return result;
}

void JNIBackend::setDouble(ObjectHandle h, int property, double value) {
    JNIEnv* env = this->env();
    env->CallStaticVoidMethod(cache_.dispatcherClass, cache_.disp_setDouble,
                               toObject(h), static_cast<jint>(property), value);
    checkJNIException(env);
}

int JNIBackend::getInt(ObjectHandle h, int property) const {
    JNIEnv* env = this->env();
    int result = env->CallStaticIntMethod(cache_.dispatcherClass, cache_.disp_getInt,
                                           toObject(h), static_cast<jint>(property));
    checkJNIException(env);
    return result;
}

void JNIBackend::setInt(ObjectHandle h, int property, int value) {
    JNIEnv* env = this->env();
    env->CallStaticVoidMethod(cache_.dispatcherClass, cache_.disp_setInt,
                               toObject(h), static_cast<jint>(property), static_cast<jint>(value));
    checkJNIException(env);
}

bool JNIBackend::getBool(ObjectHandle h, int property) const {
    JNIEnv* env = this->env();
    jboolean result = env->CallStaticBooleanMethod(cache_.dispatcherClass, cache_.disp_getBool,
                                                    toObject(h), static_cast<jint>(property));
    checkJNIException(env);
    return result == JNI_TRUE;
}

void JNIBackend::setBool(ObjectHandle h, int property, bool value) {
    JNIEnv* env = this->env();
    env->CallStaticVoidMethod(cache_.dispatcherClass, cache_.disp_setBool,
                               toObject(h), static_cast<jint>(property), static_cast<jboolean>(value));
    checkJNIException(env);
}

std::string JNIBackend::getString(ObjectHandle h, int property) const {
    JNIEnv* env = this->env();
    jobject result = env->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_getString,
                                                  toObject(h), static_cast<jint>(property));
    checkJNIException(env);
    return toString(env, static_cast<jstring>(result));
}

void JNIBackend::setString(ObjectHandle h, int property, const std::string& value) {
    JNIEnv* env = this->env();
    jstring jval = newString(env, value);
    env->CallStaticVoidMethod(cache_.dispatcherClass, cache_.disp_setString,
                               toObject(h), static_cast<jint>(property), jval);
    env->DeleteLocalRef(jval);
    checkJNIException(env);
}

iidm::optional<double> JNIBackend::getOptDouble(ObjectHandle h, int property) const {
    double val = getDouble(h, property);
    if (std::isnan(val)) return {};
    return val;
}

iidm::optional<int> JNIBackend::getOptInt(ObjectHandle h, int property) const {
    JNIEnv* env = this->env();
    jlong value = env->CallStaticLongMethod(cache_.dispatcherClass, cache_.disp_getOptInt,
                                             toObject(h), static_cast<jint>(property));
    checkJNIException(env);
    if (value == std::numeric_limits<jlong>::min()) return {}; // PropertyDispatcher.NO_INT
    return static_cast<int>(value);
}

iidm::optional<std::string> JNIBackend::getOptString(ObjectHandle h, int property) const {
    JNIEnv* env = this->env();
    jobject result = env->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_getOptString,
                                                  toObject(h), static_cast<jint>(property));
    checkJNIException(env);
    if (!result) return {};
    return toString(env, static_cast<jstring>(result));
}

std::vector<std::string> JNIBackend::getStringList(ObjectHandle h, int listCode) const {
    JNIEnv* env = this->env();
    jobject result = env->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_getStringList,
                                                  toObject(h), static_cast<jint>(listCode));
    checkJNIException(env);
    std::vector<std::string> values;
    if (!result) return values;
    jobjectArray arr = static_cast<jobjectArray>(result);
    jsize len = env->GetArrayLength(arr);
    values.reserve(static_cast<std::size_t>(len));
    for (jsize i = 0; i < len; ++i) {
        values.push_back(toString(env, static_cast<jstring>(env->GetObjectArrayElement(arr, i))));
    }
    env->DeleteLocalRef(arr);
    return values;
}

iidm::optional<std::string> JNIBackend::getKeyedString(ObjectHandle h, int property, const std::string& key) const {
    JNIEnv* env = this->env();
    jobject obj = toObject(h);
    jstring jKey = newString(env, key);
    jobject result = env->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_getKeyedString,
                                                  obj, static_cast<jint>(property), jKey);
    env->DeleteLocalRef(jKey);
    checkJNIException(env);
    if (!result) return {};
    return toString(env, static_cast<jstring>(result));
}

void JNIBackend::setKeyedString(ObjectHandle h, int property, const std::string& key,
                                const iidm::optional<std::string>& value) {
    JNIEnv* env = this->env();
    jobject obj = toObject(h);
    jstring jKey   = newString(env, key);
    jstring jValue = value ? newString(env, *value) : nullptr; // null removes the key
    env->CallStaticVoidMethod(cache_.dispatcherClass, cache_.disp_setKeyedString,
                               obj, static_cast<jint>(property), jKey, jValue);
    env->DeleteLocalRef(jKey);
    if (jValue) env->DeleteLocalRef(jValue);
    checkJNIException(env);
}

bool JNIBackend::isSameObject(ObjectHandle h, ObjectHandle other) const {
    JNIEnv* env = this->env();
    jboolean same = env->CallStaticBooleanMethod(cache_.dispatcherClass, cache_.disp_isSameObject,
                                                  toObject(h), toObject(other));
    checkJNIException(env);
    return same == JNI_TRUE;
}

// ── Navigation ────────────────────────────────────────────────────────────────

std::vector<ObjectHandle> JNIBackend::getChildren(ObjectHandle h, int childType) const {
    JNIEnv* env = this->env();
    jobject result = env->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_getChildObjects,
                                                  toObject(h), static_cast<jint>(childType));
    checkJNIException(env);
    std::vector<ObjectHandle> handles;
    if (!result) return handles;
    jobjectArray arr = static_cast<jobjectArray>(result);
    jsize len = env->GetArrayLength(arr);
    handles.reserve(static_cast<std::size_t>(len));
    for (jsize i = 0; i < len; ++i) {
        handles.push_back(toHandle(env, env->GetObjectArrayElement(arr, i)));
    }
    env->DeleteLocalRef(arr);
    return handles;
}

ObjectHandle JNIBackend::getRelated(ObjectHandle h, int relation) const {
    JNIEnv* env = this->env();
    jobject related = env->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_getRelatedObject,
                                                   toObject(h), static_cast<jint>(relation));
    checkJNIException(env);
    return toHandle(env, related);
}

ObjectHandle JNIBackend::invoke(ObjectHandle h, int code) {
    JNIEnv* env = this->env();
    jobject result = env->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_invokeObject,
                                                  toObject(h), static_cast<jint>(code));
    checkJNIException(env);
    return toHandle(env, result);
}

void JNIBackend::setRelated(ObjectHandle h, int relation, ObjectHandle value) {
    JNIEnv* env = this->env();
    env->CallStaticVoidMethod(cache_.dispatcherClass, cache_.disp_setRelatedObject,
                               toObject(h), static_cast<jint>(relation), toNullableObject(value));
    checkJNIException(env);
}

std::vector<int> JNIBackend::getIntList(ObjectHandle h, int listCode) const {
    JNIEnv* env = this->env();
    jobject result = env->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_getIntList,
                                                  toObject(h), static_cast<jint>(listCode));
    checkJNIException(env);
    std::vector<int> values;
    if (!result) return values;
    jintArray arr = static_cast<jintArray>(result);
    jsize len = env->GetArrayLength(arr);
    values.resize(static_cast<std::size_t>(len));
    if (len > 0) env->GetIntArrayRegion(arr, 0, len, reinterpret_cast<jint*>(values.data()));
    env->DeleteLocalRef(arr);
    return values;
}

double JNIBackend::evaluate(ObjectHandle h, int code, const std::vector<double>& args) const {
    JNIEnv* env = this->env();
    jdoubleArray jArgs = env->NewDoubleArray(static_cast<jsize>(args.size()));
    checkJNIException(env);
    if (!args.empty()) env->SetDoubleArrayRegion(jArgs, 0, static_cast<jsize>(args.size()), args.data());
    jdouble result = env->CallStaticDoubleMethod(cache_.dispatcherClass, cache_.disp_evaluate,
                                                  toObject(h), static_cast<jint>(code), jArgs);
    env->DeleteLocalRef(jArgs);
    checkJNIException(env);
    return result;
}

ObjectHandle JNIBackend::getRelatedByIndex(ObjectHandle h, int relation, int index) const {
    JNIEnv* env = this->env();
    jobject related = env->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_getRelatedObjectByIndex,
                                                   toObject(h), static_cast<jint>(relation),
                                                   static_cast<jint>(index));
    checkJNIException(env);
    return toHandle(env, related);
}

ObjectHandle JNIBackend::getRelatedByString(ObjectHandle h, int relation, const std::string& key) const {
    JNIEnv* env = this->env();
    jobject obj = toObject(h);
    jstring jKey = newString(env, key);
    jobject related = env->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_getRelatedObjectByString,
                                                   obj, static_cast<jint>(relation), jKey);
    env->DeleteLocalRef(jKey);
    checkJNIException(env);
    return toHandle(env, related);
}

ObjectHandle JNIBackend::findById(int objectType, const std::string& id) const {
    JNIEnv* env = this->env();
    if (!networkRef_) throw NetworkNotFoundException("No network loaded or registered with id: " + networkId_);
    jstring jId = newString(env, id);
    jobject found = env->CallStaticObjectMethod(cache_.dispatcherClass, cache_.disp_findObjectById,
                                                 networkRef_, static_cast<jint>(objectType), jId);
    env->DeleteLocalRef(jId);
    checkJNIException(env);
    return toHandle(env, found);
}

} // namespace iidm
