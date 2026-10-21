#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The GeneratorStartup extension of a generator: its market data. An object of its own, from
// Generator::getExtension<GeneratorStartup>(). Each value is NaN when undefined.
class GeneratorStartup : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::GENERATOR_STARTUP;

    GeneratorStartup() = default;
    explicit GeneratorStartup(ObjectHandle handle, BackendProvider* backend);

    // The active power target planned by the market (MW)
    double getPlannedActivePowerSetpoint() const { return getDouble(prop::EXT_GS_PLANNED_ACTIVE_POWER_SETPOINT); }
    GeneratorStartup& setPlannedActivePowerSetpoint(double plannedActivePowerSetpoint) {
        return setDouble(*this, prop::EXT_GS_PLANNED_ACTIVE_POWER_SETPOINT, plannedActivePowerSetpoint);
    }

    // What it costs to start the generator
    double getStartupCost() const { return getDouble(prop::EXT_GS_STARTUP_COST); }
    GeneratorStartup& setStartupCost(double startupCost) {
        return setDouble(*this, prop::EXT_GS_STARTUP_COST, startupCost);
    }

    // What one more unit (in general one MW) costs
    double getMarginalCost() const { return getDouble(prop::EXT_GS_MARGINAL_COST); }
    GeneratorStartup& setMarginalCost(double marginalCost) {
        return setDouble(*this, prop::EXT_GS_MARGINAL_COST, marginalCost);
    }

    // Rate of planned unavailability (no unit)
    double getPlannedOutageRate() const { return getDouble(prop::EXT_GS_PLANNED_OUTAGE_RATE); }
    GeneratorStartup& setPlannedOutageRate(double plannedOutageRate) {
        return setDouble(*this, prop::EXT_GS_PLANNED_OUTAGE_RATE, plannedOutageRate);
    }

    // Rate of forced unavailability (no unit)
    double getForcedOutageRate() const { return getDouble(prop::EXT_GS_FORCED_OUTAGE_RATE); }
    GeneratorStartup& setForcedOutageRate(double forcedOutageRate) {
        return setDouble(*this, prop::EXT_GS_FORCED_OUTAGE_RATE, forcedOutageRate);
    }
};

} // namespace iidm
