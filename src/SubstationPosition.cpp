#include <iidm/SubstationPosition.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

SubstationPosition::SubstationPosition(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

Coordinate SubstationPosition::getCoordinate() const {
    return detail::readCoordinate(backend_, backend_->getRelated(handle_, prop::COORDINATE));
}

} // namespace iidm
