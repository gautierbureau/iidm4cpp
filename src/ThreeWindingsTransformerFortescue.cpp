#include <iidm/ThreeWindingsTransformerFortescue.h>
#include <iidm/BackendProvider.h>
#include <iidm/Enums.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

ThreeWindingsTransformerFortescue::ThreeWindingsTransformerFortescue(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

LegFortescue ThreeWindingsTransformerFortescue::getLeg1() const {
    return LegFortescue(backend_->getRelatedByIndex(handle_, prop::LEG_FORTESCUE, static_cast<int>(ThreeSides::ONE)), backend_);
}

LegFortescue ThreeWindingsTransformerFortescue::getLeg2() const {
    return LegFortescue(backend_->getRelatedByIndex(handle_, prop::LEG_FORTESCUE, static_cast<int>(ThreeSides::TWO)), backend_);
}

LegFortescue ThreeWindingsTransformerFortescue::getLeg3() const {
    return LegFortescue(backend_->getRelatedByIndex(handle_, prop::LEG_FORTESCUE, static_cast<int>(ThreeSides::THREE)), backend_);
}

} // namespace iidm
