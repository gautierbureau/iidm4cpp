#include <iidm/TwoWindingsTransformer.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

double TwoWindingsTransformer::getR() const {
    return backend_->getDouble(handle_, prop::TWO_WT_R);
}

double TwoWindingsTransformer::getX() const {
    return backend_->getDouble(handle_, prop::TWO_WT_X);
}

double TwoWindingsTransformer::getG() const {
    return backend_->getDouble(handle_, prop::TWO_WT_G);
}

double TwoWindingsTransformer::getB() const {
    return backend_->getDouble(handle_, prop::TWO_WT_B);
}

double TwoWindingsTransformer::getRatedU1() const {
    return backend_->getDouble(handle_, prop::TWO_WT_RATED_U1);
}

double TwoWindingsTransformer::getRatedU2() const {
    return backend_->getDouble(handle_, prop::TWO_WT_RATED_U2);
}

iidm::optional<double> TwoWindingsTransformer::getRatedS() const {
    return backend_->getOptDouble(handle_, prop::TWO_WT_RATED_S);
}

// ── Extensions ───────────────────────────────────────────────────────────────

bool TwoWindingsTransformer::hasTwoWindingsTransformerToBeEstimated() const {
    return backend_->getBool(handle_, prop::EXT_TWO_WT_TBE_EXISTS);
}

TwoWindingsTransformerToBeEstimated TwoWindingsTransformer::getTwoWindingsTransformerToBeEstimated() const {
    return TwoWindingsTransformerToBeEstimated(handle_, backend_);
}

} // namespace iidm
