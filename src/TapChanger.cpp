#include <iidm/TapChanger.h>
#include <iidm/BackendProvider.h>
#include <iidm/PhaseTapChanger.h>
#include <iidm/PropertyCodes.h>
#include <iidm/RatioTapChanger.h>
#include "Wrap.h"
#include <utility>

namespace iidm { namespace detail {

template <class Self, class Step, class Replacer>
int TapChangerMethods<Self, Step, Replacer>::getLowTapPosition() const {
    return backend()->getInt(handle(), prop::TC_LOW_TAP_POSITION);
}

template <class Self, class Step, class Replacer>
Self& TapChangerMethods<Self, Step, Replacer>::setLowTapPosition(int lowTapPosition) {
    backend()->setInt(handle(), prop::TC_LOW_TAP_POSITION, lowTapPosition);
    return static_cast<Self&>(*this);
}

template <class Self, class Step, class Replacer>
int TapChangerMethods<Self, Step, Replacer>::getHighTapPosition() const {
    return backend()->getInt(handle(), prop::TC_HIGH_TAP_POSITION);
}

template <class Self, class Step, class Replacer>
int TapChangerMethods<Self, Step, Replacer>::getStepCount() const {
    return backend()->getInt(handle(), prop::TC_STEP_COUNT);
}

template <class Self, class Step, class Replacer>
int TapChangerMethods<Self, Step, Replacer>::getTapPosition() const {
    return backend()->getInt(handle(), prop::TC_TAP_POSITION);
}

template <class Self, class Step, class Replacer>
iidm::optional<int> TapChangerMethods<Self, Step, Replacer>::findTapPosition() const {
    return backend()->getOptInt(handle(), prop::TC_TAP_POSITION);
}

template <class Self, class Step, class Replacer>
Self& TapChangerMethods<Self, Step, Replacer>::setTapPosition(int tapPosition) {
    backend()->setInt(handle(), prop::TC_TAP_POSITION, tapPosition);
    return static_cast<Self&>(*this);
}

template <class Self, class Step, class Replacer>
Self& TapChangerMethods<Self, Step, Replacer>::unsetTapPosition() {
    backend()->invoke(handle(), prop::ACT_UNSET_TAP_POSITION);
    return static_cast<Self&>(*this);
}

template <class Self, class Step, class Replacer>
iidm::optional<int> TapChangerMethods<Self, Step, Replacer>::getSolvedTapPosition() const {
    return backend()->getOptInt(handle(), prop::TC_SOLVED_TAP_POSITION);
}

template <class Self, class Step, class Replacer>
iidm::optional<int> TapChangerMethods<Self, Step, Replacer>::findSolvedTapPosition() const {
    return getSolvedTapPosition();
}

template <class Self, class Step, class Replacer>
Self& TapChangerMethods<Self, Step, Replacer>::setSolvedTapPosition(int solvedTapPosition) {
    backend()->setInt(handle(), prop::TC_SOLVED_TAP_POSITION, solvedTapPosition);
    return static_cast<Self&>(*this);
}

template <class Self, class Step, class Replacer>
Self& TapChangerMethods<Self, Step, Replacer>::unsetSolvedTapPosition() {
    backend()->invoke(handle(), prop::ACT_UNSET_SOLVED_TAP_POSITION);
    return static_cast<Self&>(*this);
}

template <class Self, class Step, class Replacer>
Step TapChangerMethods<Self, Step, Replacer>::getStep(int tapPosition) const {
    return Step(backend()->getRelatedByIndex(handle(), Step::TYPE, tapPosition), backend());
}

template <class Self, class Step, class Replacer>
std::map<int, Step> TapChangerMethods<Self, Step, Replacer>::getAllSteps() const {
    std::map<int, Step> steps;
    int tapPosition = getLowTapPosition();
    for (Step& step : wrapAll<Step>(backend()->getChildren(handle(), Step::TYPE), backend())) {
        steps.emplace(tapPosition++, std::move(step));
    }
    return steps;
}

template <class Self, class Step, class Replacer>
iidm::optional<Step> TapChangerMethods<Self, Step, Replacer>::getCurrentStep() const {
    return wrapOptional<Step>(backend()->getRelated(handle(), prop::REL_CURRENT_STEP), backend());
}

template <class Self, class Step, class Replacer>
iidm::optional<Step> TapChangerMethods<Self, Step, Replacer>::getSolvedCurrentStep() const {
    return wrapOptional<Step>(backend()->getRelated(handle(), prop::REL_SOLVED_CURRENT_STEP), backend());
}

template <class Self, class Step, class Replacer>
iidm::optional<int> TapChangerMethods<Self, Step, Replacer>::getNeutralPosition() const {
    return backend()->getOptInt(handle(), prop::TC_NEUTRAL_POSITION);
}

template <class Self, class Step, class Replacer>
iidm::optional<Step> TapChangerMethods<Self, Step, Replacer>::getNeutralStep() const {
    return wrapOptional<Step>(backend()->getRelated(handle(), prop::REL_NEUTRAL_STEP), backend());
}

template <class Self, class Step, class Replacer>
bool TapChangerMethods<Self, Step, Replacer>::isRegulating() const {
    return backend()->getBool(handle(), prop::REGULATING);
}

template <class Self, class Step, class Replacer>
bool TapChangerMethods<Self, Step, Replacer>::hasLoadTapChangingCapabilities() const {
    return backend()->getBool(handle(), prop::TC_LOAD_TAP_CHANGING_CAPABILITIES);
}

template <class Self, class Step, class Replacer>
Self& TapChangerMethods<Self, Step, Replacer>::setLoadTapChangingCapabilities(bool loadTapChangingCapabilities) {
    backend()->setBool(handle(), prop::TC_LOAD_TAP_CHANGING_CAPABILITIES, loadTapChangingCapabilities);
    return static_cast<Self&>(*this);
}

template <class Self, class Step, class Replacer>
void TapChangerMethods<Self, Step, Replacer>::applySolvedValues() {
    backend()->invoke(handle(), prop::ACT_APPLY_SOLVED_VALUES);
}

template <class Self, class Step, class Replacer>
void TapChangerMethods<Self, Step, Replacer>::setTapPositionToSolvedTapPosition() {
    backend()->invoke(handle(), prop::ACT_SET_TAP_POSITION_TO_SOLVED_TAP_POSITION);
}

template <class Self, class Step, class Replacer>
Replacer TapChangerMethods<Self, Step, Replacer>::stepsReplacer() {
    return Replacer(backend()->invoke(handle(), prop::TC_STEPS_REPLACER), backend());
}

template <class Self, class Step, class Replacer>
void TapChangerMethods<Self, Step, Replacer>::remove() {
    backend()->invoke(handle(), prop::ACT_REMOVE);
}

template class TapChangerMethods<RatioTapChanger, RatioTapChangerStep, RatioTapChangerStepsReplacer>;
template class TapChangerMethods<PhaseTapChanger, PhaseTapChangerStep, PhaseTapChangerStepsReplacer>;

} } // namespace iidm::detail
