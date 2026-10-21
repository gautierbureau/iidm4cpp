#include <iidm/TwoWindingsTransformerPhaseAngleClock.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

TwoWindingsTransformerPhaseAngleClock::TwoWindingsTransformerPhaseAngleClock(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

int TwoWindingsTransformerPhaseAngleClock::getPhaseAngleClock() const {
    return backend_->getInt(handle_, prop::EXT_TWO_WT_PAC_CLOCK);
}

TwoWindingsTransformerPhaseAngleClock& TwoWindingsTransformerPhaseAngleClock::setPhaseAngleClock(int phaseAngleClock) {
    backend_->setInt(handle_, prop::EXT_TWO_WT_PAC_CLOCK, phaseAngleClock);
    return *this;
}

} // namespace iidm
