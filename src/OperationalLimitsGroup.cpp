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
    return detail::wrapOptional<CurrentLimits>(backend_->getRelated(handle_, prop::CURRENT_LIMITS), backend_);
}
} // namespace iidm
