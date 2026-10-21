#pragma once

#include <iidm/Coordinate.h>
#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The SubstationPosition extension of a substation: where it is. An object of its own, from
// Substation::getExtension<SubstationPosition>().
class SubstationPosition : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::SUBSTATION_POSITION;

    SubstationPosition() = default;
    explicit SubstationPosition(ObjectHandle handle, BackendProvider* backend);

    Coordinate getCoordinate() const;
};

} // namespace iidm
