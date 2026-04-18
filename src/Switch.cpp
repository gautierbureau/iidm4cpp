#include <iidm/Switch.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

Switch::Switch(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string Switch::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string Switch::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

bool Switch::isOpen() const {
    return backend_->getBool(handle_, prop::SW_OPEN);
}

Switch& Switch::setOpen(bool open) {
    backend_->setBool(handle_, prop::SW_OPEN, open);
    return *this;
}

bool Switch::isRetained() const {
    return backend_->getBool(handle_, prop::SW_RETAINED);
}

Switch& Switch::setRetained(bool retained) {
    backend_->setBool(handle_, prop::SW_RETAINED, retained);
    return *this;
}

SwitchKind Switch::getKind() const {
    return static_cast<SwitchKind>(backend_->getInt(handle_, prop::SW_KIND));
}

bool Switch::operator==(const Switch& other) const {
    return handle_ == other.handle_;
}

bool Switch::operator!=(const Switch& other) const {
    return !(*this == other);
}

} // namespace iidm
