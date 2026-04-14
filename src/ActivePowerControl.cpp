#include <iidm/ActivePowerControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

ActivePowerControl::ActivePowerControl(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

bool ActivePowerControl::isParticipate() const {
    return backend_->getBool(handle_, prop::EXT_APC_PARTICIPATE);
}

ActivePowerControl& ActivePowerControl::setParticipate(bool participate) {
    backend_->setBool(handle_, prop::EXT_APC_PARTICIPATE, participate);
    return *this;
}

double ActivePowerControl::getDroop() const {
    return backend_->getDouble(handle_, prop::EXT_APC_DROOP);
}

ActivePowerControl& ActivePowerControl::setDroop(double droop) {
    backend_->setDouble(handle_, prop::EXT_APC_DROOP, droop);
    return *this;
}

bool ActivePowerControl::operator==(const ActivePowerControl& other) const {
    return handle_ == other.handle_;
}

bool ActivePowerControl::operator!=(const ActivePowerControl& other) const {
    return !(*this == other);
}

} // namespace iidm
