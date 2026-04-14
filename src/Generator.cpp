#include <iidm/Generator.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

Generator::Generator(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string Generator::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string Generator::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

double Generator::getTargetP() const {
    return backend_->getDouble(handle_, prop::GEN_TARGET_P);
}

Generator& Generator::setTargetP(double targetP) {
    backend_->setDouble(handle_, prop::GEN_TARGET_P, targetP);
    return *this;
}

double Generator::getTargetQ() const {
    return backend_->getDouble(handle_, prop::GEN_TARGET_Q);
}

Generator& Generator::setTargetQ(double targetQ) {
    backend_->setDouble(handle_, prop::GEN_TARGET_Q, targetQ);
    return *this;
}

double Generator::getTargetV() const {
    return backend_->getDouble(handle_, prop::GEN_TARGET_V);
}

Generator& Generator::setTargetV(double targetV) {
    backend_->setDouble(handle_, prop::GEN_TARGET_V, targetV);
    return *this;
}

double Generator::getMinP() const {
    return backend_->getDouble(handle_, prop::GEN_MIN_P);
}

double Generator::getMaxP() const {
    return backend_->getDouble(handle_, prop::GEN_MAX_P);
}

bool Generator::isVoltageRegulatorOn() const {
    return backend_->getBool(handle_, prop::GEN_VOLTAGE_REGULATOR_ON);
}

Generator& Generator::setVoltageRegulatorOn(bool on) {
    backend_->setBool(handle_, prop::GEN_VOLTAGE_REGULATOR_ON, on);
    return *this;
}

EnergySource Generator::getEnergySource() const {
    return static_cast<EnergySource>(backend_->getInt(handle_, prop::GEN_ENERGY_SOURCE));
}

Terminal Generator::getTerminal() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL);
    return Terminal(termHandle, backend_);
}

void Generator::connect() {
    getTerminal().connect();
}

void Generator::disconnect() {
    getTerminal().disconnect();
}

bool Generator::isConnected() const {
    return getTerminal().isConnected();
}

bool Generator::operator==(const Generator& other) const {
    return handle_ == other.handle_;
}

bool Generator::operator!=(const Generator& other) const {
    return !(*this == other);
}

} // namespace iidm
