#pragma once

#include <iidm/Adder.h>
#include <iidm/GeneratorStartup.h>

namespace iidm {

// New GeneratorStartup extension of a generator
class GeneratorStartupAdder : public detail::AdderBase<GeneratorStartupAdder> {
public:
    GeneratorStartupAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    GeneratorStartupAdder& withPlannedActivePowerSetpoint(double plannedActivePowerSetpoint) { return doubleField(prop::EXT_GS_PLANNED_ACTIVE_POWER_SETPOINT, plannedActivePowerSetpoint); }
    GeneratorStartupAdder& withStartupCost(double startupCost) { return doubleField(prop::EXT_GS_STARTUP_COST, startupCost); }
    GeneratorStartupAdder& withMarginalCost(double marginalCost) { return doubleField(prop::EXT_GS_MARGINAL_COST, marginalCost); }
    GeneratorStartupAdder& withPlannedOutageRate(double plannedOutageRate) { return doubleField(prop::EXT_GS_PLANNED_OUTAGE_RATE, plannedOutageRate); }
    GeneratorStartupAdder& withForcedOutageRate(double forcedOutageRate) { return doubleField(prop::EXT_GS_FORCED_OUTAGE_RATE, forcedOutageRate); }

    GeneratorStartup add() { return GeneratorStartup(create(), backend_); }
};

} // namespace iidm
