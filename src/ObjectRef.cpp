#include <iidm/ObjectRef.h>
#include <iidm/BackendProvider.h>

namespace iidm { namespace detail {

bool ObjectRef::operator==(const ObjectRef& other) const {
    const bool valid = handle_ != INVALID_HANDLE;
    if (!valid || other.handle_ == INVALID_HANDLE) return valid == (other.handle_ != INVALID_HANDLE);
    if (backend_ != other.backend_) return false; // objects of different networks
    return handle_ == other.handle_ || backend_->isSameObject(handle_, other.handle_);
}

} } // namespace iidm::detail
