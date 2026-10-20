#pragma once

#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class BackendProvider;

// The ActivePowerControl extension of a generator or a battery: an object of its own, from
// Generator::getActivePowerControl() / Battery::getActivePowerControl().
class ActivePowerControl {
public:
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

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const ActivePowerControl& other) const;
    bool operator!=(const ActivePowerControl& other) const;

private:
    HandleRef handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
