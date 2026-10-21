#include <iidm/FlowsLimitsHolder.h>
#include <iidm/BackendProvider.h>
#include <iidm/BoundaryLine.h>
#include <iidm/PropertyCodes.h>
#include <iidm/ThreeWindingsTransformer.h>
#include "Wrap.h"

namespace iidm { namespace detail {

template <class Self>
std::vector<OperationalLimitsGroup> FlowsLimitsHolderMethods<Self>::getOperationalLimitsGroups() const {
    return wrapAll<OperationalLimitsGroup>(backend()->getChildren(handle(), OperationalLimitsGroup::TYPE), backend());
}

template <class Self>
iidm::optional<OperationalLimitsGroup> FlowsLimitsHolderMethods<Self>::getOperationalLimitsGroup(const std::string& id) const {
    return wrapOptional<OperationalLimitsGroup>(backend()->getRelatedByString(handle(), OperationalLimitsGroup::TYPE, id), backend());
}

template <class Self>
OperationalLimitsGroup FlowsLimitsHolderMethods<Self>::newOperationalLimitsGroup(const std::string& id) {
    HandleRef adder(backend()->invoke(handle(), OperationalLimitsGroup::TYPE), backend());
    backend()->setString(adder, prop::OLG_ID, id);
    return OperationalLimitsGroup(backend()->invoke(adder, prop::ACT_ADD), backend());
}

template <class Self>
void FlowsLimitsHolderMethods<Self>::removeOperationalLimitsGroup(const std::string& id) {
    backend()->setString(handle(), prop::REMOVE_OLG, id);
}

template <class Self>
iidm::optional<std::string> FlowsLimitsHolderMethods<Self>::getSelectedOperationalLimitsGroupId() const {
    return backend()->getOptString(handle(), prop::SELECTED_OLG_ID);
}

template <class Self>
iidm::optional<OperationalLimitsGroup> FlowsLimitsHolderMethods<Self>::getSelectedOperationalLimitsGroup() const {
    return wrapOptional<OperationalLimitsGroup>(backend()->getRelated(handle(), prop::REL_SELECTED_OLG), backend());
}

template <class Self>
std::vector<std::string> FlowsLimitsHolderMethods<Self>::getAllSelectedOperationalLimitsGroupIds() const {
    return backend()->getStringList(handle(), prop::ALL_SELECTED_OLG_IDS);
}

template <class Self>
std::vector<std::string> FlowsLimitsHolderMethods<Self>::getAllSelectedOperationalLimitsGroupIdsOrdered() const {
    return backend()->getStringList(handle(), prop::ALL_SELECTED_OLG_IDS_ORDERED);
}

template <class Self>
std::vector<OperationalLimitsGroup> FlowsLimitsHolderMethods<Self>::getAllSelectedOperationalLimitsGroups() const {
    return wrapAll<OperationalLimitsGroup>(backend()->getChildren(handle(), prop::REL_SELECTED_OLG), backend());
}

template <class Self>
Self& FlowsLimitsHolderMethods<Self>::setSelectedOperationalLimitsGroup(const std::string& id) {
    backend()->setString(handle(), prop::SELECTED_OLG_ID, id);
    return self();
}

template <class Self>
void FlowsLimitsHolderMethods<Self>::addSelectedOperationalLimitsGroups(const std::vector<std::string>& ids) {
    for (const std::string& id : ids) backend()->setString(handle(), prop::ADD_SELECTED_OLG, id);
}

template <class Self>
void FlowsLimitsHolderMethods<Self>::addSelectedOperationalLimitsGroupByPredicate(
        const std::function<bool(const std::string&)>& idPredicate) {
    std::vector<std::string> ids;
    for (const OperationalLimitsGroup& group : getOperationalLimitsGroups()) {
        std::string id = group.getId();
        if (idPredicate(id)) ids.push_back(id);
    }
    addSelectedOperationalLimitsGroups(ids);
}

template <class Self>
void FlowsLimitsHolderMethods<Self>::deselectOperationalLimitsGroups(const std::vector<std::string>& ids) {
    for (const std::string& id : ids) backend()->setString(handle(), prop::DESELECT_OLG, id);
}

template <class Self>
void FlowsLimitsHolderMethods<Self>::cancelSelectedOperationalLimitsGroup() {
    backend()->setRelated(handle(), prop::REL_SELECTED_OLG, INVALID_HANDLE);
}

template <class Self>
OperationalLimitsGroup FlowsLimitsHolderMethods<Self>::getOrCreateSelectedOperationalLimitsGroup() {
    HandleRef call(backend()->invoke(handle(), prop::REL_SELECTED_OLG), backend());
    return OperationalLimitsGroup(backend()->invoke(call, prop::ACT_ADD), backend());
}

template <class Self>
OperationalLimitsGroup FlowsLimitsHolderMethods<Self>::getOrCreateSelectedOperationalLimitsGroup(const std::string& id) {
    HandleRef call(backend()->invoke(handle(), prop::REL_SELECTED_OLG), backend());
    backend()->setString(call, prop::OLG_ID, id);
    return OperationalLimitsGroup(backend()->invoke(call, prop::ACT_ADD), backend());
}

template <class Self>
iidm::optional<CurrentLimits> FlowsLimitsHolderMethods<Self>::getCurrentLimits() const {
    return wrapOptional<CurrentLimits>(backend()->getRelated(handle(), CurrentLimits::TYPE), backend());
}

template <class Self>
iidm::optional<ActivePowerLimits> FlowsLimitsHolderMethods<Self>::getActivePowerLimits() const {
    return wrapOptional<ActivePowerLimits>(backend()->getRelated(handle(), ActivePowerLimits::TYPE), backend());
}

template <class Self>
iidm::optional<ApparentPowerLimits> FlowsLimitsHolderMethods<Self>::getApparentPowerLimits() const {
    return wrapOptional<ApparentPowerLimits>(backend()->getRelated(handle(), ApparentPowerLimits::TYPE), backend());
}

template <class Self>
std::vector<CurrentLimits> FlowsLimitsHolderMethods<Self>::getAllSelectedCurrentLimits() const {
    return wrapAll<CurrentLimits>(backend()->getChildren(handle(), CurrentLimits::TYPE), backend());
}

template <class Self>
std::vector<ActivePowerLimits> FlowsLimitsHolderMethods<Self>::getAllSelectedActivePowerLimits() const {
    return wrapAll<ActivePowerLimits>(backend()->getChildren(handle(), ActivePowerLimits::TYPE), backend());
}

template <class Self>
std::vector<ApparentPowerLimits> FlowsLimitsHolderMethods<Self>::getAllSelectedApparentPowerLimits() const {
    return wrapAll<ApparentPowerLimits>(backend()->getChildren(handle(), ApparentPowerLimits::TYPE), backend());
}

template <class Self>
iidm::optional<CurrentLimits> FlowsLimitsHolderMethods<Self>::getCurrentLimitsFromId(const std::string& id) const {
    return wrapOptional<CurrentLimits>(backend()->getRelatedByString(handle(), CurrentLimits::TYPE, id), backend());
}

template <class Self>
iidm::optional<ActivePowerLimits> FlowsLimitsHolderMethods<Self>::getActivePowerLimitsFromId(const std::string& id) const {
    return wrapOptional<ActivePowerLimits>(backend()->getRelatedByString(handle(), ActivePowerLimits::TYPE, id), backend());
}

template <class Self>
iidm::optional<ApparentPowerLimits> FlowsLimitsHolderMethods<Self>::getApparentPowerLimitsFromId(const std::string& id) const {
    return wrapOptional<ApparentPowerLimits>(backend()->getRelatedByString(handle(), ApparentPowerLimits::TYPE, id), backend());
}

template <class Self>
iidm::optional<LoadingLimits> FlowsLimitsHolderMethods<Self>::limits(LimitType type) const {
    return wrapOptional<LoadingLimits>(
        backend()->getRelatedByIndex(handle(), LoadingLimits::TYPE, static_cast<int>(type)), backend());
}

template <class Self>
std::vector<LoadingLimits> FlowsLimitsHolderMethods<Self>::allSelectedLimits(LimitType type) const {
    switch (type) {
        case LimitType::CURRENT:        return wrapAll<LoadingLimits>(backend()->getChildren(handle(), CurrentLimits::TYPE), backend());
        case LimitType::ACTIVE_POWER:   return wrapAll<LoadingLimits>(backend()->getChildren(handle(), ActivePowerLimits::TYPE), backend());
        case LimitType::APPARENT_POWER: return wrapAll<LoadingLimits>(backend()->getChildren(handle(), ApparentPowerLimits::TYPE), backend());
        default: break;
    }
    // Same failure as the backends' for these limits
    throw PropertyNotFoundException("No loading limits of type " + std::to_string(static_cast<int>(type)));
}

template class FlowsLimitsHolderMethods<ThreeWindingsTransformer::Leg>;
template class FlowsLimitsHolderMethods<BoundaryLine>;
template class FlowsLimitsHolderMethods<BranchSideLimits>;

} } // namespace iidm::detail
