#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The GeneratorStartup extension of a generator: its market data. An object of its own, from
// Generator::getGeneratorStartup(). Each value is NaN when undefined.
class GeneratorStartup : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::GENERATOR_STARTUP;

    GeneratorStartup() = default;
    explicit GeneratorStartup(ObjectHandle handle, BackendProvider* backend);

    // The active power target planned by the market (MW)
    double getPlannedActivePowerSetpoint() const;
    GeneratorStartup& setPlannedActivePowerSetpoint(double plannedActivePowerSetpoint);

    // What it costs to start the generator
    double getStartupCost() const;
    GeneratorStartup& setStartupCost(double startupCost);

    // What one more unit (in general one MW) costs
    double getMarginalCost() const;
    GeneratorStartup& setMarginalCost(double marginalCost);

    // Rate of planned unavailability (no unit)
    double getPlannedOutageRate() const;
    GeneratorStartup& setPlannedOutageRate(double plannedOutageRate);

    // Rate of forced unavailability (no unit)
    double getForcedOutageRate() const;
    GeneratorStartup& setForcedOutageRate(double forcedOutageRate);
};

} // namespace iidm
