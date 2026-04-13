#include <iidm/LccConverterStation.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

LccConverterStation::LccConverterStation(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string LccConverterStation::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string LccConverterStation::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

float LccConverterStation::getLossFactor() const {
    return static_cast<float>(backend_->getDouble(handle_, prop::LCC_LOSS_FACTOR));
}

float LccConverterStation::getPowerFactor() const {
    return static_cast<float>(backend_->getDouble(handle_, prop::LCC_POWER_FACTOR));
}

Terminal LccConverterStation::getTerminal() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL);
    return Terminal(termHandle, backend_);
}

void LccConverterStation::connect() {
    getTerminal().connect();
}

void LccConverterStation::disconnect() {
    getTerminal().disconnect();
}

bool LccConverterStation::isConnected() const {
    return getTerminal().isConnected();
}

bool LccConverterStation::operator==(const LccConverterStation& other) const {
    return handle_ == other.handle_;
}

bool LccConverterStation::operator!=(const LccConverterStation& other) const {
    return !(*this == other);
}

} // namespace iidm
