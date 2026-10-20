#pragma once

#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>

namespace iidm {
namespace detail {

struct Access;

// Root of the wrappers: the Java object a wrapper stands for, and the backend that holds it.
// Wrappers have no other state, so a wrapper converts to any of its base classes (a Generator
// to a Connectable) without losing anything.
class ObjectRef {
public:
    // Bound to an object (default-constructed wrappers, and absent related objects, are not)
    bool isValid() const { return handle_ != INVALID_HANDLE; }

    // Whether both stand for the same Java object. Handles do not tell: each lookup of an object
    // returns a new handle. Objects that are not valid are all equal.
    bool operator==(const ObjectRef& other) const;
    bool operator!=(const ObjectRef& other) const { return !(*this == other); }

protected:
    ObjectRef() = default;
    ObjectRef(ObjectHandle handle, BackendProvider* backend)
        : handle_(handle, backend), backend_(orNullBackend(backend)) {}

    HandleRef        handle_;
    BackendProvider* backend_ = nullBackend();

    friend struct Access;
};

// The handle and backend of any wrapper, for the library's own use (mixins, wrappers passing
// another wrapper to the backend).
struct Access {
    static ObjectHandle handle(const ObjectRef& object) { return object.handle_; }
    static BackendProvider* backend(const ObjectRef& object) { return object.backend_; }
};

} // namespace detail
} // namespace iidm
