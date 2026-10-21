#include <iidm/TwoWindingsTransformerPhaseAngleClock.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

TwoWindingsTransformerPhaseAngleClock::TwoWindingsTransformerPhaseAngleClock(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

} // namespace iidm
