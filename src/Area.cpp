#include <iidm/Area.h>
#include <iidm/AreaAdder.h>
#include <iidm/BackendProvider.h>
#include <iidm/BoundaryLine.h>
#include <iidm/HandleRef.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Terminal.h>
#include <iidm/VoltageLevel.h>
#include "Wrap.h"

namespace iidm {

// ── AreaBoundary ──────────────────────────────────────────────────────────────

Area AreaBoundary::getArea() const {
    return Area(backend_->getRelated(handle_, prop::AREA), backend_);
}

iidm::optional<Terminal> AreaBoundary::getTerminal() const {
    return detail::wrapOptional<Terminal>(backend_->getRelated(handle_, prop::REL_TERMINAL), backend_);
}

iidm::optional<Boundary> AreaBoundary::getBoundary() const {
    return detail::wrapOptional<Boundary>(backend_->getRelated(handle_, prop::BOUNDARY), backend_);
}

// ── AreaBoundaryAdder ─────────────────────────────────────────────────────────

AreaBoundaryAdder& AreaBoundaryAdder::setTerminal(const Terminal& terminal) {
    return relatedField(prop::REL_TERMINAL, detail::Access::handle(terminal));
}

AreaBoundaryAdder& AreaBoundaryAdder::setBoundary(const Boundary& boundary) {
    return relatedField(prop::BOUNDARY, detail::Access::handle(boundary));
}

Area AreaBoundaryAdder::add() {
    return Area(create(), backend_);
}

// ── AreaAdder ─────────────────────────────────────────────────────────────────

AreaAdder& AreaAdder::addVoltageLevel(const VoltageLevel& voltageLevel) {
    return relatedField(prop::ADD_VOLTAGE_LEVEL, detail::Access::handle(voltageLevel));
}

AreaAdder& AreaAdder::addAreaBoundary(const Terminal& terminal, bool ac) {
    return addBoundary(prop::REL_TERMINAL, detail::Access::handle(terminal), ac);
}

AreaAdder& AreaAdder::addAreaBoundary(const Boundary& boundary, bool ac) {
    return addBoundary(prop::BOUNDARY, detail::Access::handle(boundary), ac);
}

AreaAdder& AreaAdder::addBoundary(int code, ObjectHandle at, bool ac) {
    // A nested adder: its two values, then added to this one
    HandleRef boundary(backend_->invoke(adder_, prop::AREA_BOUNDARY), backend_);
    backend_->setRelated(boundary, code, at);
    backend_->setBool(boundary, prop::AREA_BOUNDARY_AC, ac);
    backend_->invoke(boundary, prop::ACT_ADD);
    return *this;
}

// ── Area ──────────────────────────────────────────────────────────────────────

std::vector<VoltageLevel> Area::getVoltageLevels() const {
    return detail::wrapAll<VoltageLevel>(backend_->getChildren(handle_, prop::VOLTAGE_LEVEL), backend_);
}

Area& Area::addVoltageLevel(const VoltageLevel& voltageLevel) {
    backend_->setRelated(handle_, prop::ADD_VOLTAGE_LEVEL, detail::Access::handle(voltageLevel));
    return *this;
}

Area& Area::removeVoltageLevel(const VoltageLevel& voltageLevel) {
    backend_->setRelated(handle_, prop::REMOVE_VOLTAGE_LEVEL, detail::Access::handle(voltageLevel));
    return *this;
}

std::vector<AreaBoundary> Area::getAreaBoundaries() const {
    return detail::wrapAll<AreaBoundary>(backend_->getChildren(handle_, prop::AREA_BOUNDARY), backend_);
}

iidm::optional<AreaBoundary> Area::getAreaBoundary(const Terminal& terminal) const {
    for (const AreaBoundary& boundary : getAreaBoundaries()) {
        iidm::optional<Terminal> at = boundary.getTerminal();
        if (at && *at == terminal) return boundary;
    }
    return {};
}

iidm::optional<AreaBoundary> Area::getAreaBoundary(const Boundary& boundary) const {
    for (const AreaBoundary& areaBoundary : getAreaBoundaries()) {
        iidm::optional<Boundary> at = areaBoundary.getBoundary();
        if (at && *at == boundary) return areaBoundary;
    }
    return {};
}

AreaBoundaryAdder Area::newAreaBoundary() {
    return AreaBoundaryAdder(backend_->invoke(handle_, prop::AREA_BOUNDARY), backend_);
}

Area& Area::removeAreaBoundary(const Terminal& terminal) {
    backend_->setRelated(handle_, prop::REMOVE_AREA_BOUNDARY, detail::Access::handle(terminal));
    return *this;
}

Area& Area::removeAreaBoundary(const Boundary& boundary) {
    backend_->setRelated(handle_, prop::REMOVE_AREA_BOUNDARY, detail::Access::handle(boundary));
    return *this;
}

void Area::remove() {
    backend_->invoke(handle_, prop::ACT_REMOVE);
}

} // namespace iidm
