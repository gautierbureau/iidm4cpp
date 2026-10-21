#include <iidm/OperationalLimitsGroup.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

OperationalLimitsGroup::OperationalLimitsGroup(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

std::string OperationalLimitsGroup::getId() const {
    return backend_->getString(handle_, prop::OLG_ID);
}

bool OperationalLimitsGroup::isEmpty() const {
    return backend_->getBool(handle_, prop::OLG_EMPTY);
}

iidm::optional<CurrentLimits> OperationalLimitsGroup::getCurrentLimits() const {
    return detail::wrapOptional<CurrentLimits>(backend_->getRelated(handle_, CurrentLimits::TYPE), backend_);
}

iidm::optional<ActivePowerLimits> OperationalLimitsGroup::getActivePowerLimits() const {
    return detail::wrapOptional<ActivePowerLimits>(backend_->getRelated(handle_, ActivePowerLimits::TYPE), backend_);
}

iidm::optional<ApparentPowerLimits> OperationalLimitsGroup::getApparentPowerLimits() const {
    return detail::wrapOptional<ApparentPowerLimits>(backend_->getRelated(handle_, ApparentPowerLimits::TYPE), backend_);
}

iidm::optional<LoadingLimits> OperationalLimitsGroup::getLoadingLimits(LimitType type) const {
    return detail::wrapOptional<LoadingLimits>(
        backend_->getRelatedByIndex(handle_, LoadingLimits::TYPE, static_cast<int>(type)), backend_);
}

CurrentLimitsAdder OperationalLimitsGroup::newCurrentLimits() {
    return CurrentLimitsAdder(backend_->invoke(handle_, CurrentLimits::TYPE), backend_);
}

ActivePowerLimitsAdder OperationalLimitsGroup::newActivePowerLimits() {
    return ActivePowerLimitsAdder(backend_->invoke(handle_, ActivePowerLimits::TYPE), backend_);
}

ApparentPowerLimitsAdder OperationalLimitsGroup::newApparentPowerLimits() {
    return ApparentPowerLimitsAdder(backend_->invoke(handle_, ApparentPowerLimits::TYPE), backend_);
}

void OperationalLimitsGroup::removeCurrentLimits() {
    backend_->setRelated(handle_, CurrentLimits::TYPE, INVALID_HANDLE);
}

void OperationalLimitsGroup::removeActivePowerLimits() {
    backend_->setRelated(handle_, ActivePowerLimits::TYPE, INVALID_HANDLE);
}

void OperationalLimitsGroup::removeApparentPowerLimits() {
    backend_->setRelated(handle_, ApparentPowerLimits::TYPE, INVALID_HANDLE);
}

} // namespace iidm
