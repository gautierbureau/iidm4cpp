#include <iidm/CoordinatedReactiveControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

CoordinatedReactiveControl::CoordinatedReactiveControl(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

double CoordinatedReactiveControl::getQPercent() const {
    return backend_->getDouble(handle_, prop::EXT_CRC_Q_PERCENT);
}

CoordinatedReactiveControl& CoordinatedReactiveControl::setQPercent(double qPercent) {
    backend_->setDouble(handle_, prop::EXT_CRC_Q_PERCENT, qPercent);
    return *this;
}

bool CoordinatedReactiveControl::operator==(const CoordinatedReactiveControl& other) const {
    return handle_ == other.handle_;
}

bool CoordinatedReactiveControl::operator!=(const CoordinatedReactiveControl& other) const {
    return !(*this == other);
}

} // namespace iidm
