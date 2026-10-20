#pragma once

#include <iidm/Adder.h>
#include <iidm/HvdcAngleDroopActivePowerControl.h>

namespace iidm {

// New HvdcAngleDroopActivePowerControl extension of an HVDC line
// (newHvdcAngleDroopActivePowerControl())
class HvdcAngleDroopActivePowerControlAdder : public detail::AdderBase<HvdcAngleDroopActivePowerControlAdder> {
public:
    HvdcAngleDroopActivePowerControlAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    HvdcAngleDroopActivePowerControlAdder& withP0(double p0)          { return doubleField(prop::EXT_HADAPC_P0, p0); }
    HvdcAngleDroopActivePowerControlAdder& withDroop(double droop)    { return doubleField(prop::EXT_HADAPC_DROOP, droop); }
    HvdcAngleDroopActivePowerControlAdder& withEnabled(bool enabled)  { return boolField(prop::EXT_HADAPC_ENABLED, enabled); }

    HvdcAngleDroopActivePowerControl add() { return HvdcAngleDroopActivePowerControl(create(), backend_); }
};

} // namespace iidm
