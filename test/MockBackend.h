#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>
#include <cmath>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace iidm::test {

// A simple in-memory mock backend for unit testing without a real JVM or GraalVM.
class MockBackend : public BackendProvider {
public:
    // ── Data stores ───────────────────────────────────────────────────────
    std::map<std::pair<ObjectHandle,int>, double>      doubles;
    std::map<std::pair<ObjectHandle,int>, int>         ints;
    std::map<std::pair<ObjectHandle,int>, bool>        bools;
    std::map<std::pair<ObjectHandle,int>, std::string> strings;
    std::map<std::pair<ObjectHandle,int>, std::vector<ObjectHandle>> children;
    std::map<std::pair<ObjectHandle,int>, ObjectHandle>              related;
    std::map<std::pair<int,std::string>, ObjectHandle>               byId;
    // Extension maps: {componentHandle, name} → extensionHandle
    std::map<std::pair<ObjectHandle,std::string>, ObjectHandle> extensions;
    // {extensionHandle, attributeKey} → string value
    std::map<std::pair<ObjectHandle,std::string>, std::string>  extensionAttrs;
    ObjectHandle networkHandle = 1;
    bool closed = false;

    // ── BackendProvider interface ─────────────────────────────────────────
    double getDouble(ObjectHandle h, int p) const override {
        auto it = doubles.find({h, p});
        if (it == doubles.end()) return std::numeric_limits<double>::quiet_NaN();
        return it->second;
    }

    int getInt(ObjectHandle h, int p) const override {
        auto it = ints.find({h, p});
        if (it == ints.end()) return 0;
        return it->second;
    }

    bool getBool(ObjectHandle h, int p) const override {
        auto it = bools.find({h, p});
        if (it == bools.end()) return false;
        return it->second;
    }

    std::string getString(ObjectHandle h, int p) const override {
        auto it = strings.find({h, p});
        if (it == strings.end()) return "";
        return it->second;
    }

    void setDouble(ObjectHandle h, int p, double v) override { doubles[{h, p}] = v; }
    void setInt   (ObjectHandle h, int p, int v)    override { ints[{h, p}] = v; }
    void setBool  (ObjectHandle h, int p, bool v)   override { bools[{h, p}] = v; }
    void setString(ObjectHandle h, int p, const std::string& v) override { strings[{h, p}] = v; }

    std::optional<double> getOptDouble(ObjectHandle h, int p) const override {
        double v = getDouble(h, p);
        if (std::isnan(v)) return std::nullopt;
        return v;
    }

    std::vector<ObjectHandle> getChildren(ObjectHandle h, int childType) const override {
        auto it = children.find({h, childType});
        if (it == children.end()) return {};
        return it->second;
    }

    ObjectHandle getRelated(ObjectHandle h, int relation) const override {
        auto it = related.find({h, relation});
        if (it == related.end()) return INVALID_HANDLE;
        return it->second;
    }

    ObjectHandle findById(int type, const std::string& id) const override {
        auto it = byId.find({type, id});
        if (it == byId.end()) return INVALID_HANDLE;
        return it->second;
    }

    ObjectHandle getExtensionHandle(ObjectHandle h, const std::string& name) const override {
        auto it = extensions.find({h, name});
        return it != extensions.end() ? it->second : INVALID_HANDLE;
    }

    std::vector<std::string> getExtensionNames(ObjectHandle h) const override {
        std::vector<std::string> result;
        for (const auto& [key, handle] : extensions) {
            if (key.first == h) result.push_back(key.second);
        }
        return result;
    }

    std::string getExtensionAttribute(ObjectHandle extHandle, const std::string& key) const override {
        auto it = extensionAttrs.find({extHandle, key});
        return it != extensionAttrs.end() ? it->second : "";
    }

    ObjectHandle getNetworkHandle() const override { return networkHandle; }
    void loadNetwork(const std::string&) override {}
    void close() override { closed = true; }
};

} // namespace iidm::test
