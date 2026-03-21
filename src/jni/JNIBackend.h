#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include "JNIMethodCache.h"
#include <jni.h>
#include <optional>
#include <string>
#include <vector>

namespace iidm {

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

    std::optional<double> getOptDouble(ObjectHandle h, int property) const override;

    std::vector<ObjectHandle> getChildren(ObjectHandle h, int childType) const override;
    ObjectHandle              getRelated(ObjectHandle h, int relation) const override;

    ObjectHandle findById(int objectType, const std::string& id) const override;
    ObjectHandle getNetworkHandle() const override;

    void loadNetwork(const std::string& filePath) override;
    void close() override;

private:
    void attachToJVM();
    void cacheMethodIds();
    void cacheClass(jclass& out, const char* name);

    // Make a global reference to a Java object and return as ObjectHandle
    ObjectHandle makeHandle(jobject obj) const;
    // Cast ObjectHandle back to jobject (global ref)
    jobject toObject(ObjectHandle h) const;
    // Wrap a Java Iterable/List into a vector of handles
    std::vector<ObjectHandle> collectionToHandles(jobject collection) const;

    std::string  networkId_;
    JavaVM*      jvm_  = nullptr;
    JNIEnv*      env_  = nullptr;
    jobject      networkRef_ = nullptr;  // global ref to the Java Network object

    JNIMethodCache cache_;
};

} // namespace iidm
