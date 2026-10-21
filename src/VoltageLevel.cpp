#include <iidm/VoltageLevel.h>
#include <iidm/SlackTerminal.h>
#include <iidm/BackendProvider.h>
#include <iidm/HandleRef.h>
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
#include <iidm/Line.h>
#include <iidm/TwoWindingsTransformer.h>
#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/Substation.h>
#include <iidm/Area.h>
#include <iostream>
#include "BoundaryLineFilter.h"
#include "Extensions.h"
#include "Wrap.h"

namespace iidm {

// ── NodeBreakerView ───────────────────────────────────────────────────────────

VoltageLevel::NodeBreakerView::NodeBreakerView(ObjectHandle h, BackendProvider* b)
    : ObjectRef(h, b) {}

std::vector<int> VoltageLevel::NodeBreakerView::getNodes() const {
    return backend_->getIntList(handle_, prop::VL_NBV_NODES);
}

iidm::optional<Terminal> VoltageLevel::NodeBreakerView::getTerminal(int node) const {
    return detail::wrapOptional<Terminal>(backend_->getRelatedByIndex(handle_, prop::REL_NBV_TERMINAL_AT_NODE, node), backend_);
}

bool VoltageLevel::NodeBreakerView::hasAttachedEquipment(int node) const {
    return backend_->evaluate(handle_, prop::VL_NBV_ATTACHED_EQUIPMENT, {static_cast<double>(node)}) != 0;
}

double VoltageLevel::NodeBreakerView::getFictitiousP0(int node) const {
    return backend_->evaluate(handle_, prop::FICTITIOUS_P0, {static_cast<double>(node)});
}

VoltageLevel::NodeBreakerView& VoltageLevel::NodeBreakerView::setFictitiousP0(int node, double p0) {
    // Two values at once: a change, applied by ACT_ADD
    HandleRef change(backend_->invoke(handle_, prop::FICTITIOUS_P0), backend_);
    backend_->setInt(change, prop::TERMINAL_NODE, node);
    backend_->setDouble(change, prop::FICTITIOUS_P0, p0);
    backend_->invoke(change, prop::ACT_ADD);
    return *this;
}

double VoltageLevel::NodeBreakerView::getFictitiousQ0(int node) const {
    return backend_->evaluate(handle_, prop::FICTITIOUS_Q0, {static_cast<double>(node)});
}

VoltageLevel::NodeBreakerView& VoltageLevel::NodeBreakerView::setFictitiousQ0(int node, double q0) {
    HandleRef change(backend_->invoke(handle_, prop::FICTITIOUS_Q0), backend_);
    backend_->setInt(change, prop::TERMINAL_NODE, node);
    backend_->setDouble(change, prop::FICTITIOUS_Q0, q0);
    backend_->invoke(change, prop::ACT_ADD);
    return *this;
}

std::vector<Switch> VoltageLevel::NodeBreakerView::getSwitches() const {
    return detail::wrapAll<Switch>(backend_->getChildren(handle_, prop::REL_NBV_SWITCH), backend_);
}

int VoltageLevel::NodeBreakerView::getSwitchCount() const {
    return detail::countOf(backend_, handle_, prop::REL_NBV_SWITCH);
}

iidm::optional<Switch> VoltageLevel::NodeBreakerView::getSwitch(const std::string& id) const {
    return detail::wrapOptional<Switch>(backend_->getRelatedByString(handle_, prop::REL_NBV_SWITCH, id), backend_);
}

std::vector<Switch> VoltageLevel::NodeBreakerView::getSwitches(int node) const {
    return detail::elementsOf<Switch>(backend_->getRelatedByIndex(handle_, prop::REL_NBV_SWITCH, node), backend_);
}

namespace {

// A switch of a node-breaker view, which must have it (powsybl's message otherwise)
Switch existingSwitch(const VoltageLevel::NodeBreakerView& view, const std::string& switchId) {
    iidm::optional<Switch> sw = view.getSwitch(switchId);
    if (!sw) throw ValidationException("Switch " + switchId + " not found");
    return *sw;
}

} // namespace

int VoltageLevel::NodeBreakerView::getNode1(const std::string& switchId) const {
    return existingSwitch(*this, switchId).getNode1();
}

int VoltageLevel::NodeBreakerView::getNode2(const std::string& switchId) const {
    return existingSwitch(*this, switchId).getNode2();
}

iidm::optional<Terminal> VoltageLevel::NodeBreakerView::getTerminal1(const std::string& switchId) const {
    return getTerminal(getNode1(switchId));
}

iidm::optional<Terminal> VoltageLevel::NodeBreakerView::getTerminal2(const std::string& switchId) const {
    return getTerminal(getNode2(switchId));
}

void VoltageLevel::NodeBreakerView::removeSwitch(const std::string& switchId) {
    backend_->setString(handle_, prop::VL_NBV_REMOVE_SWITCH, switchId);
}

std::vector<BusbarSection> VoltageLevel::NodeBreakerView::getBusbarSections() const {
    return detail::wrapAll<BusbarSection>(backend_->getChildren(handle_, prop::BUSBAR_SECTION), backend_);
}

int VoltageLevel::NodeBreakerView::getBusbarSectionCount() const {
    return detail::countOf(backend_, handle_, prop::BUSBAR_SECTION);
}

iidm::optional<BusbarSection> VoltageLevel::NodeBreakerView::getBusbarSection(const std::string& id) const {
    return detail::wrapOptional<BusbarSection>(backend_->getRelatedByString(handle_, prop::BUSBAR_SECTION, id), backend_);
}

BusbarSectionAdder VoltageLevel::NodeBreakerView::newBusbarSection() {
    return BusbarSectionAdder(backend_->invoke(handle_, prop::BUSBAR_SECTION), backend_);
}

NodeBreakerSwitchAdder VoltageLevel::NodeBreakerView::newSwitch() {
    return NodeBreakerSwitchAdder(backend_->invoke(handle_, prop::REL_NBV_SWITCH), backend_);
}

NodeBreakerSwitchAdder VoltageLevel::NodeBreakerView::newBreaker() {
    NodeBreakerSwitchAdder adder = newSwitch();
    adder.setKind(SwitchKind::BREAKER);
    return adder;
}

NodeBreakerSwitchAdder VoltageLevel::NodeBreakerView::newDisconnector() {
    NodeBreakerSwitchAdder adder = newSwitch();
    adder.setKind(SwitchKind::DISCONNECTOR);
    return adder;
}

InternalConnectionAdder VoltageLevel::NodeBreakerView::newInternalConnection() {
    return InternalConnectionAdder(backend_->invoke(handle_, prop::INTERNAL_CONNECTION), backend_);
}

std::vector<InternalConnection> VoltageLevel::NodeBreakerView::getInternalConnections() const {
    return detail::wrapAll<InternalConnection>(backend_->getChildren(handle_, prop::INTERNAL_CONNECTION), backend_);
}

int VoltageLevel::NodeBreakerView::getInternalConnectionCount() const {
    return detail::countOf(backend_, handle_, prop::INTERNAL_CONNECTION);
}

std::vector<int> VoltageLevel::NodeBreakerView::getNodesInternalConnectedTo(int node) const {
    return detail::intElementsOf(backend_->getRelatedByIndex(handle_, prop::INTERNAL_CONNECTION, node), backend_);
}

void VoltageLevel::NodeBreakerView::removeInternalConnections(int node1, int node2) {
    HandleRef removal(backend_->invoke(handle_, prop::VL_NBV_REMOVE_INTERNAL_CONNECTIONS), backend_);
    backend_->setInt(removal, prop::IC_NODE1, node1);
    backend_->setInt(removal, prop::IC_NODE2, node2);
    backend_->invoke(removal, prop::ACT_ADD);
}

// ── BusBreakerView ────────────────────────────────────────────────────────────

VoltageLevel::BusBreakerView::BusBreakerView(ObjectHandle h, BackendProvider* b)
    : ObjectRef(h, b) {}

std::vector<Bus> VoltageLevel::BusBreakerView::getBuses() const {
    return detail::wrapAll<Bus>(backend_->getChildren(handle_, prop::BUS), backend_);
}

int VoltageLevel::BusBreakerView::getBusCount() const {
    return detail::countOf(backend_, handle_, prop::BUS);
}

iidm::optional<Bus> VoltageLevel::BusBreakerView::getBus(const std::string& busId) const {
    return detail::wrapOptional<Bus>(backend_->getRelatedByString(handle_, prop::REL_BBV_BUS, busId), backend_);
}

std::vector<Bus> VoltageLevel::BusBreakerView::getBusesFromBusViewBusId(const std::string& busViewBusId) const {
    return detail::elementsOf<Bus>(backend_->getRelatedByString(handle_, prop::REL_BBV_BUSES_OF_BV_BUS, busViewBusId), backend_);
}

BusBreakerSwitchAdder VoltageLevel::BusBreakerView::newSwitch() {
    return BusBreakerSwitchAdder(backend_->invoke(handle_, prop::REL_BBV_SWITCH), backend_);
}

BusAdder VoltageLevel::BusBreakerView::newBus() {
    return BusAdder(backend_->invoke(handle_, prop::BUS), backend_);
}

void VoltageLevel::BusBreakerView::removeBus(const std::string& busId) {
    iidm::optional<Bus> bus = getBus(busId);
    if (!bus) throw ValidationException("Bus " + busId + " not found");
    backend_->invoke(detail::Access::handle(*bus), prop::ACT_REMOVE);
}

void VoltageLevel::BusBreakerView::removeAllBuses() {
    backend_->invoke(handle_, prop::ACT_REMOVE_ALL_BUSES);
}

std::vector<Switch> VoltageLevel::BusBreakerView::getSwitches() const {
    return detail::wrapAll<Switch>(backend_->getChildren(handle_, prop::REL_BBV_SWITCH), backend_);
}

int VoltageLevel::BusBreakerView::getSwitchCount() const {
    return detail::countOf(backend_, handle_, prop::REL_BBV_SWITCH);
}

iidm::optional<Switch> VoltageLevel::BusBreakerView::getSwitch(const std::string& id) const {
    return detail::wrapOptional<Switch>(backend_->getRelatedByString(handle_, prop::REL_BBV_SWITCH, id), backend_);
}

Bus VoltageLevel::BusBreakerView::getBus1(const std::string& switchId) const {
    return Bus(backend_->getRelatedByString(handle_, prop::REL_VL_BBV_BUS1, switchId), backend_);
}

Bus VoltageLevel::BusBreakerView::getBus2(const std::string& switchId) const {
    return Bus(backend_->getRelatedByString(handle_, prop::REL_VL_BBV_BUS2, switchId), backend_);
}

void VoltageLevel::BusBreakerView::removeSwitch(const std::string& switchId) {
    backend_->setString(handle_, prop::VL_BBV_REMOVE_SWITCH, switchId);
}

void VoltageLevel::BusBreakerView::removeAllSwitches() {
    backend_->invoke(handle_, prop::ACT_REMOVE_ALL_SWITCHES);
}

// ── BusView ───────────────────────────────────────────────────────────────────

std::vector<Bus> VoltageLevel::BusView::getBuses() const {
    return detail::wrapAll<Bus>(backend_->getChildren(handle_, prop::REL_BV_BUS), backend_);
}

iidm::optional<Bus> VoltageLevel::BusView::getBus(const std::string& id) const {
    return detail::wrapOptional<Bus>(backend_->getRelatedByString(handle_, prop::REL_BV_BUS, id), backend_);
}

iidm::optional<Bus> VoltageLevel::BusView::getMergedBus(const std::string& configuredBusId) const {
    return detail::wrapOptional<Bus>(backend_->getRelatedByString(handle_, prop::REL_MERGED_BUS, configuredBusId), backend_);
}

// ── VoltageLevel ──────────────────────────────────────────────────────────────

iidm::optional<Substation> VoltageLevel::getSubstation() const {
    return detail::wrapOptional<Substation>(backend_->getRelated(handle_, prop::REL_SUBSTATION), backend_);
}

void VoltageLevel::convertToTopology(TopologyKind topologyKind) {
    backend_->setInt(handle_, prop::VL_TOPOLOGY_KIND, static_cast<int>(topologyKind));
}

void VoltageLevel::printTopology() const {
    printTopology(std::cout);
}

void VoltageLevel::printTopology(std::ostream& out) const {
    out << backend_->getString(handle_, prop::VL_TOPOLOGY_TEXT);
}

std::vector<Connectable> VoltageLevel::getConnectables() const {
    return detail::wrapAll<Connectable>(backend_->getChildren(handle_, prop::CONNECTABLE), backend_);
}

iidm::optional<Connectable> VoltageLevel::getConnectable(const std::string& id) const {
    return detail::wrapOptional<Connectable>(backend_->getRelatedByString(handle_, prop::CONNECTABLE, id), backend_);
}

int VoltageLevel::getConnectableCount() const {
    return detail::countOf(backend_, handle_, prop::CONNECTABLE);
}

std::vector<Line> VoltageLevel::getLines() const {
    return detail::wrapAll<Line>(backend_->getChildren(handle_, prop::LINE), backend_);
}

std::vector<TwoWindingsTransformer> VoltageLevel::getTwoWindingsTransformers() const {
    return detail::wrapAll<TwoWindingsTransformer>(backend_->getChildren(handle_, prop::TWO_WINDINGS_TRANSFORMER), backend_);
}

std::vector<ThreeWindingsTransformer> VoltageLevel::getThreeWindingsTransformers() const {
    return detail::wrapAll<ThreeWindingsTransformer>(backend_->getChildren(handle_, prop::THREE_WINDINGS_TRANSFORMER), backend_);
}

std::vector<Ground> VoltageLevel::getGrounds() const {
    return detail::wrapAll<Ground>(backend_->getChildren(handle_, prop::GROUND), backend_);
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

std::vector<BoundaryLine> VoltageLevel::getBoundaryLines(BoundaryLineFilter filter) const {
    return detail::filterBoundaryLines(
        detail::wrapAll<BoundaryLine>(backend_->getChildren(handle_, prop::BOUNDARY_LINE), backend_), filter);
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

// ── Counts ───────────────────────────────────────────────────────────────────

int VoltageLevel::getGeneratorCount() const {
    return detail::countOf(backend_, handle_, prop::GENERATOR);
}

int VoltageLevel::getLoadCount() const {
    return detail::countOf(backend_, handle_, prop::LOAD);
}

int VoltageLevel::getBatteryCount() const {
    return detail::countOf(backend_, handle_, prop::BATTERY);
}

int VoltageLevel::getShuntCompensatorCount() const {
    return detail::countOf(backend_, handle_, prop::SHUNT_COMPENSATOR);
}

int VoltageLevel::getStaticVarCompensatorCount() const {
    return detail::countOf(backend_, handle_, prop::STATIC_VAR_COMPENSATOR);
}

int VoltageLevel::getBoundaryLineCount() const {
    return detail::countOf(backend_, handle_, prop::BOUNDARY_LINE);
}

int VoltageLevel::getLccConverterStationCount() const {
    return detail::countOf(backend_, handle_, prop::LCC_CONVERTER_STATION);
}

int VoltageLevel::getVscConverterStationCount() const {
    return detail::countOf(backend_, handle_, prop::VSC_CONVERTER_STATION);
}

int VoltageLevel::getLineCount() const {
    return detail::countOf(backend_, handle_, prop::LINE);
}

int VoltageLevel::getTwoWindingsTransformerCount() const {
    return detail::countOf(backend_, handle_, prop::TWO_WINDINGS_TRANSFORMER);
}

int VoltageLevel::getThreeWindingsTransformerCount() const {
    return detail::countOf(backend_, handle_, prop::THREE_WINDINGS_TRANSFORMER);
}

int VoltageLevel::getSwitchCount() const {
    return detail::countOf(backend_, handle_, prop::SWITCH);
}

int VoltageLevel::getGroundCount() const {
    return detail::countOf(backend_, handle_, prop::GROUND);
}

VoltageLevel::BusView VoltageLevel::getBusView() const {
    return BusView(handle_, backend_);
}

std::vector<Area> VoltageLevel::getAreas() const {
    return detail::wrapAll<Area>(backend_->getChildren(handle_, prop::AREA), backend_);
}

iidm::optional<Area> VoltageLevel::getArea(const std::string& areaType) const {
    return detail::wrapOptional<Area>(backend_->getRelatedByString(handle_, prop::AREA, areaType), backend_);
}

void VoltageLevel::addArea(const Area& area) {
    backend_->setRelated(handle_, prop::ADD_AREA, detail::Access::handle(area));
}

void VoltageLevel::removeArea(const Area& area) {
    backend_->setRelated(handle_, prop::REMOVE_AREA, detail::Access::handle(area));
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

GroundAdder VoltageLevel::newGround() {
    return GroundAdder(backend_->invoke(handle_, prop::GROUND), backend_);
}

BatteryAdder VoltageLevel::newBattery() {
    return BatteryAdder(backend_->invoke(handle_, prop::BATTERY), backend_);
}

ShuntCompensatorAdder VoltageLevel::newShuntCompensator() {
    return ShuntCompensatorAdder(backend_->invoke(handle_, prop::SHUNT_COMPENSATOR), backend_);
}

StaticVarCompensatorAdder VoltageLevel::newStaticVarCompensator() {
    return StaticVarCompensatorAdder(backend_->invoke(handle_, prop::STATIC_VAR_COMPENSATOR), backend_);
}

VscConverterStationAdder VoltageLevel::newVscConverterStation() {
    return VscConverterStationAdder(backend_->invoke(handle_, prop::VSC_CONVERTER_STATION), backend_);
}

LccConverterStationAdder VoltageLevel::newLccConverterStation() {
    return LccConverterStationAdder(backend_->invoke(handle_, prop::LCC_CONVERTER_STATION), backend_);
}

BoundaryLineAdder VoltageLevel::newBoundaryLine() {
    return BoundaryLineAdder(backend_->invoke(handle_, prop::BOUNDARY_LINE), backend_);
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

ObservabilityAreaAdder VoltageLevel::newObservabilityArea() {
    return detail::newExtension<ObservabilityArea, ObservabilityAreaAdder>(backend_, handle_);
}

void VoltageLevel::removeObservabilityArea() {
    detail::removeExtension<ObservabilityArea>(backend_, handle_);
}

} // namespace iidm
