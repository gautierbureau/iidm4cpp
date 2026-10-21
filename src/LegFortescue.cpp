#include <iidm/LegFortescue.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

LegFortescue::LegFortescue(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

double LegFortescue::getRz() const {
    return backend_->getDouble(handle_, prop::EXT_LEG_F_RZ);
}

LegFortescue& LegFortescue::setRz(double rz) {
    backend_->setDouble(handle_, prop::EXT_LEG_F_RZ, rz);
    return *this;
}

double LegFortescue::getXz() const {
    return backend_->getDouble(handle_, prop::EXT_LEG_F_XZ);
}

LegFortescue& LegFortescue::setXz(double xz) {
    backend_->setDouble(handle_, prop::EXT_LEG_F_XZ, xz);
    return *this;
}

bool LegFortescue::isFreeFluxes() const {
    return backend_->getBool(handle_, prop::EXT_LEG_F_FREE_FLUXES);
}

LegFortescue& LegFortescue::setFreeFluxes(bool freeFluxes) {
    backend_->setBool(handle_, prop::EXT_LEG_F_FREE_FLUXES, freeFluxes);
    return *this;
}

WindingConnectionType LegFortescue::getConnectionType() const {
    return static_cast<WindingConnectionType>(backend_->getInt(handle_, prop::EXT_LEG_F_CONNECTION_TYPE));
}

LegFortescue& LegFortescue::setConnectionType(WindingConnectionType connectionType) {
    backend_->setInt(handle_, prop::EXT_LEG_F_CONNECTION_TYPE, static_cast<int>(connectionType));
    return *this;
}

double LegFortescue::getGroundingR() const {
    return backend_->getDouble(handle_, prop::EXT_LEG_F_GROUNDING_R);
}

LegFortescue& LegFortescue::setGroundingR(double groundingR) {
    backend_->setDouble(handle_, prop::EXT_LEG_F_GROUNDING_R, groundingR);
    return *this;
}

double LegFortescue::getGroundingX() const {
    return backend_->getDouble(handle_, prop::EXT_LEG_F_GROUNDING_X);
}

LegFortescue& LegFortescue::setGroundingX(double groundingX) {
    backend_->setDouble(handle_, prop::EXT_LEG_F_GROUNDING_X, groundingX);
    return *this;
}

} // namespace iidm
