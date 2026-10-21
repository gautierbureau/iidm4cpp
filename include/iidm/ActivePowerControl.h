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

    bool isParticipate() const { return getBool(prop::EXT_APC_PARTICIPATE); }
    ActivePowerControl& setParticipate(bool participate) {
        return setBool(*this, prop::EXT_APC_PARTICIPATE, participate);
    }

    double getDroop() const { return getDouble(prop::EXT_APC_DROOP); }
    ActivePowerControl& setDroop(double droop) { return setDouble(*this, prop::EXT_APC_DROOP, droop); }

    double getParticipationFactor() const { return getDouble(prop::EXT_APC_PARTICIPATION_FACTOR); }
    ActivePowerControl& setParticipationFactor(double participationFactor) {
        return setDouble(*this, prop::EXT_APC_PARTICIPATION_FACTOR, participationFactor);
    }

    // Limits of the active power target used when the injection participates; empty when
    // undefined. A value outside [minP, maxP], or min > max, throws ValidationException.
    iidm::optional<double> getMinTargetP() const { return getOptDouble(prop::EXT_APC_MIN_TARGET_P); }
    ActivePowerControl& setMinTargetP(double minTargetP) {
        return setDouble(*this, prop::EXT_APC_MIN_TARGET_P, minTargetP);
    }
    ActivePowerControl& unsetMinTargetP();

    iidm::optional<double> getMaxTargetP() const { return getOptDouble(prop::EXT_APC_MAX_TARGET_P); }
    ActivePowerControl& setMaxTargetP(double maxTargetP) {
        return setDouble(*this, prop::EXT_APC_MAX_TARGET_P, maxTargetP);
    }
    ActivePowerControl& unsetMaxTargetP();
};

} // namespace iidm
