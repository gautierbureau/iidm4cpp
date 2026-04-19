#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Enums.h>
#include <iidm/Bus.h>
#include <iidm/SlackTerminal.h>
#include <iidm/BusbarSection.h>
#include <iidm/Switch.h>
#include <iidm/InternalConnection.h>
#include <iidm/Terminal.h>
#include <optional>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;
class Generator;
class Load;
class Battery;
class ShuntCompensator;
class StaticVarCompensator;
class DanglingLine;
class LccConverterStation;
class VscConverterStation;

class VoltageLevel {
public:
    // ── NodeBreakerView ──────────────────────────────────────────────────────
    class NodeBreakerView {
    public:
        NodeBreakerView() = default;
        explicit NodeBreakerView(ObjectHandle vlHandle, BackendProvider* backend);

        std::vector<Switch>             getSwitches() const;
        std::vector<BusbarSection>      getBusbarSections() const;
        std::vector<InternalConnection> getInternalConnections() const;

        std::vector<int> getNodes() const;
        std::optional<Switch> getSwitch(const std::string& id) const;
        Terminal getTerminal(int node) const;

        bool isValid() const { return handle_ != INVALID_HANDLE; }

    private:
        ObjectHandle     handle_  = INVALID_HANDLE;
        BackendProvider* backend_ = nullptr;
    };

    // ── BusBreakerView ───────────────────────────────────────────────────────
    class BusBreakerView {
    public:
        BusBreakerView() = default;
        explicit BusBreakerView(ObjectHandle vlHandle, BackendProvider* backend);

        std::vector<Bus>    getBuses()    const;
        std::vector<Switch> getSwitches() const;

        bool isValid() const { return handle_ != INVALID_HANDLE; }

    private:
        ObjectHandle     handle_  = INVALID_HANDLE;
        BackendProvider* backend_ = nullptr;
    };

    // ── VoltageLevel ─────────────────────────────────────────────────────────
    VoltageLevel() = default;
    explicit VoltageLevel(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getNominalV() const;
    std::optional<double> getLowVoltageLimit() const;
    std::optional<double> getHighVoltageLimit() const;

    TopologyKind getTopologyKind() const;

    bool hasSlackTerminal() const;
    SlackTerminal getSlackTerminal() const;

    std::vector<Switch>        getSwitches() const;
    std::vector<BusbarSection> getBusbarSections() const;

    // ── Per-injector getters ─────────────────────────────────────────────────
    std::vector<Generator>            getGenerators() const;
    std::vector<Load>                 getLoads() const;
    std::vector<Battery>              getBatteries() const;
    std::vector<ShuntCompensator>     getShuntCompensators() const;
    std::vector<StaticVarCompensator> getStaticVarCompensators() const;
    std::vector<DanglingLine>         getDanglingLines() const;
    std::vector<LccConverterStation>  getLccConverterStations() const;
    std::vector<VscConverterStation>  getVscConverterStations() const;

    NodeBreakerView getNodeBreakerView() const;
    BusBreakerView  getBusBreakerView()  const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const VoltageLevel& other) const;
    bool operator!=(const VoltageLevel& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
