#include <iidm/ThreeWindingsTransformerPhaseAngleClock.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

ThreeWindingsTransformerPhaseAngleClock::ThreeWindingsTransformerPhaseAngleClock(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

} // namespace iidm
