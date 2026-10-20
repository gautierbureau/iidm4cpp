#pragma once

#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/ActivePowerControl.h>
#include <iidm/ActivePowerControlAdder.h>
#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/ReactiveCapabilityCurve.h>
#include <iidm/Terminal.h>
#include <string>
#include <iidm/InjectionObservability.h>
#include <iidm/Measurement.h>

namespace iidm {

class BackendProvider;

class Battery {
public:
    Battery() = default;
    explicit Battery(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getTargetP() const;
    Battery& setTargetP(double targetP);

    double getTargetQ() const;
    Battery& setTargetQ(double targetQ);

    double getMinP() const;
    double getMaxP() const;

    bool hasActivePowerControl() const;
    // ExtensionNotFoundException when absent
    ActivePowerControl getActivePowerControl() const;
    ActivePowerControlAdder newActivePowerControl();
    void removeActivePowerControl();

    bool hasMinMaxReactiveLimits() const;
    MinMaxReactiveLimits getMinMaxReactiveLimits() const;

    bool hasReactiveCapabilityCurve() const;
    ReactiveCapabilityCurve getReactiveCapabilityCurve() const;

    Terminal getTerminal() const;

    void connect();
    void disconnect();
    bool isConnected() const;

    // ── State estimation extensions ──────────────────────────────────────
    bool hasInjectionObservability() const;
    InjectionObservability getInjectionObservability() const;

    bool hasMeasurements() const;
    Measurements getMeasurements() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Battery& other) const;
    bool operator!=(const Battery& other) const;

private:
    HandleRef  handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
