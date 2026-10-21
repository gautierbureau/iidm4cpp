#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The GeneratorShortCircuit extension of a generator (powsybl GeneratorShortCircuit, a
// ShortCircuitExtension): its reactances for short circuit studies. An object of its own, from
// Generator::getExtension<GeneratorShortCircuit>().
class GeneratorShortCircuit : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::GENERATOR_SHORT_CIRCUIT;

    GeneratorShortCircuit() = default;
    explicit GeneratorShortCircuit(ObjectHandle handle, BackendProvider* backend);

    // Direct-axis subtransient reactance (X''d)
    double getDirectSubtransX() const { return getDouble(prop::EXT_SC_DIRECT_SUBTRANS_X); }
    GeneratorShortCircuit& setDirectSubtransX(double directSubtransX) {
        return setDouble(*this, prop::EXT_SC_DIRECT_SUBTRANS_X, directSubtransX);
    }

    // Direct-axis transient reactance (X'd)
    double getDirectTransX() const { return getDouble(prop::EXT_SC_DIRECT_TRANS_X); }
    GeneratorShortCircuit& setDirectTransX(double directTransX) {
        return setDouble(*this, prop::EXT_SC_DIRECT_TRANS_X, directTransX);
    }

    // Reactance of a step-up transformer that is not modeled (NaN when none)
    double getStepUpTransformerX() const { return getDouble(prop::EXT_SC_STEP_UP_TRANSFORMER_X); }
    GeneratorShortCircuit& setStepUpTransformerX(double stepUpTransformerX) {
        return setDouble(*this, prop::EXT_SC_STEP_UP_TRANSFORMER_X, stepUpTransformerX);
    }
};

} // namespace iidm
