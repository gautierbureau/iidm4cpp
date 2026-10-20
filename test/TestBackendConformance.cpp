#include <gtest/gtest.h>
#include <iidm/iidm.h>
#include <iidm/HandleRef.h>
#include "RealBackends.h"
#include <cmath>
#include <cstdio>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

#if defined(IIDM_BRIDGE_GRAALVM_ENABLED) && defined(IIDM_BRIDGE_JNI_ENABLED)
#  include "graalvm/GraalVMBackend.h"
#  include "jni/JNIBackend.h"
#endif
#if defined(__GNUG__)
#  include <cxxabi.h>
#  include <cstdlib>
#endif

// The GraalVM and JNI backends must give the same answer to the same question.
//
// Each test network is loaded into both backends, in this process, and every object reachable
// from the network root is visited on both in lockstep. For every object and every code of
// PropertyCodes.def, each accessor must produce the same outcome on both backends: the same
// value, the same number of children, the same null / non-null related object (then visited in
// turn), or an exception of the same C++ type. Keyed and indexed lookups (getRelatedByString,
// getRelatedByIndex, findById) are tried with every id and node number met during the walk.
// invoke(object, TYPE), which starts an adder and changes nothing, is compared the same way. Every
// value read is then written back with the matching setter: both backends must accept or reject it
// alike, and read back the same value. Both networks must then save to the same XIIDM; last, every
// identifiable is removed (in reverse visit order), with the same outcome, and the XIIDM compared
// again. One case builds its network through the C++ adders instead of loading it.
//
// Every code is tried with every accessor on the first object reached through a given link (the
// network's GENERATOR children, a generator's REL_TERMINAL...). A code that fails there because
// it does not exist for that type of object (an "Unknown ..." code, or a code of another type:
// ClassCastException) is not tried again on the other objects reached through the same link;
// every other code is compared on every object.
//
// Needs both backends: IIDM_BRIDGE_GRAALVM_LIB and IIDM_BRIDGE_JAR (skipped otherwise).

#if defined(IIDM_BRIDGE_GRAALVM_ENABLED) && defined(IIDM_BRIDGE_JNI_ENABLED)

using namespace iidm;

namespace {

struct Code {
    const char* name;
    int value;
};

// Every code of PropertyCodes.def, once per value (the 3WT leg offsets reuse small values).
std::vector<Code> allCodes() {
    static const Code defined[] = {
#define IIDM_CODE(name, value) {#name, value},
#include <iidm/PropertyCodes.def>
#undef IIDM_CODE
    };
    std::vector<Code> codes;
    std::set<int> values;
    for (const Code& code : defined) {
        if (values.insert(code.value).second) codes.push_back(code);
    }
    return codes;
}

std::string typeName(const std::type_info& type) {
#if defined(__GNUG__)
    int status = 0;
    char* name = abi::__cxa_demangle(type.name(), nullptr, nullptr, &status);
    if (status == 0 && name) {
        std::string result(name);
        std::free(name);
        return result;
    }
#endif
    return type.name();
}

// What one call produced on one backend: a value rendered as text, or an exception.
struct Outcome {
    std::string value;
    const std::type_info* error = nullptr;
    std::string message;

