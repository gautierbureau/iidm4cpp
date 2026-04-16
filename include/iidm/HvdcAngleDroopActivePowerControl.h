#pragma once

#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl HvdcAngleDroopActivePowerControl extension attached to an HvdcLine.
class HvdcAngleDroopActivePowerControl {
public:
    HvdcAngleDroopActivePowerControl() = default;
    explicit HvdcAngleDroopActivePowerControl(ObjectHandle handle, BackendProvider* backend);

    double getDroop() const;
    HvdcAngleDroopActivePowerControl& setDroop(double droop);

    double getP0() const;
    HvdcAngleDroopActivePowerControl& setP0(double p0);

    bool isEnabled() const;
    HvdcAngleDroopActivePowerControl& setEnabled(bool enabled);

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const HvdcAngleDroopActivePowerControl& other) const;
    bool operator!=(const HvdcAngleDroopActivePowerControl& other) const;

private:
    ObjectHandle     handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
