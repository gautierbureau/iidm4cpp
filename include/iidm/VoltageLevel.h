#pragma once

#include <iidm/Connectable.h>
#include <iidm/Container.h>
#include <iidm/Enums.h>
#include <iidm/Identifiable.h>
#include <iidm/Bus.h>
#include <iidm/BusAdder.h>
#include <iidm/GeneratorAdder.h>
#include <iidm/GroundAdder.h>
#include <iidm/LoadAdder.h>
#include <iidm/SlackTerminal.h>
#include <iidm/SlackTerminalAdder.h>
#include <iidm/BusbarSection.h>
#include <iidm/Switch.h>
#include <iidm/InternalConnection.h>
#include <iidm/Terminal.h>
#include <iidm/stdcxx/optional.hpp>
#include <iosfwd>
#include <string>
#include <vector>
#include <iidm/ObservabilityArea.h>

namespace iidm {

class Generator;
class Load;
class Battery;
class ShuntCompensator;
class StaticVarCompensator;
class BoundaryLine;
class LccConverterStation;
class VscConverterStation;
class Line;
class TwoWindingsTransformer;
class ThreeWindingsTransformer;
class Substation;
class Ground;
class Area;

class VoltageLevel : public Identifiable, public detail::ContainerMethods<VoltageLevel> {
public:
    // ── NodeBreakerView ──────────────────────────────────────────────────────
    class NodeBreakerView : public detail::ObjectRef {
    public:
        NodeBreakerView() = default;
        explicit NodeBreakerView(ObjectHandle vlHandle, BackendProvider* backend);

        std::vector<int> getNodes() const;
        int getMaximumNodeIndex() const;
        // The terminal at a node (empty without one)
        iidm::optional<Terminal> getTerminal(int node) const;
        // Whether an equipment, a switch or an internal connection is at a node
        bool hasAttachedEquipment(int node) const;

        // Fictitious injection at a node, load sign convention (0 when none)
        double getFictitiousP0(int node) const;
        NodeBreakerView& setFictitiousP0(int node, double p0);
        double getFictitiousQ0(int node) const;
        NodeBreakerView& setFictitiousQ0(int node, double q0);

        std::vector<Switch> getSwitches() const;
        int getSwitchCount() const;
        iidm::optional<Switch> getSwitch(const std::string& id) const;
        // The switches at a node
        std::vector<Switch> getSwitches(int node) const;
        // The nodes of a switch, the terminals there (ValidationException when it has no such switch)
        int getNode1(const std::string& switchId) const;
        int getNode2(const std::string& switchId) const;
        iidm::optional<Terminal> getTerminal1(const std::string& switchId) const;
        iidm::optional<Terminal> getTerminal2(const std::string& switchId) const;
        void removeSwitch(const std::string& switchId);

        std::vector<BusbarSection> getBusbarSections() const;
        int getBusbarSectionCount() const;
        iidm::optional<BusbarSection> getBusbarSection(const std::string& id) const;

        std::vector<InternalConnection> getInternalConnections() const;
        int getInternalConnectionCount() const;
        std::vector<int> getNodesInternalConnectedTo(int node) const;
        // Removes every internal connection between two nodes
        void removeInternalConnections(int node1, int node2);
    };

    // ── BusBreakerView ───────────────────────────────────────────────────────
    class BusBreakerView : public detail::ObjectRef {
    public:
        BusBreakerView() = default;
        explicit BusBreakerView(ObjectHandle vlHandle, BackendProvider* backend);

        std::vector<Bus> getBuses() const;
        int getBusCount() const;
        // Empty when it has no bus with this id
        iidm::optional<Bus> getBus(const std::string& busId) const;
        // The buses of this view that make a bus of the bus view (sorted by id)
        std::vector<Bus> getBusesFromBusViewBusId(const std::string& busViewBusId) const;
        BusAdder newBus();
        // Removes a bus that nothing is connected to (else ValidationException)
        void removeBus(const std::string& busId);
        // Removes every bus, when no switch and no equipment is left (else ValidationException)
        void removeAllBuses();

        std::vector<Switch> getSwitches() const;
        int getSwitchCount() const;
        iidm::optional<Switch> getSwitch(const std::string& id) const;
        Bus getBus1(const std::string& switchId) const;
        Bus getBus2(const std::string& switchId) const;
        void removeSwitch(const std::string& switchId);
        void removeAllSwitches();
    };

