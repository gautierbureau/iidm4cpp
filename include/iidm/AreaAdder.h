#pragma once

#include <iidm/Adder.h>
#include <iidm/Area.h>
#include <string>

namespace iidm {

class Boundary;
class Terminal;
class VoltageLevel;

// New area of a network (Network::newArea())
class AreaAdder : public detail::IdentifiableAdder<AreaAdder> {
public:
    AreaAdder(ObjectHandle adder, BackendProvider* backend) : IdentifiableAdder(adder, backend) {}

    AreaAdder& setAreaType(const std::string& areaType)       { return stringField(prop::AREA_TYPE, areaType); }
    AreaAdder& setInterchangeTarget(double interchangeTarget) { return doubleField(prop::AREA_INTERCHANGE_TARGET, interchangeTarget); }
    AreaAdder& addVoltageLevel(const VoltageLevel& voltageLevel);
    // A boundary at a terminal, or at the boundary of a boundary line; AC or DC
    AreaAdder& addAreaBoundary(const Terminal& terminal, bool ac);
    AreaAdder& addAreaBoundary(const Boundary& boundary, bool ac);

    Area add() { return Area(create(), backend_); }

private:
    AreaAdder& addBoundary(int code, ObjectHandle at, bool ac);
};

} // namespace iidm
