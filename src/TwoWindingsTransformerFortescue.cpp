#include <iidm/TwoWindingsTransformerFortescue.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

TwoWindingsTransformerFortescue::TwoWindingsTransformerFortescue(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

double TwoWindingsTransformerFortescue::getRz() const {
    return backend_->getDouble(handle_, prop::EXT_TWO_WTF_RZ);
}

TwoWindingsTransformerFortescue& TwoWindingsTransformerFortescue::setRz(double rz) {
    backend_->setDouble(handle_, prop::EXT_TWO_WTF_RZ, rz);
    return *this;
}

double TwoWindingsTransformerFortescue::getXz() const {
    return backend_->getDouble(handle_, prop::EXT_TWO_WTF_XZ);
}

TwoWindingsTransformerFortescue& TwoWindingsTransformerFortescue::setXz(double xz) {
    backend_->setDouble(handle_, prop::EXT_TWO_WTF_XZ, xz);
    return *this;
}

bool TwoWindingsTransformerFortescue::isFreeFluxes() const {
    return backend_->getBool(handle_, prop::EXT_TWO_WTF_FREE_FLUXES);
}

TwoWindingsTransformerFortescue& TwoWindingsTransformerFortescue::setFreeFluxes(bool freeFluxes) {
    backend_->setBool(handle_, prop::EXT_TWO_WTF_FREE_FLUXES, freeFluxes);
    return *this;
}

double TwoWindingsTransformerFortescue::getXm() const {
    return backend_->getDouble(handle_, prop::EXT_TWO_WTF_XM);
}

TwoWindingsTransformerFortescue& TwoWindingsTransformerFortescue::setXm(double xm) {
    backend_->setDouble(handle_, prop::EXT_TWO_WTF_XM, xm);
    return *this;
}

WindingConnectionType TwoWindingsTransformerFortescue::getConnectionType1() const {
    return static_cast<WindingConnectionType>(backend_->getInt(handle_, prop::EXT_TWO_WTF_CONNECTION_TYPE1));
}

TwoWindingsTransformerFortescue& TwoWindingsTransformerFortescue::setConnectionType1(WindingConnectionType connectionType1) {
    backend_->setInt(handle_, prop::EXT_TWO_WTF_CONNECTION_TYPE1, static_cast<int>(connectionType1));
    return *this;
}

WindingConnectionType TwoWindingsTransformerFortescue::getConnectionType2() const {
    return static_cast<WindingConnectionType>(backend_->getInt(handle_, prop::EXT_TWO_WTF_CONNECTION_TYPE2));
}

TwoWindingsTransformerFortescue& TwoWindingsTransformerFortescue::setConnectionType2(WindingConnectionType connectionType2) {
    backend_->setInt(handle_, prop::EXT_TWO_WTF_CONNECTION_TYPE2, static_cast<int>(connectionType2));
    return *this;
}

double TwoWindingsTransformerFortescue::getGroundingR1() const {
    return backend_->getDouble(handle_, prop::EXT_TWO_WTF_GROUNDING_R1);
}

TwoWindingsTransformerFortescue& TwoWindingsTransformerFortescue::setGroundingR1(double groundingR1) {
    backend_->setDouble(handle_, prop::EXT_TWO_WTF_GROUNDING_R1, groundingR1);
    return *this;
}

double TwoWindingsTransformerFortescue::getGroundingX1() const {
    return backend_->getDouble(handle_, prop::EXT_TWO_WTF_GROUNDING_X1);
}

TwoWindingsTransformerFortescue& TwoWindingsTransformerFortescue::setGroundingX1(double groundingX1) {
    backend_->setDouble(handle_, prop::EXT_TWO_WTF_GROUNDING_X1, groundingX1);
    return *this;
}

double TwoWindingsTransformerFortescue::getGroundingR2() const {
    return backend_->getDouble(handle_, prop::EXT_TWO_WTF_GROUNDING_R2);
}

TwoWindingsTransformerFortescue& TwoWindingsTransformerFortescue::setGroundingR2(double groundingR2) {
    backend_->setDouble(handle_, prop::EXT_TWO_WTF_GROUNDING_R2, groundingR2);
    return *this;
}

double TwoWindingsTransformerFortescue::getGroundingX2() const {
    return backend_->getDouble(handle_, prop::EXT_TWO_WTF_GROUNDING_X2);
}

TwoWindingsTransformerFortescue& TwoWindingsTransformerFortescue::setGroundingX2(double groundingX2) {
    backend_->setDouble(handle_, prop::EXT_TWO_WTF_GROUNDING_X2, groundingX2);
    return *this;
}

} // namespace iidm
