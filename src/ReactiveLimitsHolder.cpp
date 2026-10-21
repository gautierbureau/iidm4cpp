#include <iidm/ReactiveLimitsHolder.h>
#include <iidm/Battery.h>
#include <iidm/BoundaryLine.h>
#include <iidm/BackendProvider.h>
#include <iidm/Generator.h>
#include <iidm/PropertyCodes.h>
#include <iidm/VscConverterStation.h>

namespace iidm { namespace detail {

template <class Self>
ReactiveLimits ReactiveLimitsHolderMethods<Self>::getReactiveLimits() const {
    return ReactiveLimits(backend()->getRelated(handle(), prop::REACTIVE_LIMITS), backend());
}

template <class Self>
MinMaxReactiveLimitsAdder ReactiveLimitsHolderMethods<Self>::newMinMaxReactiveLimits() {
    return MinMaxReactiveLimitsAdder(backend()->invoke(handle(), prop::MIN_MAX_REACTIVE_LIMITS), backend());
}

template <class Self>
ReactiveCapabilityCurveAdder ReactiveLimitsHolderMethods<Self>::newReactiveCapabilityCurve() {
    return ReactiveCapabilityCurveAdder(backend()->invoke(handle(), prop::REACTIVE_CAPABILITY_CURVE), backend());
}

template <class Self>
ReactiveCapabilityShapeAdder ReactiveLimitsHolderMethods<Self>::newReactiveCapabilityShape() {
    return ReactiveCapabilityShapeAdder(backend()->invoke(handle(), prop::REACTIVE_CAPABILITY_SHAPE), backend());
}

template class ReactiveLimitsHolderMethods<Generator>;
template class ReactiveLimitsHolderMethods<Battery>;
template class ReactiveLimitsHolderMethods<VscConverterStation>;
template class ReactiveLimitsHolderMethods<BoundaryLine::Generation>;

} } // namespace iidm::detail
