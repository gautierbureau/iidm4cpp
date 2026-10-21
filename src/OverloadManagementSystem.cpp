#include <iidm/OverloadManagementSystem.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Substation.h>
#include "Wrap.h"

namespace iidm {

// ── Tripping ──────────────────────────────────────────────────────────────────

// ── Trippings of each kind ────────────────────────────────────────────────────

OverloadManagementSystem::ThreeWindingsTransformerTripping&
OverloadManagementSystem::ThreeWindingsTransformerTripping::setThreeWindingsTransformerToOperateId(const std::string& transformerId) {
    backend_->setString(handle_, prop::TRIPPING_ELEMENT_ID, transformerId);
    return *this;
}

OverloadManagementSystem::ThreeWindingsTransformerTripping&
OverloadManagementSystem::ThreeWindingsTransformerTripping::setSideToOperate(ThreeSides side) {
    backend_->setInt(handle_, prop::TRIPPING_SIDE, static_cast<int>(side));
    return *this;
}

// ── OverloadManagementSystem ──────────────────────────────────────────────────

Substation OverloadManagementSystem::getSubstation() const {
    return Substation(backend_->getRelated(handle_, prop::REL_SUBSTATION), backend_);
}

std::vector<OverloadManagementSystem::Tripping> OverloadManagementSystem::getTrippings() const {
    return detail::wrapAll<Tripping>(backend_->getChildren(handle_, prop::TRIPPING), backend_);
}

void OverloadManagementSystem::addTripping(const Tripping& tripping) {
    backend_->setRelated(handle_, prop::ADD_TRIPPING, detail::Access::handle(tripping));
}

void OverloadManagementSystem::remove() {
    backend_->invoke(handle_, prop::ACT_REMOVE);
}

} // namespace iidm
