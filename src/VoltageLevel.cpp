#include <iidm/VoltageLevel.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

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

bool VoltageLevel::hasExtension(const std::string& name) const {
    return backend_->getExtensionHandle(handle_, name) != INVALID_HANDLE;
}

Extension VoltageLevel::getExtension(const std::string& name) const {
    return Extension(name, backend_->getExtensionHandle(handle_, name), backend_);
}

std::vector<Extension> VoltageLevel::getExtensions() const {
    std::vector<std::string> names = backend_->getExtensionNames(handle_);
    std::vector<Extension> result;
    result.reserve(names.size());
    for (const auto& n : names)
        result.emplace_back(n, backend_->getExtensionHandle(handle_, n), backend_);
    return result;
}

bool VoltageLevel::operator==(const VoltageLevel& other) const {
    return handle_ == other.handle_;
}

bool VoltageLevel::operator!=(const VoltageLevel& other) const {
    return !(*this == other);
}

} // namespace iidm
