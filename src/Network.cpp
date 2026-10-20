#include <iidm/Network.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

#include <ostream>

namespace iidm {

Network::Network(std::unique_ptr<BackendProvider> backend)
    : backend_(std::move(backend)) {}

Network::~Network() {
    if (backend_) {
        backend_->close();
    }
}

Network::Network(Network&&) noexcept = default;

BackendProvider* Network::backend() const {
    return backend_ ? backend_.get() : detail::nullBackend(); // moved-from Network
}
Network& Network::operator=(Network&&) noexcept = default;

std::string Network::getId() const {
    return backend()->getString(backend()->getNetworkHandle(), prop::ID);
}

std::string Network::getName() const {
    return backend()->getString(backend()->getNetworkHandle(), prop::NAME);
}

SubstationAdder Network::newSubstation() {
    return SubstationAdder(backend()->invoke(backend()->getNetworkHandle(), prop::SUBSTATION), backend());
}

VoltageLevelAdder Network::newVoltageLevel() {
    return VoltageLevelAdder(backend()->invoke(backend()->getNetworkHandle(), prop::VOLTAGE_LEVEL), backend());
}

std::string Network::getCaseDate() const {
    return backend()->getString(backend()->getNetworkHandle(), prop::NETWORK_CASE_DATE);
}

Network& Network::setCaseDate(const std::string& caseDate) {
    backend()->setString(backend()->getNetworkHandle(), prop::NETWORK_CASE_DATE, caseDate);
    return *this;
}

int Network::getForecastDistance() const {
    return backend()->getInt(backend()->getNetworkHandle(), prop::NETWORK_FORECAST_DISTANCE);
}

Network& Network::setForecastDistance(int forecastDistance) {
    backend()->setInt(backend()->getNetworkHandle(), prop::NETWORK_FORECAST_DISTANCE, forecastDistance);
    return *this;
}

std::string Network::getSourceFormat() const {
    return backend()->getString(backend()->getNetworkHandle(), prop::NETWORK_SOURCE_FORMAT);
}

BackendProvider& Network::getBackend() {
    return *backend();
}

void Network::save(const std::string& filePath) const {
    backend()->saveNetwork(filePath);
}

void Network::save(std::ostream& out, const std::string& filenameHint) const {
    const std::string bytes = backend()->saveNetworkBytes(filenameHint);
    if (!bytes.empty()) {
        out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    }
}

std::string Network::saveToBytes(const std::string& filenameHint) const {
    return backend()->saveNetworkBytes(filenameHint);
}

// ── Helper: build vector of component objects from handles ────────────────────

template<typename T>
static std::vector<T> buildList(BackendProvider* backend, int childType) {
    auto handles = backend->getChildren(backend->getNetworkHandle(), childType);
    std::vector<T> result;
    result.reserve(handles.size());
    for (auto h : handles) {
        result.emplace_back(h, backend);
    }
    return result;
}

template<typename T>
static iidm::optional<T> findById(BackendProvider* backend, int objectType, const std::string& id) {
    ObjectHandle h = backend->findById(objectType, id);
    if (h == INVALID_HANDLE) return {};
    return T(h, backend);
}

// ── Iteration helpers ────────────────────────────────────────────────────────

std::vector<Substation> Network::getSubstations() const {
    return buildList<Substation>(backend(), prop::SUBSTATION);
}

std::vector<VoltageLevel> Network::getVoltageLevels() const {
    return buildList<VoltageLevel>(backend(), prop::VOLTAGE_LEVEL);
}

std::vector<Battery> Network::getBatteries() const {
    return buildList<Battery>(backend(), prop::BATTERY);
}

std::vector<Generator> Network::getGenerators() const {
    return buildList<Generator>(backend(), prop::GENERATOR);
}

std::vector<Load> Network::getLoads() const {
    return buildList<Load>(backend(), prop::LOAD);
}

std::vector<Line> Network::getLines() const {
    return buildList<Line>(backend(), prop::LINE);
}

std::vector<TwoWindingsTransformer> Network::getTwoWindingsTransformers() const {
    return buildList<TwoWindingsTransformer>(backend(), prop::TWO_WINDINGS_TRANSFORMER);
}

std::vector<ThreeWindingsTransformer> Network::getThreeWindingsTransformers() const {
    return buildList<ThreeWindingsTransformer>(backend(), prop::THREE_WINDINGS_TRANSFORMER);
}

std::vector<HvdcLine> Network::getHvdcLines() const {
    return buildList<HvdcLine>(backend(), prop::HVDC_LINE);
}

std::vector<ShuntCompensator> Network::getShuntCompensators() const {
    return buildList<ShuntCompensator>(backend(), prop::SHUNT_COMPENSATOR);
}

std::vector<StaticVarCompensator> Network::getStaticVarCompensators() const {
    return buildList<StaticVarCompensator>(backend(), prop::STATIC_VAR_COMPENSATOR);
}

std::vector<DanglingLine> Network::getDanglingLines() const {
    return buildList<DanglingLine>(backend(), prop::DANGLING_LINE);
}

std::vector<VscConverterStation> Network::getVscConverterStations() const {
    return buildList<VscConverterStation>(backend(), prop::VSC_CONVERTER_STATION);
}

std::vector<LccConverterStation> Network::getLccConverterStations() const {
    return buildList<LccConverterStation>(backend(), prop::LCC_CONVERTER_STATION);
}

// ── Lookup by ID ─────────────────────────────────────────────────────────────

iidm::optional<Battery> Network::getBattery(const std::string& id) const {
    return findById<Battery>(backend(), prop::BATTERY, id);
}

iidm::optional<Generator> Network::getGenerator(const std::string& id) const {
    return findById<Generator>(backend(), prop::GENERATOR, id);
}

iidm::optional<Load> Network::getLoad(const std::string& id) const {
    return findById<Load>(backend(), prop::LOAD, id);
}

iidm::optional<Line> Network::getLine(const std::string& id) const {
    return findById<Line>(backend(), prop::LINE, id);
}

iidm::optional<TwoWindingsTransformer> Network::getTwoWindingsTransformer(const std::string& id) const {
    return findById<TwoWindingsTransformer>(backend(), prop::TWO_WINDINGS_TRANSFORMER, id);
}

iidm::optional<ThreeWindingsTransformer> Network::getThreeWindingsTransformer(const std::string& id) const {
    return findById<ThreeWindingsTransformer>(backend(), prop::THREE_WINDINGS_TRANSFORMER, id);
}

iidm::optional<HvdcLine> Network::getHvdcLine(const std::string& id) const {
    return findById<HvdcLine>(backend(), prop::HVDC_LINE, id);
}

iidm::optional<DanglingLine> Network::getDanglingLine(const std::string& id) const {
    return findById<DanglingLine>(backend(), prop::DANGLING_LINE, id);
}

iidm::optional<ShuntCompensator> Network::getShuntCompensator(const std::string& id) const {
    return findById<ShuntCompensator>(backend(), prop::SHUNT_COMPENSATOR, id);
}

iidm::optional<StaticVarCompensator> Network::getStaticVarCompensator(const std::string& id) const {
    return findById<StaticVarCompensator>(backend(), prop::STATIC_VAR_COMPENSATOR, id);
}

iidm::optional<VscConverterStation> Network::getVscConverterStation(const std::string& id) const {
    return findById<VscConverterStation>(backend(), prop::VSC_CONVERTER_STATION, id);
}

iidm::optional<LccConverterStation> Network::getLccConverterStation(const std::string& id) const {
    return findById<LccConverterStation>(backend(), prop::LCC_CONVERTER_STATION, id);
}

iidm::optional<Switch> Network::getSwitch(const std::string& id) const {
    return findById<Switch>(backend(), prop::SWITCH, id);
}

iidm::optional<BusbarSection> Network::getBusbarSection(const std::string& id) const {
    return findById<BusbarSection>(backend(), prop::BUSBAR_SECTION, id);
}

} // namespace iidm
