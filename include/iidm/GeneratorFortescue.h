#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The GeneratorFortescue extension of a generator: its sequence impedances (Ω) and its grounding,
// for asymmetrical studies. An object of its own, from
// Generator::getExtension<GeneratorFortescue>().
class GeneratorFortescue : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::GENERATOR_FORTESCUE;

    GeneratorFortescue() = default;
    explicit GeneratorFortescue(ObjectHandle handle, BackendProvider* backend);

    // Whether it is grounded
    bool isGrounded() const { return getBool(prop::EXT_GF_GROUNDED); }
    GeneratorFortescue& setGrounded(bool grounded) { return setBool(*this, prop::EXT_GF_GROUNDED, grounded); }

    // Zero-sequence resistance
    double getRz() const { return getDouble(prop::EXT_GF_RZ); }
    GeneratorFortescue& setRz(double rz) { return setDouble(*this, prop::EXT_GF_RZ, rz); }

    // Zero-sequence reactance
    double getXz() const { return getDouble(prop::EXT_GF_XZ); }
    GeneratorFortescue& setXz(double xz) { return setDouble(*this, prop::EXT_GF_XZ, xz); }

    // Negative-sequence resistance
    double getRn() const { return getDouble(prop::EXT_GF_RN); }
    GeneratorFortescue& setRn(double rn) { return setDouble(*this, prop::EXT_GF_RN, rn); }

    // Negative-sequence reactance
    double getXn() const { return getDouble(prop::EXT_GF_XN); }
    GeneratorFortescue& setXn(double xn) { return setDouble(*this, prop::EXT_GF_XN, xn); }

    // Resistance to ground, when grounded
    double getGroundingR() const { return getDouble(prop::EXT_GF_GROUNDING_R); }
    GeneratorFortescue& setGroundingR(double groundingR) {
        return setDouble(*this, prop::EXT_GF_GROUNDING_R, groundingR);
    }

    // Reactance to ground, when grounded
    double getGroundingX() const { return getDouble(prop::EXT_GF_GROUNDING_X); }
    GeneratorFortescue& setGroundingX(double groundingX) {
        return setDouble(*this, prop::EXT_GF_GROUNDING_X, groundingX);
    }
};

} // namespace iidm
