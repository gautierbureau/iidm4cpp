#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The StandbyAutomaton extension of a static var compensator: in standby, it holds a fixed
// susceptance until the voltage crosses a threshold, then regulates to the matching setpoint. An
// object of its own, from StaticVarCompensator::getStandbyAutomaton(). ValidationException for a
// NaN value, and in standby for a low threshold not below the high one.
class StandbyAutomaton : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::STANDBY_AUTOMATON;

    StandbyAutomaton() = default;
    explicit StandbyAutomaton(ObjectHandle handle, BackendProvider* backend);

    // In standby
    bool isStandby() const { return getBool(prop::EXT_SA_STANDBY); }
    StandbyAutomaton& setStandby(bool standby) { return setBool(*this, prop::EXT_SA_STANDBY, standby); }

    // The fixed susceptance in standby (S), between the bounds of the compensator
    double getB0() const { return getDouble(prop::EXT_SA_B0); }
    StandbyAutomaton& setB0(double b0) { return setDouble(*this, prop::EXT_SA_B0, b0); }

    // The voltage setpoint once the low voltage threshold is reached (kV)
    double getLowVoltageSetpoint() const { return getDouble(prop::EXT_SA_LOW_VOLTAGE_SETPOINT); }
    StandbyAutomaton& setLowVoltageSetpoint(double lowVoltageSetpoint) {
        return setDouble(*this, prop::EXT_SA_LOW_VOLTAGE_SETPOINT, lowVoltageSetpoint);
    }

    // The voltage setpoint once the high voltage threshold is reached (kV)
    double getHighVoltageSetpoint() const { return getDouble(prop::EXT_SA_HIGH_VOLTAGE_SETPOINT); }
    StandbyAutomaton& setHighVoltageSetpoint(double highVoltageSetpoint) {
        return setDouble(*this, prop::EXT_SA_HIGH_VOLTAGE_SETPOINT, highVoltageSetpoint);
    }

    // kV
    double getLowVoltageThreshold() const { return getDouble(prop::EXT_SA_LOW_VOLTAGE_THRESHOLD); }
    StandbyAutomaton& setLowVoltageThreshold(double lowVoltageThreshold) {
        return setDouble(*this, prop::EXT_SA_LOW_VOLTAGE_THRESHOLD, lowVoltageThreshold);
    }

    // kV
    double getHighVoltageThreshold() const { return getDouble(prop::EXT_SA_HIGH_VOLTAGE_THRESHOLD); }
    StandbyAutomaton& setHighVoltageThreshold(double highVoltageThreshold) {
        return setDouble(*this, prop::EXT_SA_HIGH_VOLTAGE_THRESHOLD, highVoltageThreshold);
    }
};

} // namespace iidm
