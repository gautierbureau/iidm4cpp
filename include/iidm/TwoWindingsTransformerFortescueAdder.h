#pragma once

#include <iidm/Adder.h>
#include <iidm/TwoWindingsTransformerFortescue.h>

namespace iidm {

// New TwoWindingsTransformerFortescue extension of a two windings transformer (newTwoWindingsTransformerFortescue())
class TwoWindingsTransformerFortescueAdder : public detail::AdderBase<TwoWindingsTransformerFortescueAdder> {
public:
    TwoWindingsTransformerFortescueAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    TwoWindingsTransformerFortescueAdder& withRz(double rz) { return doubleField(prop::EXT_TWO_WTF_RZ, rz); }
    TwoWindingsTransformerFortescueAdder& withXz(double xz) { return doubleField(prop::EXT_TWO_WTF_XZ, xz); }
    TwoWindingsTransformerFortescueAdder& withFreeFluxes(bool freeFluxes) { return boolField(prop::EXT_TWO_WTF_FREE_FLUXES, freeFluxes); }
    TwoWindingsTransformerFortescueAdder& withXm(double xm) { return doubleField(prop::EXT_TWO_WTF_XM, xm); }
    TwoWindingsTransformerFortescueAdder& withConnectionType1(WindingConnectionType connectionType1) { return intField(prop::EXT_TWO_WTF_CONNECTION_TYPE1, static_cast<int>(connectionType1)); }
    TwoWindingsTransformerFortescueAdder& withConnectionType2(WindingConnectionType connectionType2) { return intField(prop::EXT_TWO_WTF_CONNECTION_TYPE2, static_cast<int>(connectionType2)); }
    TwoWindingsTransformerFortescueAdder& withGroundingR1(double groundingR1) { return doubleField(prop::EXT_TWO_WTF_GROUNDING_R1, groundingR1); }
    TwoWindingsTransformerFortescueAdder& withGroundingX1(double groundingX1) { return doubleField(prop::EXT_TWO_WTF_GROUNDING_X1, groundingX1); }
    TwoWindingsTransformerFortescueAdder& withGroundingR2(double groundingR2) { return doubleField(prop::EXT_TWO_WTF_GROUNDING_R2, groundingR2); }
    TwoWindingsTransformerFortescueAdder& withGroundingX2(double groundingX2) { return doubleField(prop::EXT_TWO_WTF_GROUNDING_X2, groundingX2); }

    TwoWindingsTransformerFortescue add() { return TwoWindingsTransformerFortescue(create(), backend_); }
};

} // namespace iidm
