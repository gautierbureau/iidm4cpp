#include <iidm/Network.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>
#include "BoundaryLineFilter.h"
#include "Wrap.h"

#include <algorithm>
#include <ostream>

namespace iidm {

// The base class is built before owned_ takes the backend over
Network::Network(std::unique_ptr<BackendProvider> backend)
    : Identifiable(backend->getNetworkHandle(), backend.get()), owned_(std::move(backend)) {}

Network::~Network() {
    if (owned_) {
        owned_->close();
    }
}

// A moved-from Network is bound to no backend: using it throws InvalidHandleException
Network::Network(Network&& other) noexcept
    : Identifiable(std::move(other)), owned_(std::move(other.owned_)) {
    other.backend_ = detail::nullBackend();
}

Network& Network::operator=(Network&& other) noexcept {
    if (this != &other) {
        Identifiable::operator=(std::move(other));
        if (owned_) owned_->close();
        owned_ = std::move(other.owned_);
        other.backend_ = detail::nullBackend();
    }
    return *this;
}

BackendProvider& Network::getBackend() {
    return *backend_;
}

void Network::save(const std::string& filePath) const {
    backend_->saveNetwork(handle_, filePath);
}

void Network::save(std::ostream& out, const std::string& filenameHint) const {
    const std::string bytes = backend_->saveNetworkBytes(handle_, filenameHint);
    if (!bytes.empty()) {
        out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    }
}

std::string Network::saveToBytes(const std::string& filenameHint) const {
    return backend_->saveNetworkBytes(handle_, filenameHint);
}

SubstationAdder Network::newSubstation() {
    return SubstationAdder(backend_->invoke(handle_, prop::SUBSTATION), backend_);
}

VoltageLevelAdder Network::newVoltageLevel() {
    return VoltageLevelAdder(backend_->invoke(handle_, prop::VOLTAGE_LEVEL), backend_);
}

LineAdder Network::newLine() {
    return LineAdder(backend_->invoke(handle_, prop::LINE), backend_);
}

LineAdder Network::newLine(const Line& copied) {
    return detail::copyAdder<LineAdder>(backend_, handle_, prop::LINE, detail::Access::handle(copied));
}

HvdcLineAdder Network::newHvdcLine() {
    return HvdcLineAdder(backend_->invoke(handle_, prop::HVDC_LINE), backend_);
}

TieLineAdder Network::newTieLine() {
    return TieLineAdder(backend_->invoke(handle_, prop::TIE_LINE), backend_);
}

AreaAdder Network::newArea() {
    return AreaAdder(backend_->invoke(handle_, prop::AREA), backend_);
}

VoltageAngleLimitAdder Network::newVoltageAngleLimit() {
    return VoltageAngleLimitAdder(backend_->invoke(handle_, prop::VOLTAGE_ANGLE_LIMIT), backend_);
}

// ── Subnetworks ──────────────────────────────────────────────────────────────

std::vector<Network> Network::getSubnetworks() const {
    const std::vector<ObjectHandle> handles = backend_->getChildren(handle_, prop::NETWORK);
    std::vector<Network> subnetworks;
    subnetworks.reserve(handles.size());
    for (ObjectHandle h : handles) subnetworks.push_back(Network(h, backend_));
    return subnetworks;
}

iidm::optional<Network> Network::getSubnetwork(const std::string& id) const {
    const ObjectHandle h = backend_->getRelatedByString(handle_, prop::NETWORK, id);
    if (h == INVALID_HANDLE) return iidm::optional<Network>();
    return iidm::optional<Network>(Network(h, backend_));
}

Network Network::createSubnetwork(const std::string& subnetworkId, const std::string& name,
                                  const std::string& sourceFormat) {
    HandleRef pending(backend_->invoke(handle_, prop::NETWORK), backend_);
    backend_->setString(pending, prop::ID, subnetworkId);
    backend_->setString(pending, prop::NAME, name);
    backend_->setString(pending, prop::NETWORK_SOURCE_FORMAT, sourceFormat);
    return Network(backend_->invoke(pending, prop::ACT_ADD), backend_);
}

Network Network::detach() {
    return Network(backend_->invoke(handle_, prop::ACT_DETACH), backend_);
}

void Network::flatten() {
    backend_->invoke(handle_, prop::ACT_FLATTEN);
}

std::vector<Identifiable> Network::getBoundaryElements() const {
    return detail::wrapAll<Identifiable>(backend_->getChildren(handle_, prop::BOUNDARY_ELEMENT), backend_);
}

bool Network::isBoundaryElement(const Identifiable& identifiable) const {
    HandleRef check(backend_->invoke(handle_, prop::BOUNDARY_ELEMENT), backend_);
    backend_->setRelated(check, prop::IDENTIFIABLE, detail::Access::handle(identifiable));
    HandleRef found(backend_->invoke(check, prop::ACT_ADD), backend_);
    return found != INVALID_HANDLE;
}

// ── Iteration helpers ────────────────────────────────────────────────────────

namespace {

template <class T>
std::vector<T> all(BackendProvider* backend, ObjectHandle network) {
    return detail::wrapAll<T>(backend->getChildren(network, T::TYPE), backend);
}

// In this network only: a subnetwork finds its own objects
template <class T>
iidm::optional<T> find(BackendProvider* backend, ObjectHandle network, const std::string& id) {
    return detail::wrapOptional<T>(backend->getRelatedByString(network, T::TYPE, id), backend);
}

} // namespace

std::vector<Identifiable> Network::getIdentifiables() const {
    return all<Identifiable>(backend_, handle_);
}

std::vector<Connectable> Network::getConnectables() const {
    return all<Connectable>(backend_, handle_);
}

std::vector<Branch> Network::getBranches() const {
    return all<Branch>(backend_, handle_);
}

std::vector<HvdcConverterStation> Network::getHvdcConverterStations() const {
    return all<HvdcConverterStation>(backend_, handle_);
}

std::vector<Substation> Network::getSubstations() const {
    return all<Substation>(backend_, handle_);
}

std::vector<VoltageLevel> Network::getVoltageLevels() const {
    return all<VoltageLevel>(backend_, handle_);
}

std::vector<Battery> Network::getBatteries() const {
    return all<Battery>(backend_, handle_);
}

std::vector<Generator> Network::getGenerators() const {
    return all<Generator>(backend_, handle_);
}

std::vector<Load> Network::getLoads() const {
    return all<Load>(backend_, handle_);
}

std::vector<Line> Network::getLines() const {
    return all<Line>(backend_, handle_);
}

std::vector<TwoWindingsTransformer> Network::getTwoWindingsTransformers() const {
    return all<TwoWindingsTransformer>(backend_, handle_);
}

std::vector<ThreeWindingsTransformer> Network::getThreeWindingsTransformers() const {
    return all<ThreeWindingsTransformer>(backend_, handle_);
}

std::vector<HvdcLine> Network::getHvdcLines() const {
    return all<HvdcLine>(backend_, handle_);
}

std::vector<ShuntCompensator> Network::getShuntCompensators() const {
    return all<ShuntCompensator>(backend_, handle_);
}

std::vector<StaticVarCompensator> Network::getStaticVarCompensators() const {
    return all<StaticVarCompensator>(backend_, handle_);
}

std::vector<BoundaryLine> Network::getBoundaryLines(BoundaryLineFilter filter) const {
    return detail::filterBoundaryLines(all<BoundaryLine>(backend_, handle_), filter);
}

std::vector<TieLine> Network::getTieLines() const {
    return all<TieLine>(backend_, handle_);
}

std::vector<Ground> Network::getGrounds() const {
    return all<Ground>(backend_, handle_);
}

std::vector<Area> Network::getAreas() const {
    return all<Area>(backend_, handle_);
}

std::vector<VoltageAngleLimit> Network::getVoltageAngleLimits() const {
    return all<VoltageAngleLimit>(backend_, handle_);
}

std::vector<OverloadManagementSystem> Network::getOverloadManagementSystems() const {
    return all<OverloadManagementSystem>(backend_, handle_);
}

std::vector<std::string> Network::getAreaTypes() const {
    return backend_->getStringList(handle_, prop::AREA_TYPE);
}

std::vector<VscConverterStation> Network::getVscConverterStations() const {
    return all<VscConverterStation>(backend_, handle_);
}

std::vector<LccConverterStation> Network::getLccConverterStations() const {
    return all<LccConverterStation>(backend_, handle_);
}

// ── Lookup by ID ─────────────────────────────────────────────────────────────

iidm::optional<Identifiable> Network::getIdentifiable(const std::string& id) const {
    return find<Identifiable>(backend_, handle_, id);
}

iidm::optional<Connectable> Network::getConnectable(const std::string& id) const {
    return find<Connectable>(backend_, handle_, id);
}

iidm::optional<Branch> Network::getBranch(const std::string& id) const {
    return find<Branch>(backend_, handle_, id);
}

iidm::optional<HvdcConverterStation> Network::getHvdcConverterStation(const std::string& id) const {
    return find<HvdcConverterStation>(backend_, handle_, id);
}

iidm::optional<Substation> Network::getSubstation(const std::string& id) const {
    return find<Substation>(backend_, handle_, id);
}

iidm::optional<VoltageLevel> Network::getVoltageLevel(const std::string& id) const {
    return find<VoltageLevel>(backend_, handle_, id);
}

iidm::optional<Battery> Network::getBattery(const std::string& id) const {
    return find<Battery>(backend_, handle_, id);
}

iidm::optional<Generator> Network::getGenerator(const std::string& id) const {
    return find<Generator>(backend_, handle_, id);
}

iidm::optional<Load> Network::getLoad(const std::string& id) const {
    return find<Load>(backend_, handle_, id);
}

iidm::optional<Line> Network::getLine(const std::string& id) const {
    return find<Line>(backend_, handle_, id);
}

iidm::optional<TwoWindingsTransformer> Network::getTwoWindingsTransformer(const std::string& id) const {
    return find<TwoWindingsTransformer>(backend_, handle_, id);
}

iidm::optional<ThreeWindingsTransformer> Network::getThreeWindingsTransformer(const std::string& id) const {
    return find<ThreeWindingsTransformer>(backend_, handle_, id);
}

iidm::optional<HvdcLine> Network::getHvdcLine(const std::string& id) const {
    return find<HvdcLine>(backend_, handle_, id);
}

iidm::optional<BoundaryLine> Network::getBoundaryLine(const std::string& id) const {
    return find<BoundaryLine>(backend_, handle_, id);
}

iidm::optional<TieLine> Network::getTieLine(const std::string& id) const {
    return find<TieLine>(backend_, handle_, id);
}

iidm::optional<Ground> Network::getGround(const std::string& id) const {
    return find<Ground>(backend_, handle_, id);
}

iidm::optional<Area> Network::getArea(const std::string& id) const {
    return find<Area>(backend_, handle_, id);
}

iidm::optional<VoltageAngleLimit> Network::getVoltageAngleLimit(const std::string& id) const {
    return find<VoltageAngleLimit>(backend_, handle_, id);
}

iidm::optional<OverloadManagementSystem> Network::getOverloadManagementSystem(const std::string& id) const {
    return find<OverloadManagementSystem>(backend_, handle_, id);
}

iidm::optional<ShuntCompensator> Network::getShuntCompensator(const std::string& id) const {
    return find<ShuntCompensator>(backend_, handle_, id);
}

iidm::optional<StaticVarCompensator> Network::getStaticVarCompensator(const std::string& id) const {
    return find<StaticVarCompensator>(backend_, handle_, id);
}

iidm::optional<VscConverterStation> Network::getVscConverterStation(const std::string& id) const {
    return find<VscConverterStation>(backend_, handle_, id);
}

iidm::optional<LccConverterStation> Network::getLccConverterStation(const std::string& id) const {
    return find<LccConverterStation>(backend_, handle_, id);
}

iidm::optional<Switch> Network::getSwitch(const std::string& id) const {
    return find<Switch>(backend_, handle_, id);
}

iidm::optional<BusbarSection> Network::getBusbarSection(const std::string& id) const {
    return find<BusbarSection>(backend_, handle_, id);
}

std::vector<Switch> Network::getSwitches() const {
    return all<Switch>(backend_, handle_);
}

std::vector<BusbarSection> Network::getBusbarSections() const {
    return all<BusbarSection>(backend_, handle_);
}

std::vector<Substation> Network::getSubstations(const iidm::optional<Country>& country,
                                                const iidm::optional<std::string>& tsoId,
                                                const std::vector<std::string>& geographicalTags) const {
    // As powsybl filters them
    std::vector<Substation> result;
    for (Substation& substation : getSubstations()) {
        if (country && substation.getCountry() != country) continue;
        if (tsoId && substation.getTso() != tsoId) continue;
        const std::vector<std::string> tags = substation.getGeographicalTags();
        bool allTags = true;
        for (const std::string& tag : geographicalTags) {
            if (std::find(tags.begin(), tags.end(), tag) == tags.end()) {
                allTags = false;
                break;
            }
        }
        if (allTags) result.push_back(std::move(substation));
    }
    return result;
}

Network::BusBreakerView Network::getBusBreakerView() const {
    return BusBreakerView(handle_, backend_);
}

Network::BusView Network::getBusView() const {
    return BusView(handle_, backend_);
}

VariantManager Network::getVariantManager() const {
    return VariantManager(backend_->getRelated(handle_, prop::VARIANT_MANAGER), backend_);
}

// ── Validation ───────────────────────────────────────────────────────────────

ValidationLevel Network::runValidationChecks(bool throwsException) {
    return static_cast<ValidationLevel>(static_cast<int>(
        backend_->evaluate(handle_, prop::NETWORK_VALIDATION_LEVEL, {throwsException ? 1.0 : 0.0})));
}

std::set<Country> Network::getCountries() const {
    std::set<Country> countries;
    for (const std::string& name : backend_->getStringList(handle_, prop::NETWORK_COUNTRIES)) {
        iidm::optional<Country> country = countryFromName(name);
        if (country) countries.insert(*country);
    }
    return countries;
}

int Network::getCountryCount() const {
    return static_cast<int>(getCountries().size());
}

// ── Counts ───────────────────────────────────────────────────────────────────

int Network::getConnectableCount() const {
    return detail::countOf(backend_, handle_, prop::CONNECTABLE);
}

int Network::getBranchCount() const {
    return detail::countOf(backend_, handle_, prop::BRANCH);
}

int Network::getHvdcConverterStationCount() const {
    return detail::countOf(backend_, handle_, prop::HVDC_CONVERTER_STATION);
}

int Network::getSubstationCount() const {
    return detail::countOf(backend_, handle_, prop::SUBSTATION);
}

int Network::getVoltageLevelCount() const {
    return detail::countOf(backend_, handle_, prop::VOLTAGE_LEVEL);
}

int Network::getBatteryCount() const {
    return detail::countOf(backend_, handle_, prop::BATTERY);
}

int Network::getGeneratorCount() const {
    return detail::countOf(backend_, handle_, prop::GENERATOR);
}

int Network::getLoadCount() const {
    return detail::countOf(backend_, handle_, prop::LOAD);
}

int Network::getLineCount() const {
    return detail::countOf(backend_, handle_, prop::LINE);
}

int Network::getTwoWindingsTransformerCount() const {
    return detail::countOf(backend_, handle_, prop::TWO_WINDINGS_TRANSFORMER);
}

int Network::getThreeWindingsTransformerCount() const {
    return detail::countOf(backend_, handle_, prop::THREE_WINDINGS_TRANSFORMER);
}

int Network::getHvdcLineCount() const {
    return detail::countOf(backend_, handle_, prop::HVDC_LINE);
}

int Network::getShuntCompensatorCount() const {
    return detail::countOf(backend_, handle_, prop::SHUNT_COMPENSATOR);
}

int Network::getStaticVarCompensatorCount() const {
    return detail::countOf(backend_, handle_, prop::STATIC_VAR_COMPENSATOR);
}

int Network::getBoundaryLineCount() const {
    return detail::countOf(backend_, handle_, prop::BOUNDARY_LINE);
}

int Network::getTieLineCount() const {
    return detail::countOf(backend_, handle_, prop::TIE_LINE);
}

int Network::getGroundCount() const {
    return detail::countOf(backend_, handle_, prop::GROUND);
}

int Network::getAreaCount() const {
    return detail::countOf(backend_, handle_, prop::AREA);
}

int Network::getAreaTypeCount() const {
    return static_cast<int>(getAreaTypes().size());
}

int Network::getOverloadManagementSystemCount() const {
    return detail::countOf(backend_, handle_, prop::OVERLOAD_MANAGEMENT_SYSTEM);
}

int Network::getVscConverterStationCount() const {
    return detail::countOf(backend_, handle_, prop::VSC_CONVERTER_STATION);
}

int Network::getLccConverterStationCount() const {
    return detail::countOf(backend_, handle_, prop::LCC_CONVERTER_STATION);
}

int Network::getSwitchCount() const {
    return detail::countOf(backend_, handle_, prop::SWITCH);
}

int Network::getBusbarSectionCount() const {
    return detail::countOf(backend_, handle_, prop::BUSBAR_SECTION);
}

// ── Topology views ───────────────────────────────────────────────────────────

std::vector<Bus> Network::BusBreakerView::getBuses() const {
    return detail::wrapAll<Bus>(backend_->getChildren(handle_, prop::BUS), backend_);
}

int Network::BusBreakerView::getBusCount() const {
    return detail::countOf(backend_, handle_, prop::BUS);
}

std::vector<Switch> Network::BusBreakerView::getSwitches() const {
    return detail::wrapAll<Switch>(backend_->getChildren(handle_, prop::REL_BBV_SWITCH), backend_);
}

int Network::BusBreakerView::getSwitchCount() const {
    return detail::countOf(backend_, handle_, prop::REL_BBV_SWITCH);
}

iidm::optional<Bus> Network::BusBreakerView::getBus(const std::string& id) const {
    return detail::wrapOptional<Bus>(backend_->getRelatedByString(handle_, prop::REL_BBV_BUS, id), backend_);
}

std::vector<Bus> Network::BusView::getBuses() const {
    return detail::wrapAll<Bus>(backend_->getChildren(handle_, prop::REL_BV_BUS), backend_);
}

iidm::optional<Bus> Network::BusView::getBus(const std::string& id) const {
    return detail::wrapOptional<Bus>(backend_->getRelatedByString(handle_, prop::REL_BV_BUS, id), backend_);
}

std::vector<Component> Network::BusView::getConnectedComponents() const {
    return detail::wrapAll<Component>(backend_->getChildren(handle_, prop::REL_CONNECTED_COMPONENT), backend_);
}

std::vector<Component> Network::BusView::getSynchronousComponents() const {
    return detail::wrapAll<Component>(backend_->getChildren(handle_, prop::REL_SYNCHRONOUS_COMPONENT), backend_);
}

} // namespace iidm
