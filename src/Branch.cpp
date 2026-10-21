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
TwoSides BranchMethods<Self>::getSide(const Terminal& terminal) const {
    if (getTerminal1() == terminal) return TwoSides::ONE;
    if (getTerminal2() == terminal) return TwoSides::TWO;
    throw ValidationException("The terminal is not connected to branch " + self().getId());
}

template <class Self>
BranchSideLimits BranchMethods<Self>::sideLimits(TwoSides side) const {
    return BranchSideLimits(backend()->getRelatedByIndex(handle(), prop::FLOWS_LIMITS_HOLDER, static_cast<int>(side)), backend());
}

// ── Limit groups ─────────────────────────────────────────────────────────────

template <class Self>
std::vector<OperationalLimitsGroup> BranchMethods<Self>::getOperationalLimitsGroups1() const {
    return sideLimits(TwoSides::ONE).getOperationalLimitsGroups();
}

template <class Self>
std::vector<OperationalLimitsGroup> BranchMethods<Self>::getOperationalLimitsGroups2() const {
    return sideLimits(TwoSides::TWO).getOperationalLimitsGroups();
}

template <class Self>
iidm::optional<OperationalLimitsGroup> BranchMethods<Self>::getOperationalLimitsGroup1(const std::string& id) const {
    return sideLimits(TwoSides::ONE).getOperationalLimitsGroup(id);
}

template <class Self>
iidm::optional<OperationalLimitsGroup> BranchMethods<Self>::getOperationalLimitsGroup2(const std::string& id) const {
    return sideLimits(TwoSides::TWO).getOperationalLimitsGroup(id);
}

template <class Self>
OperationalLimitsGroup BranchMethods<Self>::newOperationalLimitsGroup1(const std::string& id) {
    return sideLimits(TwoSides::ONE).newOperationalLimitsGroup(id);
}

template <class Self>
OperationalLimitsGroup BranchMethods<Self>::newOperationalLimitsGroup2(const std::string& id) {
    return sideLimits(TwoSides::TWO).newOperationalLimitsGroup(id);
}

template <class Self>
void BranchMethods<Self>::removeOperationalLimitsGroup1(const std::string& id) {
    sideLimits(TwoSides::ONE).removeOperationalLimitsGroup(id);
}

template <class Self>
void BranchMethods<Self>::removeOperationalLimitsGroup2(const std::string& id) {
    sideLimits(TwoSides::TWO).removeOperationalLimitsGroup(id);
}

// ── Selection ────────────────────────────────────────────────────────────────

template <class Self>
iidm::optional<std::string> BranchMethods<Self>::getSelectedOperationalLimitsGroupId1() const {
    return sideLimits(TwoSides::ONE).getSelectedOperationalLimitsGroupId();
}

template <class Self>
iidm::optional<std::string> BranchMethods<Self>::getSelectedOperationalLimitsGroupId2() const {
    return sideLimits(TwoSides::TWO).getSelectedOperationalLimitsGroupId();
}

template <class Self>
iidm::optional<OperationalLimitsGroup> BranchMethods<Self>::getSelectedOperationalLimitsGroup1() const {
    return sideLimits(TwoSides::ONE).getSelectedOperationalLimitsGroup();
}

template <class Self>
iidm::optional<OperationalLimitsGroup> BranchMethods<Self>::getSelectedOperationalLimitsGroup2() const {
    return sideLimits(TwoSides::TWO).getSelectedOperationalLimitsGroup();
}

template <class Self>
std::vector<std::string> BranchMethods<Self>::getAllSelectedOperationalLimitsGroupIds(TwoSides side) const {
    return sideLimits(side).getAllSelectedOperationalLimitsGroupIds();
}

template <class Self>
std::vector<std::string> BranchMethods<Self>::getAllSelectedOperationalLimitsGroupIdsOrdered(TwoSides side) const {
    return sideLimits(side).getAllSelectedOperationalLimitsGroupIdsOrdered();
}

template <class Self>
std::vector<OperationalLimitsGroup> BranchMethods<Self>::getAllSelectedOperationalLimitsGroups(TwoSides side) const {
    return sideLimits(side).getAllSelectedOperationalLimitsGroups();
}

template <class Self>
Self& BranchMethods<Self>::setSelectedOperationalLimitsGroup1(const std::string& id) {
    sideLimits(TwoSides::ONE).setSelectedOperationalLimitsGroup(id);
    return self();
}

template <class Self>
Self& BranchMethods<Self>::setSelectedOperationalLimitsGroup2(const std::string& id) {
    sideLimits(TwoSides::TWO).setSelectedOperationalLimitsGroup(id);
    return self();
}

