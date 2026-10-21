#include <iidm/ManualFrequencyRestorationReserve.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

ManualFrequencyRestorationReserve::ManualFrequencyRestorationReserve(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

bool ManualFrequencyRestorationReserve::isParticipate() const {
    return backend_->getBool(handle_, prop::EXT_MFRR_PARTICIPATE);
}

ManualFrequencyRestorationReserve& ManualFrequencyRestorationReserve::setParticipate(bool participate) {
    backend_->setBool(handle_, prop::EXT_MFRR_PARTICIPATE, participate);
    return *this;
}

} // namespace iidm
