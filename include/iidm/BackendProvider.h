#pragma once

#include <iidm/IidmException.h>
#include <iidm/ObjectHandle.h>
#include <iidm/stdcxx/optional.hpp>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace iidm {

namespace detail {
class HandleTracker;
}

// The complete set of primitive operations any backend must provide.
// Component classes call exclusively into this interface; they have no
// backend-specific code.
class BackendProvider {
public:
    BackendProvider();
    virtual ~BackendProvider();

    // ── Handle lifetime ──────────────────────────────────────────────────
    // Wrappers hold their handle through HandleRef, which calls releaseHandle() when the
    // last wrapper using a handle is destroyed. Default: nothing to release.
    virtual void releaseHandle(ObjectHandle /*h*/) {}
    // Tracker shared by the HandleRefs of this backend.
    const std::shared_ptr<detail::HandleTracker>& handleTracker() const { return tracker_; }
    // Number of handles currently held by wrappers (diagnostics, tests).
    std::size_t liveHandleCount() const;

    // ── Scalar property getters ──────────────────────────────────────────
    virtual double      getDouble(ObjectHandle h, int property) const = 0;
    virtual int         getInt(ObjectHandle h, int property) const = 0;
    virtual bool        getBool(ObjectHandle h, int property) const = 0;
    virtual std::string getString(ObjectHandle h, int property) const = 0;

    // ── Scalar property setters ──────────────────────────────────────────
    virtual void setDouble(ObjectHandle h, int property, double value) = 0;
    virtual void setInt   (ObjectHandle h, int property, int value) = 0;
    virtual void setBool  (ObjectHandle h, int property, bool value) = 0;
    virtual void setString(ObjectHandle h, int property, const std::string& value) = 0;

    // ── Optional scalars (returns empty optional when undefined/NaN) ───────
    virtual iidm::optional<double> getOptDouble(ObjectHandle h, int property) const = 0;

    // Properties that may have no value, as they are (getString(h, NAME) is the name or else the
    // id, getOptString(h, NAME) the name only). Any int or string property can be read this way.
    // Not pure, like the primitives below, so that existing BackendProvider implementations keep
    // compiling.
    virtual iidm::optional<int> getOptInt(ObjectHandle /*h*/, int property) const {
        throw IidmException("getOptInt not supported by this backend (property " + std::to_string(property) + ")");
    }
    virtual iidm::optional<std::string> getOptString(ObjectHandle /*h*/, int property) const {
        throw IidmException("getOptString not supported by this backend (property " + std::to_string(property) + ")");
    }

    // ── String lists (property names, aliases...) ────────────────────────
    virtual std::vector<std::string> getStringList(ObjectHandle /*h*/, int listCode) const {
        throw IidmException("getStringList not supported by this backend (list " + std::to_string(listCode) + ")");
    }

    // ── Keyed strings (properties, aliases) ──────────────────────────────
    // Empty when there is no value under the key; setting an empty optional removes the key.
    virtual iidm::optional<std::string> getKeyedString(ObjectHandle /*h*/, int property, const std::string& /*key*/) const {
        throw IidmException("getKeyedString not supported by this backend (property " + std::to_string(property) + ")");
    }
    virtual void setKeyedString(ObjectHandle /*h*/, int property, const std::string& /*key*/,
                                const iidm::optional<std::string>& /*value*/) {
        throw IidmException("setKeyedString not supported by this backend (property " + std::to_string(property) + ")");
    }

    // ── Methods that take arguments ──────────────────────────────────────
    // The result of a method of the object that takes numeric arguments (ints, bools and enum
    // ordinals are passed as doubles): evaluate(limits, prop::RL_MIN_Q, {p}) is the powsybl
    // getMinQ(p) of reactive limits. A bool result is 1 or 0.
    virtual double evaluate(ObjectHandle /*h*/, int code, const std::vector<double>& /*args*/) const {
        throw IidmException("evaluate not supported by this backend (code " + std::to_string(code) + ")");
    }

