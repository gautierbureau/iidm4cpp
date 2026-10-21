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

ThreeWindingsTransformer ThreeWindingsTransformer::Leg::getTransformer() const {
    return ThreeWindingsTransformer(backend_->getRelated(handle_, prop::THREE_WINDINGS_TRANSFORMER), backend_);
}

// ── ThreeWindingsTransformer ──────────────────────────────────────────────────

iidm::optional<Substation> ThreeWindingsTransformer::getSubstation() const {
    return detail::wrapOptional<Substation>(backend_->getRelated(handle_, prop::REL_SUBSTATION), backend_);
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

bool ThreeWindingsTransformer::isOverloaded(double limitReductionValue) const {
    return backend_->evaluate(handle_, prop::OVERLOADED, {limitReductionValue}) != 0;
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

bool ThreeWindingsTransformer::hasThreeWindingsTransformerPhaseAngleClock() const {
    return detail::hasExtension<ThreeWindingsTransformerPhaseAngleClock>(backend_, handle_);
}

ThreeWindingsTransformerPhaseAngleClock ThreeWindingsTransformer::getThreeWindingsTransformerPhaseAngleClock() const {
    return detail::getExtension<ThreeWindingsTransformerPhaseAngleClock>(backend_, handle_, "ThreeWindingsTransformerPhaseAngleClock");
}

ThreeWindingsTransformerPhaseAngleClockAdder ThreeWindingsTransformer::newThreeWindingsTransformerPhaseAngleClock() {
    return detail::newExtension<ThreeWindingsTransformerPhaseAngleClock, ThreeWindingsTransformerPhaseAngleClockAdder>(backend_, handle_);
}

void ThreeWindingsTransformer::removeThreeWindingsTransformerPhaseAngleClock() {
    detail::removeExtension<ThreeWindingsTransformerPhaseAngleClock>(backend_, handle_);
}

bool ThreeWindingsTransformer::hasThreeWindingsTransformerFortescue() const {
    return detail::hasExtension<ThreeWindingsTransformerFortescue>(backend_, handle_);
}

ThreeWindingsTransformerFortescue ThreeWindingsTransformer::getThreeWindingsTransformerFortescue() const {
    return detail::getExtension<ThreeWindingsTransformerFortescue>(backend_, handle_, "ThreeWindingsTransformerFortescue");
}

ThreeWindingsTransformerFortescueAdder ThreeWindingsTransformer::newThreeWindingsTransformerFortescue() {
    return detail::newExtension<ThreeWindingsTransformerFortescue, ThreeWindingsTransformerFortescueAdder>(backend_, handle_);
}

void ThreeWindingsTransformer::removeThreeWindingsTransformerFortescue() {
    detail::removeExtension<ThreeWindingsTransformerFortescue>(backend_, handle_);
}

} // namespace iidm
