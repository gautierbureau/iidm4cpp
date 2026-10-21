#pragma once

#include <iidm/Adder.h>
#include <iidm/BusbarSection.h>

namespace iidm {

// New busbar section of a node-breaker voltage level (VoltageLevel::NodeBreakerView::newBusbarSection())
class BusbarSectionAdder : public detail::ConnectableAdder<BusbarSectionAdder> {
public:
    BusbarSectionAdder(ObjectHandle adder, BackendProvider* backend) : ConnectableAdder(adder, backend) {}

    BusbarSectionAdder& setNode(int node) { return intField(prop::TERMINAL_NODE, node); }

    BusbarSection add() { return BusbarSection(create(), backend_); }
};

} // namespace iidm
