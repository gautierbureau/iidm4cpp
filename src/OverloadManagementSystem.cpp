#include <iidm/OverloadManagementSystem.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Substation.h>
#include "Wrap.h"

namespace iidm {

// ── Tripping ──────────────────────────────────────────────────────────────────

OverloadManagementSystem::Tripping::Type OverloadManagementSystem::Tripping::getType() const {
    return static_cast<Type>(backend_->getInt(handle_, prop::TRIPPING_TYPE));
}

std::string OverloadManagementSystem::Tripping::getKey() const {
    return backend_->getString(handle_, prop::TRIPPING_KEY);
}

std::string OverloadManagementSystem::Tripping::getNameOrKey() const {
    return backend_->getString(handle_, prop::TRIPPING_NAME);
}

OverloadManagementSystem::Tripping& OverloadManagementSystem::Tripping::setName(const std::string& name) {
    backend_->setString(handle_, prop::TRIPPING_NAME, name);
    return *this;
}

double OverloadManagementSystem::Tripping::getCurrentLimit() const {
    return backend_->getDouble(handle_, prop::TRIPPING_CURRENT_LIMIT);
}

OverloadManagementSystem::Tripping& OverloadManagementSystem::Tripping::setCurrentLimit(double currentLimit) {
    backend_->setDouble(handle_, prop::TRIPPING_CURRENT_LIMIT, currentLimit);
    return *this;
}

bool OverloadManagementSystem::Tripping::isOpenAction() const {
    return backend_->getBool(handle_, prop::TRIPPING_OPEN_ACTION);
}

OverloadManagementSystem::Tripping& OverloadManagementSystem::Tripping::setOpenAction(bool openAction) {
    backend_->setBool(handle_, prop::TRIPPING_OPEN_ACTION, openAction);
    return *this;
}

// ── Trippings of each kind ────────────────────────────────────────────────────

std::string OverloadManagementSystem::SwitchTripping::getSwitchToOperateId() const {
    return backend_->getString(handle_, prop::TRIPPING_ELEMENT_ID);
}

OverloadManagementSystem::SwitchTripping& OverloadManagementSystem::SwitchTripping::setSwitchToOperateId(const std::string& switchId) {
    backend_->setString(handle_, prop::TRIPPING_ELEMENT_ID, switchId);
    return *this;
}

std::string OverloadManagementSystem::BranchTripping::getBranchToOperateId() const {
    return backend_->getString(handle_, prop::TRIPPING_ELEMENT_ID);
}

OverloadManagementSystem::BranchTripping& OverloadManagementSystem::BranchTripping::setBranchToOperateId(const std::string& branchId) {
    backend_->setString(handle_, prop::TRIPPING_ELEMENT_ID, branchId);
    return *this;
}

TwoSides OverloadManagementSystem::BranchTripping::getSideToOperate() const {
    return static_cast<TwoSides>(backend_->getInt(handle_, prop::TRIPPING_SIDE));
}

OverloadManagementSystem::BranchTripping& OverloadManagementSystem::BranchTripping::setSideToOperate(TwoSides side) {
    backend_->setInt(handle_, prop::TRIPPING_SIDE, static_cast<int>(side));
    return *this;
}

std::string OverloadManagementSystem::ThreeWindingsTransformerTripping::getThreeWindingsTransformerToOperateId() const {
    return backend_->getString(handle_, prop::TRIPPING_ELEMENT_ID);
}

OverloadManagementSystem::ThreeWindingsTransformerTripping&
OverloadManagementSystem::ThreeWindingsTransformerTripping::setThreeWindingsTransformerToOperateId(const std::string& transformerId) {
    backend_->setString(handle_, prop::TRIPPING_ELEMENT_ID, transformerId);
    return *this;
}

ThreeSides OverloadManagementSystem::ThreeWindingsTransformerTripping::getSideToOperate() const {
    return static_cast<ThreeSides>(backend_->getInt(handle_, prop::TRIPPING_SIDE));
}

OverloadManagementSystem::ThreeWindingsTransformerTripping&
OverloadManagementSystem::ThreeWindingsTransformerTripping::setSideToOperate(ThreeSides side) {
    backend_->setInt(handle_, prop::TRIPPING_SIDE, static_cast<int>(side));
    return *this;
}

// ── OverloadManagementSystem ──────────────────────────────────────────────────

bool OverloadManagementSystem::isEnabled() const {
    return backend_->getBool(handle_, prop::AUTOMATION_SYSTEM_ENABLED);
}

OverloadManagementSystem& OverloadManagementSystem::setEnabled(bool enabled) {
    backend_->setBool(handle_, prop::AUTOMATION_SYSTEM_ENABLED, enabled);
    return *this;
}

Substation OverloadManagementSystem::getSubstation() const {
    return Substation(backend_->getRelated(handle_, prop::REL_SUBSTATION), backend_);
}

std::string OverloadManagementSystem::getMonitoredElementId() const {
    return backend_->getString(handle_, prop::OMS_MONITORED_ELEMENT_ID);
}

ThreeSides OverloadManagementSystem::getMonitoredSide() const {
    return static_cast<ThreeSides>(backend_->getInt(handle_, prop::OMS_MONITORED_SIDE));
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
