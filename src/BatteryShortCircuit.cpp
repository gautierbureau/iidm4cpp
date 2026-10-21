#include <iidm/BatteryShortCircuit.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

BatteryShortCircuit::BatteryShortCircuit(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

double BatteryShortCircuit::getDirectSubtransX() const {
    return backend_->getDouble(handle_, prop::EXT_SC_DIRECT_SUBTRANS_X);
}

BatteryShortCircuit& BatteryShortCircuit::setDirectSubtransX(double directSubtransX) {
    backend_->setDouble(handle_, prop::EXT_SC_DIRECT_SUBTRANS_X, directSubtransX);
    return *this;
}

double BatteryShortCircuit::getDirectTransX() const {
    return backend_->getDouble(handle_, prop::EXT_SC_DIRECT_TRANS_X);
}

BatteryShortCircuit& BatteryShortCircuit::setDirectTransX(double directTransX) {
    backend_->setDouble(handle_, prop::EXT_SC_DIRECT_TRANS_X, directTransX);
    return *this;
}

double BatteryShortCircuit::getStepUpTransformerX() const {
    return backend_->getDouble(handle_, prop::EXT_SC_STEP_UP_TRANSFORMER_X);
}

BatteryShortCircuit& BatteryShortCircuit::setStepUpTransformerX(double stepUpTransformerX) {
    backend_->setDouble(handle_, prop::EXT_SC_STEP_UP_TRANSFORMER_X, stepUpTransformerX);
    return *this;
}

} // namespace iidm
