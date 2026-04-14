#include <iidm/StaticVarCompensator.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

StaticVarCompensator::StaticVarCompensator(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string StaticVarCompensator::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string StaticVarCompensator::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

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

Terminal StaticVarCompensator::getTerminal() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL);
    return Terminal(termHandle, backend_);
}

void StaticVarCompensator::connect() {
    getTerminal().connect();
}

void StaticVarCompensator::disconnect() {
    getTerminal().disconnect();
}

bool StaticVarCompensator::isConnected() const {
    return getTerminal().isConnected();
}

bool StaticVarCompensator::hasExtension(const std::string& name) const {
    return backend_->getExtensionHandle(handle_, name) != INVALID_HANDLE;
}

Extension StaticVarCompensator::getExtension(const std::string& name) const {
    return Extension(name, backend_->getExtensionHandle(handle_, name), backend_);
}

std::vector<Extension> StaticVarCompensator::getExtensions() const {
    std::vector<std::string> names = backend_->getExtensionNames(handle_);
    std::vector<Extension> result;
    result.reserve(names.size());
    for (const auto& n : names)
        result.emplace_back(n, backend_->getExtensionHandle(handle_, n), backend_);
    return result;
}

bool StaticVarCompensator::operator==(const StaticVarCompensator& other) const {
    return handle_ == other.handle_;
}

bool StaticVarCompensator::operator!=(const StaticVarCompensator& other) const {
    return !(*this == other);
}

} // namespace iidm