    bool threw() const { return error != nullptr; }
    bool sameAs(const Outcome& other) const {
        if (threw() || other.threw()) return threw() && other.threw() && *error == *other.error;
        return value == other.value;
    }
    // Messages are not compared: they may name backend-specific objects.
    std::string describe() const {
        return threw() ? "throws " + typeName(*error) + " \"" + message + "\"" : "returns " + value;
    }
    // The code does not exist for this type of object, whatever its state.
    bool notForThisType() const {
        return threw() && *error == typeid(PropertyNotFoundException)
            && (message.compare(0, 8, "Unknown ") == 0 || message.find("ClassCastException") != std::string::npos);
    }
};

template <class F>
Outcome attempt(F call) {
    Outcome outcome;
    try {
        outcome.value = call();
    } catch (const std::exception& e) {
        outcome.error = &typeid(e);
        outcome.message = e.what();
    }
    return outcome;
}

std::string show(double value) {
    if (std::isnan(value)) return "NaN";
    char text[32];
    std::snprintf(text, sizeof text, "%.17g", value);
    return text;
}

std::string show(ObjectHandle related, std::vector<ObjectHandle>& objects) {
    if (related == INVALID_HANDLE) return "null";
    objects.push_back(related);
    return "an object";
}

// The read accessors of BackendProvider. Navigation results are rendered as a count (or null)
// and their handles are added to `objects` so that the walk can visit them.
typedef std::string (*ReadFn)(BackendProvider&, ObjectHandle, int code, std::vector<ObjectHandle>& objects);

struct Read {
    const char* name;
    ReadFn call;
};

enum ReadIndex { GET_DOUBLE, GET_OPT_DOUBLE, GET_INT, GET_BOOL, GET_STRING, GET_INT_LIST, GET_CHILDREN, GET_RELATED, READ_COUNT };

const Read kReads[READ_COUNT] = {
    {"getDouble", [](BackendProvider& b, ObjectHandle h, int code, std::vector<ObjectHandle>&) -> std::string {
        return show(b.getDouble(h, code));
    }},
    {"getOptDouble", [](BackendProvider& b, ObjectHandle h, int code, std::vector<ObjectHandle>&) -> std::string {
        iidm::optional<double> value = b.getOptDouble(h, code);
        return value ? show(*value) : "empty";
    }},
    {"getInt", [](BackendProvider& b, ObjectHandle h, int code, std::vector<ObjectHandle>&) -> std::string {
        return std::to_string(b.getInt(h, code));
    }},
    {"getBool", [](BackendProvider& b, ObjectHandle h, int code, std::vector<ObjectHandle>&) -> std::string {
        return b.getBool(h, code) ? "true" : "false";
    }},
    {"getString", [](BackendProvider& b, ObjectHandle h, int code, std::vector<ObjectHandle>&) -> std::string {
        return "\"" + b.getString(h, code) + "\"";
    }},
    {"getIntList", [](BackendProvider& b, ObjectHandle h, int code, std::vector<ObjectHandle>&) -> std::string {
        std::string text = "[";
        for (int value : b.getIntList(h, code)) text += " " + std::to_string(value);
        return text + " ]";
    }},
    {"getChildren", [](BackendProvider& b, ObjectHandle h, int code, std::vector<ObjectHandle>& objects) -> std::string {
        objects = b.getChildren(h, code);
        return std::to_string(objects.size()) + " children";
    }},
    {"getRelated", [](BackendProvider& b, ObjectHandle h, int code, std::vector<ObjectHandle>& objects) -> std::string {
        return show(b.getRelated(h, code), objects);
    }},
};

// Reads that also take a key (getRelatedByString) or an index (getRelatedByIndex).
typedef std::string (*LookupFn)(BackendProvider&, ObjectHandle, int code, const std::string& key, int index,
                                std::vector<ObjectHandle>& objects);

struct Lookup {
    const char* name;
    LookupFn call;
};

enum LookupIndex { BY_STRING, BY_INDEX, LOOKUP_COUNT };

const Lookup kLookups[LOOKUP_COUNT] = {
    {"getRelatedByString", [](BackendProvider& b, ObjectHandle h, int code, const std::string& key, int,
                              std::vector<ObjectHandle>& objects) -> std::string {
        return show(b.getRelatedByString(h, code, key), objects);
    }},
    {"getRelatedByIndex", [](BackendProvider& b, ObjectHandle h, int code, const std::string&, int index,
                             std::vector<ObjectHandle>& objects) -> std::string {
        return show(b.getRelatedByIndex(h, code, index), objects);
    }},
};

// An object returned by getRelated(code), on each backend: written back with setRelated.
struct Related {
    int code;
    HandleRef g;
    HandleRef j;
};

// The same Java object, as seen by each backend.
struct Twin {
    HandleRef g;       // GraalVM handle
    HandleRef j;       // JNI handle
    std::string path;  // how the walk reached it, for messages
    std::string link;  // the accessor and code that led to it: objects reached through the same link have the same type
    bool identifiable = false;
    std::vector<std::pair<int, int>> readable; // (read, code) of the scalar reads that returned a value
    std::vector<int> indices;                  // values of its int lists (node numbers): getRelatedByIndex keys
    std::vector<Related> related;
};

// Code sets kept per link: one per read, one per lookup, then the type codes invoke accepts
enum CodeSet { CREATION = READ_COUNT + LOOKUP_COUNT, CODE_SET_COUNT };

class Conformance {
public:
    Conformance(BackendProvider& g, BackendProvider& j) : g_(g), j_(j), codes_(allCodes()) {}

