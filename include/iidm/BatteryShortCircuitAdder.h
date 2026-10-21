#pragma once

#include <iidm/Adder.h>
#include <iidm/BatteryShortCircuit.h>

namespace iidm {

// New BatteryShortCircuit extension of a battery (newBatteryShortCircuit())
class BatteryShortCircuitAdder : public detail::AdderBase<BatteryShortCircuitAdder> {
public:
    BatteryShortCircuitAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    BatteryShortCircuitAdder& withDirectSubtransX(double directSubtransX) { return doubleField(prop::EXT_SC_DIRECT_SUBTRANS_X, directSubtransX); }
    BatteryShortCircuitAdder& withDirectTransX(double directTransX) { return doubleField(prop::EXT_SC_DIRECT_TRANS_X, directTransX); }
    BatteryShortCircuitAdder& withStepUpTransformerX(double stepUpTransformerX) { return doubleField(prop::EXT_SC_STEP_UP_TRANSFORMER_X, stepUpTransformerX); }

    BatteryShortCircuit add() { return BatteryShortCircuit(create(), backend_); }
};

} // namespace iidm
