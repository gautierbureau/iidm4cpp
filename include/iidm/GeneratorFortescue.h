#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The GeneratorFortescue extension of a generator: its sequence impedances (Ω) and its
// grounding, for asymmetrical studies. An object of its own, from Generator::getGeneratorFortescue().
class GeneratorFortescue : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::GENERATOR_FORTESCUE;

    GeneratorFortescue() = default;
    explicit GeneratorFortescue(ObjectHandle handle, BackendProvider* backend);

    // Whether it is grounded
    bool isGrounded() const;
    GeneratorFortescue& setGrounded(bool grounded);

    // Zero-sequence resistance
    double getRz() const;
    GeneratorFortescue& setRz(double rz);

    // Zero-sequence reactance
    double getXz() const;
    GeneratorFortescue& setXz(double xz);

    // Negative-sequence resistance
    double getRn() const;
    GeneratorFortescue& setRn(double rn);

    // Negative-sequence reactance
    double getXn() const;
    GeneratorFortescue& setXn(double xn);

    // Resistance to ground, when grounded
    double getGroundingR() const;
    GeneratorFortescue& setGroundingR(double groundingR);

    // Reactance to ground, when grounded
    double getGroundingX() const;
    GeneratorFortescue& setGroundingX(double groundingX);
};

} // namespace iidm
