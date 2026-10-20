#pragma once

#include <iidm/CoordinatedReactiveControl.h>
#include <iidm/Enums.h>
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

    // ── Extensions ───────────────────────────────────────────────────────
    bool hasCoordinatedReactiveControl() const;
    CoordinatedReactiveControl getCoordinatedReactiveControl() const;
};

} // namespace iidm
