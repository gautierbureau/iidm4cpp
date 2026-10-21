#pragma once

#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The TwoWindingsTransformerFortescue extension of a two windings transformer: its zero-sequence
// impedance, magnetizing reactance and windings, for asymmetrical studies. An object of its own,
// from TwoWindingsTransformer::getTwoWindingsTransformerFortescue().
class TwoWindingsTransformerFortescue : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::TWO_WINDINGS_TRANSFORMER_FORTESCUE;

    TwoWindingsTransformerFortescue() = default;
    explicit TwoWindingsTransformerFortescue(ObjectHandle handle, BackendProvider* backend);

    // Zero-sequence resistance (Ω)
    double getRz() const;
    TwoWindingsTransformerFortescue& setRz(double rz);

    // Zero-sequence reactance (Ω)
    double getXz() const;
    TwoWindingsTransformerFortescue& setXz(double xz);

    // Whether the magnetizing impedance is infinite (fluxes are free)
    bool isFreeFluxes() const;
    TwoWindingsTransformerFortescue& setFreeFluxes(bool freeFluxes);

    // Magnetizing reactance (Ω), when not infinite
    double getXm() const;
    TwoWindingsTransformerFortescue& setXm(double xm);

    // How the winding of each side is connected
    WindingConnectionType getConnectionType1() const;
    TwoWindingsTransformerFortescue& setConnectionType1(WindingConnectionType connectionType1);

    WindingConnectionType getConnectionType2() const;
    TwoWindingsTransformerFortescue& setConnectionType2(WindingConnectionType connectionType2);

    // Impedance to ground of each side (Ω)
    double getGroundingR1() const;
    TwoWindingsTransformerFortescue& setGroundingR1(double groundingR1);

    double getGroundingX1() const;
    TwoWindingsTransformerFortescue& setGroundingX1(double groundingX1);

    double getGroundingR2() const;
    TwoWindingsTransformerFortescue& setGroundingR2(double groundingR2);

    double getGroundingX2() const;
    TwoWindingsTransformerFortescue& setGroundingX2(double groundingX2);
};

} // namespace iidm
