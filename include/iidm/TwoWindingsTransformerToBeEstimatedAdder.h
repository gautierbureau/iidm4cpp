#pragma once

#include <iidm/Adder.h>
#include <iidm/TwoWindingsTransformerToBeEstimated.h>

namespace iidm {

// New TwoWindingsTransformerToBeEstimated extension of a two windings transformer
// (newTwoWindingsTransformerToBeEstimated()). Nothing is estimated by default.
class TwoWindingsTransformerToBeEstimatedAdder : public detail::AdderBase<TwoWindingsTransformerToBeEstimatedAdder> {
public:
    TwoWindingsTransformerToBeEstimatedAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    TwoWindingsTransformerToBeEstimatedAdder& withRatioTapChangerStatus(bool toBeEstimated) { return boolField(prop::EXT_TWO_WT_TBE_RTC, toBeEstimated); }
    TwoWindingsTransformerToBeEstimatedAdder& withPhaseTapChangerStatus(bool toBeEstimated) { return boolField(prop::EXT_TWO_WT_TBE_PTC, toBeEstimated); }

    TwoWindingsTransformerToBeEstimated add() { return TwoWindingsTransformerToBeEstimated(create(), backend_); }
};

} // namespace iidm
