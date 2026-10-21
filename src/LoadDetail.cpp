#include <iidm/LoadDetail.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

LoadDetail::LoadDetail(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

double LoadDetail::getFixedActivePower() const {
    return backend_->getDouble(handle_, prop::EXT_LD_FIXED_ACTIVE_POWER);
}

LoadDetail& LoadDetail::setFixedActivePower(double fixedActivePower) {
    backend_->setDouble(handle_, prop::EXT_LD_FIXED_ACTIVE_POWER, fixedActivePower);
    return *this;
}

double LoadDetail::getFixedReactivePower() const {
    return backend_->getDouble(handle_, prop::EXT_LD_FIXED_REACTIVE_POWER);
}

LoadDetail& LoadDetail::setFixedReactivePower(double fixedReactivePower) {
    backend_->setDouble(handle_, prop::EXT_LD_FIXED_REACTIVE_POWER, fixedReactivePower);
    return *this;
}

double LoadDetail::getVariableActivePower() const {
    return backend_->getDouble(handle_, prop::EXT_LD_VARIABLE_ACTIVE_POWER);
}

LoadDetail& LoadDetail::setVariableActivePower(double variableActivePower) {
    backend_->setDouble(handle_, prop::EXT_LD_VARIABLE_ACTIVE_POWER, variableActivePower);
    return *this;
}

double LoadDetail::getVariableReactivePower() const {
    return backend_->getDouble(handle_, prop::EXT_LD_VARIABLE_REACTIVE_POWER);
}

LoadDetail& LoadDetail::setVariableReactivePower(double variableReactivePower) {
    backend_->setDouble(handle_, prop::EXT_LD_VARIABLE_REACTIVE_POWER, variableReactivePower);
    return *this;
}

} // namespace iidm
