#include <iidm/LinePosition.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

LinePosition::LinePosition(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

std::vector<Coordinate> LinePosition::getCoordinates() const {
    std::vector<Coordinate> coordinates;
    for (ObjectHandle coordinate : backend_->getChildren(handle_, prop::COORDINATE)) {
        coordinates.push_back(detail::readCoordinate(backend_, coordinate));
    }
    return coordinates;
}

} // namespace iidm
