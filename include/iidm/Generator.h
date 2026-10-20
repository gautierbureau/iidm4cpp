#pragma once

#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/ActivePowerControl.h>
#include <iidm/ActivePowerControlAdder.h>
#include <iidm/CoordinatedReactiveControl.h>
#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/ReactiveCapabilityCurve.h>
#include <iidm/Enums.h>
#include <iidm/Terminal.h>
#include <iidm/VoltageRegulation.h>
#include <iidm/VoltageRegulationAdder.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <iidm/InjectionObservability.h>
#include <iidm/Measurement.h>

namespace iidm {

class BackendProvider;

class Generator {
public:
    Generator() = default;
    explicit Generator(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getTargetP() const;
    Generator& setTargetP(double targetP);

    double getMinP() const;
    Generator& setMinP(double minP);

    double getMaxP() const;
    Generator& setMaxP(double maxP);

    double getRatedS() const;
    Generator& setRatedS(double ratedS);

    bool isCondenser() const;

    EnergySource getEnergySource() const;
    Generator& setEnergySource(EnergySource energySource);

    // ── Voltage regulation ───────────────────────────────────────────────
    // Local targets: the target V of a local voltage regulation; the target Q when not regulating
    // or regulating Q locally (see VoltageRegulation)
    double getLocalTargetV() const;
    Generator& setLocalTargetV(double localTargetV);
    double getLocalTargetQ() const;
    Generator& setLocalTargetQ(double localTargetQ);

    // Empty when the generator has no voltage regulation
    iidm::optional<VoltageRegulation> getVoltageRegulation() const;
    // A new voltage regulation, which replaces the current one at build()
    VoltageRegulationBuilder newVoltageRegulation();
    void removeVoltageRegulation();

    // Has a voltage regulation, and it is regulating
    bool isRegulating() const;
    // Target of the regulation if it regulates V (or Q) remotely, else the local target
    double getRegulatingTargetV() const;
    double getRegulatingTargetQ() const;
    // Terminal of a remote regulation, else the generator's own terminal
    Terminal getRegulatingTerminal() const;

    Terminal getTerminal() const;

    bool hasActivePowerControl() const;
    // ExtensionNotFoundException when absent
    ActivePowerControl getActivePowerControl() const;
    ActivePowerControlAdder newActivePowerControl();
    void removeActivePowerControl();

    bool hasCoordinatedReactiveControl() const;
    CoordinatedReactiveControl getCoordinatedReactiveControl() const;

    bool hasMinMaxReactiveLimits() const;
    MinMaxReactiveLimits getMinMaxReactiveLimits() const;

    bool hasReactiveCapabilityCurve() const;
    ReactiveCapabilityCurve getReactiveCapabilityCurve() const;

    void connect();
    void disconnect();
    bool isConnected() const;

    // Removes the generator from the network
    void remove();

    // ── State estimation extensions ──────────────────────────────────────
    bool hasInjectionObservability() const;
    InjectionObservability getInjectionObservability() const;

    bool hasMeasurements() const;
    Measurements getMeasurements() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Generator& other) const;
    bool operator!=(const Generator& other) const;

private:
    HandleRef handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
