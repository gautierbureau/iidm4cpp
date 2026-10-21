#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/ThreeWindingsTransformerFortescue.h>

namespace iidm {

// New ThreeWindingsTransformerFortescue extension of a three windings transformer
// (newThreeWindingsTransformerFortescue()). As in powsybl, leg1(), leg2() or leg3() selects the leg
// that the next with*() set: newThreeWindingsTransformerFortescue().leg1().withRz(1).leg2().withRz(2).add()
class ThreeWindingsTransformerFortescueAdder : public detail::AdderBase<ThreeWindingsTransformerFortescueAdder> {
public:
    ThreeWindingsTransformerFortescueAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    ThreeWindingsTransformerFortescueAdder& leg1() { return intField(prop::EXT_THREE_WTF_LEG, static_cast<int>(ThreeSides::ONE)); }
    ThreeWindingsTransformerFortescueAdder& leg2() { return intField(prop::EXT_THREE_WTF_LEG, static_cast<int>(ThreeSides::TWO)); }
    ThreeWindingsTransformerFortescueAdder& leg3() { return intField(prop::EXT_THREE_WTF_LEG, static_cast<int>(ThreeSides::THREE)); }

    ThreeWindingsTransformerFortescueAdder& withRz(double rz) { return doubleField(prop::EXT_LEG_F_RZ, rz); }
    ThreeWindingsTransformerFortescueAdder& withXz(double xz) { return doubleField(prop::EXT_LEG_F_XZ, xz); }
    ThreeWindingsTransformerFortescueAdder& withFreeFluxes(bool freeFluxes) { return boolField(prop::EXT_LEG_F_FREE_FLUXES, freeFluxes); }
    ThreeWindingsTransformerFortescueAdder& withConnectionType(WindingConnectionType connectionType) {
        return intField(prop::EXT_LEG_F_CONNECTION_TYPE, static_cast<int>(connectionType));
    }
    ThreeWindingsTransformerFortescueAdder& withGroundingR(double groundingR) { return doubleField(prop::EXT_LEG_F_GROUNDING_R, groundingR); }
    ThreeWindingsTransformerFortescueAdder& withGroundingX(double groundingX) { return doubleField(prop::EXT_LEG_F_GROUNDING_X, groundingX); }

    ThreeWindingsTransformerFortescue add() { return ThreeWindingsTransformerFortescue(create(), backend_); }
};

} // namespace iidm
