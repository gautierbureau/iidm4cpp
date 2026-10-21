#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The ThreeWindingsTransformerPhaseAngleClock extension of a three windings transformer: an
// object of its own, from ThreeWindingsTransformer::getThreeWindingsTransformerPhaseAngleClock().
class ThreeWindingsTransformerPhaseAngleClock : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::THREE_WINDINGS_TRANSFORMER_PHASE_ANGLE_CLOCK;

    ThreeWindingsTransformerPhaseAngleClock() = default;
    explicit ThreeWindingsTransformerPhaseAngleClock(ObjectHandle handle, BackendProvider* backend);

    // The phase angle clock of legs 2 and 3 (0 to 11: ValidationException otherwise)
    int getPhaseAngleClockLeg2() const { return getInt(prop::EXT_THREE_WT_PAC_CLOCK_LEG2); }
    ThreeWindingsTransformerPhaseAngleClock& setPhaseAngleClockLeg2(int phaseAngleClockLeg2) {
        return setInt(*this, prop::EXT_THREE_WT_PAC_CLOCK_LEG2, phaseAngleClockLeg2);
    }

    int getPhaseAngleClockLeg3() const { return getInt(prop::EXT_THREE_WT_PAC_CLOCK_LEG3); }
    ThreeWindingsTransformerPhaseAngleClock& setPhaseAngleClockLeg3(int phaseAngleClockLeg3) {
        return setInt(*this, prop::EXT_THREE_WT_PAC_CLOCK_LEG3, phaseAngleClockLeg3);
    }
};

} // namespace iidm
