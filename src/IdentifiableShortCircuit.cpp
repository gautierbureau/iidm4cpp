#include <iidm/IdentifiableShortCircuit.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

IdentifiableShortCircuit::IdentifiableShortCircuit(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

double IdentifiableShortCircuit::getIpMin() const {
    return backend_->getDouble(handle_, prop::EXT_ISC_IP_MIN);
}

IdentifiableShortCircuit& IdentifiableShortCircuit::setIpMin(double ipMin) {
    backend_->setDouble(handle_, prop::EXT_ISC_IP_MIN, ipMin);
    return *this;
}

double IdentifiableShortCircuit::getIpMax() const {
    return backend_->getDouble(handle_, prop::EXT_ISC_IP_MAX);
}

IdentifiableShortCircuit& IdentifiableShortCircuit::setIpMax(double ipMax) {
    backend_->setDouble(handle_, prop::EXT_ISC_IP_MAX, ipMax);
    return *this;
}

} // namespace iidm
