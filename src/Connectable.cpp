#include <iidm/Connectable.h>
#include <iidm/BackendProvider.h>
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
    return backend_->getBool(handle_, prop::EXT_MEAS_EXISTS);
}

Measurements Connectable::getMeasurements() const {
    return Measurements(handle_, backend_);
}

} // namespace iidm
