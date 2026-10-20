#include <iidm/Switch.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

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

int Switch::getNode1() const {
    return backend_->getInt(handle_, prop::SW_NODE1);
}

int Switch::getNode2() const {
    return backend_->getInt(handle_, prop::SW_NODE2);
}

} // namespace iidm
