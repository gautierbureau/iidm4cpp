#include <iidm/HvdcLine.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

HvdcLine::HvdcLine(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string HvdcLine::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string HvdcLine::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

double HvdcLine::getR() const {
    return backend_->getDouble(handle_, prop::HVDC_R);
}

double HvdcLine::getNominalV() const {
    return backend_->getDouble(handle_, prop::HVDC_NOMINAL_V);
}

double HvdcLine::getActivePowerSetpoint() const {
    return backend_->getDouble(handle_, prop::HVDC_ACTIVE_POWER_SETPOINT);
}

HvdcLine& HvdcLine::setActivePowerSetpoint(double value) {
    backend_->setDouble(handle_, prop::HVDC_ACTIVE_POWER_SETPOINT, value);
    return *this;
}

double HvdcLine::getMaxP() const {
    return backend_->getDouble(handle_, prop::HVDC_MAX_P);
}

HvdcConverterStationMode HvdcLine::getConvertersMode() const {
    return static_cast<HvdcConverterStationMode>(backend_->getInt(handle_, prop::HVDC_CONVERTERS_MODE));
}

HvdcLine& HvdcLine::setConvertersMode(HvdcConverterStationMode mode) {
    backend_->setInt(handle_, prop::HVDC_CONVERTERS_MODE, static_cast<int>(mode));
    return *this;
}

bool HvdcLine::hasExtension(const std::string& name) const {
    return backend_->getExtensionHandle(handle_, name) != INVALID_HANDLE;
}

Extension HvdcLine::getExtension(const std::string& name) const {
    return Extension(name, backend_->getExtensionHandle(handle_, name), backend_);
}

std::vector<Extension> HvdcLine::getExtensions() const {
    std::vector<std::string> names = backend_->getExtensionNames(handle_);
    std::vector<Extension> result;
    result.reserve(names.size());
    for (const auto& n : names)
        result.emplace_back(n, backend_->getExtensionHandle(handle_, n), backend_);
    return result;
}

bool HvdcLine::operator==(const HvdcLine& other) const {
    return handle_ == other.handle_;
}

bool HvdcLine::operator!=(const HvdcLine& other) const {
    return !(*this == other);
}

} // namespace iidm
