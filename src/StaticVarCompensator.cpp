#include <iidm/StaticVarCompensator.h>
#include <iidm/VoltagePerReactivePowerControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

double StaticVarCompensator::getBMin() const {
    return backend_->getDouble(handle_, prop::SVC_B_MIN);
}

double StaticVarCompensator::getBMax() const {
    return backend_->getDouble(handle_, prop::SVC_B_MAX);
}

double StaticVarCompensator::getVoltageSetpoint() const {
    return backend_->getDouble(handle_, prop::SVC_VOLTAGE_SETPOINT);
}

StaticVarCompensator& StaticVarCompensator::setVoltageSetpoint(double value) {
    backend_->setDouble(handle_, prop::SVC_VOLTAGE_SETPOINT, value);
    return *this;
}

double StaticVarCompensator::getReactivePowerSetpoint() const {
    return backend_->getDouble(handle_, prop::SVC_REACTIVE_POWER_SETPOINT);
}

StaticVarCompensator& StaticVarCompensator::setReactivePowerSetpoint(double value) {
    backend_->setDouble(handle_, prop::SVC_REACTIVE_POWER_SETPOINT, value);
    return *this;
}

StaticVarCompensatorRegulationMode StaticVarCompensator::getRegulationMode() const {
    return static_cast<StaticVarCompensatorRegulationMode>(
        backend_->getInt(handle_, prop::SVC_REGULATION_MODE));
}

StaticVarCompensator& StaticVarCompensator::setRegulationMode(StaticVarCompensatorRegulationMode mode) {
    backend_->setInt(handle_, prop::SVC_REGULATION_MODE, static_cast<int>(mode));
    return *this;
}

Terminal StaticVarCompensator::getRegulatingTerminal() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_REGULATING_TERMINAL);
    return Terminal(termHandle, backend_);
}

bool StaticVarCompensator::hasVoltagePerReactivePowerControl() const {
    return backend_->getBool(handle_, prop::EXT_VPRC_EXISTS);
}

VoltagePerReactivePowerControl StaticVarCompensator::getVoltagePerReactivePowerControl() const {
    return VoltagePerReactivePowerControl(handle_, backend_);
}

} // namespace iidm
