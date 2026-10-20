#pragma once

#include <cstdint>

namespace iidm {

// An ObjectHandle is an opaque integer that uniquely identifies a Java
// object within a backend session.  It is valid for the lifetime of the
// Network that produced it.
using ObjectHandle = std::int64_t;
constexpr ObjectHandle INVALID_HANDLE = 0;

class BackendProvider;

namespace detail {
// Backend of objects that are not bound to a network (default-constructed wrappers,
// moved-from Network): every call throws InvalidHandleException instead of
// dereferencing a null pointer.
BackendProvider* nullBackend() noexcept;
inline BackendProvider* orNullBackend(BackendProvider* backend) noexcept {
    return backend ? backend : nullBackend();
}
} // namespace detail

} // namespace iidm
