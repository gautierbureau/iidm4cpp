#include <iidm/ActivePowerControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include <limits>

namespace iidm {

ActivePowerControl::ActivePowerControl(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

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

double ActivePowerControl::getParticipationFactor() const {
    return backend_->getDouble(handle_, prop::EXT_APC_PARTICIPATION_FACTOR);
}

ActivePowerControl& ActivePowerControl::setParticipationFactor(double participationFactor) {
    backend_->setDouble(handle_, prop::EXT_APC_PARTICIPATION_FACTOR, participationFactor);
    return *this;
}

iidm::optional<double> ActivePowerControl::getMinTargetP() const {
    return backend_->getOptDouble(handle_, prop::EXT_APC_MIN_TARGET_P);
}

ActivePowerControl& ActivePowerControl::setMinTargetP(double minTargetP) {
    backend_->setDouble(handle_, prop::EXT_APC_MIN_TARGET_P, minTargetP);
    return *this;
}

ActivePowerControl& ActivePowerControl::unsetMinTargetP() {
    return setMinTargetP(std::numeric_limits<double>::quiet_NaN());
}

iidm::optional<double> ActivePowerControl::getMaxTargetP() const {
    return backend_->getOptDouble(handle_, prop::EXT_APC_MAX_TARGET_P);
}

ActivePowerControl& ActivePowerControl::setMaxTargetP(double maxTargetP) {
    backend_->setDouble(handle_, prop::EXT_APC_MAX_TARGET_P, maxTargetP);
    return *this;
}

ActivePowerControl& ActivePowerControl::unsetMaxTargetP() {
    return setMaxTargetP(std::numeric_limits<double>::quiet_NaN());
}

} // namespace iidm
