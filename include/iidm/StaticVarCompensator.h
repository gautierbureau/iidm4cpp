#pragma once

#include <iidm/Injection.h>
#include <iidm/StandbyAutomaton.h>
#include <iidm/StandbyAutomatonAdder.h>
#include <iidm/VoltageRegulationHolder.h>

namespace iidm {

// Its voltage regulation (and the former VoltagePerReactivePowerControl slope) is a
// VoltageRegulation object: see VoltageRegulationHolderMethods
class StaticVarCompensator : public Injection, public detail::VoltageRegulationHolderMethods<StaticVarCompensator> {
public:
    static constexpr int TYPE = prop::STATIC_VAR_COMPENSATOR;

    StaticVarCompensator() = default;
    explicit StaticVarCompensator(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    double getBmin() const;
    StaticVarCompensator& setBmin(double bMin);

    double getBmax() const;
    StaticVarCompensator& setBmax(double bMax);

    // ── Extensions ───────────────────────────────────────────────────────
    // An extension's getter throws ExtensionNotFoundException when it is absent
    bool hasStandbyAutomaton() const;
    StandbyAutomaton getStandbyAutomaton() const;
    StandbyAutomatonAdder newStandbyAutomaton();
    void removeStandbyAutomaton();
};

} // namespace iidm
