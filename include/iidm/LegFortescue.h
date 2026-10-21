#pragma once

#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// A leg of the ThreeWindingsTransformerFortescue extension of a three windings transformer: its
// zero-sequence impedance and winding (powsybl LegFortescue).
class LegFortescue : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::LEG_FORTESCUE;

    LegFortescue() = default;
    explicit LegFortescue(ObjectHandle handle, BackendProvider* backend);

    // Zero-sequence resistance (Ω)
    double getRz() const { return getDouble(prop::EXT_LEG_F_RZ); }
    LegFortescue& setRz(double rz) { return setDouble(*this, prop::EXT_LEG_F_RZ, rz); }

    // Zero-sequence reactance (Ω)
    double getXz() const { return getDouble(prop::EXT_LEG_F_XZ); }
    LegFortescue& setXz(double xz) { return setDouble(*this, prop::EXT_LEG_F_XZ, xz); }

    // Whether the magnetizing impedance is infinite (fluxes are free)
    bool isFreeFluxes() const { return getBool(prop::EXT_LEG_F_FREE_FLUXES); }
    LegFortescue& setFreeFluxes(bool freeFluxes) { return setBool(*this, prop::EXT_LEG_F_FREE_FLUXES, freeFluxes); }

    // How the winding is connected
    WindingConnectionType getConnectionType() const {
        return static_cast<WindingConnectionType>(getInt(prop::EXT_LEG_F_CONNECTION_TYPE));
    }
    LegFortescue& setConnectionType(WindingConnectionType connectionType) {
        return setInt(*this, prop::EXT_LEG_F_CONNECTION_TYPE, static_cast<int>(connectionType));
    }

    // Impedance to ground (Ω)
    double getGroundingR() const { return getDouble(prop::EXT_LEG_F_GROUNDING_R); }
    LegFortescue& setGroundingR(double groundingR) { return setDouble(*this, prop::EXT_LEG_F_GROUNDING_R, groundingR); }

    double getGroundingX() const { return getDouble(prop::EXT_LEG_F_GROUNDING_X); }
    LegFortescue& setGroundingX(double groundingX) { return setDouble(*this, prop::EXT_LEG_F_GROUNDING_X, groundingX); }
};

} // namespace iidm
