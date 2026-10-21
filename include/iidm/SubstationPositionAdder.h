#pragma once

#include <iidm/Adder.h>
#include <iidm/Coordinate.h>
#include <iidm/SubstationPosition.h>

namespace iidm {

// New SubstationPosition extension of a substation (newSubstationPosition())
class SubstationPositionAdder : public detail::AdderBase<SubstationPositionAdder> {
public:
    SubstationPositionAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    SubstationPositionAdder& withCoordinate(const Coordinate& coordinate) {
        detail::addCoordinate(backend_, adder_, coordinate);
        return *this;
    }

    SubstationPosition add() { return SubstationPosition(create(), backend_); }
};

} // namespace iidm
