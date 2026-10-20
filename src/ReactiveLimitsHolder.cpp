#include <iidm/ReactiveLimitsHolder.h>
#include <iidm/Battery.h>
#include <iidm/BackendProvider.h>
#include <iidm/Generator.h>
#include <iidm/PropertyCodes.h>
#include <iidm/VscConverterStation.h>

namespace iidm { namespace detail {

template <class Self>
ReactiveLimits ReactiveLimitsHolderMethods<Self>::getReactiveLimits() const {
    return ReactiveLimits(backend()->getRelated(handle(), prop::REACTIVE_LIMITS), backend());
}

template class ReactiveLimitsHolderMethods<Generator>;
template class ReactiveLimitsHolderMethods<Battery>;
template class ReactiveLimitsHolderMethods<VscConverterStation>;

} } // namespace iidm::detail
