#include <iidm/Substation.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

Substation::Substation(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string Substation::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string Substation::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

std::optional<Country> Substation::getCountry() const {
    auto optVal = backend_->getOptDouble(handle_, prop::COUNTRY);
    if (!optVal.has_value()) return std::nullopt;
    return static_cast<Country>(static_cast<int>(*optVal));
}

std::vector<VoltageLevel> Substation::getVoltageLevels() const {
    auto handles = backend_->getChildren(handle_, prop::VOLTAGE_LEVEL);
    std::vector<VoltageLevel> result;
    result.reserve(handles.size());
    for (auto h : handles) {
        result.emplace_back(h, backend_);
    }
    return result;
}

bool Substation::hasExtension(const std::string& name) const {
    return backend_->getExtensionHandle(handle_, name) != INVALID_HANDLE;
}

Extension Substation::getExtension(const std::string& name) const {
    return Extension(name, backend_->getExtensionHandle(handle_, name), backend_);
}

std::vector<Extension> Substation::getExtensions() const {
    std::vector<std::string> names = backend_->getExtensionNames(handle_);
    std::vector<Extension> result;
    result.reserve(names.size());
    for (const auto& n : names)
        result.emplace_back(n, backend_->getExtensionHandle(handle_, n), backend_);
    return result;
}

bool Substation::operator==(const Substation& other) const {
    return handle_ == other.handle_;
}

bool Substation::operator!=(const Substation& other) const {
    return !(*this == other);
}

} // namespace iidm
