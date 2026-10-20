#include <iidm/ActivePowerControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include <limits>

namespace iidm {

ActivePowerControl::ActivePowerControl(ObjectHandle handle, BackendProvider* backend, int codeBase)
    : handle_(handle), backend_(detail::orNullBackend(backend)), codeBase_(codeBase) {}

ActivePowerControl::ActivePowerControl(ObjectHandle handle, BackendProvider* backend,
                                       int droopCode, int /*participateCode*/)
    : ActivePowerControl(handle, backend, droopCode - (prop::EXT_APC_DROOP - prop::EXT_APC_EXISTS)) {}

// Maps a Generator code (EXT_APC_*) onto the code block of this wrapper.
int ActivePowerControl::code(int generatorCode) const {
    return codeBase_ + (generatorCode - prop::EXT_APC_EXISTS);
}

bool ActivePowerControl::isParticipate() const {
    return backend_->getBool(handle_, code(prop::EXT_APC_PARTICIPATE));
}

ActivePowerControl& ActivePowerControl::setParticipate(bool participate) {
    backend_->setBool(handle_, code(prop::EXT_APC_PARTICIPATE), participate);
    return *this;
}

double ActivePowerControl::getDroop() const {
    return backend_->getDouble(handle_, code(prop::EXT_APC_DROOP));
}

ActivePowerControl& ActivePowerControl::setDroop(double droop) {
    backend_->setDouble(handle_, code(prop::EXT_APC_DROOP), droop);
    return *this;
}

double ActivePowerControl::getParticipationFactor() const {
    return backend_->getDouble(handle_, code(prop::EXT_APC_PARTICIPATION_FACTOR));
}

ActivePowerControl& ActivePowerControl::setParticipationFactor(double participationFactor) {
    backend_->setDouble(handle_, code(prop::EXT_APC_PARTICIPATION_FACTOR), participationFactor);
    return *this;
}

iidm::optional<double> ActivePowerControl::getMinTargetP() const {
    return backend_->getOptDouble(handle_, code(prop::EXT_APC_MIN_TARGET_P));
}

ActivePowerControl& ActivePowerControl::setMinTargetP(double minTargetP) {
    backend_->setDouble(handle_, code(prop::EXT_APC_MIN_TARGET_P), minTargetP);
    return *this;
}

ActivePowerControl& ActivePowerControl::unsetMinTargetP() {
    return setMinTargetP(std::numeric_limits<double>::quiet_NaN());
}

iidm::optional<double> ActivePowerControl::getMaxTargetP() const {
    return backend_->getOptDouble(handle_, code(prop::EXT_APC_MAX_TARGET_P));
}

ActivePowerControl& ActivePowerControl::setMaxTargetP(double maxTargetP) {
    backend_->setDouble(handle_, code(prop::EXT_APC_MAX_TARGET_P), maxTargetP);
    return *this;
}

ActivePowerControl& ActivePowerControl::unsetMaxTargetP() {
    return setMaxTargetP(std::numeric_limits<double>::quiet_NaN());
}

bool ActivePowerControl::operator==(const ActivePowerControl& other) const {
    return handle_ == other.handle_;
}

bool ActivePowerControl::operator!=(const ActivePowerControl& other) const {
    return !(*this == other);
}

} // namespace iidm
