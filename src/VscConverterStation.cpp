#include <iidm/VscConverterStation.h>
#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/ReactiveCapabilityCurve.h>
#include <iidm/HvdcLine.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

bool VscConverterStation::isVoltageRegulatorOn() const {
    return backend_->getBool(handle_, prop::VSC_VOLTAGE_REGULATOR_ON);
}

VscConverterStation& VscConverterStation::setVoltageRegulatorOn(bool on) {
    backend_->setBool(handle_, prop::VSC_VOLTAGE_REGULATOR_ON, on);
    return *this;
}

double VscConverterStation::getVoltageSetpoint() const {
    return backend_->getDouble(handle_, prop::VSC_VOLTAGE_SETPOINT);
}

VscConverterStation& VscConverterStation::setVoltageSetpoint(double value) {
    backend_->setDouble(handle_, prop::VSC_VOLTAGE_SETPOINT, value);
    return *this;
}

double VscConverterStation::getReactivePowerSetpoint() const {
    return backend_->getDouble(handle_, prop::VSC_REACTIVE_POWER_SETPOINT);
}

VscConverterStation& VscConverterStation::setReactivePowerSetpoint(double value) {
    backend_->setDouble(handle_, prop::VSC_REACTIVE_POWER_SETPOINT, value);
    return *this;
}

bool VscConverterStation::hasMinMaxReactiveLimits() const {
    return backend_->getInt(handle_, prop::VSC_REACTIVE_LIMITS_KIND) == 1;
}

MinMaxReactiveLimits VscConverterStation::getMinMaxReactiveLimits() const {
    return MinMaxReactiveLimits(handle_, backend_, prop::VSC_MIN_Q, prop::VSC_MAX_Q);
}

bool VscConverterStation::hasReactiveCapabilityCurve() const {
    return backend_->getInt(handle_, prop::VSC_REACTIVE_LIMITS_KIND) == 2;
}

ReactiveCapabilityCurve VscConverterStation::getReactiveCapabilityCurve() const {
    return ReactiveCapabilityCurve(handle_, backend_);
}

Terminal VscConverterStation::getRegulatingTerminal() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_REGULATING_TERMINAL);
    return Terminal(termHandle, backend_);
}

} // namespace iidm
