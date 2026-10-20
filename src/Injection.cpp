#include <iidm/Injection.h>
#include <iidm/BackendProvider.h>
#include "Extensions.h"

namespace iidm {

Terminal Injection::getTerminal() const {
    return Terminal(backend_->getRelated(handle_, prop::REL_TERMINAL), backend_);
}

// ── Extensions ───────────────────────────────────────────────────────────────

bool Injection::hasActivePowerControl() const {
    return detail::hasExtension<ActivePowerControl>(backend_, handle_);
}

ActivePowerControl Injection::getActivePowerControl() const {
    return detail::getExtension<ActivePowerControl>(backend_, handle_, "ActivePowerControl");
}

ActivePowerControlAdder Injection::newActivePowerControl() {
    return detail::newExtension<ActivePowerControl, ActivePowerControlAdder>(backend_, handle_);
}

void Injection::removeActivePowerControl() {
    detail::removeExtension<ActivePowerControl>(backend_, handle_);
}

bool Injection::hasInjectionObservability() const {
    return detail::hasExtension<InjectionObservability>(backend_, handle_);
}

InjectionObservability Injection::getInjectionObservability() const {
    return detail::getExtension<InjectionObservability>(backend_, handle_, "InjectionObservability");
}

InjectionObservabilityAdder Injection::newInjectionObservability() {
    return detail::newExtension<InjectionObservability, InjectionObservabilityAdder>(backend_, handle_);
}

void Injection::removeInjectionObservability() {
    detail::removeExtension<InjectionObservability>(backend_, handle_);
}

} // namespace iidm
