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

    // ── Navigation (children, terminals, etc.) ───────────────────────────
    virtual std::vector<ObjectHandle> getChildren(ObjectHandle h, int childType) const = 0;
    virtual ObjectHandle              getRelated(ObjectHandle h, int relation) const = 0;
    virtual std::vector<int>          getIntList(ObjectHandle h, int listCode) const = 0;
    virtual ObjectHandle              getRelatedByIndex(ObjectHandle h, int relation, int index) const = 0;
    virtual ObjectHandle              getRelatedByString(ObjectHandle h, int relation, const std::string& key) const = 0;

    // Sets an object-valued property (e.g. REL_SLACK_TERMINAL); INVALID_HANDLE stands for null.
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
    virtual void saveNetwork(const std::string& filePath) = 0;

    // ── In-memory load/save ──────────────────────────────────────────────
    // Load a network from an in-memory byte buffer. `filenameHint` lets
    // PowSyBl pick the right importer (e.g. "network.xiidm").
    virtual void loadNetworkBytes(const char* data, std::size_t length,
                                  const std::string& filenameHint) = 0;
    // Serialize the current network into an in-memory byte buffer.
    // `filenameHint` selects the export format (only "*.xiidm" is supported today).
    virtual std::string saveNetworkBytes(const std::string& filenameHint) const = 0;

    virtual void close() = 0;

protected:
    // For close(): releases every handle still held by wrappers; later releases are no-ops.
    void releaseAllHandles() noexcept;

private:
    std::shared_ptr<detail::HandleTracker> tracker_;
};

} // namespace iidm
