#include <iidm/TapChanger.h>
#include <iidm/BackendProvider.h>
#include <iidm/PhaseTapChanger.h>
#include <iidm/PropertyCodes.h>
#include <iidm/RatioTapChanger.h>
#include "Wrap.h"
#include <utility>

namespace iidm { namespace detail {

template <class Self, class Step>
int TapChangerMethods<Self, Step>::getLowTapPosition() const {
    return backend()->getInt(handle(), prop::TC_LOW_TAP_POSITION);
}

template <class Self, class Step>
Self& TapChangerMethods<Self, Step>::setLowTapPosition(int lowTapPosition) {
    backend()->setInt(handle(), prop::TC_LOW_TAP_POSITION, lowTapPosition);
    return static_cast<Self&>(*this);
}

template <class Self, class Step>
int TapChangerMethods<Self, Step>::getHighTapPosition() const {
    return backend()->getInt(handle(), prop::TC_HIGH_TAP_POSITION);
}

template <class Self, class Step>
int TapChangerMethods<Self, Step>::getStepCount() const {
    return backend()->getInt(handle(), prop::TC_STEP_COUNT);
}

template <class Self, class Step>
int TapChangerMethods<Self, Step>::getTapPosition() const {
    return backend()->getInt(handle(), prop::TC_TAP_POSITION);
}

template <class Self, class Step>
iidm::optional<int> TapChangerMethods<Self, Step>::findTapPosition() const {
    return backend()->getOptInt(handle(), prop::TC_TAP_POSITION);
}

template <class Self, class Step>
Self& TapChangerMethods<Self, Step>::setTapPosition(int tapPosition) {
    backend()->setInt(handle(), prop::TC_TAP_POSITION, tapPosition);
    return static_cast<Self&>(*this);
}

template <class Self, class Step>
Self& TapChangerMethods<Self, Step>::unsetTapPosition() {
    backend()->invoke(handle(), prop::ACT_UNSET_TAP_POSITION);
    return static_cast<Self&>(*this);
}

template <class Self, class Step>
iidm::optional<int> TapChangerMethods<Self, Step>::getSolvedTapPosition() const {
    return backend()->getOptInt(handle(), prop::TC_SOLVED_TAP_POSITION);
}

template <class Self, class Step>
Self& TapChangerMethods<Self, Step>::setSolvedTapPosition(int solvedTapPosition) {
    backend()->setInt(handle(), prop::TC_SOLVED_TAP_POSITION, solvedTapPosition);
    return static_cast<Self&>(*this);
}

template <class Self, class Step>
Self& TapChangerMethods<Self, Step>::unsetSolvedTapPosition() {
    backend()->invoke(handle(), prop::ACT_UNSET_SOLVED_TAP_POSITION);
    return static_cast<Self&>(*this);
}

template <class Self, class Step>
Step TapChangerMethods<Self, Step>::getStep(int tapPosition) const {
    return Step(backend()->getRelatedByIndex(handle(), Step::TYPE, tapPosition), backend());
}

template <class Self, class Step>
std::map<int, Step> TapChangerMethods<Self, Step>::getAllSteps() const {
    std::map<int, Step> steps;
    int tapPosition = getLowTapPosition();
    for (Step& step : wrapAll<Step>(backend()->getChildren(handle(), Step::TYPE), backend())) {
        steps.emplace(tapPosition++, std::move(step));
    }
    return steps;
}

template <class Self, class Step>
iidm::optional<Step> TapChangerMethods<Self, Step>::getCurrentStep() const {
    return wrapOptional<Step>(backend()->getRelated(handle(), prop::REL_CURRENT_STEP), backend());
}

template <class Self, class Step>
iidm::optional<Step> TapChangerMethods<Self, Step>::getSolvedCurrentStep() const {
    return wrapOptional<Step>(backend()->getRelated(handle(), prop::REL_SOLVED_CURRENT_STEP), backend());
}

template <class Self, class Step>
iidm::optional<int> TapChangerMethods<Self, Step>::getNeutralPosition() const {
    return backend()->getOptInt(handle(), prop::TC_NEUTRAL_POSITION);
}

template <class Self, class Step>
iidm::optional<Step> TapChangerMethods<Self, Step>::getNeutralStep() const {
    return wrapOptional<Step>(backend()->getRelated(handle(), prop::REL_NEUTRAL_STEP), backend());
}

template <class Self, class Step>
bool TapChangerMethods<Self, Step>::isRegulating() const {
    return backend()->getBool(handle(), prop::REGULATING);
}

template <class Self, class Step>
bool TapChangerMethods<Self, Step>::hasLoadTapChangingCapabilities() const {
    return backend()->getBool(handle(), prop::TC_LOAD_TAP_CHANGING_CAPABILITIES);
}

template <class Self, class Step>
Self& TapChangerMethods<Self, Step>::setLoadTapChangingCapabilities(bool loadTapChangingCapabilities) {
    backend()->setBool(handle(), prop::TC_LOAD_TAP_CHANGING_CAPABILITIES, loadTapChangingCapabilities);
    return static_cast<Self&>(*this);
}

template <class Self, class Step>
void TapChangerMethods<Self, Step>::applySolvedValues() {
    backend()->invoke(handle(), prop::ACT_APPLY_SOLVED_VALUES);
}

template <class Self, class Step>
void TapChangerMethods<Self, Step>::setTapPositionToSolvedTapPosition() {
    backend()->invoke(handle(), prop::ACT_SET_TAP_POSITION_TO_SOLVED_TAP_POSITION);
}

template <class Self, class Step>
void TapChangerMethods<Self, Step>::remove() {
    backend()->invoke(handle(), prop::ACT_REMOVE);
}

template class TapChangerMethods<RatioTapChanger, RatioTapChangerStep>;
template class TapChangerMethods<PhaseTapChanger, PhaseTapChangerStep>;

} } // namespace iidm::detail
