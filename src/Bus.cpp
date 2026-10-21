#include <iidm/Bus.h>
#include <iidm/VoltageLevel.h>
#include <iidm/BackendProvider.h>
#include <iidm/Battery.h>
#include <iidm/BoundaryLine.h>
#include <iidm/Generator.h>
#include <iidm/IidmException.h>
#include <iidm/LccConverterStation.h>
#include <iidm/Line.h>
#include <iidm/Load.h>
#include <iidm/PropertyCodes.h>
#include <iidm/ShuntCompensator.h>
#include <iidm/StaticVarCompensator.h>
#include <iidm/Terminal.h>
#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/TwoWindingsTransformer.h>
#include <iidm/VscConverterStation.h>
#include "BoundaryLineFilter.h"
#include "Wrap.h"

namespace iidm {

void Bus::unsetSolvedValues() {
    backend_->invoke(handle_, prop::ACT_UNSET_SOLVED_VALUES);
}

iidm::optional<Component> Bus::getConnectedComponent() const {
    return detail::wrapOptional<Component>(backend_->getRelated(handle_, prop::REL_CONNECTED_COMPONENT), backend_);
}

iidm::optional<Component> Bus::getSynchronousComponent() const {
    return detail::wrapOptional<Component>(backend_->getRelated(handle_, prop::REL_SYNCHRONOUS_COMPONENT), backend_);
}

VoltageLevel Bus::getVoltageLevel() const {
    ObjectHandle vlHandle = backend_->getRelated(handle_, prop::REL_VOLTAGE_LEVEL);
    return VoltageLevel(vlHandle, backend_);
}

std::vector<Terminal> Bus::getConnectedTerminals() const {
    return detail::wrapAll<Terminal>(backend_->getChildren(handle_, prop::TERMINAL), backend_);
}

int Bus::getConnectedTerminalCount() const {
    return detail::countOf(backend_, handle_, prop::TERMINAL);
}

std::vector<Line> Bus::getLines() const {
    return detail::wrapAll<Line>(backend_->getChildren(handle_, prop::LINE), backend_);
}

std::vector<TwoWindingsTransformer> Bus::getTwoWindingsTransformers() const {
    return detail::wrapAll<TwoWindingsTransformer>(backend_->getChildren(handle_, prop::TWO_WINDINGS_TRANSFORMER), backend_);
}

std::vector<ThreeWindingsTransformer> Bus::getThreeWindingsTransformers() const {
    return detail::wrapAll<ThreeWindingsTransformer>(backend_->getChildren(handle_, prop::THREE_WINDINGS_TRANSFORMER), backend_);
}

std::vector<Generator> Bus::getGenerators() const {
    return detail::wrapAll<Generator>(backend_->getChildren(handle_, prop::GENERATOR), backend_);
}

std::vector<Battery> Bus::getBatteries() const {
    return detail::wrapAll<Battery>(backend_->getChildren(handle_, prop::BATTERY), backend_);
}

std::vector<Load> Bus::getLoads() const {
    return detail::wrapAll<Load>(backend_->getChildren(handle_, prop::LOAD), backend_);
}

std::vector<ShuntCompensator> Bus::getShuntCompensators() const {
    return detail::wrapAll<ShuntCompensator>(backend_->getChildren(handle_, prop::SHUNT_COMPENSATOR), backend_);
}

std::vector<BoundaryLine> Bus::getBoundaryLines(BoundaryLineFilter filter) const {
    return detail::filterBoundaryLines(
        detail::wrapAll<BoundaryLine>(backend_->getChildren(handle_, prop::BOUNDARY_LINE), backend_), filter);
}

std::vector<StaticVarCompensator> Bus::getStaticVarCompensators() const {
    return detail::wrapAll<StaticVarCompensator>(backend_->getChildren(handle_, prop::STATIC_VAR_COMPENSATOR), backend_);
}

std::vector<LccConverterStation> Bus::getLccConverterStations() const {
    return detail::wrapAll<LccConverterStation>(backend_->getChildren(handle_, prop::LCC_CONVERTER_STATION), backend_);
}

std::vector<VscConverterStation> Bus::getVscConverterStations() const {
    return detail::wrapAll<VscConverterStation>(backend_->getChildren(handle_, prop::VSC_CONVERTER_STATION), backend_);
}

} // namespace iidm
