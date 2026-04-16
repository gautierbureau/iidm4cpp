#pragma once

#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl VoltagePerReactivePowerControl extension attached to a StaticVarCompensator.
class VoltagePerReactivePowerControl {
public:
    VoltagePerReactivePowerControl() = default;
    explicit VoltagePerReactivePowerControl(ObjectHandle handle, BackendProvider* backend);

    double getSlope() const;
    VoltagePerReactivePowerControl& setSlope(double slope);

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const VoltagePerReactivePowerControl& other) const;
    bool operator!=(const VoltagePerReactivePowerControl& other) const;

private:
    ObjectHandle     handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
