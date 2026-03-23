#include <iidm/Bus.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

Bus::Bus(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string Bus::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string Bus::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

double Bus::getV() const {
    return backend_->getDouble(handle_, prop::BUS_V);
}

Bus& Bus::setV(double v) {
    backend_->setDouble(handle_, prop::BUS_V, v);
    return *this;
}

double Bus::getAngle() const {
    return backend_->getDouble(handle_, prop::BUS_ANGLE);
}

Bus& Bus::setAngle(double angle) {
    backend_->setDouble(handle_, prop::BUS_ANGLE, angle);
    return *this;
}

bool Bus::operator==(const Bus& other) const {
    return handle_ == other.handle_;
}

bool Bus::operator!=(const Bus& other) const {
    return !(*this == other);
}

} // namespace iidm
