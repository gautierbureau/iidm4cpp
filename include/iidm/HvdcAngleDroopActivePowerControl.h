#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl HvdcAngleDroopActivePowerControl extension attached to an HvdcLine.
class HvdcAngleDroopActivePowerControl : public detail::ObjectRef {
public:
    HvdcAngleDroopActivePowerControl() = default;
    explicit HvdcAngleDroopActivePowerControl(ObjectHandle handle, BackendProvider* backend);

    double getDroop() const;
    HvdcAngleDroopActivePowerControl& setDroop(double droop);

    double getP0() const;
    HvdcAngleDroopActivePowerControl& setP0(double p0);

    bool isEnabled() const;
    HvdcAngleDroopActivePowerControl& setEnabled(bool enabled);
};

} // namespace iidm
