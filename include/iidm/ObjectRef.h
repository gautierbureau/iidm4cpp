#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/HandleRef.h>
#include <iidm/IidmException.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>
#include <string>

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

    // Whether the object is a T (Java instanceof): identifiable.is<Generator>(),
    // limits.is<ReactiveCapabilityCurve>()...
    template <class T> bool is() const { return backend_->getBool(handle_, T::TYPE); }
    // This object as a T; PropertyNotFoundException when it is not one
    template <class T> T as() const {
        if (!is<T>()) {
            throw PropertyNotFoundException("The object is not of the requested type (type code "
                                            + std::to_string(T::TYPE) + ")");
        }
        return T(handle_, backend_);
    }

protected:
    ObjectRef() = default;
    ObjectRef(ObjectHandle handle, BackendProvider* backend)
        : handle_(handle, backend), backend_(orNullBackend(backend)) {}

    // The accessors of the wrappers, by property code: double getP0() const { return getDouble(prop::LOAD_P0); }
    double getDouble(int code) const { return backend_->getDouble(handle_, code); }
    int getInt(int code) const { return backend_->getInt(handle_, code); }
    bool getBool(int code) const { return backend_->getBool(handle_, code); }
    std::string getString(int code) const { return backend_->getString(handle_, code); }
    iidm::optional<double> getOptDouble(int code) const { return backend_->getOptDouble(handle_, code); }
    iidm::optional<int> getOptInt(int code) const { return backend_->getOptInt(handle_, code); }
    iidm::optional<std::string> getOptString(int code) const { return backend_->getOptString(handle_, code); }

    // Setters return the wrapper: Load& setP0(double p0) { return setDouble(*this, prop::LOAD_P0, p0); }
    template <class Self> Self& setDouble(Self& self, int code, double value) {
        backend_->setDouble(handle_, code, value);
        return self;
    }
    template <class Self> Self& setInt(Self& self, int code, int value) {
        backend_->setInt(handle_, code, value);
        return self;
    }
    template <class Self> Self& setBool(Self& self, int code, bool value) {
        backend_->setBool(handle_, code, value);
        return self;
    }
    template <class Self> Self& setString(Self& self, int code, const std::string& value) {
        backend_->setString(handle_, code, value);
        return self;
    }

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
