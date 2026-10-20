#pragma once

#include <iidm/Injection.h>
#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/ReactiveCapabilityCurve.h>
#include <iidm/VoltageRegulationHolder.h>

namespace iidm {

class Battery : public Injection, public detail::VoltageRegulationHolderMethods<Battery> {
public:
    static constexpr int TYPE = prop::BATTERY;

    Battery() = default;
    explicit Battery(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    double getTargetP() const;
    Battery& setTargetP(double targetP);

    double getMinP() const;
    double getMaxP() const;

    // ── Reactive limits ──────────────────────────────────────────────────
    bool hasMinMaxReactiveLimits() const;
    MinMaxReactiveLimits getMinMaxReactiveLimits() const;

    bool hasReactiveCapabilityCurve() const;
    ReactiveCapabilityCurve getReactiveCapabilityCurve() const;
};

} // namespace iidm
