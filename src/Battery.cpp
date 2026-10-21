#include <iidm/Battery.h>
#include <iidm/ActivePowerControl.h>
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

Battery& Battery::setMinP(double minP) {
    backend_->setDouble(handle_, prop::BAT_MIN_P, minP);
    return *this;
}

double Battery::getMaxP() const {
    return backend_->getDouble(handle_, prop::BAT_MAX_P);
}

Battery& Battery::setMaxP(double maxP) {
    backend_->setDouble(handle_, prop::BAT_MAX_P, maxP);
    return *this;
}

void Battery::applySolvedValues() {
    backend_->invoke(handle_, prop::ACT_APPLY_SOLVED_VALUES);
}

void Battery::setTargetPToP() {
    backend_->invoke(handle_, prop::ACT_SET_TARGET_P_TO_P);
}

} // namespace iidm
