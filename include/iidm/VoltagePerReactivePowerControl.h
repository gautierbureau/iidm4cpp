#pragma once

#include <iidm/HandleRef.h>
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
    HandleRef  handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
