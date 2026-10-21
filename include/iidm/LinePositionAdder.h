#pragma once

#include <iidm/Adder.h>
#include <iidm/Coordinate.h>
#include <iidm/LinePosition.h>
#include <vector>

namespace iidm {

// New LinePosition extension of a line or a boundary line (newLinePosition())
class LinePositionAdder : public detail::AdderBase<LinePositionAdder> {
public:
    LinePositionAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    // In order
    LinePositionAdder& withCoordinates(const std::vector<Coordinate>& coordinates) {
        for (const Coordinate& coordinate : coordinates) detail::addCoordinate(backend_, adder_, coordinate);
        return *this;
    }

    LinePosition add() { return LinePosition(create(), backend_); }
};

} // namespace iidm
