#include <iidm/Coordinate.h>
#include <iidm/BackendProvider.h>
#include <iidm/HandleRef.h>
#include <iidm/PropertyCodes.h>

namespace iidm { namespace detail {

Coordinate readCoordinate(BackendProvider* backend, ObjectHandle coordinate) {
    HandleRef owned(coordinate, backend);
    return Coordinate(backend->getDouble(coordinate, prop::COORD_LATITUDE), backend->getDouble(coordinate, prop::COORD_LONGITUDE));
}

void addCoordinate(BackendProvider* backend, ObjectHandle positionAdder, const Coordinate& coordinate) {
    HandleRef added(backend->invoke(positionAdder, prop::COORDINATE), backend);
    backend->setDouble(added, prop::COORD_LATITUDE, coordinate.latitude);
    backend->setDouble(added, prop::COORD_LONGITUDE, coordinate.longitude);
    backend->invoke(added, prop::ACT_ADD);
}

} } // namespace iidm::detail
