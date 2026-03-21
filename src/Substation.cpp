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

bool Substation::operator==(const Substation& other) const {
    return handle_ == other.handle_;
}

bool Substation::operator!=(const Substation& other) const {
    return !(*this == other);
}

} // namespace iidm
