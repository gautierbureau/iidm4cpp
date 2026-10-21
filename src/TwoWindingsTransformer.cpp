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

bool TwoWindingsTransformer::hasTwoWindingsTransformerFortescue() const {
    return detail::hasExtension<TwoWindingsTransformerFortescue>(backend_, handle_);
}

TwoWindingsTransformerFortescue TwoWindingsTransformer::getTwoWindingsTransformerFortescue() const {
    return detail::getExtension<TwoWindingsTransformerFortescue>(backend_, handle_, "TwoWindingsTransformerFortescue");
}

TwoWindingsTransformerFortescueAdder TwoWindingsTransformer::newTwoWindingsTransformerFortescue() {
    return detail::newExtension<TwoWindingsTransformerFortescue, TwoWindingsTransformerFortescueAdder>(backend_, handle_);
}

void TwoWindingsTransformer::removeTwoWindingsTransformerFortescue() {
    detail::removeExtension<TwoWindingsTransformerFortescue>(backend_, handle_);
}

bool TwoWindingsTransformer::hasTwoWindingsTransformerPhaseAngleClock() const {
    return detail::hasExtension<TwoWindingsTransformerPhaseAngleClock>(backend_, handle_);
}

TwoWindingsTransformerPhaseAngleClock TwoWindingsTransformer::getTwoWindingsTransformerPhaseAngleClock() const {
    return detail::getExtension<TwoWindingsTransformerPhaseAngleClock>(backend_, handle_, "TwoWindingsTransformerPhaseAngleClock");
}

TwoWindingsTransformerPhaseAngleClockAdder TwoWindingsTransformer::newTwoWindingsTransformerPhaseAngleClock() {
    return detail::newExtension<TwoWindingsTransformerPhaseAngleClock, TwoWindingsTransformerPhaseAngleClockAdder>(backend_, handle_);
}

void TwoWindingsTransformer::removeTwoWindingsTransformerPhaseAngleClock() {
    detail::removeExtension<TwoWindingsTransformerPhaseAngleClock>(backend_, handle_);
}

} // namespace iidm
