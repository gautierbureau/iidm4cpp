#include <iidm/Network.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

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

std::string Network::getCaseDate() const {
    return backend_->getString(handle_, prop::NETWORK_CASE_DATE);
}

Network& Network::setCaseDate(const std::string& caseDate) {
    backend_->setString(handle_, prop::NETWORK_CASE_DATE, caseDate);
    return *this;
}

int Network::getForecastDistance() const {
    return backend_->getInt(handle_, prop::NETWORK_FORECAST_DISTANCE);
}

Network& Network::setForecastDistance(int forecastDistance) {
    backend_->setInt(handle_, prop::NETWORK_FORECAST_DISTANCE, forecastDistance);
    return *this;
}

std::string Network::getSourceFormat() const {
    return backend_->getString(handle_, prop::NETWORK_SOURCE_FORMAT);
}

BackendProvider& Network::getBackend() {
    return *backend_;
}

void Network::save(const std::string& filePath) const {
    backend_->saveNetwork(filePath);
}

void Network::save(std::ostream& out, const std::string& filenameHint) const {
    const std::string bytes = backend_->saveNetworkBytes(filenameHint);
    if (!bytes.empty()) {
        out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    }
}

std::string Network::saveToBytes(const std::string& filenameHint) const {
    return backend_->saveNetworkBytes(filenameHint);
}

SubstationAdder Network::newSubstation() {
    return SubstationAdder(backend_->invoke(handle_, prop::SUBSTATION), backend_);
}

VoltageLevelAdder Network::newVoltageLevel() {
    return VoltageLevelAdder(backend_->invoke(handle_, prop::VOLTAGE_LEVEL), backend_);
}

// ── Iteration helpers ────────────────────────────────────────────────────────

namespace {

template <class T>
std::vector<T> all(BackendProvider* backend, ObjectHandle network) {
    return detail::wrapAll<T>(backend->getChildren(network, T::TYPE), backend);
}

template <class T>
iidm::optional<T> find(BackendProvider* backend, const std::string& id) {
    return detail::wrapOptional<T>(backend->findById(T::TYPE, id), backend);
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

std::vector<DanglingLine> Network::getDanglingLines() const {
    return all<DanglingLine>(backend_, handle_);
}

std::vector<VscConverterStation> Network::getVscConverterStations() const {
    return all<VscConverterStation>(backend_, handle_);
}

std::vector<LccConverterStation> Network::getLccConverterStations() const {
    return all<LccConverterStation>(backend_, handle_);
}

// ── Lookup by ID ─────────────────────────────────────────────────────────────

iidm::optional<Identifiable> Network::getIdentifiable(const std::string& id) const {
    return find<Identifiable>(backend_, id);
}

iidm::optional<Connectable> Network::getConnectable(const std::string& id) const {
    return find<Connectable>(backend_, id);
}

iidm::optional<Branch> Network::getBranch(const std::string& id) const {
    return find<Branch>(backend_, id);
}

iidm::optional<HvdcConverterStation> Network::getHvdcConverterStation(const std::string& id) const {
    return find<HvdcConverterStation>(backend_, id);
}

iidm::optional<Substation> Network::getSubstation(const std::string& id) const {
    return find<Substation>(backend_, id);
}

iidm::optional<VoltageLevel> Network::getVoltageLevel(const std::string& id) const {
    return find<VoltageLevel>(backend_, id);
}

iidm::optional<Battery> Network::getBattery(const std::string& id) const {
    return find<Battery>(backend_, id);
}

iidm::optional<Generator> Network::getGenerator(const std::string& id) const {
    return find<Generator>(backend_, id);
}

iidm::optional<Load> Network::getLoad(const std::string& id) const {
    return find<Load>(backend_, id);
}

iidm::optional<Line> Network::getLine(const std::string& id) const {
    return find<Line>(backend_, id);
}

iidm::optional<TwoWindingsTransformer> Network::getTwoWindingsTransformer(const std::string& id) const {
    return find<TwoWindingsTransformer>(backend_, id);
}

iidm::optional<ThreeWindingsTransformer> Network::getThreeWindingsTransformer(const std::string& id) const {
    return find<ThreeWindingsTransformer>(backend_, id);
}

iidm::optional<HvdcLine> Network::getHvdcLine(const std::string& id) const {
    return find<HvdcLine>(backend_, id);
}

iidm::optional<DanglingLine> Network::getDanglingLine(const std::string& id) const {
    return find<DanglingLine>(backend_, id);
}

iidm::optional<ShuntCompensator> Network::getShuntCompensator(const std::string& id) const {
    return find<ShuntCompensator>(backend_, id);
}

iidm::optional<StaticVarCompensator> Network::getStaticVarCompensator(const std::string& id) const {
    return find<StaticVarCompensator>(backend_, id);
}

iidm::optional<VscConverterStation> Network::getVscConverterStation(const std::string& id) const {
    return find<VscConverterStation>(backend_, id);
}

iidm::optional<LccConverterStation> Network::getLccConverterStation(const std::string& id) const {
    return find<LccConverterStation>(backend_, id);
}

iidm::optional<Switch> Network::getSwitch(const std::string& id) const {
    return find<Switch>(backend_, id);
}

iidm::optional<BusbarSection> Network::getBusbarSection(const std::string& id) const {
    return find<BusbarSection>(backend_, id);
}

} // namespace iidm
