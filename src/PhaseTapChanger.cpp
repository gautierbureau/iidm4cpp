#include <iidm/PhaseTapChanger.h>
#include <iidm/BackendProvider.h>

namespace iidm {

PhaseTapChanger::PhaseTapChanger(ObjectHandle handle, BackendProvider* backend,
                                   const PhaseTapChangerConfig& cfg)
    : handle_(handle), backend_(backend), cfg_(cfg) {}

int PhaseTapChanger::getTapPosition() const {
    return backend_->getInt(handle_, cfg_.tapPosition);
}
PhaseTapChanger& PhaseTapChanger::setTapPosition(int pos) {
    backend_->setInt(handle_, cfg_.tapPosition, pos);
    return *this;
}
int PhaseTapChanger::getLowTapPosition()  const { return backend_->getInt(handle_, cfg_.lowTap);  }
int PhaseTapChanger::getHighTapPosition() const { return backend_->getInt(handle_, cfg_.highTap); }

bool PhaseTapChanger::isRegulating() const {
    return backend_->getBool(handle_, cfg_.regulating);
}
PhaseTapChanger& PhaseTapChanger::setRegulating(bool reg) {
    backend_->setBool(handle_, cfg_.regulating, reg);
    return *this;
}
PhaseTapChangerRegulationMode PhaseTapChanger::getRegulationMode() const {
    return static_cast<PhaseTapChangerRegulationMode>(
        backend_->getInt(handle_, cfg_.regulationMode));
}
PhaseTapChanger& PhaseTapChanger::setRegulationMode(PhaseTapChangerRegulationMode mode) {
    backend_->setInt(handle_, cfg_.regulationMode, static_cast<int>(mode));
    return *this;
}
double PhaseTapChanger::getRegulationValue() const {
    return backend_->getDouble(handle_, cfg_.regulationValue);
}
PhaseTapChanger& PhaseTapChanger::setRegulationValue(double val) {
    backend_->setDouble(handle_, cfg_.regulationValue, val);
    return *this;
}

std::vector<PhaseTapChangerStep> PhaseTapChanger::getAllSteps() const {
    auto handles = backend_->getChildren(handle_, cfg_.stepChildType);
    std::vector<PhaseTapChangerStep> steps;
    steps.reserve(handles.size());
    for (auto h : handles) steps.emplace_back(h, backend_);
    return steps;
}

PhaseTapChangerStep PhaseTapChanger::getCurrentStep() const {
    auto handles = backend_->getChildren(handle_, cfg_.stepChildType);
    int idx = getTapPosition() - getLowTapPosition();
    if (idx >= 0 && static_cast<std::size_t>(idx) < handles.size())
        return PhaseTapChangerStep(handles[idx], backend_);
    return PhaseTapChangerStep();
}

bool PhaseTapChanger::operator==(const PhaseTapChanger& other) const {
    return handle_ == other.handle_ && cfg_.exists == other.cfg_.exists;
}
bool PhaseTapChanger::operator!=(const PhaseTapChanger& other) const {
    return !(*this == other);
}

} // namespace iidm
