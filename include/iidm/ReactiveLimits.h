#pragma once

#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>
#include <iidm/PropertiesHolder.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

// The reactive power limits of a generator, battery or VSC converter station, of one of three
// kinds: MinMaxReactiveLimits, ReactiveCapabilityCurve, ReactiveCapabilityShape. getKind() or
// is<T>() tell which, as<T>() gives them as that kind.
class ReactiveLimits : public detail::ObjectRef, public detail::PropertiesHolder<ReactiveLimits> {
public:
    static constexpr int TYPE = prop::REACTIVE_LIMITS;

    ReactiveLimits() = default;
    explicit ReactiveLimits(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    ReactiveLimitsKind getKind() const;

    // The limits at an active power
    double getMinQ(double p) const;
    double getMaxQ(double p) const;
};

} // namespace iidm
