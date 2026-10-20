#include <iidm/Battery.h>
#include <iidm/ActivePowerControl.h>
#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/ReactiveCapabilityCurve.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

double Battery::getTargetP() const {
    return backend_->getDouble(handle_, prop::BAT_TARGET_P);
}

Battery& Battery::setTargetP(double targetP) {
    backend_->setDouble(handle_, prop::BAT_TARGET_P, targetP);
    return *this;
}

double Battery::getMinP() const {
    return backend_->getDouble(handle_, prop::BAT_MIN_P);
}

double Battery::getMaxP() const {
    return backend_->getDouble(handle_, prop::BAT_MAX_P);
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

} // namespace iidm
