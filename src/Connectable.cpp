#include <iidm/Connectable.h>
#include <iidm/BackendProvider.h>
#include "Extensions.h"
#include "Wrap.h"

namespace iidm {

std::vector<Terminal> Connectable::getTerminals() const {
    return detail::wrapAll<Terminal>(backend_->getChildren(handle_, prop::TERMINAL), backend_);
}

bool Connectable::connect() {
    // The connectable itself when this connected it (its handle is released right away)
    return HandleRef(backend_->invoke(handle_, prop::ACT_CONNECT), backend_) != INVALID_HANDLE;
}

bool Connectable::disconnect() {
    return HandleRef(backend_->invoke(handle_, prop::ACT_DISCONNECT), backend_) != INVALID_HANDLE;
}

void Connectable::unsetSolvedValues() {
    backend_->invoke(handle_, prop::ACT_UNSET_SOLVED_VALUES);
}

bool Connectable::isEquivalent() const {
    return backend_->getBool(handle_, prop::EQUIVALENT);
}

Connectable& Connectable::setEquivalent(bool equivalent) {
    backend_->setBool(handle_, prop::EQUIVALENT, equivalent);
    return *this;
}

void Connectable::remove() {
    backend_->invoke(handle_, prop::ACT_REMOVE);
}

// ── Extensions ───────────────────────────────────────────────────────────────

bool Connectable::hasMeasurements() const {
    return detail::hasExtension<Measurements>(backend_, handle_);
}

Measurements Connectable::getMeasurements() const {
    return detail::getExtension<Measurements>(backend_, handle_, "Measurements");
}

MeasurementsAdder Connectable::newMeasurements() {
    return detail::newExtension<Measurements, MeasurementsAdder>(backend_, handle_);
}

void Connectable::removeMeasurements() {
    detail::removeExtension<Measurements>(backend_, handle_);
}

} // namespace iidm
