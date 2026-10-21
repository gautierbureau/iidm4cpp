#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/Branch.h>
#include <iidm/Connectable.h>
#include <iidm/HvdcConverterStation.h>
#include <iidm/Identifiable.h>
#include <iidm/Substation.h>
#include <iidm/SubstationAdder.h>
#include <iidm/VoltageLevel.h>
#include <iidm/VoltageLevelAdder.h>
#include <iidm/Battery.h>
#include <iidm/Generator.h>
#include <iidm/Load.h>
#include <iidm/Line.h>
#include <iidm/TwoWindingsTransformer.h>
#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/HvdcLine.h>
#include <iidm/BoundaryLine.h>
#include <iidm/ShuntCompensator.h>
#include <iidm/StaticVarCompensator.h>
#include <iidm/VscConverterStation.h>
#include <iidm/LccConverterStation.h>
#include <iidm/BusbarSection.h>
#include <iidm/Switch.h>
#include <iosfwd>
#include <memory>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class NetworkFactory;

// A network (powsybl Network), created by NetworkFactory. It owns its backend session: it
// cannot be copied, and the objects taken from it are valid while it lives.
class Network : public Identifiable {
public:
    ~Network();
    Network(Network&&) noexcept;
    Network& operator=(Network&&) noexcept;

    Network(const Network&) = delete;
    Network& operator=(const Network&) = delete;

    // ISO-8601 date-time, e.g. "2026-01-31T12:00Z"
    std::string getCaseDate() const;
    Network& setCaseDate(const std::string& caseDate);
    // In minutes
    int getForecastDistance() const;
    Network& setForecastDistance(int forecastDistance);
    std::string getSourceFormat() const;

    // Save the network to an XIIDM file.
    void save(const std::string& filePath) const;

    // Save the network into a std::ostream. `filenameHint` selects the
    // exporter (default "network.xiidm").
    void save(std::ostream& out,
              const std::string& filenameHint = "network.xiidm") const;

    // Serialize the network into a byte buffer (XIIDM by default).
    std::string saveToBytes(const std::string& filenameHint = "network.xiidm") const;

    // ── Iteration helpers ────────────────────────────────────────────────
    std::vector<Identifiable>            getIdentifiables() const;
    std::vector<Connectable>             getConnectables() const;
    std::vector<Branch>                  getBranches() const;
    std::vector<HvdcConverterStation>    getHvdcConverterStations() const;
    std::vector<Substation>              getSubstations() const;
    std::vector<VoltageLevel>            getVoltageLevels() const;
    std::vector<Battery>                 getBatteries() const;
    std::vector<Generator>               getGenerators() const;
    std::vector<Load>                    getLoads() const;
    std::vector<Line>                    getLines() const;
    std::vector<TwoWindingsTransformer>  getTwoWindingsTransformers() const;
    std::vector<ThreeWindingsTransformer> getThreeWindingsTransformers() const;
    std::vector<HvdcLine>                getHvdcLines() const;
    std::vector<ShuntCompensator>        getShuntCompensators() const;
    std::vector<StaticVarCompensator>    getStaticVarCompensators() const;
    std::vector<BoundaryLine>            getBoundaryLines() const;
    std::vector<VscConverterStation>     getVscConverterStations() const;
    std::vector<LccConverterStation>     getLccConverterStations() const;

    // ── Lookup by ID (or alias) ──────────────────────────────────────────
    iidm::optional<Identifiable>            getIdentifiable(const std::string& id) const;
    iidm::optional<Connectable>             getConnectable(const std::string& id) const;
    iidm::optional<Branch>                  getBranch(const std::string& id) const;
    iidm::optional<HvdcConverterStation>    getHvdcConverterStation(const std::string& id) const;
    iidm::optional<Substation>              getSubstation(const std::string& id) const;
    iidm::optional<VoltageLevel>            getVoltageLevel(const std::string& id) const;
    iidm::optional<Battery>                 getBattery(const std::string& id) const;
    iidm::optional<Generator>               getGenerator(const std::string& id) const;
    iidm::optional<Load>                    getLoad(const std::string& id) const;
    iidm::optional<Line>                    getLine(const std::string& id) const;
    iidm::optional<TwoWindingsTransformer>  getTwoWindingsTransformer(const std::string& id) const;
    iidm::optional<ThreeWindingsTransformer> getThreeWindingsTransformer(const std::string& id) const;
    iidm::optional<HvdcLine>               getHvdcLine(const std::string& id) const;
    iidm::optional<BoundaryLine>           getBoundaryLine(const std::string& id) const;
    iidm::optional<ShuntCompensator>       getShuntCompensator(const std::string& id) const;
    iidm::optional<StaticVarCompensator>   getStaticVarCompensator(const std::string& id) const;
    iidm::optional<VscConverterStation>    getVscConverterStation(const std::string& id) const;
    iidm::optional<LccConverterStation>    getLccConverterStation(const std::string& id) const;
    iidm::optional<Switch>                  getSwitch(const std::string& id) const;
    iidm::optional<BusbarSection>           getBusbarSection(const std::string& id) const;

    // ── Creation ─────────────────────────────────────────────────────────────
    SubstationAdder   newSubstation();
    // A voltage level without substation
    VoltageLevelAdder newVoltageLevel();

    // Backend accessor (advanced usage only)
    BackendProvider& getBackend();

private:
    friend class NetworkFactory;
    explicit Network(std::unique_ptr<BackendProvider> backend);

    std::unique_ptr<BackendProvider> owned_;
};

} // namespace iidm
