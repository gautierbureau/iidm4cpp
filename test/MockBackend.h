#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>
#include <cmath>
#include <map>
#include <memory>
#include <set>
#include <tuple>
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
    // Optional values: absent from the map = no value
    std::map<std::pair<ObjectHandle,int>, int>         optInts;
    std::map<std::pair<ObjectHandle,int>, std::string> optStrings;
    std::map<std::pair<ObjectHandle,int>, std::vector<std::string>> stringLists;
    std::map<std::tuple<ObjectHandle,int,std::string>, std::string>  keyedStrings;
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

    iidm::optional<int> getOptInt(ObjectHandle h, int p) const override {
        auto it = optInts.find({h, p});
        if (it == optInts.end()) return {};
        return it->second;
    }

    iidm::optional<std::string> getOptString(ObjectHandle h, int p) const override {
        auto it = optStrings.find({h, p});
        if (it == optStrings.end()) return {};
        return it->second;
    }

    std::vector<std::string> getStringList(ObjectHandle h, int listCode) const override {
        auto it = stringLists.find({h, listCode});
        if (it == stringLists.end()) return {};
        return it->second;
    }

    iidm::optional<std::string> getKeyedString(ObjectHandle h, int p, const std::string& key) const override {
        auto it = keyedStrings.find(std::make_tuple(h, p, key));
        if (it == keyedStrings.end()) return {};
        return it->second;
    }

    void setKeyedString(ObjectHandle h, int p, const std::string& key, const iidm::optional<std::string>& value) override {
        if (value) keyedStrings[std::make_tuple(h, p, key)] = *value;
        else keyedStrings.erase(std::make_tuple(h, p, key));
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

    void setRelated(ObjectHandle h, int relation, ObjectHandle value) override {
        related[{h, relation}] = value;
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

    // ── Creation and removal ──────────────────────────────────────────────
    // Every invoke is recorded. invoke(parent, type) returns a new adder handle (lastAdder);
    // invoke(adder, ACT_ADD) returns addResults[adder] (INVALID_HANDLE by default);
    // ACT_CONNECT / ACT_DISCONNECT return the object (it changed) unless listed in unchanged.
    std::vector<std::pair<ObjectHandle,int>> invocations;
    std::map<ObjectHandle, ObjectHandle> addResults;
    ObjectHandle nextAdder = 100000;
    ObjectHandle lastAdder = INVALID_HANDLE;
    std::string createdNetworkId;
    std::set<ObjectHandle> unchanged;

    ObjectHandle invoke(ObjectHandle h, int code) override {
        invocations.push_back(std::make_pair(h, code));
        if (code == prop::ACT_REMOVE) return INVALID_HANDLE;
        if (code == prop::ACT_CONNECT || code == prop::ACT_DISCONNECT) return unchanged.count(h) ? INVALID_HANDLE : h;
        if (code == prop::ACT_ADD) {
            auto it = addResults.find(h);
            return it == addResults.end() ? INVALID_HANDLE : it->second;
        }
        lastAdder = nextAdder++;
        return lastAdder;
    }

    void createNetwork(const std::string& id) override { createdNetworkId = id; }
};

} // namespace iidm::test
