#include <iidm/RatioTapChanger.h>
#include <iidm/BackendProvider.h>
#include <cmath>

namespace iidm {

RatioTapChanger::RatioTapChanger(ObjectHandle handle, BackendProvider* backend,
                                   const RatioTapChangerConfig& cfg)
    : handle_(handle), backend_(backend), cfg_(cfg) {}

int RatioTapChanger::getTapPosition() const {
    return backend_->getInt(handle_, cfg_.tapPosition);
}
RatioTapChanger& RatioTapChanger::setTapPosition(int pos) {
    backend_->setInt(handle_, cfg_.tapPosition, pos);
    return *this;
}
int RatioTapChanger::getLowTapPosition()  const { return backend_->getInt(handle_, cfg_.lowTap);  }
int RatioTapChanger::getHighTapPosition() const { return backend_->getInt(handle_, cfg_.highTap); }

bool RatioTapChanger::isRegulating() const {
    return backend_->getBool(handle_, cfg_.regulating);
}
RatioTapChanger& RatioTapChanger::setRegulating(bool reg) {
    backend_->setBool(handle_, cfg_.regulating, reg);
    return *this;
}
double RatioTapChanger::getTargetV() const {
    return backend_->getDouble(handle_, cfg_.targetV);
}
RatioTapChanger& RatioTapChanger::setTargetV(double v) {
    backend_->setDouble(handle_, cfg_.targetV, v);
    return *this;
}

std::optional<double> RatioTapChanger::getTargetDeadband() const {
    double v = backend_->getDouble(handle_, cfg_.targetDeadband);
    if (std::isnan(v)) return std::nullopt;
    return v;
}

std::string RatioTapChanger::getRegulationTerminalId() const {
    return backend_->getString(handle_, cfg_.regulationTerminalId);
}

std::vector<RatioTapChangerStep> RatioTapChanger::getAllSteps() const {
    auto handles = backend_->getChildren(handle_, cfg_.stepChildType);
    std::vector<RatioTapChangerStep> steps;
    steps.reserve(handles.size());
    for (auto h : handles) steps.emplace_back(h, backend_);
    return steps;
}

RatioTapChangerStep RatioTapChanger::getCurrentStep() const {
    auto handles = backend_->getChildren(handle_, cfg_.stepChildType);
    int idx = getTapPosition() - getLowTapPosition();
    if (idx >= 0 && static_cast<std::size_t>(idx) < handles.size())
        return RatioTapChangerStep(handles[idx], backend_);
    return RatioTapChangerStep();
}

bool RatioTapChanger::operator==(const RatioTapChanger& other) const {
    return handle_ == other.handle_ && cfg_.exists == other.cfg_.exists;
}
bool RatioTapChanger::operator!=(const RatioTapChanger& other) const {
    return !(*this == other);
}

} // namespace iidm
