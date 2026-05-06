#include <iidm/OperationalLimitsGroup.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {
OperationalLimitsGroup::OperationalLimitsGroup(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string OperationalLimitsGroup::getId() const {
    return backend_->getString(handle_, prop::OLG_ID);
}
bool OperationalLimitsGroup::isEmpty() const {
    return backend_->getBool(handle_, prop::OLG_EMPTY);
}
iidm::optional<CurrentLimits> OperationalLimitsGroup::getCurrentLimits() const {
    ObjectHandle h = backend_->getRelated(handle_, prop::REL_OLG_CURRENT_LIMITS);
    if (h == INVALID_HANDLE) return {};
    return CurrentLimits(h, backend_);
}
} // namespace iidm
