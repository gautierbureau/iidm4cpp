#pragma once

#include <iidm/ObjectHandle.h>
#include <string>

namespace iidm {

class BackendProvider;

// Wraps an IIDM extension object attached to an identifiable network element.
// Attribute values are returned as strings regardless of underlying type
// (callers may parse as needed).  An invalid Extension (isValid() == false)
// represents a missing extension.
class Extension {
public:
    Extension() = default;
    explicit Extension(std::string name, ObjectHandle handle, BackendProvider* backend);

    // Extension name as registered in PowSyBl (e.g. "activePowerControl").
    std::string getName() const { return name_; }

    // Returns the named attribute as a string.
    // Derives the getter by capitalising the first letter of key and trying
    // getKey() then isKey() on the underlying Java object.
    // Throws if the attribute is unknown for this extension type.
    std::string getAttribute(const std::string& key) const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Extension& other) const;
    bool operator!=(const Extension& other) const;

private:
    std::string      name_;
    ObjectHandle     handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
