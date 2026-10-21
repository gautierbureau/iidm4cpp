#include <iidm/Overload.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

TemporaryLimit Overload::getTemporaryLimit() const {
    return TemporaryLimit(backend_->getRelated(handle_, prop::TEMPORARY_LIMIT), backend_);
}

double Overload::getPreviousLimit() const {
    return backend_->getDouble(handle_, prop::OVERLOAD_PREVIOUS_LIMIT);
}

std::string Overload::getPreviousLimitName() const {
    return backend_->getString(handle_, prop::OVERLOAD_PREVIOUS_LIMIT_NAME);
}

std::string Overload::getOperationalLimitsGroupId() const {
    return backend_->getString(handle_, prop::OVERLOAD_OLG_ID);
}

double Overload::getLimitScalingCoefficient() const {
    return backend_->getDouble(handle_, prop::OVERLOAD_LIMIT_SCALING);
}

} // namespace iidm
