#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class BackendProvider;

// The ActivePowerControl extension of an injection (generator, battery...): an object of its own,
// from Injection::getActivePowerControl().
class ActivePowerControl : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::ACTIVE_POWER_CONTROL;

    ActivePowerControl() = default;
    explicit ActivePowerControl(ObjectHandle handle, BackendProvider* backend);

    bool isParticipate() const;
    ActivePowerControl& setParticipate(bool participate);

    double getDroop() const;
    ActivePowerControl& setDroop(double droop);

    double getParticipationFactor() const;
    ActivePowerControl& setParticipationFactor(double participationFactor);

    // Limits of the active power target used when the injection participates; empty when
    // undefined. A value outside [minP, maxP], or min > max, throws ValidationException.
    iidm::optional<double> getMinTargetP() const;
    ActivePowerControl& setMinTargetP(double minTargetP);
    ActivePowerControl& unsetMinTargetP();

    iidm::optional<double> getMaxTargetP() const;
    ActivePowerControl& setMaxTargetP(double maxTargetP);
    ActivePowerControl& unsetMaxTargetP();
};

} // namespace iidm
