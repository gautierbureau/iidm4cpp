#pragma once

#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

// A geographical position, in degrees (powsybl Coordinate): a value, as in the position extensions
struct Coordinate {
    double latitude;
    double longitude;

    Coordinate() : latitude(0), longitude(0) {}
    Coordinate(double latitude, double longitude) : latitude(latitude), longitude(longitude) {}

    bool operator==(const Coordinate& other) const { return latitude == other.latitude && longitude == other.longitude; }
    bool operator!=(const Coordinate& other) const { return !(*this == other); }
};

namespace detail {

// A coordinate object of a position extension, read; its handle is released
Coordinate readCoordinate(BackendProvider* backend, ObjectHandle coordinate);
// Adds a coordinate to the adder of a new position extension (after those added before)
void addCoordinate(BackendProvider* backend, ObjectHandle positionAdder, const Coordinate& coordinate);

} // namespace detail
} // namespace iidm
