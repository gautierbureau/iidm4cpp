#include <iidm/TapChangerHolder.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/TwoWindingsTransformer.h>
#include "Wrap.h"

namespace iidm { namespace detail {

template <class Self>
bool TapChangerHolderMethods<Self>::hasRatioTapChanger() const {
    return static_cast<bool>(getRatioTapChanger());
}

template <class Self>
iidm::optional<RatioTapChanger> TapChangerHolderMethods<Self>::getRatioTapChanger() const {
    return wrapOptional<RatioTapChanger>(backend()->getRelated(handle(), prop::RATIO_TAP_CHANGER), backend());
}

template <class Self>
bool TapChangerHolderMethods<Self>::hasPhaseTapChanger() const {
    return static_cast<bool>(getPhaseTapChanger());
}

template <class Self>
iidm::optional<PhaseTapChanger> TapChangerHolderMethods<Self>::getPhaseTapChanger() const {
    return wrapOptional<PhaseTapChanger>(backend()->getRelated(handle(), prop::PHASE_TAP_CHANGER), backend());
}

template <class Self>
RatioTapChangerAdder TapChangerHolderMethods<Self>::newRatioTapChanger() {
    return RatioTapChangerAdder(backend()->invoke(handle(), prop::RATIO_TAP_CHANGER), backend());
}

template <class Self>
PhaseTapChangerAdder TapChangerHolderMethods<Self>::newPhaseTapChanger() {
    return PhaseTapChangerAdder(backend()->invoke(handle(), prop::PHASE_TAP_CHANGER), backend());
}

template <class Self>
RatioTapChangerAdder TapChangerHolderMethods<Self>::newRatioTapChanger(const RatioTapChanger& copied) {
    return copyAdder<RatioTapChangerAdder>(backend(), handle(), prop::RATIO_TAP_CHANGER, Access::handle(copied));
}

template <class Self>
PhaseTapChangerAdder TapChangerHolderMethods<Self>::newPhaseTapChanger(const PhaseTapChanger& copied) {
    return copyAdder<PhaseTapChangerAdder>(backend(), handle(), prop::PHASE_TAP_CHANGER, Access::handle(copied));
}

template class TapChangerHolderMethods<TwoWindingsTransformer>;
template class TapChangerHolderMethods<ThreeWindingsTransformer::Leg>;

} } // namespace iidm::detail
