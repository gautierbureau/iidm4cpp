#include <iidm/VoltageRegulation.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

VoltageRegulation::VoltageRegulation(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

iidm::optional<RegulationMode> VoltageRegulation::getMode() const {
    int mode = backend_->getInt(handle_, prop::VR_MODE);
    if (mode < 0) return {};
    return static_cast<RegulationMode>(mode);
}

iidm::optional<Terminal> VoltageRegulation::getTerminal() const {
    ObjectHandle terminal = backend_->getRelated(handle_, prop::REL_TERMINAL);
    if (terminal == INVALID_HANDLE) return {};
    return Terminal(terminal, backend_);
}

VoltageRegulation& VoltageRegulation::setTerminal(const Terminal& terminal, double targetValue) {
    // One change applied at once: separate setters would go through an invalid state
    // (a remote regulation without target value, or a local one with a target value)
    HandleRef change(backend_->invoke(handle_, prop::REL_TERMINAL), backend_);
    backend_->setRelated(change, prop::REL_TERMINAL, detail::Access::handle(terminal));
    backend_->setDouble(change, prop::VR_TARGET_VALUE, targetValue);
    backend_->invoke(change, prop::ACT_ADD);
    return *this;
}

} // namespace iidm
