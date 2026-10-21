#include <iidm/OperatingStatus.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

OperatingStatus::OperatingStatus(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

OperatingStatus::Status OperatingStatus::getStatus() const {
    return static_cast<OperatingStatus::Status>(backend_->getInt(handle_, prop::EXT_OS_STATUS));
}

OperatingStatus& OperatingStatus::setStatus(Status status) {
    backend_->setInt(handle_, prop::EXT_OS_STATUS, static_cast<int>(status));
    return *this;
}

} // namespace iidm
