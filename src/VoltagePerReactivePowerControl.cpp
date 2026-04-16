#include <iidm/VoltagePerReactivePowerControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

VoltagePerReactivePowerControl::VoltagePerReactivePowerControl(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

double VoltagePerReactivePowerControl::getSlope() const {
    return backend_->getDouble(handle_, prop::EXT_VPRC_SLOPE);
}

VoltagePerReactivePowerControl& VoltagePerReactivePowerControl::setSlope(double slope) {
    backend_->setDouble(handle_, prop::EXT_VPRC_SLOPE, slope);
    return *this;
}

bool VoltagePerReactivePowerControl::operator==(const VoltagePerReactivePowerControl& other) const {
    return handle_ == other.handle_;
}

bool VoltagePerReactivePowerControl::operator!=(const VoltagePerReactivePowerControl& other) const {
    return !(*this == other);
}

} // namespace iidm
