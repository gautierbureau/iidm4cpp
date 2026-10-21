#include <iidm/ObservabilityQuality.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

ObservabilityQuality::ObservabilityQuality(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

iidm::optional<bool> ObservabilityQuality::isRedundant() const {
    const int redundant = backend_->getInt(handle_, prop::OBS_QUALITY_REDUNDANT);
    if (redundant < 0) return {};
    return redundant != 0;
}

ObservabilityQuality& ObservabilityQuality::setRedundant(iidm::optional<bool> redundant) {
    backend_->setInt(handle_, prop::OBS_QUALITY_REDUNDANT, redundant ? (*redundant ? 1 : 0) : -1);
    return *this;
}

} // namespace iidm
