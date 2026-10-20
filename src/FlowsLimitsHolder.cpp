#include <iidm/FlowsLimitsHolder.h>
#include <iidm/BackendProvider.h>
#include <iidm/DanglingLine.h>
#include <iidm/PropertyCodes.h>
#include <iidm/ThreeWindingsTransformer.h>
#include "Wrap.h"

namespace iidm { namespace detail {

template <class Self>
std::vector<OperationalLimitsGroup> FlowsLimitsHolderMethods<Self>::getOperationalLimitsGroups() const {
    return wrapAll<OperationalLimitsGroup>(backend()->getChildren(handle(), prop::OPERATIONAL_LIMITS_GROUP), backend());
}

template <class Self>
iidm::optional<OperationalLimitsGroup> FlowsLimitsHolderMethods<Self>::getSelectedOperationalLimitsGroup() const {
    return wrapOptional<OperationalLimitsGroup>(backend()->getRelated(handle(), prop::REL_SELECTED_OLG), backend());
}

template <class Self>
iidm::optional<CurrentLimits> FlowsLimitsHolderMethods<Self>::getCurrentLimits() const {
    return wrapOptional<CurrentLimits>(backend()->getRelated(handle(), prop::CURRENT_LIMITS), backend());
}

template class FlowsLimitsHolderMethods<ThreeWindingsTransformer::Leg>;
template class FlowsLimitsHolderMethods<DanglingLine>;

} } // namespace iidm::detail
