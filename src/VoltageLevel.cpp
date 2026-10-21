#include <iidm/VoltageLevel.h>
#include <iidm/SlackTerminal.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>
#include <iidm/Bus.h>
#include <iidm/Switch.h>
#include <iidm/BusbarSection.h>
#include <iidm/InternalConnection.h>
#include <iidm/Terminal.h>
#include <iidm/Generator.h>
#include <iidm/Load.h>
#include <iidm/Battery.h>
#include <iidm/ShuntCompensator.h>
#include <iidm/StaticVarCompensator.h>
#include <iidm/BoundaryLine.h>
#include <iidm/LccConverterStation.h>
#include <iidm/VscConverterStation.h>
#include "Extensions.h"
#include "Wrap.h"

namespace iidm {

// ── NodeBreakerView ───────────────────────────────────────────────────────────

VoltageLevel::NodeBreakerView::NodeBreakerView(ObjectHandle h, BackendProvider* b)
    : ObjectRef(h, b) {}

std::vector<Switch> VoltageLevel::NodeBreakerView::getSwitches() const {
    auto handles = backend_->getChildren(handle_, prop::SWITCH);
    std::vector<Switch> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

std::vector<BusbarSection> VoltageLevel::NodeBreakerView::getBusbarSections() const {
    auto handles = backend_->getChildren(handle_, prop::BUSBAR_SECTION);
    std::vector<BusbarSection> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

std::vector<InternalConnection> VoltageLevel::NodeBreakerView::getInternalConnections() const {
    auto handles = backend_->getChildren(handle_, prop::INTERNAL_CONNECTION);
    std::vector<InternalConnection> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

std::vector<int> VoltageLevel::NodeBreakerView::getNodes() const {
    return backend_->getIntList(handle_, prop::VL_NBV_NODES);
}

iidm::optional<Switch> VoltageLevel::NodeBreakerView::getSwitch(const std::string& id) const {
    ObjectHandle h = backend_->findById(prop::SWITCH, id);
    if (h == INVALID_HANDLE) return {};
    return Switch(h, backend_);
}

Terminal VoltageLevel::NodeBreakerView::getTerminal(int node) const {
    ObjectHandle h = backend_->getRelatedByIndex(handle_, prop::REL_NBV_TERMINAL_AT_NODE, node);
    return Terminal(h, backend_);
}

// ── BusBreakerView ────────────────────────────────────────────────────────────

VoltageLevel::BusBreakerView::BusBreakerView(ObjectHandle h, BackendProvider* b)
    : ObjectRef(h, b) {}

std::vector<Bus> VoltageLevel::BusBreakerView::getBuses() const {
    auto handles = backend_->getChildren(handle_, prop::BUS);
    std::vector<Bus> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

std::vector<Switch> VoltageLevel::BusBreakerView::getSwitches() const {
    auto handles = backend_->getChildren(handle_, prop::SWITCH);
    std::vector<Switch> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

Bus VoltageLevel::BusBreakerView::getBus1(const std::string& switchId) const {
    return Bus(backend_->getRelatedByString(handle_, prop::REL_VL_BBV_BUS1, switchId), backend_);
}

Bus VoltageLevel::BusBreakerView::getBus2(const std::string& switchId) const {
    return Bus(backend_->getRelatedByString(handle_, prop::REL_VL_BBV_BUS2, switchId), backend_);
}

iidm::optional<Bus> VoltageLevel::BusBreakerView::getBus(const std::string& busId) const {
    return detail::wrapOptional<Bus>(backend_->getRelatedByString(handle_, prop::REL_BBV_BUS, busId), backend_);
}

BusAdder VoltageLevel::BusBreakerView::newBus() {
    return BusAdder(backend_->invoke(handle_, prop::BUS), backend_);
}

void VoltageLevel::BusBreakerView::removeBus(const std::string& busId) {
    iidm::optional<Bus> bus = getBus(busId);
    if (!bus) throw ValidationException("Bus " + busId + " not found");
    backend_->invoke(detail::Access::handle(*bus), prop::ACT_REMOVE);
}

// ── VoltageLevel ──────────────────────────────────────────────────────────────

double VoltageLevel::getNominalV() const {
    return backend_->getDouble(handle_, prop::VL_NOMINAL_V);
}

iidm::optional<double> VoltageLevel::getLowVoltageLimit() const {
    return backend_->getOptDouble(handle_, prop::VL_LOW_VOLTAGE_LIMIT);
}

iidm::optional<double> VoltageLevel::getHighVoltageLimit() const {
    return backend_->getOptDouble(handle_, prop::VL_HIGH_VOLTAGE_LIMIT);
}

TopologyKind VoltageLevel::getTopologyKind() const {
    return static_cast<TopologyKind>(backend_->getInt(handle_, prop::VL_TOPOLOGY_KIND));
}

std::vector<Connectable> VoltageLevel::getConnectables() const {
    return detail::wrapAll<Connectable>(backend_->getChildren(handle_, prop::CONNECTABLE), backend_);
}

std::vector<Switch> VoltageLevel::getSwitches() const {
    auto handles = backend_->getChildren(handle_, prop::SWITCH);
    std::vector<Switch> result;
    result.reserve(handles.size());
    for (auto h : handles) {
        result.emplace_back(h, backend_);
    }
    return result;
}

std::vector<BusbarSection> VoltageLevel::getBusbarSections() const {
    auto handles = backend_->getChildren(handle_, prop::BUSBAR_SECTION);
    std::vector<BusbarSection> result;
    result.reserve(handles.size());
    for (auto h : handles) {
        result.emplace_back(h, backend_);
    }
    return result;
}

std::vector<Generator> VoltageLevel::getGenerators() const {
    auto handles = backend_->getChildren(handle_, prop::GENERATOR);
    std::vector<Generator> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

std::vector<Load> VoltageLevel::getLoads() const {
    auto handles = backend_->getChildren(handle_, prop::LOAD);
    std::vector<Load> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

std::vector<Battery> VoltageLevel::getBatteries() const {
    auto handles = backend_->getChildren(handle_, prop::BATTERY);
    std::vector<Battery> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

std::vector<ShuntCompensator> VoltageLevel::getShuntCompensators() const {
    auto handles = backend_->getChildren(handle_, prop::SHUNT_COMPENSATOR);
    std::vector<ShuntCompensator> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

std::vector<StaticVarCompensator> VoltageLevel::getStaticVarCompensators() const {
    auto handles = backend_->getChildren(handle_, prop::STATIC_VAR_COMPENSATOR);
    std::vector<StaticVarCompensator> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

std::vector<BoundaryLine> VoltageLevel::getBoundaryLines() const {
    auto handles = backend_->getChildren(handle_, prop::BOUNDARY_LINE);
    std::vector<BoundaryLine> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

std::vector<LccConverterStation> VoltageLevel::getLccConverterStations() const {
    auto handles = backend_->getChildren(handle_, prop::LCC_CONVERTER_STATION);
    std::vector<LccConverterStation> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

std::vector<VscConverterStation> VoltageLevel::getVscConverterStations() const {
    auto handles = backend_->getChildren(handle_, prop::VSC_CONVERTER_STATION);
    std::vector<VscConverterStation> result;
    result.reserve(handles.size());
    for (auto h : handles) result.emplace_back(h, backend_);
    return result;
}

VoltageLevel::NodeBreakerView VoltageLevel::getNodeBreakerView() const {
    return NodeBreakerView(handle_, backend_);
}

GeneratorAdder VoltageLevel::newGenerator() {
    return GeneratorAdder(backend_->invoke(handle_, prop::GENERATOR), backend_);
}

LoadAdder VoltageLevel::newLoad() {
    return LoadAdder(backend_->invoke(handle_, prop::LOAD), backend_);
}

void VoltageLevel::remove() {
    backend_->invoke(handle_, prop::ACT_REMOVE);
}

VoltageLevel::BusBreakerView VoltageLevel::getBusBreakerView() const {
    return BusBreakerView(handle_, backend_);
}

// ── Extensions ───────────────────────────────────────────────────────────────

bool VoltageLevel::hasSlackTerminal() const {
    return detail::hasExtension<SlackTerminal>(backend_, handle_);
}

SlackTerminal VoltageLevel::getSlackTerminal() const {
    return detail::getExtension<SlackTerminal>(backend_, handle_, "SlackTerminal");
}

SlackTerminalAdder VoltageLevel::newSlackTerminal() {
    return detail::newExtension<SlackTerminal, SlackTerminalAdder>(backend_, handle_);
}

void VoltageLevel::removeSlackTerminal() {
    detail::removeExtension<SlackTerminal>(backend_, handle_);
}

bool VoltageLevel::hasObservabilityArea() const {
    return detail::hasExtension<ObservabilityArea>(backend_, handle_);
}

ObservabilityArea VoltageLevel::getObservabilityArea() const {
    return detail::getExtension<ObservabilityArea>(backend_, handle_, "ObservabilityArea");
}

void VoltageLevel::removeObservabilityArea() {
    detail::removeExtension<ObservabilityArea>(backend_, handle_);
}

} // namespace iidm
