#include <iidm/OperationalLimitsGroup.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

OperationalLimitsGroup::OperationalLimitsGroup(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

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

CurrentLimitsAdder OperationalLimitsGroup::newCurrentLimits(const CurrentLimits& copied) {
    return detail::copyAdder<CurrentLimitsAdder>(backend_, handle_, prop::CURRENT_LIMITS, detail::Access::handle(copied));
}

ActivePowerLimitsAdder OperationalLimitsGroup::newActivePowerLimits(const ActivePowerLimits& copied) {
    return detail::copyAdder<ActivePowerLimitsAdder>(backend_, handle_, prop::ACTIVE_POWER_LIMITS, detail::Access::handle(copied));
}

ApparentPowerLimitsAdder OperationalLimitsGroup::newApparentPowerLimits(const ApparentPowerLimits& copied) {
    return detail::copyAdder<ApparentPowerLimitsAdder>(backend_, handle_, prop::APPARENT_POWER_LIMITS, detail::Access::handle(copied));
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
