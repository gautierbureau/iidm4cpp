#include <iidm/TwoWindingsTransformer.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Substation.h>
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

} // namespace iidm
