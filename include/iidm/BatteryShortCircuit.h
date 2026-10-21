#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The BatteryShortCircuit extension of a battery (powsybl BatteryShortCircuit, a ShortCircuitExtension): its
// reactances for short circuit studies. An object of its own, from Battery::getBatteryShortCircuit().
class BatteryShortCircuit : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::BATTERY_SHORT_CIRCUIT;

    BatteryShortCircuit() = default;
    explicit BatteryShortCircuit(ObjectHandle handle, BackendProvider* backend);

    // Direct-axis subtransient reactance (X''d)
    double getDirectSubtransX() const;
    BatteryShortCircuit& setDirectSubtransX(double directSubtransX);

    // Direct-axis transient reactance (X'd)
    double getDirectTransX() const;
    BatteryShortCircuit& setDirectTransX(double directTransX);

    // Reactance of a step-up transformer that is not modeled (NaN when none)
    double getStepUpTransformerX() const;
    BatteryShortCircuit& setStepUpTransformerX(double stepUpTransformerX);
};

} // namespace iidm
