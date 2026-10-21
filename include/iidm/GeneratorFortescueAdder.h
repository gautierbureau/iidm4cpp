#pragma once

#include <iidm/Adder.h>
#include <iidm/GeneratorFortescue.h>

namespace iidm {

// New GeneratorFortescue extension of a generator
class GeneratorFortescueAdder : public detail::AdderBase<GeneratorFortescueAdder> {
public:
    GeneratorFortescueAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    GeneratorFortescueAdder& withGrounded(bool grounded) { return boolField(prop::EXT_GF_GROUNDED, grounded); }
    GeneratorFortescueAdder& withRz(double rz) { return doubleField(prop::EXT_GF_RZ, rz); }
    GeneratorFortescueAdder& withXz(double xz) { return doubleField(prop::EXT_GF_XZ, xz); }
    GeneratorFortescueAdder& withRn(double rn) { return doubleField(prop::EXT_GF_RN, rn); }
    GeneratorFortescueAdder& withXn(double xn) { return doubleField(prop::EXT_GF_XN, xn); }
    GeneratorFortescueAdder& withGroundingR(double groundingR) { return doubleField(prop::EXT_GF_GROUNDING_R, groundingR); }
    GeneratorFortescueAdder& withGroundingX(double groundingX) { return doubleField(prop::EXT_GF_GROUNDING_X, groundingX); }

    GeneratorFortescue add() { return GeneratorFortescue(create(), backend_); }
};

} // namespace iidm
