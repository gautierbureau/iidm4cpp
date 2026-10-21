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
    double getRz() const { return getDouble(prop::EXT_TWO_WTF_RZ); }
    TwoWindingsTransformerFortescue& setRz(double rz) { return setDouble(*this, prop::EXT_TWO_WTF_RZ, rz); }

    // Zero-sequence reactance (Ω)
    double getXz() const { return getDouble(prop::EXT_TWO_WTF_XZ); }
    TwoWindingsTransformerFortescue& setXz(double xz) { return setDouble(*this, prop::EXT_TWO_WTF_XZ, xz); }

    // Whether the magnetizing impedance is infinite (fluxes are free)
    bool isFreeFluxes() const { return getBool(prop::EXT_TWO_WTF_FREE_FLUXES); }
    TwoWindingsTransformerFortescue& setFreeFluxes(bool freeFluxes) {
        return setBool(*this, prop::EXT_TWO_WTF_FREE_FLUXES, freeFluxes);
    }

    // Magnetizing reactance (Ω), when not infinite
    double getXm() const { return getDouble(prop::EXT_TWO_WTF_XM); }
    TwoWindingsTransformerFortescue& setXm(double xm) { return setDouble(*this, prop::EXT_TWO_WTF_XM, xm); }

    // How the winding of each side is connected
    WindingConnectionType getConnectionType1() const {
        return static_cast<WindingConnectionType>(getInt(prop::EXT_TWO_WTF_CONNECTION_TYPE1));
    }
    TwoWindingsTransformerFortescue& setConnectionType1(WindingConnectionType connectionType1) {
        return setInt(*this, prop::EXT_TWO_WTF_CONNECTION_TYPE1, static_cast<int>(connectionType1));
    }

    WindingConnectionType getConnectionType2() const {
        return static_cast<WindingConnectionType>(getInt(prop::EXT_TWO_WTF_CONNECTION_TYPE2));
    }
    TwoWindingsTransformerFortescue& setConnectionType2(WindingConnectionType connectionType2) {
        return setInt(*this, prop::EXT_TWO_WTF_CONNECTION_TYPE2, static_cast<int>(connectionType2));
    }

    // Impedance to ground of each side (Ω)
    double getGroundingR1() const { return getDouble(prop::EXT_TWO_WTF_GROUNDING_R1); }
    TwoWindingsTransformerFortescue& setGroundingR1(double groundingR1) {
        return setDouble(*this, prop::EXT_TWO_WTF_GROUNDING_R1, groundingR1);
    }

    double getGroundingX1() const { return getDouble(prop::EXT_TWO_WTF_GROUNDING_X1); }
    TwoWindingsTransformerFortescue& setGroundingX1(double groundingX1) {
        return setDouble(*this, prop::EXT_TWO_WTF_GROUNDING_X1, groundingX1);
    }

    double getGroundingR2() const { return getDouble(prop::EXT_TWO_WTF_GROUNDING_R2); }
    TwoWindingsTransformerFortescue& setGroundingR2(double groundingR2) {
        return setDouble(*this, prop::EXT_TWO_WTF_GROUNDING_R2, groundingR2);
    }

    double getGroundingX2() const { return getDouble(prop::EXT_TWO_WTF_GROUNDING_X2); }
    TwoWindingsTransformerFortescue& setGroundingX2(double groundingX2) {
        return setDouble(*this, prop::EXT_TWO_WTF_GROUNDING_X2, groundingX2);
    }
};

} // namespace iidm
