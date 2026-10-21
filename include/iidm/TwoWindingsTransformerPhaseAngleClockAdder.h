#pragma once

#include <iidm/Adder.h>
#include <iidm/TwoWindingsTransformerPhaseAngleClock.h>

namespace iidm {

// New TwoWindingsTransformerPhaseAngleClock extension of a two windings transformer (newTwoWindingsTransformerPhaseAngleClock())
class TwoWindingsTransformerPhaseAngleClockAdder : public detail::AdderBase<TwoWindingsTransformerPhaseAngleClockAdder> {
public:
    TwoWindingsTransformerPhaseAngleClockAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    TwoWindingsTransformerPhaseAngleClockAdder& withPhaseAngleClock(int phaseAngleClock) { return intField(prop::EXT_TWO_WT_PAC_CLOCK, phaseAngleClock); }

    TwoWindingsTransformerPhaseAngleClock add() { return TwoWindingsTransformerPhaseAngleClock(create(), backend_); }
};

} // namespace iidm
