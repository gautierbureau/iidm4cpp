#include <iidm/PhaseTapChanger.h>
#include <iidm/BackendProvider.h>
#include "Wrap.h"

namespace iidm {

PhaseTapChanger& PhaseTapChanger::setRegulating(bool regulating) {
    backend_->setBool(handle_, prop::REGULATING, regulating);
    return *this;
}

PhaseTapChangerRegulationMode PhaseTapChanger::getRegulationMode() const {
    return static_cast<PhaseTapChangerRegulationMode>(backend_->getInt(handle_, prop::PTC_REGULATION_MODE));
}

PhaseTapChanger& PhaseTapChanger::setRegulationMode(PhaseTapChangerRegulationMode regulationMode) {
    backend_->setInt(handle_, prop::PTC_REGULATION_MODE, static_cast<int>(regulationMode));
    return *this;
}

double PhaseTapChanger::getRegulationValue() const {
    return backend_->getDouble(handle_, prop::PTC_REGULATION_VALUE);
}

PhaseTapChanger& PhaseTapChanger::setRegulationValue(double regulationValue) {
    backend_->setDouble(handle_, prop::PTC_REGULATION_VALUE, regulationValue);
    return *this;
}

double PhaseTapChanger::getTargetDeadband() const {
    return backend_->getDouble(handle_, prop::PTC_TARGET_DEADBAND);
}

PhaseTapChanger& PhaseTapChanger::setTargetDeadband(double targetDeadband) {
    backend_->setDouble(handle_, prop::PTC_TARGET_DEADBAND, targetDeadband);
    return *this;
}

iidm::optional<Terminal> PhaseTapChanger::getRegulationTerminal() const {
    return detail::wrapOptional<Terminal>(backend_->getRelated(handle_, prop::REL_REGULATION_TERMINAL), backend_);
}

PhaseTapChanger& PhaseTapChanger::setRegulationTerminal(const Terminal& regulationTerminal) {
    backend_->setRelated(handle_, prop::REL_REGULATION_TERMINAL, detail::Access::handle(regulationTerminal));
    return *this;
}

} // namespace iidm
