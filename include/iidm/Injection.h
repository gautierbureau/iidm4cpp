#pragma once

#include <iidm/ActivePowerControl.h>
#include <iidm/ActivePowerControlAdder.h>
#include <iidm/Connectable.h>
#include <iidm/InjectionObservability.h>
#include <iidm/InjectionObservabilityAdder.h>
#include <iidm/Terminal.h>

namespace iidm {

// Equipment with one terminal (powsybl Injection): generators, loads, batteries, shunts...
class Injection : public Connectable {
public:
    static constexpr int TYPE = prop::INJECTION;

    Injection() = default;
    explicit Injection(ObjectHandle handle, BackendProvider* backend) : Connectable(handle, backend) {}

    Terminal getTerminal() const;

    // ── Extensions ───────────────────────────────────────────────────────
    // An extension's getter throws ExtensionNotFoundException when it is absent
    bool hasActivePowerControl() const;
    ActivePowerControl getActivePowerControl() const;
    ActivePowerControlAdder newActivePowerControl();
    void removeActivePowerControl();

    bool hasInjectionObservability() const;
    InjectionObservability getInjectionObservability() const;
    InjectionObservabilityAdder newInjectionObservability();
    void removeInjectionObservability();
};

} // namespace iidm
