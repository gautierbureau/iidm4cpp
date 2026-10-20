#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Substation.h>
#include "Extensions.h"
#include "Wrap.h"

namespace iidm {

// ── Leg ──────────────────────────────────────────────────────────────────────

Terminal ThreeWindingsTransformer::Leg::getTerminal() const {
    return Terminal(backend_->getRelated(handle_, prop::REL_TERMINAL), backend_);
}

ThreeSides ThreeWindingsTransformer::Leg::getSide() const {
    return static_cast<ThreeSides>(backend_->getInt(handle_, prop::LEG_SIDE));
}

ThreeWindingsTransformer ThreeWindingsTransformer::Leg::getTransformer() const {
    return ThreeWindingsTransformer(backend_->getRelated(handle_, prop::THREE_WINDINGS_TRANSFORMER), backend_);
}

double ThreeWindingsTransformer::Leg::getR() const {
    return backend_->getDouble(handle_, prop::LEG_R);
}

ThreeWindingsTransformer::Leg& ThreeWindingsTransformer::Leg::setR(double r) {
    backend_->setDouble(handle_, prop::LEG_R, r);
    return *this;
}

double ThreeWindingsTransformer::Leg::getX() const {
    return backend_->getDouble(handle_, prop::LEG_X);
}

ThreeWindingsTransformer::Leg& ThreeWindingsTransformer::Leg::setX(double x) {
    backend_->setDouble(handle_, prop::LEG_X, x);
    return *this;
}

double ThreeWindingsTransformer::Leg::getG() const {
    return backend_->getDouble(handle_, prop::LEG_G);
}

ThreeWindingsTransformer::Leg& ThreeWindingsTransformer::Leg::setG(double g) {
    backend_->setDouble(handle_, prop::LEG_G, g);
    return *this;
}

double ThreeWindingsTransformer::Leg::getB() const {
    return backend_->getDouble(handle_, prop::LEG_B);
}

ThreeWindingsTransformer::Leg& ThreeWindingsTransformer::Leg::setB(double b) {
    backend_->setDouble(handle_, prop::LEG_B, b);
    return *this;
}

double ThreeWindingsTransformer::Leg::getRatedU() const {
    return backend_->getDouble(handle_, prop::LEG_RATED_U);
}

ThreeWindingsTransformer::Leg& ThreeWindingsTransformer::Leg::setRatedU(double ratedU) {
    backend_->setDouble(handle_, prop::LEG_RATED_U, ratedU);
    return *this;
}

iidm::optional<double> ThreeWindingsTransformer::Leg::getRatedS() const {
    return backend_->getOptDouble(handle_, prop::LEG_RATED_S);
}

ThreeWindingsTransformer::Leg& ThreeWindingsTransformer::Leg::setRatedS(double ratedS) {
    backend_->setDouble(handle_, prop::LEG_RATED_S, ratedS);
    return *this;
}

// ── ThreeWindingsTransformer ──────────────────────────────────────────────────

double ThreeWindingsTransformer::getRatedU0() const {
    return backend_->getDouble(handle_, prop::THREE_WT_RATED_U0);
}

Substation ThreeWindingsTransformer::getSubstation() const {
    return Substation(backend_->getRelated(handle_, prop::REL_SUBSTATION), backend_);
}

ThreeWindingsTransformer::Leg ThreeWindingsTransformer::getLeg1() const {
    return getLeg(ThreeSides::ONE);
}

ThreeWindingsTransformer::Leg ThreeWindingsTransformer::getLeg2() const {
    return getLeg(ThreeSides::TWO);
}

ThreeWindingsTransformer::Leg ThreeWindingsTransformer::getLeg3() const {
    return getLeg(ThreeSides::THREE);
}

ThreeWindingsTransformer::Leg ThreeWindingsTransformer::getLeg(ThreeSides side) const {
    return Leg(backend_->getRelatedByIndex(handle_, Leg::TYPE, static_cast<int>(side)), backend_);
}

std::vector<ThreeWindingsTransformer::Leg> ThreeWindingsTransformer::getLegs() const {
    return detail::wrapAll<Leg>(backend_->getChildren(handle_, Leg::TYPE), backend_);
}

Terminal ThreeWindingsTransformer::getTerminal(ThreeSides side) const {
    return Terminal(backend_->getRelatedByIndex(handle_, prop::REL_TERMINAL, static_cast<int>(side)), backend_);
}

Terminal ThreeWindingsTransformer::getTerminal(const std::string& voltageLevelId) const {
    return Terminal(backend_->getRelatedByString(handle_, prop::REL_TERMINAL, voltageLevelId), backend_);
}

// ── Extensions ───────────────────────────────────────────────────────────────

bool ThreeWindingsTransformer::hasThreeWindingsTransformerToBeEstimated() const {
    return detail::hasExtension<ThreeWindingsTransformerToBeEstimated>(backend_, handle_);
}

ThreeWindingsTransformerToBeEstimated ThreeWindingsTransformer::getThreeWindingsTransformerToBeEstimated() const {
    return detail::getExtension<ThreeWindingsTransformerToBeEstimated>(backend_, handle_, "ThreeWindingsTransformerToBeEstimated");
}

ThreeWindingsTransformerToBeEstimatedAdder ThreeWindingsTransformer::newThreeWindingsTransformerToBeEstimated() {
    return detail::newExtension<ThreeWindingsTransformerToBeEstimated, ThreeWindingsTransformerToBeEstimatedAdder>(backend_, handle_);
}

void ThreeWindingsTransformer::removeThreeWindingsTransformerToBeEstimated() {
    detail::removeExtension<ThreeWindingsTransformerToBeEstimated>(backend_, handle_);
}

} // namespace iidm
