#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The IdentifiableShortCircuit extension of an identifiable, a voltage level in general: the
// allowable peak short-circuit currents. An object of its own, from
// Identifiable::getExtension<IdentifiableShortCircuit>().
class IdentifiableShortCircuit : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::IDENTIFIABLE_SHORT_CIRCUIT;

    IdentifiableShortCircuit() = default;
    explicit IdentifiableShortCircuit(ObjectHandle handle, BackendProvider* backend);

    // Minimum allowable peak short-circuit current (A; NaN when undefined)
    double getIpMin() const { return getDouble(prop::EXT_ISC_IP_MIN); }
    IdentifiableShortCircuit& setIpMin(double ipMin) { return setDouble(*this, prop::EXT_ISC_IP_MIN, ipMin); }

    // Maximum allowable peak short-circuit current (A)
    double getIpMax() const { return getDouble(prop::EXT_ISC_IP_MAX); }
    IdentifiableShortCircuit& setIpMax(double ipMax) { return setDouble(*this, prop::EXT_ISC_IP_MAX, ipMax); }
};

} // namespace iidm