    void run() {
        compareInvalidHandles();
        Twin root;
        root.g = HandleRef(g_.getNetworkHandle(), &g_);
        root.j = HandleRef(j_.getNetworkHandle(), &j_);
        root.path = "network";
        root.link = "network";
        pending_.push_back(root);
        std::size_t lookedUp = 0;
        while (!pending_.empty()) {
            while (!pending_.empty()) {
                Twin twin = pending_.front();
                pending_.pop_front();
                visit(twin);
            }
            // Lookups take their keys from the whole walk; what they find is visited in the next round
            if (lookedUp == 0) findEveryId();
            for (; lookedUp < visited_.size(); ++lookedUp) lookUp(*visited_[lookedUp]);
        }
        for (const std::unique_ptr<Twin>& twin : visited_) writeBack(*twin);
        compareSavedNetworks();
        removeAll();
        compareSavedNetworks();
    }

    std::vector<std::string> mismatches;
    std::size_t comparisons = 0;
    std::size_t values = 0; // comparisons where both backends returned (not threw)
    std::size_t writes = 0; // setter calls accepted by both backends
    std::size_t adders = 0; // adders started by both backends
    std::size_t removals = 0;
    std::size_t objects() const { return visited_.size(); }
    std::size_t links() const { return codesByLink_.size(); }

private:
    BackendProvider& g_;
    BackendProvider& j_;
    const std::vector<Code> codes_;
    std::deque<Twin> pending_;
    std::vector<std::unique_ptr<Twin>> visited_;
    std::set<std::string> seen_; // ids of the identifiables visited, fingerprints of the other objects
    std::set<std::string> ids_;  // every id-like string read: keys for getRelatedByString and findById
    std::set<std::string> lookedUpLinks_;
    // For each link, the codes that exist for that type of object: per read, then per lookup
    std::map<std::string, std::vector<std::set<int>>> codesByLink_;

    bool compare(const std::string& what, const std::string& path, const char* code,
                 const Outcome& g, const Outcome& j) {
        ++comparisons;
        if (g.sameAs(j)) return true;
        mismatches.push_back(path + ": " + what + "(" + code + ")\n    GraalVM " + g.describe()
                             + "\n    JNI     " + j.describe());
        return false;
    }

    void compareInvalidHandles() {
        std::vector<ObjectHandle> none;
        for (const Read& read : kReads) {
            compare(read.name, "INVALID_HANDLE", "ID",
                    attempt([&] { return read.call(g_, INVALID_HANDLE, prop::ID, none); }),
                    attempt([&] { return read.call(j_, INVALID_HANDLE, prop::ID, none); }));
        }
    }

    // Twins built from the objects that a read or lookup returned on each backend.
    std::vector<Twin> twins(const Twin& from, const std::string& link, bool numbered,
                            const std::vector<ObjectHandle>& objectsG, const std::vector<ObjectHandle>& objectsJ) {
        std::vector<Twin> result;
        for (std::size_t i = 0; i < objectsG.size(); ++i) {
            Twin twin;
            twin.g = HandleRef(objectsG[i], &g_);
            twin.j = HandleRef(i < objectsJ.size() ? objectsJ[i] : INVALID_HANDLE, &j_);
            twin.link = link;
            twin.path = from.path + " > " + link + (numbered ? "[" + std::to_string(i) + "]" : "");
            result.push_back(twin);
        }
        for (std::size_t i = objectsG.size(); i < objectsJ.size(); ++i) j_.releaseHandle(objectsJ[i]);
        return result;
    }

