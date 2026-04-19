#include <iidm/Battery.h>
#include <iidm/ActivePowerControl.h>
#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/ReactiveCapabilityCurve.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

Battery::Battery(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string Battery::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string Battery::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

double Battery::getTargetP() const {
    return backend_->getDouble(handle_, prop::BAT_TARGET_P);
}

Battery& Battery::setTargetP(double targetP) {
    backend_->setDouble(handle_, prop::BAT_TARGET_P, targetP);
    return *this;
}

double Battery::getTargetQ() const {
    return backend_->getDouble(handle_, prop::BAT_TARGET_Q);
}

Battery& Battery::setTargetQ(double targetQ) {
    backend_->setDouble(handle_, prop::BAT_TARGET_Q, targetQ);
    return *this;
}

double Battery::getMinP() const {
    return backend_->getDouble(handle_, prop::BAT_MIN_P);
}

double Battery::getMaxP() const {
    return backend_->getDouble(handle_, prop::BAT_MAX_P);
}

bool Battery::hasActivePowerControl() const {
    return backend_->getBool(handle_, prop::EXT_BAT_APC_EXISTS);
}

ActivePowerControl Battery::getActivePowerControl() const {
    return ActivePowerControl(handle_, backend_, prop::EXT_BAT_APC_DROOP, prop::EXT_BAT_APC_PARTICIPATE);
}

bool Battery::hasMinMaxReactiveLimits() const {
    return backend_->getInt(handle_, prop::BAT_REACTIVE_LIMITS_KIND) == 1;
}

MinMaxReactiveLimits Battery::getMinMaxReactiveLimits() const {
    return MinMaxReactiveLimits(handle_, backend_, prop::BAT_MIN_Q, prop::BAT_MAX_Q);
}

bool Battery::hasReactiveCapabilityCurve() const {
    return backend_->getInt(handle_, prop::BAT_REACTIVE_LIMITS_KIND) == 2;
}

ReactiveCapabilityCurve Battery::getReactiveCapabilityCurve() const {
    return ReactiveCapabilityCurve(handle_, backend_);
}

Terminal Battery::getTerminal() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL);
    return Terminal(termHandle, backend_);
}

void Battery::connect() {
    getTerminal().connect();
}

void Battery::disconnect() {
    getTerminal().disconnect();
}

bool Battery::isConnected() const {
    return getTerminal().isConnected();
}

bool Battery::operator==(const Battery& other) const {
    return handle_ == other.handle_;
}

bool Battery::operator!=(const Battery& other) const {
    return !(*this == other);
}

} // namespace iidm
