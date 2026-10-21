#pragma once

#include <iidm/CoordinatedReactiveControl.h>
#include <iidm/CoordinatedReactiveControlAdder.h>
#include <iidm/Enums.h>
#include <iidm/GeneratorEntsoeCategory.h>
#include <iidm/GeneratorEntsoeCategoryAdder.h>
#include <iidm/GeneratorShortCircuit.h>
#include <iidm/GeneratorShortCircuitAdder.h>
#include <iidm/GeneratorStartup.h>
#include <iidm/GeneratorStartupAdder.h>
#include <iidm/Injection.h>
#include <iidm/ReactiveLimitsHolder.h>
#include <iidm/Terminal.h>
#include <iidm/VoltageRegulationHolder.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class Generator : public Injection,
                  public detail::VoltageRegulationHolderMethods<Generator>,
                  public detail::ReactiveLimitsHolderMethods<Generator> {
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

    // ── Solved values ────────────────────────────────────────────────────
    // Its targets (P, Q, V) from the solved values: all of them; the target P
    // (setTargetQToQ() and setTargetVToV() are those of the voltage regulation holder)
    void applySolvedValues();
    void setTargetPToP();

    // ── Extensions ───────────────────────────────────────────────────────
    bool hasCoordinatedReactiveControl() const;
    CoordinatedReactiveControl getCoordinatedReactiveControl() const;
    CoordinatedReactiveControlAdder newCoordinatedReactiveControl();
    void removeCoordinatedReactiveControl();

    bool hasGeneratorShortCircuit() const;
    GeneratorShortCircuit getGeneratorShortCircuit() const;
    GeneratorShortCircuitAdder newGeneratorShortCircuit();
    void removeGeneratorShortCircuit();

    bool hasGeneratorEntsoeCategory() const;
    GeneratorEntsoeCategory getGeneratorEntsoeCategory() const;
    GeneratorEntsoeCategoryAdder newGeneratorEntsoeCategory();
    void removeGeneratorEntsoeCategory();

    bool hasGeneratorStartup() const;
    GeneratorStartup getGeneratorStartup() const;
    GeneratorStartupAdder newGeneratorStartup();
    void removeGeneratorStartup();
};

} // namespace iidm