template <class Self>
void BranchMethods<Self>::addSelectedOperationalLimitsGroups(TwoSides side, const std::vector<std::string>& ids) {
    sideLimits(side).addSelectedOperationalLimitsGroups(ids);
}

template <class Self>
void BranchMethods<Self>::deselectOperationalLimitsGroups(TwoSides side, const std::vector<std::string>& ids) {
    sideLimits(side).deselectOperationalLimitsGroups(ids);
}

template <class Self>
void BranchMethods<Self>::cancelSelectedOperationalLimitsGroup1() {
    sideLimits(TwoSides::ONE).cancelSelectedOperationalLimitsGroup();
}

template <class Self>
void BranchMethods<Self>::cancelSelectedOperationalLimitsGroup2() {
    sideLimits(TwoSides::TWO).cancelSelectedOperationalLimitsGroup();
}

template <class Self>
OperationalLimitsGroup BranchMethods<Self>::getOrCreateSelectedOperationalLimitsGroup1() {
    return sideLimits(TwoSides::ONE).getOrCreateSelectedOperationalLimitsGroup();
}

template <class Self>
OperationalLimitsGroup BranchMethods<Self>::getOrCreateSelectedOperationalLimitsGroup2() {
    return sideLimits(TwoSides::TWO).getOrCreateSelectedOperationalLimitsGroup();
}

template <class Self>
OperationalLimitsGroup BranchMethods<Self>::getOrCreateSelectedOperationalLimitsGroup1(const std::string& id) {
    return sideLimits(TwoSides::ONE).getOrCreateSelectedOperationalLimitsGroup(id);
}

template <class Self>
OperationalLimitsGroup BranchMethods<Self>::getOrCreateSelectedOperationalLimitsGroup2(const std::string& id) {
    return sideLimits(TwoSides::TWO).getOrCreateSelectedOperationalLimitsGroup(id);
}

// ── Limits in force ──────────────────────────────────────────────────────────

template <class Self>
iidm::optional<CurrentLimits> BranchMethods<Self>::getCurrentLimits1() const {
    return getCurrentLimits(TwoSides::ONE);
}

template <class Self>
iidm::optional<CurrentLimits> BranchMethods<Self>::getCurrentLimits2() const {
    return getCurrentLimits(TwoSides::TWO);
}

template <class Self>
iidm::optional<CurrentLimits> BranchMethods<Self>::getCurrentLimits(TwoSides side) const {
    return sideLimits(side).getCurrentLimits();
}

template <class Self>
std::vector<CurrentLimits> BranchMethods<Self>::getAllSelectedCurrentLimits(TwoSides side) const {
    return sideLimits(side).getAllSelectedCurrentLimits();
}

template <class Self>
iidm::optional<ActivePowerLimits> BranchMethods<Self>::getActivePowerLimits1() const {
    return getActivePowerLimits(TwoSides::ONE);
}

template <class Self>
iidm::optional<ActivePowerLimits> BranchMethods<Self>::getActivePowerLimits2() const {
    return getActivePowerLimits(TwoSides::TWO);
}

template <class Self>
iidm::optional<ActivePowerLimits> BranchMethods<Self>::getActivePowerLimits(TwoSides side) const {
    return sideLimits(side).getActivePowerLimits();
}

template <class Self>
std::vector<ActivePowerLimits> BranchMethods<Self>::getAllSelectedActivePowerLimits(TwoSides side) const {
    return sideLimits(side).getAllSelectedActivePowerLimits();
}

template <class Self>
iidm::optional<ApparentPowerLimits> BranchMethods<Self>::getApparentPowerLimits1() const {
    return getApparentPowerLimits(TwoSides::ONE);
}

template <class Self>
iidm::optional<ApparentPowerLimits> BranchMethods<Self>::getApparentPowerLimits2() const {
    return getApparentPowerLimits(TwoSides::TWO);
}

template <class Self>
iidm::optional<ApparentPowerLimits> BranchMethods<Self>::getApparentPowerLimits(TwoSides side) const {
    return sideLimits(side).getApparentPowerLimits();
}

template <class Self>
std::vector<ApparentPowerLimits> BranchMethods<Self>::getAllSelectedApparentPowerLimits(TwoSides side) const {
    return sideLimits(side).getAllSelectedApparentPowerLimits();
}

template <class Self>
iidm::optional<LoadingLimits> BranchMethods<Self>::getLimits(LimitType type, TwoSides side) const {
    return sideLimits(side).limits(type);
}

template <class Self>
std::vector<LoadingLimits> BranchMethods<Self>::getAllSelectedLimits(LimitType type, TwoSides side) const {
    return sideLimits(side).allSelectedLimits(type);
}

// ── Extensions ───────────────────────────────────────────────────────────────

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
