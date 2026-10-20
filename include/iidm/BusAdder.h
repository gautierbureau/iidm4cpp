#pragma once

#include <iidm/Adder.h>
#include <iidm/Bus.h>

namespace iidm {

// New bus of a bus-breaker voltage level (VoltageLevel::BusBreakerView::newBus())
class BusAdder : public detail::IdentifiableAdder<BusAdder> {
public:
    BusAdder(ObjectHandle adder, BackendProvider* backend) : IdentifiableAdder(adder, backend) {}

    Bus add() { return Bus(create(), backend_); }
};

} // namespace iidm
