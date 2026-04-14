#include <iidm/VscConverterStation.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

VscConverterStation::VscConverterStation(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string VscConverterStation::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string VscConverterStation::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

float VscConverterStation::getLossFactor() const {
    return static_cast<float>(backend_->getDouble(handle_, prop::VSC_LOSS_FACTOR));
}

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

Terminal VscConverterStation::getTerminal() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL);
    return Terminal(termHandle, backend_);
}

void VscConverterStation::connect() {
    getTerminal().connect();
}

void VscConverterStation::disconnect() {
    getTerminal().disconnect();
}

bool VscConverterStation::isConnected() const {
    return getTerminal().isConnected();
}

bool VscConverterStation::hasExtension(const std::string& name) const {
    return backend_->getExtensionHandle(handle_, name) != INVALID_HANDLE;
}

Extension VscConverterStation::getExtension(const std::string& name) const {
    return Extension(name, backend_->getExtensionHandle(handle_, name), backend_);
}

std::vector<Extension> VscConverterStation::getExtensions() const {
    std::vector<std::string> names = backend_->getExtensionNames(handle_);
    std::vector<Extension> result;
    result.reserve(names.size());
    for (const auto& n : names)
        result.emplace_back(n, backend_->getExtensionHandle(handle_, n), backend_);
    return result;
}

bool VscConverterStation::operator==(const VscConverterStation& other) const {
    return handle_ == other.handle_;
}

bool VscConverterStation::operator!=(const VscConverterStation& other) const {
    return !(*this == other);
}

} // namespace iidm
