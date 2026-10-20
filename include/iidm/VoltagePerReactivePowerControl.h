#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl VoltagePerReactivePowerControl extension attached to a StaticVarCompensator.
class VoltagePerReactivePowerControl : public detail::ObjectRef {
public:
    VoltagePerReactivePowerControl() = default;
    explicit VoltagePerReactivePowerControl(ObjectHandle handle, BackendProvider* backend);

    double getSlope() const;
    VoltagePerReactivePowerControl& setSlope(double slope);
};

} // namespace iidm
