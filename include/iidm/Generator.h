#pragma once

#include <iidm/CoordinatedReactiveControl.h>
#include <iidm/Enums.h>
#include <iidm/Injection.h>
#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/ReactiveCapabilityCurve.h>
#include <iidm/Terminal.h>
#include <iidm/VoltageRegulation.h>
#include <iidm/VoltageRegulationAdder.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class Generator : public Injection {
public:
    static constexpr int TYPE = prop::GENERATOR;

    Generator() = default;
    explicit Generator(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

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

    // ── Reactive limits ──────────────────────────────────────────────────
    bool hasMinMaxReactiveLimits() const;
    MinMaxReactiveLimits getMinMaxReactiveLimits() const;

    bool hasReactiveCapabilityCurve() const;
    ReactiveCapabilityCurve getReactiveCapabilityCurve() const;

    // ── Extensions ───────────────────────────────────────────────────────
    bool hasCoordinatedReactiveControl() const;
    CoordinatedReactiveControl getCoordinatedReactiveControl() const;
};

} // namespace iidm
