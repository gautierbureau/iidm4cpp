#pragma once

#include <iidm/Connectable.h>
#include <iidm/Enums.h>
#include <iidm/Identifiable.h>
#include <iidm/Bus.h>
#include <iidm/BusAdder.h>
#include <iidm/GeneratorAdder.h>
#include <iidm/LoadAdder.h>
#include <iidm/SlackTerminal.h>
#include <iidm/SlackTerminalAdder.h>
#include <iidm/BusbarSection.h>
#include <iidm/Switch.h>
#include <iidm/InternalConnection.h>
#include <iidm/Terminal.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>
#include <iidm/ObservabilityArea.h>

namespace iidm {

class Generator;
class Load;
class Battery;
class ShuntCompensator;
class StaticVarCompensator;
class DanglingLine;
class LccConverterStation;
class VscConverterStation;

class VoltageLevel : public Identifiable {
public:
    // ── NodeBreakerView ──────────────────────────────────────────────────────
    class NodeBreakerView : public detail::ObjectRef {
    public:
        NodeBreakerView() = default;
        explicit NodeBreakerView(ObjectHandle vlHandle, BackendProvider* backend);

        std::vector<Switch>             getSwitches() const;
        std::vector<BusbarSection>      getBusbarSections() const;
        std::vector<InternalConnection> getInternalConnections() const;

        std::vector<int> getNodes() const;
        iidm::optional<Switch> getSwitch(const std::string& id) const;
        Terminal getTerminal(int node) const;
    };

    // ── BusBreakerView ───────────────────────────────────────────────────────
    class BusBreakerView : public detail::ObjectRef {
    public:
        BusBreakerView() = default;
        explicit BusBreakerView(ObjectHandle vlHandle, BackendProvider* backend);

        std::vector<Bus>    getBuses()    const;
        std::vector<Switch> getSwitches() const;

        Bus getBus1(const std::string& switchId) const;
        Bus getBus2(const std::string& switchId) const;

        // Empty when it has no bus with this id
        iidm::optional<Bus> getBus(const std::string& busId) const;
        BusAdder newBus();
        // Removes a bus that nothing is connected to (else ValidationException)
        void removeBus(const std::string& busId);
    };

    // ── VoltageLevel ─────────────────────────────────────────────────────────
    static constexpr int TYPE = prop::VOLTAGE_LEVEL;

    VoltageLevel() = default;
    explicit VoltageLevel(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    double getNominalV() const;
    iidm::optional<double> getLowVoltageLimit() const;
    iidm::optional<double> getHighVoltageLimit() const;

    TopologyKind getTopologyKind() const;

    std::vector<Switch>        getSwitches() const;
    std::vector<BusbarSection> getBusbarSections() const;

    // Every equipment connected to it
    std::vector<Connectable> getConnectables() const;

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

    // ── Creation and removal ─────────────────────────────────────────────────
    GeneratorAdder newGenerator();
    LoadAdder      newLoad();
    // Removes the voltage level and its equipment; refused (ValidationException) while
    // branches connect it to other voltage levels
    void remove();

    // ── Extensions ───────────────────────────────────────────────────────
    // An extension's getter throws ExtensionNotFoundException when it is absent
    bool hasSlackTerminal() const;
    SlackTerminal getSlackTerminal() const;
    SlackTerminalAdder newSlackTerminal();
    void removeSlackTerminal();

    bool hasObservabilityArea() const;
    ObservabilityArea getObservabilityArea() const;
    void removeObservabilityArea();
};

} // namespace iidm
