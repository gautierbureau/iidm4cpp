#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/Substation.h>
#include <iidm/VoltageLevel.h>
#include <iidm/Generator.h>
#include <iidm/Load.h>
#include <iidm/Line.h>
#include <iidm/TwoWindingsTransformer.h>
#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/HvdcLine.h>
#include <iidm/DanglingLine.h>
#include <iidm/ShuntCompensator.h>
#include <iidm/StaticVarCompensator.h>
#include <iidm/VscConverterStation.h>
#include <iidm/LccConverterStation.h>
#include <iidm/BusbarSection.h>
#include <iidm/Switch.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace iidm {

class NetworkFactory;

class Network {
public:
    ~Network();
    Network(Network&&) noexcept;
    Network& operator=(Network&&) noexcept;

    // No copy: a Network owns its backend session
    Network(const Network&) = delete;
    Network& operator=(const Network&) = delete;

    std::string getId() const;
    std::string getName() const;

    // ── Iteration helpers ────────────────────────────────────────────────
    std::vector<Substation>              getSubstations() const;
    std::vector<VoltageLevel>            getVoltageLevels() const;
    std::vector<Generator>               getGenerators() const;
    std::vector<Load>                    getLoads() const;
    std::vector<Line>                    getLines() const;
    std::vector<TwoWindingsTransformer>  getTwoWindingsTransformers() const;
    std::vector<ThreeWindingsTransformer> getThreeWindingsTransformers() const;
    std::vector<HvdcLine>                getHvdcLines() const;
    std::vector<ShuntCompensator>        getShuntCompensators() const;
    std::vector<StaticVarCompensator>    getStaticVarCompensators() const;
    std::vector<DanglingLine>            getDanglingLines() const;
    std::vector<VscConverterStation>     getVscConverterStations() const;
    std::vector<LccConverterStation>     getLccConverterStations() const;

    // ── Lookup by ID ─────────────────────────────────────────────────────
    std::optional<Generator>               getGenerator(const std::string& id) const;
    std::optional<Load>                    getLoad(const std::string& id) const;
    std::optional<Line>                    getLine(const std::string& id) const;
    std::optional<TwoWindingsTransformer>  getTwoWindingsTransformer(const std::string& id) const;
    std::optional<ThreeWindingsTransformer> getThreeWindingsTransformer(const std::string& id) const;
    std::optional<HvdcLine>               getHvdcLine(const std::string& id) const;
    std::optional<DanglingLine>           getDanglingLine(const std::string& id) const;
    std::optional<ShuntCompensator>       getShuntCompensator(const std::string& id) const;
    std::optional<StaticVarCompensator>   getStaticVarCompensator(const std::string& id) const;
    std::optional<VscConverterStation>    getVscConverterStation(const std::string& id) const;
    std::optional<LccConverterStation>    getLccConverterStation(const std::string& id) const;
    std::optional<Switch>                  getSwitch(const std::string& id) const;
    std::optional<BusbarSection>           getBusbarSection(const std::string& id) const;

    // Backend accessor (advanced usage only)
    BackendProvider& getBackend();

private:
    friend class NetworkFactory;
    explicit Network(std::unique_ptr<BackendProvider> backend);
    std::unique_ptr<BackendProvider> backend_;
};

} // namespace iidm