    void visit(Twin& twin) {
        // Identifiables are visited once, through the first link that reaches them
        std::vector<ObjectHandle> none;
        Outcome idG = attempt([&] { return kReads[GET_STRING].call(g_, twin.g, prop::ID, none); });
        Outcome idJ = attempt([&] { return kReads[GET_STRING].call(j_, twin.j, prop::ID, none); });
        if (!compare("getString", twin.path, "ID", idG, idJ)) return;
        if (!idG.threw()) {
            if (!seen_.insert("id " + idG.value).second) return;
            twin.path += " " + idG.value;
            twin.identifiable = true;
        }

        std::vector<std::set<int>>& known = codesByLink_[twin.link];
        bool firstOfItsType = known.empty();
        if (firstOfItsType) known.resize(CODE_SET_COUNT);
        std::string fingerprint;
        std::vector<Twin> next;
        bool otherType = false;
        for (int read = 0; read < READ_COUNT; ++read) {
            for (const Code& code : codes_) {
                bool exists = known[read].count(code.value) > 0;
                if (!firstOfItsType && !exists) continue;
                if (probe(twin, read, code, fingerprint, next).notForThisType()) {
                    otherType = otherType || exists;
                } else {
                    known[read].insert(code.value);
                }
            }
        }
        if (otherType) {
            // Reached through the same link as objects of another type: try the other codes too
            for (int read = 0; read < READ_COUNT; ++read) {
                for (const Code& code : codes_) {
                    if (known[read].count(code.value)) continue;
                    if (!probe(twin, read, code, fingerprint, next).notForThisType()) known[read].insert(code.value);
                }
            }
        }

        probeCreation(twin, known[CREATION], firstOfItsType);

        // Objects without id (terminals, steps, limits...) are recognized by what they returned
        if (idG.threw() && !seen_.insert("object " + fingerprint).second) return;
        visited_.push_back(std::unique_ptr<Twin>(new Twin(twin)));
        for (const Twin& target : next) pending_.push_back(target);
    }

    // Compares one read of one code, and records what the rest of the walk needs from it.
    Outcome probe(Twin& twin, int read, const Code& code, std::string& fingerprint, std::vector<Twin>& next) {
        std::vector<ObjectHandle> objectsG, objectsJ;
        Outcome g = attempt([&] { return kReads[read].call(g_, twin.g, code.value, objectsG); });
        Outcome j = attempt([&] { return kReads[read].call(j_, twin.j, code.value, objectsJ); });
        const std::string link = std::string(kReads[read].name) + "(" + code.name + ")";
        std::vector<Twin> targets = twins(twin, link, read == GET_CHILDREN, objectsG, objectsJ);
        if (!compare(kReads[read].name, twin.path, code.name, g, j) || g.threw()) return g;

        ++values;
        fingerprint += link + " " + g.value + "\n";
        switch (read) {
        case GET_DOUBLE: case GET_INT: case GET_BOOL:
            twin.readable.push_back(std::make_pair(read, code.value));
            break;
        case GET_STRING: {
            twin.readable.push_back(std::make_pair(read, code.value));
            std::string name = code.name;
            if (name == "ID" || (name.size() > 3 && name.compare(name.size() - 3, 3, "_ID") == 0)) {
                ids_.insert(g.value.substr(1, g.value.size() - 2));
            }
            break;
        }
        case GET_INT_LIST:
            for (int value : g_.getIntList(twin.g, code.value)) twin.indices.push_back(value);
            break;
        case GET_RELATED:
            for (const Twin& target : targets) {
                Related related = {code.value, target.g, target.j};
                twin.related.push_back(related);
            }
            break;
        default:
            break;
        }
        next.insert(next.end(), targets.begin(), targets.end());
        return g;
    }

    // invoke(object, TYPE) starts an adder and changes nothing: compared, then released unused.
    // ACT_ADD and ACT_REMOVE change the network: removal is compared in removeAll.
    void probeCreation(Twin& twin, std::set<int>& creatable, bool firstOfItsType) {
        for (const Code& code : codes_) {
            if (code.value == prop::ACT_ADD || code.value == prop::ACT_REMOVE) continue;
            if (!firstOfItsType && !creatable.count(code.value)) continue;
            std::vector<ObjectHandle> objectsG, objectsJ;
            Outcome g = attempt([&] { return show(g_.invoke(twin.g, code.value), objectsG); });
            Outcome j = attempt([&] { return show(j_.invoke(twin.j, code.value), objectsJ); });
            for (ObjectHandle h : objectsG) g_.releaseHandle(h);
            for (ObjectHandle h : objectsJ) j_.releaseHandle(h);
            if (compare("invoke", twin.path, code.name, g, j) && !g.threw()) ++adders;
            if (!g.notForThisType()) creatable.insert(code.value);
        }
    }

