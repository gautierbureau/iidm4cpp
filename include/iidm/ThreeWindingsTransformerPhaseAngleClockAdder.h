#pragma once

#include <iidm/Adder.h>
#include <iidm/ThreeWindingsTransformerPhaseAngleClock.h>

namespace iidm {

// New ThreeWindingsTransformerPhaseAngleClock extension of a three windings transformer
class ThreeWindingsTransformerPhaseAngleClockAdder : public detail::AdderBase<ThreeWindingsTransformerPhaseAngleClockAdder> {
public:
    ThreeWindingsTransformerPhaseAngleClockAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    ThreeWindingsTransformerPhaseAngleClockAdder& withPhaseAngleClockLeg2(int phaseAngleClockLeg2) { return intField(prop::EXT_THREE_WT_PAC_CLOCK_LEG2, phaseAngleClockLeg2); }
    ThreeWindingsTransformerPhaseAngleClockAdder& withPhaseAngleClockLeg3(int phaseAngleClockLeg3) { return intField(prop::EXT_THREE_WT_PAC_CLOCK_LEG3, phaseAngleClockLeg3); }

    ThreeWindingsTransformerPhaseAngleClock add() { return ThreeWindingsTransformerPhaseAngleClock(create(), backend_); }
};

} // namespace iidm
