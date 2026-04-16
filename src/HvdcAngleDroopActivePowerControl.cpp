#include <iidm/HvdcAngleDroopActivePowerControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

HvdcAngleDroopActivePowerControl::HvdcAngleDroopActivePowerControl(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

double HvdcAngleDroopActivePowerControl::getDroop() const {
    return backend_->getDouble(handle_, prop::EXT_HADAPC_DROOP);
}

HvdcAngleDroopActivePowerControl& HvdcAngleDroopActivePowerControl::setDroop(double droop) {
    backend_->setDouble(handle_, prop::EXT_HADAPC_DROOP, droop);
    return *this;
}

double HvdcAngleDroopActivePowerControl::getP0() const {
    return backend_->getDouble(handle_, prop::EXT_HADAPC_P0);
}

HvdcAngleDroopActivePowerControl& HvdcAngleDroopActivePowerControl::setP0(double p0) {
    backend_->setDouble(handle_, prop::EXT_HADAPC_P0, p0);
    return *this;
}

bool HvdcAngleDroopActivePowerControl::isEnabled() const {
    return backend_->getBool(handle_, prop::EXT_HADAPC_ENABLED);
}

HvdcAngleDroopActivePowerControl& HvdcAngleDroopActivePowerControl::setEnabled(bool enabled) {
    backend_->setBool(handle_, prop::EXT_HADAPC_ENABLED, enabled);
    return *this;
}

bool HvdcAngleDroopActivePowerControl::operator==(const HvdcAngleDroopActivePowerControl& other) const {
    return handle_ == other.handle_;
}

bool HvdcAngleDroopActivePowerControl::operator!=(const HvdcAngleDroopActivePowerControl& other) const {
    return !(*this == other);
}

} // namespace iidm
