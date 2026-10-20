#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The HvdcAngleDroopActivePowerControl extension of an HVDC line: an object of its own, from
// HvdcLine::getHvdcAngleDroopActivePowerControl(). Its values are floats in powsybl.
class HvdcAngleDroopActivePowerControl : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::HVDC_ANGLE_DROOP_ACTIVE_POWER_CONTROL;

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
