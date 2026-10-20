#pragma once

#include <iidm/ActivePowerControl.h>
#include <iidm/Adder.h>

namespace iidm {

// New ActivePowerControl extension of a generator or battery (newActivePowerControl())
class ActivePowerControlAdder : public detail::AdderBase<ActivePowerControlAdder> {
public:
    ActivePowerControlAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    ActivePowerControlAdder& withParticipate(bool participate)                 { return boolField(prop::EXT_APC_PARTICIPATE, participate); }
    ActivePowerControlAdder& withDroop(double droop)                           { return doubleField(prop::EXT_APC_DROOP, droop); }
    ActivePowerControlAdder& withParticipationFactor(double participationFactor) { return doubleField(prop::EXT_APC_PARTICIPATION_FACTOR, participationFactor); }
    ActivePowerControlAdder& withMinTargetP(double minTargetP)                 { return doubleField(prop::EXT_APC_MIN_TARGET_P, minTargetP); }
    ActivePowerControlAdder& withMaxTargetP(double maxTargetP)                 { return doubleField(prop::EXT_APC_MAX_TARGET_P, maxTargetP); }

    ActivePowerControl add() { return ActivePowerControl(create(), backend_); }
};

} // namespace iidm
