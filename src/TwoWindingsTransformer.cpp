#include <iidm/TwoWindingsTransformer.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "Extensions.h"

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
    return detail::hasExtension<TwoWindingsTransformerToBeEstimated>(backend_, handle_);
}

TwoWindingsTransformerToBeEstimated TwoWindingsTransformer::getTwoWindingsTransformerToBeEstimated() const {
    return detail::getExtension<TwoWindingsTransformerToBeEstimated>(backend_, handle_, "TwoWindingsTransformerToBeEstimated");
}

TwoWindingsTransformerToBeEstimatedAdder TwoWindingsTransformer::newTwoWindingsTransformerToBeEstimated() {
    return detail::newExtension<TwoWindingsTransformerToBeEstimated, TwoWindingsTransformerToBeEstimatedAdder>(backend_, handle_);
}

void TwoWindingsTransformer::removeTwoWindingsTransformerToBeEstimated() {
    detail::removeExtension<TwoWindingsTransformerToBeEstimated>(backend_, handle_);
}

} // namespace iidm
