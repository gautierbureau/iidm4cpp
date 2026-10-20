#include <iidm/BusbarSection.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

BusbarSection::BusbarSection(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(detail::orNullBackend(backend)) {}

std::string BusbarSection::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string BusbarSection::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

double BusbarSection::getV() const {
    return backend_->getDouble(handle_, prop::BBS_V);
}

double BusbarSection::getAngle() const {
    return backend_->getDouble(handle_, prop::BBS_ANGLE);
}

Terminal BusbarSection::getTerminal() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL);
    return Terminal(termHandle, backend_);
}

bool BusbarSection::operator==(const BusbarSection& other) const {
    return handle_ == other.handle_;
}

bool BusbarSection::operator!=(const BusbarSection& other) const {
    return !(*this == other);
}

// ── State estimation extensions ──────────────────────────────────────────────

bool BusbarSection::hasInjectionObservability() const {
    return backend_->getBool(handle_, prop::EXT_INJ_OBS_EXISTS);
}

InjectionObservability BusbarSection::getInjectionObservability() const {
    return InjectionObservability(handle_, backend_);
}

bool BusbarSection::hasMeasurements() const {
    return backend_->getBool(handle_, prop::EXT_MEAS_EXISTS);
}

Measurements BusbarSection::getMeasurements() const {
    return Measurements(handle_, backend_);
}

} // namespace iidm