    // Removes every identifiable, in reverse visit order: equipment before its voltage level
    void removeAll() {
        for (auto it = visited_.rbegin(); it != visited_.rend(); ++it) {
            Twin& twin = **it;
            if (!twin.identifiable) continue;
            Outcome g = attempt([&] { g_.invoke(twin.g, prop::ACT_REMOVE); return std::string(); });
            Outcome j = attempt([&] { j_.invoke(twin.j, prop::ACT_REMOVE); return std::string(); });
            if (compare("invoke", twin.path, "ACT_REMOVE", g, j) && !g.threw()) ++removals;
        }
    }

    // getRelatedByString with every id read during the walk as key, getRelatedByIndex with the
    // object's int list values (node numbers) as index. Like reads, every code is tried on the
    // first object of each link (with key "" and index -1), then only the codes that exist.
    void lookUp(Twin& twin) {
        std::vector<std::set<int>>& known = codesByLink_[twin.link];
        const bool firstOfItsType = lookedUpLinks_.insert(twin.link).second;
        for (int lookup = 0; lookup < LOOKUP_COUNT; ++lookup) {
            std::set<int>& exists = known[READ_COUNT + lookup];
            for (const Code& code : codes_) {
                if (!firstOfItsType && !exists.count(code.value)) continue;
                Outcome probe = lookUp(twin, lookup, code, "", -1);
                if (probe.notForThisType()) continue;
                exists.insert(code.value);
                if (probe.threw() && *probe.error == typeid(ExtensionNotFoundException)) continue; // whatever the key
                if (lookup == BY_STRING) {
                    for (const std::string& key : ids_) lookUp(twin, lookup, code, key, -1);
                } else {
                    for (int index : twin.indices) lookUp(twin, lookup, code, "", index);
                }
            }
        }
    }

    Outcome lookUp(Twin& twin, int lookup, const Code& code, const std::string& key, int index) {
        std::vector<ObjectHandle> objectsG, objectsJ;
        Outcome g = attempt([&] { return kLookups[lookup].call(g_, twin.g, code.value, key, index, objectsG); });
        Outcome j = attempt([&] { return kLookups[lookup].call(j_, twin.j, code.value, key, index, objectsJ); });
        const std::string link = std::string(kLookups[lookup].name) + "(" + code.name + ")";
        std::vector<Twin> targets = twins(twin, link + (lookup == BY_STRING ? " \"" + key + "\"" : " " + std::to_string(index)),
                                          false, objectsG, objectsJ);
        for (Twin& target : targets) target.link = link;
        if (compare(kLookups[lookup].name, twin.path, code.name, g, j) && !g.threw()) {
            ++values;
            for (const Twin& target : targets) pending_.push_back(target);
        }
        return g;
    }

    // findById for every object type and every id read during the walk.
    void findEveryId() {
        Twin network;
        network.path = "network";
        for (const Code& code : codes_) {
            std::vector<ObjectHandle> none;
            Outcome probe = attempt([&] { return show(g_.findById(code.value, ""), none); });
            for (ObjectHandle h : none) g_.releaseHandle(h);
            if (probe.notForThisType()) continue;
            for (const std::string& id : ids_) {
                std::vector<ObjectHandle> objectsG, objectsJ;
                Outcome g = attempt([&] { return show(g_.findById(code.value, id), objectsG); });
                Outcome j = attempt([&] { return show(j_.findById(code.value, id), objectsJ); });
                const std::string link = std::string("findById(") + code.name + ")";
                std::vector<Twin> targets = twins(network, link + " \"" + id + "\"", false, objectsG, objectsJ);
                for (Twin& target : targets) target.link = link;
                if (compare("findById", "network", code.name, g, j) && !g.threw()) {
                    ++values;
                    for (const Twin& target : targets) pending_.push_back(target);
                }
            }
        }
    }

    const char* nameOf(int value) const {
        for (const Code& code : codes_) {
            if (code.value == value) return code.name;
        }
        return "?";
    }