    // ── Identity ─────────────────────────────────────────────────────────
    // Whether two handles stand for the same object: each lookup returns a new handle.
    virtual bool isSameObject(ObjectHandle h, ObjectHandle other) const {
        return h == other;
    }

    // ── Navigation (children, terminals, etc.) ───────────────────────────
    virtual std::vector<ObjectHandle> getChildren(ObjectHandle h, int childType) const = 0;
    virtual ObjectHandle              getRelated(ObjectHandle h, int relation) const = 0;
    virtual std::vector<int>          getIntList(ObjectHandle h, int listCode) const = 0;
    virtual ObjectHandle              getRelatedByIndex(ObjectHandle h, int relation, int index) const = 0;
    virtual ObjectHandle              getRelatedByString(ObjectHandle h, int relation, const std::string& key) const = 0;

    // Sets an object-valued property (e.g. REL_REGULATION_TERMINAL); INVALID_HANDLE stands for null.
    // Not pure so that existing BackendProvider implementations keep compiling.
    virtual void setRelated(ObjectHandle /*h*/, int relation, ObjectHandle /*value*/) {
        throw IidmException("setRelated not supported by this backend (relation " + std::to_string(relation) + ")");
    }

    // ── Creation and removal ─────────────────────────────────────────────
    // invoke(parent, <type code>): starts a new object of that type under parent and returns an
    //   adder; its fields are set with the setters and the object's own property codes, and
    //   nothing changes in the network until invoke(adder, prop::ACT_ADD), which adds the object
    //   and returns it (INVALID_HANDLE for a nested adder, e.g. a generator's voltage regulation).
    // invoke(object, prop::ACT_REMOVE): removes an identifiable from its network.
    // A change that takes several values at once goes the same way: invoke(regulation,
    // prop::REL_TERMINAL), set its terminal and target value, then invoke(change, prop::ACT_ADD).
    // Not pure so that existing BackendProvider implementations keep compiling.
    virtual ObjectHandle invoke(ObjectHandle /*h*/, int code) {
        throw IidmException("invoke not supported by this backend (code " + std::to_string(code) + ")");
    }

    // ── Lookup by ID ─────────────────────────────────────────────────────
    virtual ObjectHandle findById(int objectType, const std::string& id) const = 0;

    // ── Network root ─────────────────────────────────────────────────────
    virtual ObjectHandle getNetworkHandle() const = 0;

    // ── Lifecycle ────────────────────────────────────────────────────────
    virtual void loadNetwork(const std::string& filePath) = 0;
    // A new, empty network with this id becomes the backend's network.
    virtual void createNetwork(const std::string& id) {
        throw IidmException("createNetwork not supported by this backend (network " + id + ")");
    }
    // Saves a network of this backend to an XIIDM file: its network (getNetworkHandle()), one of
    // its subnetworks, or a network detached from it.
    virtual void saveNetwork(ObjectHandle network, const std::string& filePath) = 0;

    // ── In-memory load/save ──────────────────────────────────────────────
    // Load a network from an in-memory byte buffer. `filenameHint` lets
    // PowSyBl pick the right importer (e.g. "network.xiidm").
    virtual void loadNetworkBytes(const char* data, std::size_t length,
                                  const std::string& filenameHint) = 0;
    // Serialize a network of this backend (as for saveNetwork) into an in-memory byte buffer.
    // `filenameHint` selects the export format (only "*.xiidm" is supported today).
    virtual std::string saveNetworkBytes(ObjectHandle network, const std::string& filenameHint) const = 0;

    virtual void close() = 0;

protected:
    // For close(): releases every handle still held by wrappers; later releases are no-ops.
    void releaseAllHandles() noexcept;

private:
    std::shared_ptr<detail::HandleTracker> tracker_;
};

} // namespace iidm
