#include <iidm/ActivePowerControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include <limits>

namespace iidm {

ActivePowerControl::ActivePowerControl(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

ActivePowerControl& ActivePowerControl::unsetMinTargetP() {
    return setMinTargetP(std::numeric_limits<double>::quiet_NaN());
}

ActivePowerControl& ActivePowerControl::unsetMaxTargetP() {
    return setMaxTargetP(std::numeric_limits<double>::quiet_NaN());
}

} // namespace iidm
