#pragma once

#include <iidm/Adder.h>
#include <iidm/Identifiable.h>
#include <iidm/ObjectRef.h>
#include <iidm/PropertiesHolder.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class Area;
class Boundary;
class Terminal;
class VoltageLevel;

// Where power leaves an area (powsybl AreaBoundary): a terminal, or the boundary of a boundary line
class AreaBoundary : public detail::ObjectRef, public detail::PropertiesHolder<AreaBoundary> {
public:
    static constexpr int TYPE = prop::AREA_BOUNDARY;

    AreaBoundary() = default;
    explicit AreaBoundary(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    Area getArea() const;
    // What it is: a terminal or the boundary of a boundary line (the other is empty)
    iidm::optional<Terminal> getTerminal() const;
    iidm::optional<Boundary> getBoundary() const;
    // In the AC interchange of the area, else in the DC one
    bool isAc() const;
    // The flows out of the area through it
    double getP() const;
    double getQ() const;
};

// New boundary of an area (Area::newAreaBoundary()): a terminal or a boundary; add() returns the area
class AreaBoundaryAdder : public detail::AdderBase<AreaBoundaryAdder> {
public:
    AreaBoundaryAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    AreaBoundaryAdder& setTerminal(const Terminal& terminal);
    AreaBoundaryAdder& setBoundary(const Boundary& boundary);
    AreaBoundaryAdder& setAc(bool ac) { return boolField(prop::AREA_BOUNDARY_AC, ac); }

    Area add();
};

// A group of voltage levels, of a type: a control area, a bidding zone... (powsybl Area)
class Area : public Identifiable {
public:
    static constexpr int TYPE = prop::AREA;

    Area() = default;
    explicit Area(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    std::string getAreaType() const;

    // Sorted by id (include <iidm/VoltageLevel.h> to use them)
    std::vector<VoltageLevel> getVoltageLevels() const;
    Area& addVoltageLevel(const VoltageLevel& voltageLevel);
    Area& removeVoltageLevel(const VoltageLevel& voltageLevel);

    std::vector<AreaBoundary> getAreaBoundaries() const;
    // The boundary at a terminal, or at the boundary of a boundary line (empty without one)
    iidm::optional<AreaBoundary> getAreaBoundary(const Terminal& terminal) const;
    iidm::optional<AreaBoundary> getAreaBoundary(const Boundary& boundary) const;
    AreaBoundaryAdder newAreaBoundary();
    Area& removeAreaBoundary(const Terminal& terminal);
    Area& removeAreaBoundary(const Boundary& boundary);

    // Empty when unset; NaN unsets it
    iidm::optional<double> getInterchangeTarget() const;
    Area& setInterchangeTarget(double interchangeTarget);
    // The power leaving it through its boundaries: the AC ones, the DC ones, all of them
    double getAcInterchange() const;
    double getDcInterchange() const;
    double getInterchange() const;

    void remove();
};

} // namespace iidm
