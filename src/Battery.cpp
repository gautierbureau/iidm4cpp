#include <iidm/Battery.h>
#include <iidm/ActivePowerControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>
#include "Extensions.h"

namespace iidm {

void Battery::applySolvedValues() {
    backend_->invoke(handle_, prop::ACT_APPLY_SOLVED_VALUES);
}

void Battery::setTargetPToP() {
    backend_->invoke(handle_, prop::ACT_SET_TARGET_P_TO_P);
}

bool Battery::hasBatteryShortCircuit() const {
    return detail::hasExtension<BatteryShortCircuit>(backend_, handle_);
}

BatteryShortCircuit Battery::getBatteryShortCircuit() const {
    return detail::getExtension<BatteryShortCircuit>(backend_, handle_, "BatteryShortCircuit");
}

BatteryShortCircuitAdder Battery::newBatteryShortCircuit() {
    return detail::newExtension<BatteryShortCircuit, BatteryShortCircuitAdder>(backend_, handle_);
}

void Battery::removeBatteryShortCircuit() {
    detail::removeExtension<BatteryShortCircuit>(backend_, handle_);
}

} // namespace iidm
