#include <iidm/TwoWindingsTransformer.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Substation.h>
#include "Extensions.h"
#include "Wrap.h"

namespace iidm {

iidm::optional<Substation> TwoWindingsTransformer::getSubstation() const {
    return detail::wrapOptional<Substation>(backend_->getRelated(handle_, prop::REL_SUBSTATION), backend_);
}

double TwoWindingsTransformer::getR() const {
    return backend_->getDouble(handle_, prop::TWO_WT_R);
}

TwoWindingsTransformer& TwoWindingsTransformer::setR(double r) {
    backend_->setDouble(handle_, prop::TWO_WT_R, r);
    return *this;
}

double TwoWindingsTransformer::getX() const {
    return backend_->getDouble(handle_, prop::TWO_WT_X);
}

TwoWindingsTransformer& TwoWindingsTransformer::setX(double x) {
    backend_->setDouble(handle_, prop::TWO_WT_X, x);
    return *this;
}

double TwoWindingsTransformer::getG() const {
    return backend_->getDouble(handle_, prop::TWO_WT_G);
}

TwoWindingsTransformer& TwoWindingsTransformer::setG(double g) {
    backend_->setDouble(handle_, prop::TWO_WT_G, g);
    return *this;
}

double TwoWindingsTransformer::getB() const {
    return backend_->getDouble(handle_, prop::TWO_WT_B);
}

TwoWindingsTransformer& TwoWindingsTransformer::setB(double b) {
    backend_->setDouble(handle_, prop::TWO_WT_B, b);
    return *this;
}

double TwoWindingsTransformer::getRatedU1() const {
    return backend_->getDouble(handle_, prop::TWO_WT_RATED_U1);
}

TwoWindingsTransformer& TwoWindingsTransformer::setRatedU1(double ratedU1) {
    backend_->setDouble(handle_, prop::TWO_WT_RATED_U1, ratedU1);
    return *this;
}

double TwoWindingsTransformer::getRatedU2() const {
    return backend_->getDouble(handle_, prop::TWO_WT_RATED_U2);
}

TwoWindingsTransformer& TwoWindingsTransformer::setRatedU2(double ratedU2) {
    backend_->setDouble(handle_, prop::TWO_WT_RATED_U2, ratedU2);
    return *this;
}

iidm::optional<double> TwoWindingsTransformer::getRatedS() const {
    return backend_->getOptDouble(handle_, prop::TWO_WT_RATED_S);
}

TwoWindingsTransformer& TwoWindingsTransformer::setRatedS(double ratedS) {
    backend_->setDouble(handle_, prop::TWO_WT_RATED_S, ratedS);
    return *this;
}

// ── Solved values ────────────────────────────────────────────────────────────

void TwoWindingsTransformer::applySolvedValues() {
    backend_->invoke(handle_, prop::ACT_APPLY_SOLVED_VALUES);
}

void TwoWindingsTransformer::setRatioTapPositionToSolvedTapPosition() {
    backend_->invoke(handle_, prop::ACT_SET_RATIO_TAP_POSITION_TO_SOLVED_TAP_POSITION);
}

void TwoWindingsTransformer::setPhaseTapPositionToSolvedTapPosition() {
    backend_->invoke(handle_, prop::ACT_SET_PHASE_TAP_POSITION_TO_SOLVED_TAP_POSITION);
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
