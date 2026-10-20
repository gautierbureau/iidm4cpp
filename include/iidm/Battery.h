#pragma once

#include <iidm/Injection.h>
#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/ReactiveCapabilityCurve.h>

namespace iidm {

class Battery : public Injection {
public:
    static constexpr int TYPE = prop::BATTERY;

    Battery() = default;
    explicit Battery(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    double getTargetP() const;
    Battery& setTargetP(double targetP);

    double getTargetQ() const;
    Battery& setTargetQ(double targetQ);

    double getMinP() const;
    double getMaxP() const;

    // ── Reactive limits ──────────────────────────────────────────────────
    bool hasMinMaxReactiveLimits() const;
    MinMaxReactiveLimits getMinMaxReactiveLimits() const;

    bool hasReactiveCapabilityCurve() const;
    ReactiveCapabilityCurve getReactiveCapabilityCurve() const;
};

} // namespace iidm
