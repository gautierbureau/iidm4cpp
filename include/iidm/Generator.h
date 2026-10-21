#pragma once

#include <iidm/CoordinatedReactiveControl.h>
#include <iidm/CoordinatedReactiveControlAdder.h>
#include <iidm/Enums.h>
#include <iidm/GeneratorEntsoeCategory.h>
#include <iidm/GeneratorEntsoeCategoryAdder.h>
#include <iidm/GeneratorFortescue.h>
#include <iidm/GeneratorFortescueAdder.h>
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

    double getTargetP() const { return getDouble(prop::GEN_TARGET_P); }
    Generator& setTargetP(double targetP) { return setDouble(*this, prop::GEN_TARGET_P, targetP); }

    double getMinP() const { return getDouble(prop::GEN_MIN_P); }
    Generator& setMinP(double minP) { return setDouble(*this, prop::GEN_MIN_P, minP); }

    double getMaxP() const { return getDouble(prop::GEN_MAX_P); }
    Generator& setMaxP(double maxP) { return setDouble(*this, prop::GEN_MAX_P, maxP); }

    double getRatedS() const { return getDouble(prop::GEN_RATED_S); }
    Generator& setRatedS(double ratedS) { return setDouble(*this, prop::GEN_RATED_S, ratedS); }

    bool isCondenser() const { return getBool(prop::GEN_CONDENSER); }

    EnergySource getEnergySource() const { return static_cast<EnergySource>(getInt(prop::GEN_ENERGY_SOURCE)); }
    Generator& setEnergySource(EnergySource energySource) {
        return setInt(*this, prop::GEN_ENERGY_SOURCE, static_cast<int>(energySource));
    }

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

    bool hasGeneratorFortescue() const;
    GeneratorFortescue getGeneratorFortescue() const;
    GeneratorFortescueAdder newGeneratorFortescue();
    void removeGeneratorFortescue();
};

} // namespace iidm
