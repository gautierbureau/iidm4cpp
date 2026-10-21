#include <iidm/TieLine.h>
#include <iidm/BackendProvider.h>
#include <iidm/HandleRef.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

iidm::optional<std::string> TieLine::getPairingKey() const {
    return backend_->getOptString(handle_, prop::BL_PAIRING_KEY);
}

double TieLine::getR() const {
    return backend_->getDouble(handle_, prop::LINE_R);
}

double TieLine::getX() const {
    return backend_->getDouble(handle_, prop::LINE_X);
}

double TieLine::getG1() const {
    return backend_->getDouble(handle_, prop::LINE_G1);
}

double TieLine::getB1() const {
    return backend_->getDouble(handle_, prop::LINE_B1);
}

double TieLine::getG2() const {
    return backend_->getDouble(handle_, prop::LINE_G2);
}

double TieLine::getB2() const {
    return backend_->getDouble(handle_, prop::LINE_B2);
}

BoundaryLine TieLine::getBoundaryLine1() const {
    return getBoundaryLine(TwoSides::ONE);
}

BoundaryLine TieLine::getBoundaryLine2() const {
    return getBoundaryLine(TwoSides::TWO);
}

BoundaryLine TieLine::getBoundaryLine(TwoSides side) const {
    return BoundaryLine(backend_->getRelatedByIndex(handle_, prop::BOUNDARY_LINE, static_cast<int>(side)), backend_);
}

iidm::optional<BoundaryLine> TieLine::getBoundaryLine(const std::string& voltageLevelId) const {
    return detail::wrapOptional<BoundaryLine>(backend_->getRelatedByString(handle_, prop::BOUNDARY_LINE, voltageLevelId), backend_);
}

bool TieLine::connectBoundaryLines() {
    // The tie line itself when this connected them (its handle is released right away)
    return HandleRef(backend_->invoke(handle_, prop::ACT_CONNECT), backend_) != INVALID_HANDLE;
}

bool TieLine::disconnectBoundaryLines() {
    return HandleRef(backend_->invoke(handle_, prop::ACT_DISCONNECT), backend_) != INVALID_HANDLE;
}

void TieLine::remove() {
    backend_->invoke(handle_, prop::ACT_REMOVE);
}

void TieLine::remove(bool updateBoundaryLines) {
    backend_->invoke(handle_, updateBoundaryLines ? prop::ACT_REMOVE_UPDATING_BOUNDARY_LINES : prop::ACT_REMOVE);
}

} // namespace iidm
