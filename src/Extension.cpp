#include <iidm/Extension.h>
#include <iidm/BackendProvider.h>

namespace iidm {

Extension::Extension(std::string name, ObjectHandle handle, BackendProvider* backend)
    : name_(std::move(name)), handle_(handle), backend_(backend) {}

std::string Extension::getAttribute(const std::string& key) const {
    return backend_->getExtensionAttribute(handle_, key);
}

bool Extension::operator==(const Extension& other) const {
    return handle_ == other.handle_;
}

bool Extension::operator!=(const Extension& other) const {
    return !(*this == other);
}

} // namespace iidm
