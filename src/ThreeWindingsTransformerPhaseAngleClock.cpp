#include <iidm/ThreeWindingsTransformerPhaseAngleClock.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

ThreeWindingsTransformerPhaseAngleClock::ThreeWindingsTransformerPhaseAngleClock(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

int ThreeWindingsTransformerPhaseAngleClock::getPhaseAngleClockLeg2() const {
    return backend_->getInt(handle_, prop::EXT_THREE_WT_PAC_CLOCK_LEG2);
}

ThreeWindingsTransformerPhaseAngleClock& ThreeWindingsTransformerPhaseAngleClock::setPhaseAngleClockLeg2(int phaseAngleClockLeg2) {
    backend_->setInt(handle_, prop::EXT_THREE_WT_PAC_CLOCK_LEG2, phaseAngleClockLeg2);
    return *this;
}

int ThreeWindingsTransformerPhaseAngleClock::getPhaseAngleClockLeg3() const {
    return backend_->getInt(handle_, prop::EXT_THREE_WT_PAC_CLOCK_LEG3);
}

ThreeWindingsTransformerPhaseAngleClock& ThreeWindingsTransformerPhaseAngleClock::setPhaseAngleClockLeg3(int phaseAngleClockLeg3) {
    backend_->setInt(handle_, prop::EXT_THREE_WT_PAC_CLOCK_LEG3, phaseAngleClockLeg3);
    return *this;
}

} // namespace iidm
