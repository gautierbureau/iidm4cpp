#include <iidm/Branch.h>
#include <iidm/BackendProvider.h>
#include <iidm/Line.h>
#include <iidm/TwoWindingsTransformer.h>
#include "Extensions.h"
#include "Wrap.h"

namespace iidm { namespace detail {

template <class Self>
Terminal BranchMethods<Self>::getTerminal1() const {
    return Terminal(backend()->getRelated(handle(), prop::REL_TERMINAL1), backend());
}

template <class Self>
Terminal BranchMethods<Self>::getTerminal2() const {
    return Terminal(backend()->getRelated(handle(), prop::REL_TERMINAL2), backend());
}

template <class Self>
Terminal BranchMethods<Self>::getTerminal(TwoSides side) const {
    return side == TwoSides::ONE ? getTerminal1() : getTerminal2();
}

template <class Self>
Terminal BranchMethods<Self>::getTerminal(const std::string& voltageLevelId) const {
    return Terminal(backend()->getRelatedByString(handle(), prop::REL_TERMINAL, voltageLevelId), backend());
}

template <class Self>
iidm::optional<CurrentLimits> BranchMethods<Self>::getCurrentLimits1() const {
    return wrapOptional<CurrentLimits>(backend()->getRelated(handle(), prop::REL_CURRENT_LIMITS1), backend());
}

template <class Self>
iidm::optional<CurrentLimits> BranchMethods<Self>::getCurrentLimits2() const {
    return wrapOptional<CurrentLimits>(backend()->getRelated(handle(), prop::REL_CURRENT_LIMITS2), backend());
}

template <class Self>
std::vector<OperationalLimitsGroup> BranchMethods<Self>::getOperationalLimitsGroups1() const {
    return wrapAll<OperationalLimitsGroup>(backend()->getChildren(handle(), prop::OLG_SIDE1), backend());
}

template <class Self>
std::vector<OperationalLimitsGroup> BranchMethods<Self>::getOperationalLimitsGroups2() const {
    return wrapAll<OperationalLimitsGroup>(backend()->getChildren(handle(), prop::OLG_SIDE2), backend());
}

template <class Self>
iidm::optional<OperationalLimitsGroup> BranchMethods<Self>::getSelectedOperationalLimitsGroup1() const {
    return wrapOptional<OperationalLimitsGroup>(backend()->getRelated(handle(), prop::REL_SELECTED_OLG1), backend());
}

template <class Self>
iidm::optional<OperationalLimitsGroup> BranchMethods<Self>::getSelectedOperationalLimitsGroup2() const {
    return wrapOptional<OperationalLimitsGroup>(backend()->getRelated(handle(), prop::REL_SELECTED_OLG2), backend());
}

template <class Self>
bool BranchMethods<Self>::hasBranchObservability() const {
    return hasExtension<BranchObservability>(backend(), handle());
}

template <class Self>
BranchObservability BranchMethods<Self>::getBranchObservability() const {
    return getExtension<BranchObservability>(backend(), handle(), "BranchObservability");
}

template <class Self>
BranchObservabilityAdder BranchMethods<Self>::newBranchObservability() {
    return newExtension<BranchObservability, BranchObservabilityAdder>(backend(), handle());
}

template <class Self>
void BranchMethods<Self>::removeBranchObservability() {
    removeExtension<BranchObservability>(backend(), handle());
}

template class BranchMethods<Branch>;
template class BranchMethods<Line>;
template class BranchMethods<TwoWindingsTransformer>;

} } // namespace iidm::detail
