#include <iidm/VoltageRegulation.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

VoltageRegulation::VoltageRegulation(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

double VoltageRegulation::getTargetValue() const {
    return backend_->getDouble(handle_, prop::VR_TARGET_VALUE);
}

VoltageRegulation& VoltageRegulation::setTargetValue(double targetValue) {
    backend_->setDouble(handle_, prop::VR_TARGET_VALUE, targetValue);
    return *this;
}

double VoltageRegulation::getTargetDeadband() const {
    return backend_->getDouble(handle_, prop::VR_TARGET_DEADBAND);
}

VoltageRegulation& VoltageRegulation::setTargetDeadband(double targetDeadband) {
    backend_->setDouble(handle_, prop::VR_TARGET_DEADBAND, targetDeadband);
    return *this;
}

double VoltageRegulation::getSlope() const {
    return backend_->getDouble(handle_, prop::VR_SLOPE);
}

VoltageRegulation& VoltageRegulation::setSlope(double slope) {
    backend_->setDouble(handle_, prop::VR_SLOPE, slope);
    return *this;
}

iidm::optional<RegulationMode> VoltageRegulation::getMode() const {
    int mode = backend_->getInt(handle_, prop::VR_MODE);
    if (mode < 0) return {};
    return static_cast<RegulationMode>(mode);
}

VoltageRegulation& VoltageRegulation::setMode(RegulationMode mode) {
    backend_->setInt(handle_, prop::VR_MODE, static_cast<int>(mode));
    return *this;
}

bool VoltageRegulation::isRegulating() const {
    return backend_->getBool(handle_, prop::VR_REGULATING);
}

VoltageRegulation& VoltageRegulation::setRegulating(bool regulating) {
    backend_->setBool(handle_, prop::VR_REGULATING, regulating);
    return *this;
}

bool VoltageRegulation::isWithTerminal() const {
    return backend_->getBool(handle_, prop::VR_WITH_TERMINAL);
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
