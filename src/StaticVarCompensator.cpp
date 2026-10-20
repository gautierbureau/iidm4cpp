#include <iidm/StaticVarCompensator.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

double StaticVarCompensator::getBmin() const {
    return backend_->getDouble(handle_, prop::SVC_B_MIN);
}

StaticVarCompensator& StaticVarCompensator::setBmin(double bMin) {
    backend_->setDouble(handle_, prop::SVC_B_MIN, bMin);
    return *this;
}

double StaticVarCompensator::getBmax() const {
    return backend_->getDouble(handle_, prop::SVC_B_MAX);
}

StaticVarCompensator& StaticVarCompensator::setBmax(double bMax) {
    backend_->setDouble(handle_, prop::SVC_B_MAX, bMax);
    return *this;
}

} // namespace iidm
