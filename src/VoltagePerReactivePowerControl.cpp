#include <iidm/VoltagePerReactivePowerControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

VoltagePerReactivePowerControl::VoltagePerReactivePowerControl(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

double VoltagePerReactivePowerControl::getSlope() const {
    return backend_->getDouble(handle_, prop::EXT_VPRC_SLOPE);
}

VoltagePerReactivePowerControl& VoltagePerReactivePowerControl::setSlope(double slope) {
    backend_->setDouble(handle_, prop::EXT_VPRC_SLOPE, slope);
    return *this;
}

} // namespace iidm
