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
    bool isStandby() const;
    StandbyAutomaton& setStandby(bool standby);

    // The fixed susceptance in standby (S), between the bounds of the compensator
    double getB0() const;
    StandbyAutomaton& setB0(double b0);

    // The voltage setpoint once the low voltage threshold is reached (kV)
    double getLowVoltageSetpoint() const;
    StandbyAutomaton& setLowVoltageSetpoint(double lowVoltageSetpoint);

    // The voltage setpoint once the high voltage threshold is reached (kV)
    double getHighVoltageSetpoint() const;
    StandbyAutomaton& setHighVoltageSetpoint(double highVoltageSetpoint);

    // kV
    double getLowVoltageThreshold() const;
    StandbyAutomaton& setLowVoltageThreshold(double lowVoltageThreshold);

    // kV
    double getHighVoltageThreshold() const;
    StandbyAutomaton& setHighVoltageThreshold(double highVoltageThreshold);
};

} // namespace iidm
