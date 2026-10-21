#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include "JNIMethodCache.h"
#include <jni.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

// Backend for the embedded mode: C++ runs inside (or next to) a JVM that holds the network.
//
// Every operation is a single JNI call to the object API of PropertyDispatcher — the same
// Java dispatch table as the GraalVM backend, so both backends behave identically. Handles
// are JNI global references, released when the last wrapper using them is destroyed.
class JNIBackend : public BackendProvider {
public:
    explicit JNIBackend(const std::string& networkId = "");
    ~JNIBackend() override;

    // ── BackendProvider implementation ───────────────────────────────────
    double      getDouble(ObjectHandle h, int property) const override;
    int         getInt(ObjectHandle h, int property) const override;
    bool        getBool(ObjectHandle h, int property) const override;
    std::string getString(ObjectHandle h, int property) const override;

    void setDouble(ObjectHandle h, int property, double value) override;
    void setInt   (ObjectHandle h, int property, int value) override;
    void setBool  (ObjectHandle h, int property, bool value) override;
    void setString(ObjectHandle h, int property, const std::string& value) override;

    iidm::optional<double> getOptDouble(ObjectHandle h, int property) const override;
    iidm::optional<int>         getOptInt(ObjectHandle h, int property) const override;
    iidm::optional<std::string> getOptString(ObjectHandle h, int property) const override;
    std::vector<std::string>    getStringList(ObjectHandle h, int listCode) const override;
    iidm::optional<std::string> getKeyedString(ObjectHandle h, int property, const std::string& key) const override;
    void setKeyedString(ObjectHandle h, int property, const std::string& key,
                        const iidm::optional<std::string>& value) override;
    double evaluate(ObjectHandle h, int code, const std::vector<double>& args) const override;
    bool isSameObject(ObjectHandle h, ObjectHandle other) const override;

    std::vector<ObjectHandle> getChildren(ObjectHandle h, int childType) const override;
    ObjectHandle              getRelated(ObjectHandle h, int relation) const override;
    void                      setRelated(ObjectHandle h, int relation, ObjectHandle value) override;
    std::vector<int>          getIntList(ObjectHandle h, int listCode) const override;
    ObjectHandle              getRelatedByIndex(ObjectHandle h, int relation, int index) const override;
    ObjectHandle              getRelatedByString(ObjectHandle h, int relation, const std::string& key) const override;

    ObjectHandle invoke(ObjectHandle h, int code) override;

    ObjectHandle findById(int objectType, const std::string& id) const override;
    ObjectHandle getNetworkHandle() const override;

    void loadNetwork(const std::string& filePath) override;
    void createNetwork(const std::string& id) override;
    void saveNetwork(ObjectHandle network, const std::string& filePath) override;
    void loadNetworkBytes(const char* data, std::size_t length,
                          const std::string& filenameHint) override;
    std::string saveNetworkBytes(ObjectHandle network, const std::string& filenameHint) const override;
    void close() override;

    void releaseHandle(ObjectHandle h) override;

private:
    void attachToJVM();
    void cacheMethodIds();
    void cacheClass(jclass& out, const char* name);
    void bindRegisteredNetwork();
    // After a network was (re)registered under networkId_: drops the old reference, binds the new one.
    void rebindNetwork(const char* operation);

    // Global reference for a local one (and deletes the local one)
    ObjectHandle toHandle(jobject local) const;
    jobject toObject(ObjectHandle h) const;          // throws InvalidHandleException for 0
    jobject toNullableObject(ObjectHandle h) const;  // INVALID_HANDLE → null
    std::string toString(jstring local) const;       // deletes the local reference
    jstring newString(const std::string& value) const; // a new local reference (UTF-8 → Java)

    std::string  networkId_;
    JavaVM*      jvm_  = nullptr;
    JNIEnv*      env_  = nullptr;
    jobject      networkRef_ = nullptr;  // global ref to the Java Network object

    JNIMethodCache cache_;
};

} // namespace iidm
