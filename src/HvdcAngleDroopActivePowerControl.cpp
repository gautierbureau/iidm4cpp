#include <iidm/HvdcAngleDroopActivePowerControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

HvdcAngleDroopActivePowerControl::HvdcAngleDroopActivePowerControl(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

} // namespace iidm
