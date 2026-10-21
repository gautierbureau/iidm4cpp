#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/ThreeWindingsTransformerToBeEstimated.h>

namespace iidm {

// New ThreeWindingsTransformerToBeEstimated extension of a three windings transformer. Nothing is
// estimated by default.
class ThreeWindingsTransformerToBeEstimatedAdder : public detail::AdderBase<ThreeWindingsTransformerToBeEstimatedAdder> {
public:
    ThreeWindingsTransformerToBeEstimatedAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    ThreeWindingsTransformerToBeEstimatedAdder& withRatioTapChanger1Status(bool toBeEstimated) { return withRatioTapChangerStatus(ThreeSides::ONE, toBeEstimated); }
    ThreeWindingsTransformerToBeEstimatedAdder& withRatioTapChanger2Status(bool toBeEstimated) { return withRatioTapChangerStatus(ThreeSides::TWO, toBeEstimated); }
    ThreeWindingsTransformerToBeEstimatedAdder& withRatioTapChanger3Status(bool toBeEstimated) { return withRatioTapChangerStatus(ThreeSides::THREE, toBeEstimated); }
    ThreeWindingsTransformerToBeEstimatedAdder& withRatioTapChangerStatus(ThreeSides side, bool toBeEstimated) {
        return boolField(prop::EXT_THREE_WT_TBE_RTC1 + static_cast<int>(side), toBeEstimated);
    }

    ThreeWindingsTransformerToBeEstimatedAdder& withPhaseTapChanger1Status(bool toBeEstimated) { return withPhaseTapChangerStatus(ThreeSides::ONE, toBeEstimated); }
    ThreeWindingsTransformerToBeEstimatedAdder& withPhaseTapChanger2Status(bool toBeEstimated) { return withPhaseTapChangerStatus(ThreeSides::TWO, toBeEstimated); }
    ThreeWindingsTransformerToBeEstimatedAdder& withPhaseTapChanger3Status(bool toBeEstimated) { return withPhaseTapChangerStatus(ThreeSides::THREE, toBeEstimated); }
    ThreeWindingsTransformerToBeEstimatedAdder& withPhaseTapChangerStatus(ThreeSides side, bool toBeEstimated) {
        return boolField(prop::EXT_THREE_WT_TBE_PTC1 + static_cast<int>(side), toBeEstimated);
    }

    ThreeWindingsTransformerToBeEstimated add() { return ThreeWindingsTransformerToBeEstimated(create(), backend_); }
};

} // namespace iidm
