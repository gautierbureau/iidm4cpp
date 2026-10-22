#include <iidm/TieLine.h>
#include <iidm/BackendProvider.h>
#include <iidm/HandleRef.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

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

bool TieLine::connectBoundaryLines(SwitchPredicate switches) {
    return detail::connectThrough(backend_, handle_, prop::CONNECT, switches, -1);
}

bool TieLine::connectBoundaryLines(SwitchPredicate switches, TwoSides side) {
    return detail::connectThrough(backend_, handle_, prop::CONNECT, switches, static_cast<int>(side));
}

bool TieLine::disconnectBoundaryLines(SwitchPredicate switches) {
    return detail::connectThrough(backend_, handle_, prop::DISCONNECT, switches, -1);
}

bool TieLine::disconnectBoundaryLines(SwitchPredicate switches, TwoSides side) {
    return detail::connectThrough(backend_, handle_, prop::DISCONNECT, switches, static_cast<int>(side));
}

void TieLine::remove() {
    backend_->invoke(handle_, prop::ACT_REMOVE);
}

void TieLine::remove(bool updateBoundaryLines) {
    backend_->invoke(handle_, updateBoundaryLines ? prop::ACT_REMOVE_UPDATING_BOUNDARY_LINES : prop::ACT_REMOVE);
}

} // namespace iidm
