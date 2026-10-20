#include <iidm/CoordinatedReactiveControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

CoordinatedReactiveControl::CoordinatedReactiveControl(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

double CoordinatedReactiveControl::getQPercent() const {
    return backend_->getDouble(handle_, prop::EXT_CRC_Q_PERCENT);
}

CoordinatedReactiveControl& CoordinatedReactiveControl::setQPercent(double qPercent) {
    backend_->setDouble(handle_, prop::EXT_CRC_Q_PERCENT, qPercent);
    return *this;
}

} // namespace iidm
