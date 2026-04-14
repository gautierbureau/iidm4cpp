#pragma once

#include <iidm/ObjectHandle.h>
#include <optional>
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

    // ── Optional scalars (returns std::nullopt when undefined/NaN) ───────
    virtual std::optional<double> getOptDouble(ObjectHandle h, int property) const = 0;

    // ── Navigation (children, terminals, etc.) ───────────────────────────
    virtual std::vector<ObjectHandle> getChildren(ObjectHandle h, int childType) const = 0;
    virtual ObjectHandle              getRelated(ObjectHandle h, int relation) const = 0;

    // ── Lookup by ID ─────────────────────────────────────────────────────
    virtual ObjectHandle findById(int objectType, const std::string& id) const = 0;

    // ── Extensions ───────────────────────────────────────────────────────
    // Returns a handle to the named extension on the identifiable object,
    // or INVALID_HANDLE when the extension is absent.
    virtual ObjectHandle getExtensionHandle(ObjectHandle h, const std::string& name) const = 0;
    // Returns the names of all extensions attached to the identifiable object.
    virtual std::vector<std::string> getExtensionNames(ObjectHandle h) const = 0;
    // Returns the named attribute of an extension object as a string.
    // Tries getKey() first, then isKey() on the underlying Java object.
    virtual std::string getExtensionAttribute(ObjectHandle extensionHandle, const std::string& key) const = 0;

    // ── Network root ─────────────────────────────────────────────────────
    virtual ObjectHandle getNetworkHandle() const = 0;

    // ── Lifecycle ────────────────────────────────────────────────────────
    virtual void loadNetwork(const std::string& filePath) = 0;
    virtual void close() = 0;
};

} // namespace iidm
