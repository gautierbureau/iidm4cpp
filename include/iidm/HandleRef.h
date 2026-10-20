#pragma once

#include <iidm/ObjectHandle.h>
#include <memory>

namespace iidm {

class BackendProvider;

namespace detail {
class HandleTracker;
}

// A backend object handle owned by a wrapper (Generator, Terminal...).
//
// Every handle returned by a backend holds a Java-side reference (a JNI global ref, or a
// GraalVM registry entry). HandleRef counts the wrappers using a handle, across copies and
// across sub-wrappers built from the same handle (extensions and views read through their
// host), and asks the backend to release it when the last one is destroyed. Converts
// implicitly to ObjectHandle, so wrappers use it as a plain handle.
class HandleRef {
public:
    HandleRef() = default;
    HandleRef(ObjectHandle handle, BackendProvider* backend);
    HandleRef(const HandleRef& other);
    HandleRef(HandleRef&& other) noexcept;
    HandleRef& operator=(const HandleRef& other);
    HandleRef& operator=(HandleRef&& other) noexcept;
    ~HandleRef();

    operator ObjectHandle() const { return handle_; }

private:
    void retain();
    void release() noexcept;

    ObjectHandle                           handle_ = INVALID_HANDLE;
    std::shared_ptr<detail::HandleTracker> tracker_;
};

} // namespace iidm
