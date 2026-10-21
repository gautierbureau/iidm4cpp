#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/Switch.h>
#include <string>

namespace iidm {

// New switch of a node-breaker voltage level (VoltageLevel::NodeBreakerView::newSwitch(), newBreaker(),
// newDisconnector()), between two nodes
class NodeBreakerSwitchAdder : public detail::IdentifiableAdder<NodeBreakerSwitchAdder> {
public:
    NodeBreakerSwitchAdder(ObjectHandle adder, BackendProvider* backend) : IdentifiableAdder(adder, backend) {}

    NodeBreakerSwitchAdder& setNode1(int node1) { return intField(prop::SW_NODE1, node1); }
    NodeBreakerSwitchAdder& setNode2(int node2) { return intField(prop::SW_NODE2, node2); }
    NodeBreakerSwitchAdder& setKind(SwitchKind kind) { return intField(prop::SW_KIND, static_cast<int>(kind)); }
    NodeBreakerSwitchAdder& setOpen(bool open) { return boolField(prop::SW_OPEN, open); }
    // Kept in the bus-breaker view of the voltage level
    NodeBreakerSwitchAdder& setRetained(bool retained) { return boolField(prop::SW_RETAINED, retained); }

    Switch add() { return Switch(create(), backend_); }
};

// New switch of a bus-breaker voltage level (VoltageLevel::BusBreakerView::newSwitch()), between two buses
class BusBreakerSwitchAdder : public detail::IdentifiableAdder<BusBreakerSwitchAdder> {
public:
    BusBreakerSwitchAdder(ObjectHandle adder, BackendProvider* backend) : IdentifiableAdder(adder, backend) {}

    BusBreakerSwitchAdder& setBus1(const std::string& busId) { return stringField(prop::SW_BUS1, busId); }
    BusBreakerSwitchAdder& setBus2(const std::string& busId) { return stringField(prop::SW_BUS2, busId); }
    BusBreakerSwitchAdder& setOpen(bool open) { return boolField(prop::SW_OPEN, open); }

    Switch add() { return Switch(create(), backend_); }
};

// New internal connection of a node-breaker voltage level
// (VoltageLevel::NodeBreakerView::newInternalConnection()): a link between two nodes
class InternalConnectionAdder : public detail::AdderBase<InternalConnectionAdder> {
public:
    InternalConnectionAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    InternalConnectionAdder& setNode1(int node1) { return intField(prop::IC_NODE1, node1); }
    InternalConnectionAdder& setNode2(int node2) { return intField(prop::IC_NODE2, node2); }

    void add() { create(); }
};

} // namespace iidm
