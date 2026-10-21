#include <iidm/GeneratorShortCircuit.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

GeneratorShortCircuit::GeneratorShortCircuit(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

double GeneratorShortCircuit::getDirectSubtransX() const {
    return backend_->getDouble(handle_, prop::EXT_SC_DIRECT_SUBTRANS_X);
}

GeneratorShortCircuit& GeneratorShortCircuit::setDirectSubtransX(double directSubtransX) {
    backend_->setDouble(handle_, prop::EXT_SC_DIRECT_SUBTRANS_X, directSubtransX);
    return *this;
}

double GeneratorShortCircuit::getDirectTransX() const {
    return backend_->getDouble(handle_, prop::EXT_SC_DIRECT_TRANS_X);
}

GeneratorShortCircuit& GeneratorShortCircuit::setDirectTransX(double directTransX) {
    backend_->setDouble(handle_, prop::EXT_SC_DIRECT_TRANS_X, directTransX);
    return *this;
}

double GeneratorShortCircuit::getStepUpTransformerX() const {
    return backend_->getDouble(handle_, prop::EXT_SC_STEP_UP_TRANSFORMER_X);
}

GeneratorShortCircuit& GeneratorShortCircuit::setStepUpTransformerX(double stepUpTransformerX) {
    backend_->setDouble(handle_, prop::EXT_SC_STEP_UP_TRANSFORMER_X, stepUpTransformerX);
    return *this;
}

} // namespace iidm
