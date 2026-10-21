#include <iidm/StandbyAutomaton.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

StandbyAutomaton::StandbyAutomaton(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

bool StandbyAutomaton::isStandby() const {
    return backend_->getBool(handle_, prop::EXT_SA_STANDBY);
}

StandbyAutomaton& StandbyAutomaton::setStandby(bool standby) {
    backend_->setBool(handle_, prop::EXT_SA_STANDBY, standby);
    return *this;
}

double StandbyAutomaton::getB0() const {
    return backend_->getDouble(handle_, prop::EXT_SA_B0);
}

StandbyAutomaton& StandbyAutomaton::setB0(double b0) {
    backend_->setDouble(handle_, prop::EXT_SA_B0, b0);
    return *this;
}

double StandbyAutomaton::getLowVoltageSetpoint() const {
    return backend_->getDouble(handle_, prop::EXT_SA_LOW_VOLTAGE_SETPOINT);
}

StandbyAutomaton& StandbyAutomaton::setLowVoltageSetpoint(double lowVoltageSetpoint) {
    backend_->setDouble(handle_, prop::EXT_SA_LOW_VOLTAGE_SETPOINT, lowVoltageSetpoint);
    return *this;
}

double StandbyAutomaton::getHighVoltageSetpoint() const {
    return backend_->getDouble(handle_, prop::EXT_SA_HIGH_VOLTAGE_SETPOINT);
}

StandbyAutomaton& StandbyAutomaton::setHighVoltageSetpoint(double highVoltageSetpoint) {
    backend_->setDouble(handle_, prop::EXT_SA_HIGH_VOLTAGE_SETPOINT, highVoltageSetpoint);
    return *this;
}

double StandbyAutomaton::getLowVoltageThreshold() const {
    return backend_->getDouble(handle_, prop::EXT_SA_LOW_VOLTAGE_THRESHOLD);
}

StandbyAutomaton& StandbyAutomaton::setLowVoltageThreshold(double lowVoltageThreshold) {
    backend_->setDouble(handle_, prop::EXT_SA_LOW_VOLTAGE_THRESHOLD, lowVoltageThreshold);
    return *this;
}

double StandbyAutomaton::getHighVoltageThreshold() const {
    return backend_->getDouble(handle_, prop::EXT_SA_HIGH_VOLTAGE_THRESHOLD);
}

StandbyAutomaton& StandbyAutomaton::setHighVoltageThreshold(double highVoltageThreshold) {
    backend_->setDouble(handle_, prop::EXT_SA_HIGH_VOLTAGE_THRESHOLD, highVoltageThreshold);
    return *this;
}

} // namespace iidm
