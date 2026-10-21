#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/Branch.h>
#include <iidm/Connectable.h>
#include <iidm/Container.h>
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
#include <set>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class NetworkFactory;

// A network (powsybl Network), created by NetworkFactory. It owns its backend session: it
// cannot be copied, and the objects taken from it are valid while it lives. The network of an
// object (Identifiable::getNetwork()) is a view of it, which owns nothing.
class Network : public Identifiable, public detail::ContainerMethods<Network> {
public:
    // ── Topology views ───────────────────────────────────────────────────
    // The buses and switches of the bus-breaker views of all its voltage levels
    class BusBreakerView : public detail::ObjectRef {
    public:
        BusBreakerView() = default;
        explicit BusBreakerView(ObjectHandle networkHandle, BackendProvider* backend) : ObjectRef(networkHandle, backend) {}

        std::vector<Bus> getBuses() const;
        int getBusCount() const;
        std::vector<Switch> getSwitches() const;
        int getSwitchCount() const;
        // Empty when there is no such bus
        iidm::optional<Bus> getBus(const std::string& id) const;
    };

    // The buses of the bus views of all its voltage levels
    class BusView : public detail::ObjectRef {
    public:
        BusView() = default;
        explicit BusView(ObjectHandle networkHandle, BackendProvider* backend) : ObjectRef(networkHandle, backend) {}

        std::vector<Bus> getBuses() const;
        // Empty when there is no such bus
        iidm::optional<Bus> getBus(const std::string& id) const;
    };

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

    // ── Validation ───────────────────────────────────────────────────────
    ValidationLevel getValidationLevel() const;
    // The lowest level its values may fall to (EQUIPMENT: steady state values may be missing)
    Network& setMinimumAcceptableValidationLevel(ValidationLevel validationLevel);
    // Validates every value again: the level reached, or ValidationException with throwsException
    ValidationLevel runValidationChecks(bool throwsException = true);

    // The countries of its substations
    std::set<Country> getCountries() const;
    int getCountryCount() const;

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
    std::vector<Switch>                  getSwitches() const;
    std::vector<BusbarSection>           getBusbarSections() const;
    // Those of a country (any when empty), of a TSO (any when empty), with all these tags
    std::vector<Substation> getSubstations(const iidm::optional<Country>& country, const iidm::optional<std::string>& tsoId,
                                           const std::vector<std::string>& geographicalTags = {}) const;

    BusBreakerView getBusBreakerView() const;
    BusView getBusView() const;

    // ── Counts ───────────────────────────────────────────────────────────
    int getConnectableCount() const;
    int getBranchCount() const;
    int getHvdcConverterStationCount() const;
    int getSubstationCount() const;
    int getVoltageLevelCount() const;
    int getBatteryCount() const;
    int getGeneratorCount() const;
    int getLoadCount() const;
    int getLineCount() const;
    int getTwoWindingsTransformerCount() const;
    int getThreeWindingsTransformerCount() const;
    int getHvdcLineCount() const;
    int getShuntCompensatorCount() const;
    int getStaticVarCompensatorCount() const;
    int getBoundaryLineCount() const;
    int getVscConverterStationCount() const;
    int getLccConverterStationCount() const;
    int getSwitchCount() const;
    int getBusbarSectionCount() const;

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
    friend class Identifiable;
    explicit Network(std::unique_ptr<BackendProvider> backend);
    // A view: it owns nothing
    Network(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    std::unique_ptr<BackendProvider> owned_;
};

} // namespace iidm
