#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The HvdcAngleDroopActivePowerControl extension of an HVDC line: an object of its own, from
// HvdcLine::getExtension<HvdcAngleDroopActivePowerControl>(). Its values are floats in powsybl.
class HvdcAngleDroopActivePowerControl : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::HVDC_ANGLE_DROOP_ACTIVE_POWER_CONTROL;

    HvdcAngleDroopActivePowerControl() = default;
    explicit HvdcAngleDroopActivePowerControl(ObjectHandle handle, BackendProvider* backend);

    double getDroop() const { return getDouble(prop::EXT_HADAPC_DROOP); }
    HvdcAngleDroopActivePowerControl& setDroop(double droop) { return setDouble(*this, prop::EXT_HADAPC_DROOP, droop); }

    double getP0() const { return getDouble(prop::EXT_HADAPC_P0); }
    HvdcAngleDroopActivePowerControl& setP0(double p0) { return setDouble(*this, prop::EXT_HADAPC_P0, p0); }

    bool isEnabled() const { return getBool(prop::EXT_HADAPC_ENABLED); }
    HvdcAngleDroopActivePowerControl& setEnabled(bool enabled) {
        return setBool(*this, prop::EXT_HADAPC_ENABLED, enabled);
    }
};

} // namespace iidm
