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
    double getRz() const;
    LegFortescue& setRz(double rz);

    // Zero-sequence reactance (Ω)
    double getXz() const;
    LegFortescue& setXz(double xz);

    // Whether the magnetizing impedance is infinite (fluxes are free)
    bool isFreeFluxes() const;
    LegFortescue& setFreeFluxes(bool freeFluxes);

    // How the winding is connected
    WindingConnectionType getConnectionType() const;
    LegFortescue& setConnectionType(WindingConnectionType connectionType);

    // Impedance to ground (Ω)
    double getGroundingR() const;
    LegFortescue& setGroundingR(double groundingR);

    double getGroundingX() const;
    LegFortescue& setGroundingX(double groundingX);
};

} // namespace iidm
