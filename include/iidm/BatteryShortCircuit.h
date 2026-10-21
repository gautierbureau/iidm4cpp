#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The BatteryShortCircuit extension of a battery (powsybl BatteryShortCircuit, a
// ShortCircuitExtension): its reactances for short circuit studies. An object of its own, from
// Battery::getExtension<BatteryShortCircuit>().
class BatteryShortCircuit : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::BATTERY_SHORT_CIRCUIT;

    BatteryShortCircuit() = default;
    explicit BatteryShortCircuit(ObjectHandle handle, BackendProvider* backend);

    // Direct-axis subtransient reactance (X''d)
    double getDirectSubtransX() const { return getDouble(prop::EXT_SC_DIRECT_SUBTRANS_X); }
    BatteryShortCircuit& setDirectSubtransX(double directSubtransX) {
        return setDouble(*this, prop::EXT_SC_DIRECT_SUBTRANS_X, directSubtransX);
    }

    // Direct-axis transient reactance (X'd)
    double getDirectTransX() const { return getDouble(prop::EXT_SC_DIRECT_TRANS_X); }
    BatteryShortCircuit& setDirectTransX(double directTransX) {
        return setDouble(*this, prop::EXT_SC_DIRECT_TRANS_X, directTransX);
    }

    // Reactance of a step-up transformer that is not modeled (NaN when none)
    double getStepUpTransformerX() const { return getDouble(prop::EXT_SC_STEP_UP_TRANSFORMER_X); }
    BatteryShortCircuit& setStepUpTransformerX(double stepUpTransformerX) {
        return setDouble(*this, prop::EXT_SC_STEP_UP_TRANSFORMER_X, stepUpTransformerX);
    }
};

} // namespace iidm
