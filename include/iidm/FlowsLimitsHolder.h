#pragma once

#include <iidm/Enums.h>
#include <iidm/LoadingLimits.h>
#include <iidm/ObjectRef.h>
#include <iidm/OperationalLimitsGroup.h>
#include <iidm/PropertyCodes.h>
#include <iidm/stdcxx/optional.hpp>
#include <functional>
#include <string>
#include <vector>

namespace iidm {
namespace detail {

// The limits of what has one set of them (powsybl FlowsLimitsHolder): three windings transformer
// legs, dangling lines, and each side of a branch (see BranchMethods). The limits are in groups,
// of which one or more are selected: their limits are those in force.
template <class Self>
class FlowsLimitsHolderMethods {
public:
    std::vector<OperationalLimitsGroup> getOperationalLimitsGroups() const;
    // Empty when there is no such group
    iidm::optional<OperationalLimitsGroup> getOperationalLimitsGroup(const std::string& id) const;
    // A new group, empty: limits are added to it
    OperationalLimitsGroup newOperationalLimitsGroup(const std::string& id);
    void removeOperationalLimitsGroup(const std::string& id);

    // ── Selection ────────────────────────────────────────────────────────
    // Empty when no group is selected
    iidm::optional<std::string> getSelectedOperationalLimitsGroupId() const;
    iidm::optional<OperationalLimitsGroup> getSelectedOperationalLimitsGroup() const;
    // Several groups can be selected: their ids sorted, and in the order of their selection
    std::vector<std::string> getAllSelectedOperationalLimitsGroupIds() const;
    std::vector<std::string> getAllSelectedOperationalLimitsGroupIdsOrdered() const;
    std::vector<OperationalLimitsGroup> getAllSelectedOperationalLimitsGroups() const;
    // Selects this group only
    Self& setSelectedOperationalLimitsGroup(const std::string& id);
    // Selects these groups too; those whose id passes the test
    void addSelectedOperationalLimitsGroups(const std::vector<std::string>& ids);
    void addSelectedOperationalLimitsGroupByPredicate(const std::function<bool(const std::string&)>& idPredicate);
    void deselectOperationalLimitsGroups(const std::vector<std::string>& ids);
    // Deselects every group
    void cancelSelectedOperationalLimitsGroup();
    // The selected group, or a new one selected; with an id: that group, selected (created if needed)
    OperationalLimitsGroup getOrCreateSelectedOperationalLimitsGroup();
    OperationalLimitsGroup getOrCreateSelectedOperationalLimitsGroup(const std::string& id);

    // ── Limits in force ──────────────────────────────────────────────────
    // Those of the selected group; empty when it has none (or no group is selected)
    iidm::optional<CurrentLimits> getCurrentLimits() const;
    iidm::optional<ActivePowerLimits> getActivePowerLimits() const;
    iidm::optional<ApparentPowerLimits> getApparentPowerLimits() const;
    // Those of every selected group
    std::vector<CurrentLimits> getAllSelectedCurrentLimits() const;
    std::vector<ActivePowerLimits> getAllSelectedActivePowerLimits() const;
    std::vector<ApparentPowerLimits> getAllSelectedApparentPowerLimits() const;
    // Those of a group; empty when there is no such group, or it has none
    iidm::optional<CurrentLimits> getCurrentLimitsFromId(const std::string& id) const;
    iidm::optional<ActivePowerLimits> getActivePowerLimitsFromId(const std::string& id) const;
    iidm::optional<ApparentPowerLimits> getApparentPowerLimitsFromId(const std::string& id) const;

protected:
    FlowsLimitsHolderMethods() = default;
    ~FlowsLimitsHolderMethods() = default;

    // Limits of a type of the selected group, of every selected group (3WT legs, branch sides)
    iidm::optional<LoadingLimits> limits(LimitType type) const;
    std::vector<LoadingLimits> allSelectedLimits(LimitType type) const;

private:
    const Self& self() const { return static_cast<const Self&>(*this); }
    Self& self() { return static_cast<Self&>(*this); }
    ObjectHandle handle() const { return Access::handle(self()); }
    BackendProvider* backend() const { return Access::backend(self()); }
};

// One side of a branch, as a holder of flows limits: what the per-side limits methods of a branch
// go through. powsybl has no object for it.
class BranchSideLimits : public ObjectRef, public FlowsLimitsHolderMethods<BranchSideLimits> {
public:
    static constexpr int TYPE = prop::FLOWS_LIMITS_HOLDER;

    BranchSideLimits() = default;
    explicit BranchSideLimits(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    using FlowsLimitsHolderMethods<BranchSideLimits>::limits;
    using FlowsLimitsHolderMethods<BranchSideLimits>::allSelectedLimits;
};

} // namespace detail
} // namespace iidm
