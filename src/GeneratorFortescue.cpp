#include <iidm/GeneratorFortescue.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

GeneratorFortescue::GeneratorFortescue(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

bool GeneratorFortescue::isGrounded() const {
    return backend_->getBool(handle_, prop::EXT_GF_GROUNDED);
}

GeneratorFortescue& GeneratorFortescue::setGrounded(bool grounded) {
    backend_->setBool(handle_, prop::EXT_GF_GROUNDED, grounded);
    return *this;
}

double GeneratorFortescue::getRz() const {
    return backend_->getDouble(handle_, prop::EXT_GF_RZ);
}

GeneratorFortescue& GeneratorFortescue::setRz(double rz) {
    backend_->setDouble(handle_, prop::EXT_GF_RZ, rz);
    return *this;
}

double GeneratorFortescue::getXz() const {
    return backend_->getDouble(handle_, prop::EXT_GF_XZ);
}

GeneratorFortescue& GeneratorFortescue::setXz(double xz) {
    backend_->setDouble(handle_, prop::EXT_GF_XZ, xz);
    return *this;
}

double GeneratorFortescue::getRn() const {
    return backend_->getDouble(handle_, prop::EXT_GF_RN);
}

GeneratorFortescue& GeneratorFortescue::setRn(double rn) {
    backend_->setDouble(handle_, prop::EXT_GF_RN, rn);
    return *this;
}

double GeneratorFortescue::getXn() const {
    return backend_->getDouble(handle_, prop::EXT_GF_XN);
}

GeneratorFortescue& GeneratorFortescue::setXn(double xn) {
    backend_->setDouble(handle_, prop::EXT_GF_XN, xn);
    return *this;
}

double GeneratorFortescue::getGroundingR() const {
    return backend_->getDouble(handle_, prop::EXT_GF_GROUNDING_R);
}

GeneratorFortescue& GeneratorFortescue::setGroundingR(double groundingR) {
    backend_->setDouble(handle_, prop::EXT_GF_GROUNDING_R, groundingR);
    return *this;
}

double GeneratorFortescue::getGroundingX() const {
    return backend_->getDouble(handle_, prop::EXT_GF_GROUNDING_X);
}

GeneratorFortescue& GeneratorFortescue::setGroundingX(double groundingX) {
    backend_->setDouble(handle_, prop::EXT_GF_GROUNDING_X, groundingX);
    return *this;
}

} // namespace iidm
