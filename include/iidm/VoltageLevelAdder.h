#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/VoltageLevel.h>

namespace iidm {

// New voltage level of a substation (Substation::newVoltageLevel()) or of a network, without
// substation (Network::newVoltageLevel())
class VoltageLevelAdder : public detail::IdentifiableAdder<VoltageLevelAdder> {
public:
    VoltageLevelAdder(ObjectHandle adder, BackendProvider* backend) : IdentifiableAdder(adder, backend) {}

    VoltageLevelAdder& setNominalV(double nominalV)                 { return doubleField(prop::VL_NOMINAL_V, nominalV); }
    VoltageLevelAdder& setLowVoltageLimit(double lowVoltageLimit)   { return doubleField(prop::VL_LOW_VOLTAGE_LIMIT, lowVoltageLimit); }
    VoltageLevelAdder& setHighVoltageLimit(double highVoltageLimit) { return doubleField(prop::VL_HIGH_VOLTAGE_LIMIT, highVoltageLimit); }
    VoltageLevelAdder& setTopologyKind(TopologyKind topologyKind)   { return intField(prop::VL_TOPOLOGY_KIND, static_cast<int>(topologyKind)); }

    VoltageLevel add() { return VoltageLevel(create(), backend_); }
};

} // namespace iidm