    // ── BusView ──────────────────────────────────────────────────────────────
    // The buses that the closed switches make of its topology
    class BusView : public detail::ObjectRef {
    public:
        BusView() = default;
        explicit BusView(ObjectHandle vlHandle, BackendProvider* backend) : ObjectRef(vlHandle, backend) {}

        std::vector<Bus> getBuses() const;
        // Empty when there is no such bus
        iidm::optional<Bus> getBus(const std::string& id) const;
        // The bus that holds a configured bus (bus-breaker topology) or a busbar section
        // (node-breaker topology); empty when it is in none (PropertyNotFoundException for an
        // unknown id)
        iidm::optional<Bus> getMergedBus(const std::string& configuredBusId) const;
    };

    // ── VoltageLevel ─────────────────────────────────────────────────────────
    static constexpr int TYPE = prop::VOLTAGE_LEVEL;

    VoltageLevel() = default;
    explicit VoltageLevel(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    // Empty when it is not in a substation
    iidm::optional<Substation> getSubstation() const;

    double getNominalV() const;
    VoltageLevel& setNominalV(double nominalV);
    iidm::optional<double> getLowVoltageLimit() const;
    // NaN: none
    VoltageLevel& setLowVoltageLimit(double lowVoltageLimit);
    iidm::optional<double> getHighVoltageLimit() const;
    VoltageLevel& setHighVoltageLimit(double highVoltageLimit);

    TopologyKind getTopologyKind() const;
    // To bus-breaker topology (powsybl cannot convert to node-breaker yet)
    void convertToTopology(TopologyKind topologyKind);
    // Its topology as text, as powsybl prints it
    void printTopology() const;
    void printTopology(std::ostream& out) const;

    std::vector<Switch>        getSwitches() const;
    std::vector<BusbarSection> getBusbarSections() const;

    // Every equipment connected to it; one of them (empty when it has no such equipment)
    std::vector<Connectable> getConnectables() const;
    iidm::optional<Connectable> getConnectable(const std::string& id) const;
    int getConnectableCount() const;

    // ── Per-injector getters ─────────────────────────────────────────────────
    std::vector<Generator>            getGenerators() const;
    std::vector<Load>                 getLoads() const;
    std::vector<Battery>              getBatteries() const;
    std::vector<ShuntCompensator>     getShuntCompensators() const;
    std::vector<StaticVarCompensator> getStaticVarCompensators() const;
    std::vector<BoundaryLine>         getBoundaryLines(BoundaryLineFilter filter = BoundaryLineFilter::ALL) const;
    std::vector<LccConverterStation>  getLccConverterStations() const;
    std::vector<VscConverterStation>  getVscConverterStations() const;
    std::vector<Line>                 getLines() const;
    std::vector<TwoWindingsTransformer> getTwoWindingsTransformers() const;
    std::vector<ThreeWindingsTransformer> getThreeWindingsTransformers() const;
    std::vector<Ground>               getGrounds() const;

    // ── Counts ───────────────────────────────────────────────────────────────
    int getGeneratorCount() const;
    int getLoadCount() const;
    int getBatteryCount() const;
    int getShuntCompensatorCount() const;
    int getStaticVarCompensatorCount() const;
    int getBoundaryLineCount() const;
    int getLccConverterStationCount() const;
    int getVscConverterStationCount() const;
    int getLineCount() const;
    int getTwoWindingsTransformerCount() const;
    int getThreeWindingsTransformerCount() const;
    int getSwitchCount() const;
    int getGroundCount() const;

    NodeBreakerView getNodeBreakerView() const;
    BusBreakerView  getBusBreakerView()  const;
    BusView         getBusView()         const;

    // ── Areas (include <iidm/Area.h> to use them) ────────────────────────────
    std::vector<Area> getAreas() const;
    // Its area of a type (empty without one)
    iidm::optional<Area> getArea(const std::string& areaType) const;
    void addArea(const Area& area);
    void removeArea(const Area& area);

    // ── Creation and removal ─────────────────────────────────────────────────
    GeneratorAdder newGenerator();
    LoadAdder      newLoad();
    GroundAdder    newGround();
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
