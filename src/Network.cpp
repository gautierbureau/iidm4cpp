#include <iidm/Network.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

Network::Network(std::unique_ptr<BackendProvider> backend)
    : backend_(std::move(backend)) {}

Network::~Network() {
    if (backend_) {
        backend_->close();
    }
}

Network::Network(Network&&) noexcept = default;
Network& Network::operator=(Network&&) noexcept = default;

std::string Network::getId() const {
    return backend_->getString(backend_->getNetworkHandle(), prop::ID);
}

std::string Network::getName() const {
    return backend_->getString(backend_->getNetworkHandle(), prop::NAME);
}

BackendProvider& Network::getBackend() {
    return *backend_;
}

void Network::save(const std::string& filePath) const {
    backend_->saveNetwork(filePath);
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
static std::optional<T> findById(BackendProvider* backend, int objectType, const std::string& id) {
    ObjectHandle h = backend->findById(objectType, id);
    if (h == INVALID_HANDLE) return std::nullopt;
    return T(h, backend);
}

// ── Iteration helpers ────────────────────────────────────────────────────────

std::vector<Substation> Network::getSubstations() const {
    return buildList<Substation>(backend_.get(), prop::SUBSTATION);
}

std::vector<VoltageLevel> Network::getVoltageLevels() const {
    return buildList<VoltageLevel>(backend_.get(), prop::VOLTAGE_LEVEL);
}

std::vector<Battery> Network::getBatteries() const {
    return buildList<Battery>(backend_.get(), prop::BATTERY);
}

std::vector<Generator> Network::getGenerators() const {
    return buildList<Generator>(backend_.get(), prop::GENERATOR);
}

std::vector<Load> Network::getLoads() const {
    return buildList<Load>(backend_.get(), prop::LOAD);
}

std::vector<Line> Network::getLines() const {
    return buildList<Line>(backend_.get(), prop::LINE);
}

std::vector<TwoWindingsTransformer> Network::getTwoWindingsTransformers() const {
    return buildList<TwoWindingsTransformer>(backend_.get(), prop::TWO_WINDINGS_TRANSFORMER);
}

std::vector<ThreeWindingsTransformer> Network::getThreeWindingsTransformers() const {
    return buildList<ThreeWindingsTransformer>(backend_.get(), prop::THREE_WINDINGS_TRANSFORMER);
}

std::vector<HvdcLine> Network::getHvdcLines() const {
    return buildList<HvdcLine>(backend_.get(), prop::HVDC_LINE);
}

std::vector<ShuntCompensator> Network::getShuntCompensators() const {
    return buildList<ShuntCompensator>(backend_.get(), prop::SHUNT_COMPENSATOR);
}

std::vector<StaticVarCompensator> Network::getStaticVarCompensators() const {
    return buildList<StaticVarCompensator>(backend_.get(), prop::STATIC_VAR_COMPENSATOR);
}

std::vector<DanglingLine> Network::getDanglingLines() const {
    return buildList<DanglingLine>(backend_.get(), prop::DANGLING_LINE);
}

std::vector<VscConverterStation> Network::getVscConverterStations() const {
    return buildList<VscConverterStation>(backend_.get(), prop::VSC_CONVERTER_STATION);
}

std::vector<LccConverterStation> Network::getLccConverterStations() const {
    return buildList<LccConverterStation>(backend_.get(), prop::LCC_CONVERTER_STATION);
}

// ── Lookup by ID ─────────────────────────────────────────────────────────────

std::optional<Battery> Network::getBattery(const std::string& id) const {
    return findById<Battery>(backend_.get(), prop::BATTERY, id);
}

std::optional<Generator> Network::getGenerator(const std::string& id) const {
    return findById<Generator>(backend_.get(), prop::GENERATOR, id);
}

std::optional<Load> Network::getLoad(const std::string& id) const {
    return findById<Load>(backend_.get(), prop::LOAD, id);
}

std::optional<Line> Network::getLine(const std::string& id) const {
    return findById<Line>(backend_.get(), prop::LINE, id);
}

std::optional<TwoWindingsTransformer> Network::getTwoWindingsTransformer(const std::string& id) const {
    return findById<TwoWindingsTransformer>(backend_.get(), prop::TWO_WINDINGS_TRANSFORMER, id);
}

std::optional<ThreeWindingsTransformer> Network::getThreeWindingsTransformer(const std::string& id) const {
    return findById<ThreeWindingsTransformer>(backend_.get(), prop::THREE_WINDINGS_TRANSFORMER, id);
}

std::optional<HvdcLine> Network::getHvdcLine(const std::string& id) const {
    return findById<HvdcLine>(backend_.get(), prop::HVDC_LINE, id);
}

std::optional<DanglingLine> Network::getDanglingLine(const std::string& id) const {
    return findById<DanglingLine>(backend_.get(), prop::DANGLING_LINE, id);
}

std::optional<ShuntCompensator> Network::getShuntCompensator(const std::string& id) const {
    return findById<ShuntCompensator>(backend_.get(), prop::SHUNT_COMPENSATOR, id);
}

std::optional<StaticVarCompensator> Network::getStaticVarCompensator(const std::string& id) const {
    return findById<StaticVarCompensator>(backend_.get(), prop::STATIC_VAR_COMPENSATOR, id);
}

std::optional<VscConverterStation> Network::getVscConverterStation(const std::string& id) const {
    return findById<VscConverterStation>(backend_.get(), prop::VSC_CONVERTER_STATION, id);
}

std::optional<LccConverterStation> Network::getLccConverterStation(const std::string& id) const {
    return findById<LccConverterStation>(backend_.get(), prop::LCC_CONVERTER_STATION, id);
}

std::optional<Switch> Network::getSwitch(const std::string& id) const {
    return findById<Switch>(backend_.get(), prop::SWITCH, id);
}

std::optional<BusbarSection> Network::getBusbarSection(const std::string& id) const {
    return findById<BusbarSection>(backend_.get(), prop::BUSBAR_SECTION, id);
}

} // namespace iidm
