#include <iidm/Injection.h>
#include <iidm/BackendProvider.h>

namespace iidm {

Terminal Injection::getTerminal() const {
    return Terminal(backend_->getRelated(handle_, prop::REL_TERMINAL), backend_);
}

// ── Extensions ───────────────────────────────────────────────────────────────

bool Injection::hasActivePowerControl() const {
    HandleRef extension(backend_->getRelated(handle_, prop::ACTIVE_POWER_CONTROL), backend_);
    return extension != INVALID_HANDLE;
}

ActivePowerControl Injection::getActivePowerControl() const {
    ActivePowerControl extension(backend_->getRelated(handle_, prop::ACTIVE_POWER_CONTROL), backend_);
    if (!extension.isValid()) {
        throw ExtensionNotFoundException("ActivePowerControl extension not present on " + getId());
    }
    return extension;
}

ActivePowerControlAdder Injection::newActivePowerControl() {
    return ActivePowerControlAdder(backend_->invoke(handle_, prop::ACTIVE_POWER_CONTROL), backend_);
}

void Injection::removeActivePowerControl() {
    backend_->setRelated(handle_, prop::ACTIVE_POWER_CONTROL, INVALID_HANDLE);
}

bool Injection::hasInjectionObservability() const {
    return backend_->getBool(handle_, prop::EXT_INJ_OBS_EXISTS);
}

InjectionObservability Injection::getInjectionObservability() const {
    return InjectionObservability(handle_, backend_);
}

} // namespace iidm