    // Writes back every value read with the matching setter, then reads it again.
    void writeBack(Twin& twin) {
        std::vector<ObjectHandle> none;
        for (const std::pair<int, int>& value : twin.readable) {
            const int read = value.first;
            const int code = value.second;
            std::string setter;
            Outcome g, j;
            try {
                switch (read) {
                case GET_DOUBLE: {
                    setter = "setDouble";
                    double v = g_.getDouble(twin.g, code);
                    g = attempt([&] { g_.setDouble(twin.g, code, v); return std::string(); });
                    j = attempt([&] { j_.setDouble(twin.j, code, v); return std::string(); });
                    break;
                }
                case GET_INT: {
                    setter = "setInt";
                    int v = g_.getInt(twin.g, code);
                    g = attempt([&] { g_.setInt(twin.g, code, v); return std::string(); });
                    j = attempt([&] { j_.setInt(twin.j, code, v); return std::string(); });
                    break;
                }
                case GET_BOOL: {
                    setter = "setBool";
                    bool v = g_.getBool(twin.g, code);
                    g = attempt([&] { g_.setBool(twin.g, code, v); return std::string(); });
                    j = attempt([&] { j_.setBool(twin.j, code, v); return std::string(); });
                    break;
                }
                default: {
                    setter = "setString";
                    std::string v = g_.getString(twin.g, code);
                    g = attempt([&] { g_.setString(twin.g, code, v); return std::string(); });
                    j = attempt([&] { j_.setString(twin.j, code, v); return std::string(); });
                    break;
                }
                }
            } catch (const std::exception&) {
                continue; // no longer readable after an earlier write: already compared
            }
            if (!compare(setter, twin.path, nameOf(code), g, j) || g.threw()) continue;
            ++writes;
            compare(std::string(kReads[read].name) + " after " + setter, twin.path, nameOf(code),
                    attempt([&] { return kReads[read].call(g_, twin.g, code, none); }),
                    attempt([&] { return kReads[read].call(j_, twin.j, code, none); }));
        }
        for (const Related& related : twin.related) {
            Outcome g = attempt([&] { g_.setRelated(twin.g, related.code, related.g); return std::string(); });
            Outcome j = attempt([&] { j_.setRelated(twin.j, related.code, related.j); return std::string(); });
            if (compare("setRelated", twin.path, nameOf(related.code), g, j) && !g.threw()) ++writes;
        }
    }

