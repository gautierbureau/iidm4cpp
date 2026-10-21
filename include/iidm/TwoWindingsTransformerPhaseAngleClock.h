#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The TwoWindingsTransformerPhaseAngleClock extension of a two windings transformer: an object
// of its own, from TwoWindingsTransformer::getTwoWindingsTransformerPhaseAngleClock().
class TwoWindingsTransformerPhaseAngleClock : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::TWO_WINDINGS_TRANSFORMER_PHASE_ANGLE_CLOCK;

    TwoWindingsTransformerPhaseAngleClock() = default;
    explicit TwoWindingsTransformerPhaseAngleClock(ObjectHandle handle, BackendProvider* backend);

    // The phase angle clock (0 to 11: ValidationException otherwise)
    int getPhaseAngleClock() const { return getInt(prop::EXT_TWO_WT_PAC_CLOCK); }
    TwoWindingsTransformerPhaseAngleClock& setPhaseAngleClock(int phaseAngleClock) {
        return setInt(*this, prop::EXT_TWO_WT_PAC_CLOCK, phaseAngleClock);
    }
};

} // namespace iidm
