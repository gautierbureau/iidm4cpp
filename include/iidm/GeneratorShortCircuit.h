#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The GeneratorShortCircuit extension of a generator (powsybl GeneratorShortCircuit, a ShortCircuitExtension): its
// reactances for short circuit studies. An object of its own, from Generator::getGeneratorShortCircuit().
class GeneratorShortCircuit : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::GENERATOR_SHORT_CIRCUIT;

    GeneratorShortCircuit() = default;
    explicit GeneratorShortCircuit(ObjectHandle handle, BackendProvider* backend);

    // Direct-axis subtransient reactance (X''d)
    double getDirectSubtransX() const;
    GeneratorShortCircuit& setDirectSubtransX(double directSubtransX);

    // Direct-axis transient reactance (X'd)
    double getDirectTransX() const;
    GeneratorShortCircuit& setDirectTransX(double directTransX);

    // Reactance of a step-up transformer that is not modeled (NaN when none)
    double getStepUpTransformerX() const;
    GeneratorShortCircuit& setStepUpTransformerX(double stepUpTransformerX);
};

} // namespace iidm