    void compareSavedNetworks() {
        Outcome g = attempt([&] { return g_.saveNetworkBytes("network.xiidm"); });
        Outcome j = attempt([&] { return j_.saveNetworkBytes("network.xiidm"); });
        if (g.threw() || j.threw() || g.value == j.value) {
            compare("saveNetworkBytes", "network", "", g, j);
            return;
        }
        ++comparisons;
        std::size_t at = 0;
        while (at < g.value.size() && at < j.value.size() && g.value[at] == j.value[at]) ++at;
        std::size_t from = g.value.rfind('\n', at);
        from = from == std::string::npos ? 0 : from + 1;
        mismatches.push_back("network: saveNetworkBytes() differs from:\n    GraalVM " + g.value.substr(from, 160)
                             + "\n    JNI     " + j.value.substr(from, 160));
    }
};

// Runs the walk on a network prepared the same way on both backends.
void checkConformance(const std::string& name, const std::function<void(BackendProvider&)>& prepare) {
    const char* lib = test::envPath("IIDM_BRIDGE_GRAALVM_LIB");
    if (!lib || !test::ensureJvm()) GTEST_SKIP() << "IIDM_BRIDGE_GRAALVM_LIB and IIDM_BRIDGE_JAR must both be set";

    GraalVMBackend graalvm(lib);
    JNIBackend jni("backend_conformance_test");
    prepare(graalvm);
    prepare(jni);

    Conformance conformance(graalvm, jni);
    conformance.run();
    std::printf("%s: %zu objects (%zu kinds of links), %zu comparisons, %zu values, %zu writes, %zu adders, %zu removals\n",
                name.c_str(), conformance.objects(), conformance.links(), conformance.comparisons, conformance.values,
                conformance.writes, conformance.adders, conformance.removals);

    std::string report;
    for (std::size_t i = 0; i < conformance.mismatches.size() && i < 30; ++i) report += conformance.mismatches[i] + "\n";
    EXPECT_TRUE(conformance.mismatches.empty())
        << conformance.mismatches.size() << " differences between GraalVM and JNI, first ones:\n" << report;
    EXPECT_GT(conformance.values, 100u);
    EXPECT_GT(conformance.writes, 10u);
    EXPECT_GT(conformance.adders, 0u);
    EXPECT_GT(conformance.removals, 0u);
}

void checkConformance(const char* network) {
    const std::string xiidm = test::readFile(network);
    ASSERT_FALSE(xiidm.empty()) << network << " not found";
    checkConformance(network, [&](BackendProvider& b) { b.loadNetworkBytes(xiidm.data(), xiidm.size(), "network.xiidm"); });
}

// The same network built through the C++ adders on each backend
void buildNetwork(BackendProvider& b) {
    b.createNetwork("created");
    const ObjectHandle network = b.getNetworkHandle();
    b.setString(network, prop::NETWORK_CASE_DATE, "2026-01-31T12:00Z"); // else the creation time
    b.setInt(network, prop::NETWORK_FORECAST_DISTANCE, 60);
    Substation s = SubstationAdder(b.invoke(network, prop::SUBSTATION), &b)
                     .setId("S").setCountry(Country::FR).setTso("RTE").add();
    VoltageLevel vl1 = s.newVoltageLevel().setId("VL1").setNominalV(400).setLowVoltageLimit(380)
                         .setHighVoltageLimit(420).setTopologyKind(TopologyKind::BUS_BREAKER).add();
    vl1.getBusBreakerView().newBus().setId("B1").add();
    vl1.getBusBreakerView().newBus().setId("B1b").setName("spare").add();
    // Without substation
    VoltageLevel vl2 = VoltageLevelAdder(b.invoke(network, prop::VOLTAGE_LEVEL), &b)
                         .setId("VL2").setNominalV(225).setTopologyKind(TopologyKind::BUS_BREAKER).add();
    vl2.getBusBreakerView().newBus().setId("B2").add();
    Load load = vl2.newLoad().setId("L").setName("load").setBus("B2").setConnectableBus("B2")
                  .setP0(80).setQ0(10).setLoadType(LoadType::AUXILIARY).add();
    vl2.newLoad().setId("L2").setConnectableBus("B2").setP0(5).setQ0(1).setFictitious(true).add(); // disconnected
    Generator g1 = vl1.newGenerator().setId("G1").setBus("B1").setMinP(0).setMaxP(200).setTargetP(100)
                     .setRatedS(250).setEnergySource(EnergySource::NUCLEAR).setLocalTargetV(405).setLocalTargetQ(0)
                     .newVoltageRegulation().withMode(RegulationMode::VOLTAGE).withRegulating(true).withTargetDeadband(1).add()
                     .add();
    g1.newActivePowerControl().withDroop(4).withParticipate(true).withParticipationFactor(0.5).withMaxTargetP(150).add();
    vl2.newGenerator().setId("G2").setBus("B2").setMinP(-50).setMaxP(50).setTargetP(10).setLocalTargetQ(5)
        .newVoltageRegulation().withMode(RegulationMode::REACTIVE_POWER).withTerminal(load.getTerminal())
            .withTargetValue(-10).withRegulating(true).add()
        .add();
    vl2.newGenerator().setId("G3").setBus("B2").setMinP(0).setMaxP(10).setTargetP(1).setLocalTargetQ(0).add();
}

} // namespace

TEST(BackendConformanceTest, IEEE14) {
    checkConformance("../examples/IEEE14.xiidm");
}

TEST(BackendConformanceTest, Extensions) {
    checkConformance("../examples/extensions_test.xiidm");
}

TEST(BackendConformanceTest, StateEstimation) {
    checkConformance("../examples/state_estimation_test.xiidm");
}

TEST(BackendConformanceTest, BackendErrors) {
    checkConformance("data/backend_errors_test.xiidm");
}

TEST(BackendConformanceTest, CreatedFromScratch) {
    checkConformance("created from scratch", buildNetwork);
}

#endif
