#pragma once

#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/ObjectRef.h>
#include <iidm/ReactiveCapabilityCurve.h>
#include <iidm/ReactiveCapabilityShape.h>
#include <iidm/ReactiveLimits.h>

namespace iidm {
namespace detail {

// What has reactive limits (powsybl ReactiveLimitsHolder): generators, batteries, VSC converter
// stations.
template <class Self>
class ReactiveLimitsHolderMethods {
public:
    ReactiveLimits getReactiveLimits() const;
    // The reactive limits as a MinMaxReactiveLimits, ReactiveCapabilityCurve or
    // ReactiveCapabilityShape: ValidationException when they are of another kind
    template <class L> L getReactiveLimits() const {
        return L(backend()->getRelated(handle(), L::TYPE), backend());
    }

protected:
    ReactiveLimitsHolderMethods() = default;
    ~ReactiveLimitsHolderMethods() = default;

private:
    const Self& self() const { return static_cast<const Self&>(*this); }
    ObjectHandle handle() const { return Access::handle(self()); }
    BackendProvider* backend() const { return Access::backend(self()); }
};

} // namespace detail
} // namespace iidm
