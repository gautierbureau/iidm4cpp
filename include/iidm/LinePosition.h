#pragma once

#include <iidm/Coordinate.h>
#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>
#include <vector>

namespace iidm {

class BackendProvider;

// The LinePosition extension of a line or a boundary line: its path. An object of its own, from
// Line::getLinePosition() or BoundaryLine::getLinePosition().
class LinePosition : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::LINE_POSITION;

    LinePosition() = default;
    explicit LinePosition(ObjectHandle handle, BackendProvider* backend);

    // In order
    std::vector<Coordinate> getCoordinates() const;
};

} // namespace iidm
