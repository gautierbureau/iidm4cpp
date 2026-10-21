#pragma once

#include <iidm/Adder.h>
#include <iidm/GeneratorShortCircuit.h>

namespace iidm {

// New GeneratorShortCircuit extension of a generator (newGeneratorShortCircuit())
class GeneratorShortCircuitAdder : public detail::AdderBase<GeneratorShortCircuitAdder> {
public:
    GeneratorShortCircuitAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    GeneratorShortCircuitAdder& withDirectSubtransX(double directSubtransX) { return doubleField(prop::EXT_SC_DIRECT_SUBTRANS_X, directSubtransX); }
    GeneratorShortCircuitAdder& withDirectTransX(double directTransX) { return doubleField(prop::EXT_SC_DIRECT_TRANS_X, directTransX); }
    GeneratorShortCircuitAdder& withStepUpTransformerX(double stepUpTransformerX) { return doubleField(prop::EXT_SC_STEP_UP_TRANSFORMER_X, stepUpTransformerX); }

    GeneratorShortCircuit add() { return GeneratorShortCircuit(create(), backend_); }
};

} // namespace iidm
