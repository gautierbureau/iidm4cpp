#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Substation.h>
#include "Extensions.h"
#include "OverloadChecks.h"
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

iidm::optional<Substation> ThreeWindingsTransformer::getSubstation() const {
    return detail::wrapOptional<Substation>(backend_->getRelated(handle_, prop::REL_SUBSTATION), backend_);
}

double ThreeWindingsTransformer::getRatedU0() const {
    return backend_->getDouble(handle_, prop::THREE_WT_RATED_U0);
}

ThreeWindingsTransformer& ThreeWindingsTransformer::setRatedU0(double ratedU0) {
    backend_->setDouble(handle_, prop::THREE_WT_RATED_U0, ratedU0);
    return *this;
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

ThreeSides ThreeWindingsTransformer::getSide(const Terminal& terminal) const {
    for (ThreeSides side : {ThreeSides::ONE, ThreeSides::TWO, ThreeSides::THREE}) {
        if (getTerminal(side) == terminal) return side;
    }
    throw ValidationException("The terminal is not connected to three windings transformer " + getId());
}

// ── Solved values ────────────────────────────────────────────────────────────

void ThreeWindingsTransformer::applySolvedValues() {
    backend_->invoke(handle_, prop::ACT_APPLY_SOLVED_VALUES);
}

void ThreeWindingsTransformer::setRatioTapPositionToSolvedTapPosition() {
    backend_->invoke(handle_, prop::ACT_SET_RATIO_TAP_POSITION_TO_SOLVED_TAP_POSITION);
}

void ThreeWindingsTransformer::setPhaseTapPositionToSolvedTapPosition() {
    backend_->invoke(handle_, prop::ACT_SET_PHASE_TAP_POSITION_TO_SOLVED_TAP_POSITION);
}

// ── Overloads ────────────────────────────────────────────────────────────────

bool ThreeWindingsTransformer::isOverloaded() const {
    return backend_->getBool(handle_, prop::OVERLOADED);
}

bool ThreeWindingsTransformer::isOverloaded(double limitReductionValue) const {
    return backend_->evaluate(handle_, prop::OVERLOADED, {limitReductionValue}) != 0;
}

int ThreeWindingsTransformer::getOverloadDuration() const {
    return backend_->getInt(handle_, prop::OVERLOAD_DURATION);
}

bool ThreeWindingsTransformer::checkPermanentLimit(ThreeSides side, LimitType type) const {
    return detail::overPermanentLimit(backend_, handle_, static_cast<int>(side), iidm::optional<double>(), type);
}

bool ThreeWindingsTransformer::checkPermanentLimit(ThreeSides side, double limitReductionValue, LimitType type) const {
    return detail::overPermanentLimit(backend_, handle_, static_cast<int>(side), iidm::optional<double>(limitReductionValue), type);
}

bool ThreeWindingsTransformer::checkPermanentLimit1(LimitType type) const {
    return checkPermanentLimit(ThreeSides::ONE, type);
}

bool ThreeWindingsTransformer::checkPermanentLimit1(double limitReductionValue, LimitType type) const {
    return checkPermanentLimit(ThreeSides::ONE, limitReductionValue, type);
}

bool ThreeWindingsTransformer::checkPermanentLimit2(LimitType type) const {
    return checkPermanentLimit(ThreeSides::TWO, type);
}

bool ThreeWindingsTransformer::checkPermanentLimit2(double limitReductionValue, LimitType type) const {
    return checkPermanentLimit(ThreeSides::TWO, limitReductionValue, type);
}

bool ThreeWindingsTransformer::checkPermanentLimit3(LimitType type) const {
    return checkPermanentLimit(ThreeSides::THREE, type);
}

bool ThreeWindingsTransformer::checkPermanentLimit3(double limitReductionValue, LimitType type) const {
    return checkPermanentLimit(ThreeSides::THREE, limitReductionValue, type);
}

iidm::optional<Overload> ThreeWindingsTransformer::checkTemporaryLimits(ThreeSides side, LimitType type) const {
    return detail::overloadOf(backend_, handle_, static_cast<int>(side), iidm::optional<double>(), type);
}

iidm::optional<Overload> ThreeWindingsTransformer::checkTemporaryLimits(ThreeSides side, double limitReductionValue,
                                                                        LimitType type) const {
    return detail::overloadOf(backend_, handle_, static_cast<int>(side), iidm::optional<double>(limitReductionValue), type);
}

iidm::optional<Overload> ThreeWindingsTransformer::checkTemporaryLimits1(LimitType type) const {
    return checkTemporaryLimits(ThreeSides::ONE, type);
}

iidm::optional<Overload> ThreeWindingsTransformer::checkTemporaryLimits1(double limitReductionValue, LimitType type) const {
    return checkTemporaryLimits(ThreeSides::ONE, limitReductionValue, type);
}

iidm::optional<Overload> ThreeWindingsTransformer::checkTemporaryLimits2(LimitType type) const {
    return checkTemporaryLimits(ThreeSides::TWO, type);
}

iidm::optional<Overload> ThreeWindingsTransformer::checkTemporaryLimits2(double limitReductionValue, LimitType type) const {
    return checkTemporaryLimits(ThreeSides::TWO, limitReductionValue, type);
}

iidm::optional<Overload> ThreeWindingsTransformer::checkTemporaryLimits3(LimitType type) const {
    return checkTemporaryLimits(ThreeSides::THREE, type);
}

iidm::optional<Overload> ThreeWindingsTransformer::checkTemporaryLimits3(double limitReductionValue, LimitType type) const {
    return checkTemporaryLimits(ThreeSides::THREE, limitReductionValue, type);
}

std::vector<Overload> ThreeWindingsTransformer::checkAllTemporaryLimits(ThreeSides side, LimitType type) const {
    return detail::allOverloadsOf(backend_, handle_, static_cast<int>(side), iidm::optional<double>(), type);
}

std::vector<Overload> ThreeWindingsTransformer::checkAllTemporaryLimits(ThreeSides side, double limitReductionValue,
                                                                       LimitType type) const {
    return detail::allOverloadsOf(backend_, handle_, static_cast<int>(side), iidm::optional<double>(limitReductionValue), type);
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
