#pragma once

#include <iidm/Adder.h>
#include <iidm/StandbyAutomaton.h>

namespace iidm {

// New StandbyAutomaton extension of a static var compensator
class StandbyAutomatonAdder : public detail::AdderBase<StandbyAutomatonAdder> {
public:
    StandbyAutomatonAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    StandbyAutomatonAdder& withStandbyStatus(bool standby) { return boolField(prop::EXT_SA_STANDBY, standby); }
    StandbyAutomatonAdder& withB0(double b0) { return doubleField(prop::EXT_SA_B0, b0); }
    StandbyAutomatonAdder& withLowVoltageSetpoint(double lowVoltageSetpoint) { return doubleField(prop::EXT_SA_LOW_VOLTAGE_SETPOINT, lowVoltageSetpoint); }
    StandbyAutomatonAdder& withHighVoltageSetpoint(double highVoltageSetpoint) { return doubleField(prop::EXT_SA_HIGH_VOLTAGE_SETPOINT, highVoltageSetpoint); }
    StandbyAutomatonAdder& withLowVoltageThreshold(double lowVoltageThreshold) { return doubleField(prop::EXT_SA_LOW_VOLTAGE_THRESHOLD, lowVoltageThreshold); }
    StandbyAutomatonAdder& withHighVoltageThreshold(double highVoltageThreshold) { return doubleField(prop::EXT_SA_HIGH_VOLTAGE_THRESHOLD, highVoltageThreshold); }

    StandbyAutomaton add() { return StandbyAutomaton(create(), backend_); }
};

} // namespace iidm
