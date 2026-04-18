#include <iidm/VoltageLevel.h>
#include <iidm/SlackTerminal.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Switch.h>
#include <iidm/BusbarSection.h>

namespace iidm {

VoltageLevel::VoltageLevel(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string VoltageLevel::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string VoltageLevel::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

double VoltageLevel::getNominalV() const {
    return backend_->getDouble(handle_, prop::VL_NOMINAL_V);
}

std::optional<double> VoltageLevel::getLowVoltageLimit() const {
    return backend_->getOptDouble(handle_, prop::VL_LOW_VOLTAGE_LIMIT);
}

std::optional<double> VoltageLevel::getHighVoltageLimit() const {
    return backend_->getOptDouble(handle_, prop::VL_HIGH_VOLTAGE_LIMIT);
}

TopologyKind VoltageLevel::getTopologyKind() const {
    return static_cast<TopologyKind>(backend_->getInt(handle_, prop::VL_TOPOLOGY_KIND));
}

bool VoltageLevel::hasSlackTerminal() const {
    return backend_->getBool(handle_, prop::EXT_ST_EXISTS);
}

SlackTerminal VoltageLevel::getSlackTerminal() const {
    return SlackTerminal(handle_, backend_);
}

std::vector<Switch> VoltageLevel::getSwitches() const {
    auto handles = backend_->getChildren(handle_, prop::SWITCH);
    std::vector<Switch> result;
    result.reserve(handles.size());
    for (auto h : handles) {
        result.emplace_back(h, backend_);
    }
    return result;
}

std::vector<BusbarSection> VoltageLevel::getBusbarSections() const {
    auto handles = backend_->getChildren(handle_, prop::BUSBAR_SECTION);
    std::vector<BusbarSection> result;
    result.reserve(handles.size());
    for (auto h : handles) {
        result.emplace_back(h, backend_);
    }
    return result;
}

bool VoltageLevel::operator==(const VoltageLevel& other) const {
    return handle_ == other.handle_;
}

bool VoltageLevel::operator!=(const VoltageLevel& other) const {
    return !(*this == other);
}

} // namespace iidm
