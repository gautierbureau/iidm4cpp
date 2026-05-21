#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>
#include <cmath>
#include <map>
#include <memory>
#include <iidm/stdcxx/optional.hpp>
// When building without C++17, iidm::optional<T> is boost::optional<T>.
// Include the Boost I/O header so GTest can print optional values on failure.
#ifdef BOOST_OPTIONAL_FLC_19NOV2002_HPP
#  include <boost/optional/optional_io.hpp>
#endif
#include <string>
#include <vector>

// std::make_unique is C++14; provide a minimal polyfill for C++11 builds.
#if __cplusplus < 201402L && !(defined(_MSVC_LANG) && _MSVC_LANG >= 201402L)
namespace std {
template <typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}
#endif

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
    std::map<std::pair<ObjectHandle,int>, std::vector<int>>          intLists;
    std::map<std::tuple<ObjectHandle,int,int>, ObjectHandle>                         relatedByIndex;
    std::map<std::tuple<ObjectHandle,int,std::string>, ObjectHandle>                 relatedByString;
    ObjectHandle networkHandle = 1;
    bool closed = false;
    std::string lastSavedPath;
    // Stream-API hooks
    std::string lastLoadedBytes;
    std::string lastLoadedFilenameHint;
    mutable std::string lastSavedFilenameHint;
    std::string bytesToReturnOnSave;

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

    iidm::optional<double> getOptDouble(ObjectHandle h, int p) const override {
        double v = getDouble(h, p);
        if (std::isnan(v)) return {};
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

    std::vector<int> getIntList(ObjectHandle h, int listCode) const override {
        auto it = intLists.find({h, listCode});
        if (it == intLists.end()) return {};
        return it->second;
    }

    ObjectHandle getRelatedByIndex(ObjectHandle h, int relation, int index) const override {
        auto it = relatedByIndex.find({h, relation, index});
        if (it == relatedByIndex.end()) return INVALID_HANDLE;
        return it->second;
    }

    ObjectHandle getRelatedByString(ObjectHandle h, int relation, const std::string& key) const override {
        auto it = relatedByString.find({h, relation, key});
        if (it == relatedByString.end()) return INVALID_HANDLE;
        return it->second;
    }

    ObjectHandle findById(int type, const std::string& id) const override {
        auto it = byId.find({type, id});
        if (it == byId.end()) return INVALID_HANDLE;
        return it->second;
    }

    ObjectHandle getNetworkHandle() const override { return networkHandle; }
    void loadNetwork(const std::string&) override {}
    void saveNetwork(const std::string& path) override { lastSavedPath = path; }
    void loadNetworkBytes(const char* data, std::size_t length,
                          const std::string& filenameHint) override {
        lastLoadedBytes.assign(data, length);
        lastLoadedFilenameHint = filenameHint;
    }
    std::string saveNetworkBytes(const std::string& filenameHint) const override {
        lastSavedFilenameHint = filenameHint;
        return bytesToReturnOnSave;
    }
    void close() override { closed = true; }
};

} // namespace iidm::test
