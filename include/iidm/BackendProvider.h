#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/stdcxx/optional.hpp>
#include <cstddef>
#include <string>
#include <vector>

namespace iidm {

// The complete set of primitive operations any backend must provide.
// Component classes call exclusively into this interface; they have no
// backend-specific code.
class BackendProvider {
public:
    virtual ~BackendProvider() = default;

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

    // ── Lookup by ID ─────────────────────────────────────────────────────
    virtual ObjectHandle findById(int objectType, const std::string& id) const = 0;

    // ── Network root ─────────────────────────────────────────────────────
    virtual ObjectHandle getNetworkHandle() const = 0;

    // ── Lifecycle ────────────────────────────────────────────────────────
    virtual void loadNetwork(const std::string& filePath) = 0;
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
};

} // namespace iidm
