#include <iidm/ShuntCompensator.h>
#include <iidm/ShuntCompensatorNonLinearModel.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

ShuntCompensator::ShuntCompensator(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle, backend), backend_(detail::orNullBackend(backend)) {}

std::string ShuntCompensator::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string ShuntCompensator::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

int ShuntCompensator::getSectionCount() const {
    return backend_->getInt(handle_, prop::SHUNT_SECTION_COUNT);
}

ShuntCompensator& ShuntCompensator::setSectionCount(int count) {
    backend_->setInt(handle_, prop::SHUNT_SECTION_COUNT, count);
    return *this;
}

int ShuntCompensator::getMaximumSectionCount() const {
    return backend_->getInt(handle_, prop::SHUNT_MAX_SECTION_COUNT);
}

bool ShuntCompensator::hasLinearModel() const {
    return backend_->getInt(handle_, prop::SHUNT_MODEL_KIND) == 0;
}

double ShuntCompensator::getBPerSection() const {
    return backend_->getDouble(handle_, prop::SHUNT_B_PER_SECTION);
}

double ShuntCompensator::getGPerSection() const {
    return backend_->getDouble(handle_, prop::SHUNT_G_PER_SECTION);
}

double ShuntCompensator::getB() const {
    return backend_->getDouble(handle_, prop::SHUNT_B);
}

bool ShuntCompensator::hasNonLinearModel() const {
    return backend_->getInt(handle_, prop::SHUNT_MODEL_KIND) == 1;
}

ShuntCompensatorNonLinearModel ShuntCompensator::getNonLinearModel() const {
    return ShuntCompensatorNonLinearModel(handle_, backend_);
}

Terminal ShuntCompensator::getTerminal() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL);
    return Terminal(termHandle, backend_);
}

bool ShuntCompensator::isVoltageRegulatorOn() const {
    return backend_->getBool(handle_, prop::SHUNT_VOLTAGE_REGULATOR_ON);
}

double ShuntCompensator::getTargetV() const {
    return backend_->getDouble(handle_, prop::SHUNT_TARGET_V);
}

std::string ShuntCompensator::getRegulatingTerminalId() const {
    return backend_->getString(handle_, prop::SHUNT_REGULATING_TERMINAL_ID);
}

Terminal ShuntCompensator::getRegulatingTerminal() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_REGULATING_TERMINAL);
    return Terminal(termHandle, backend_);
}

void ShuntCompensator::connect() {
    getTerminal().connect();
}

void ShuntCompensator::disconnect() {
    getTerminal().disconnect();
}

bool ShuntCompensator::isConnected() const {
    return getTerminal().isConnected();
}

bool ShuntCompensator::operator==(const ShuntCompensator& other) const {
    return handle_ == other.handle_;
}

bool ShuntCompensator::operator!=(const ShuntCompensator& other) const {
    return !(*this == other);
}

// ── State estimation extensions ──────────────────────────────────────────────

bool ShuntCompensator::hasInjectionObservability() const {
    return backend_->getBool(handle_, prop::EXT_INJ_OBS_EXISTS);
}

InjectionObservability ShuntCompensator::getInjectionObservability() const {
    return InjectionObservability(handle_, backend_);
}

bool ShuntCompensator::hasMeasurements() const {
    return backend_->getBool(handle_, prop::EXT_MEAS_EXISTS);
}

Measurements ShuntCompensator::getMeasurements() const {
    return Measurements(handle_, backend_);
}

bool ShuntCompensator::hasDiscreteMeasurements() const {
    return backend_->getBool(handle_, prop::EXT_DMEAS_EXISTS);
}

DiscreteMeasurements ShuntCompensator::getDiscreteMeasurements() const {
    return DiscreteMeasurements(handle_, backend_);
}

} // namespace iidm
