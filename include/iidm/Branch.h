#pragma once

#include <iidm/BranchObservability.h>
#include <iidm/BranchObservabilityAdder.h>
#include <iidm/Enums.h>
#include <iidm/FlowsLimitsHolder.h>
#include <iidm/Identifiable.h>
#include <iidm/LoadingLimits.h>
#include <iidm/OperationalLimitsGroup.h>
#include <iidm/Overload.h>
#include <iidm/Terminal.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <type_traits>
#include <vector>

namespace iidm {

namespace detail {

// What every branch has (powsybl Branch): two terminals, and limits on each side. A mixin
// rather than a base class, as in powsybl a branch is not always a Connectable (a TieLine).
//
// The limits of each side are those of a flows limits holder (see FlowsLimitsHolderMethods): in
// groups, of which the selected ones are in force. Each holder method has a version per side
// (getOperationalLimitsGroups1, getOperationalLimitsGroups2), or takes the side.
template <class Self>
class BranchMethods {
public:
    Terminal getTerminal1() const;
    Terminal getTerminal2() const;
    Terminal getTerminal(TwoSides side) const;
    // The terminal in this voltage level
    Terminal getTerminal(const std::string& voltageLevelId) const;
    // The side of one of its terminals; ValidationException for another terminal
    TwoSides getSide(const Terminal& terminal) const;

    // ── Limit groups ─────────────────────────────────────────────────────
    std::vector<OperationalLimitsGroup> getOperationalLimitsGroups1() const;
    std::vector<OperationalLimitsGroup> getOperationalLimitsGroups2() const;
    iidm::optional<OperationalLimitsGroup> getOperationalLimitsGroup1(const std::string& id) const;
    iidm::optional<OperationalLimitsGroup> getOperationalLimitsGroup2(const std::string& id) const;
    OperationalLimitsGroup newOperationalLimitsGroup1(const std::string& id);
    OperationalLimitsGroup newOperationalLimitsGroup2(const std::string& id);
    void removeOperationalLimitsGroup1(const std::string& id);
    void removeOperationalLimitsGroup2(const std::string& id);

    // ── Selection ────────────────────────────────────────────────────────
    iidm::optional<std::string> getSelectedOperationalLimitsGroupId1() const;
    iidm::optional<std::string> getSelectedOperationalLimitsGroupId2() const;
    iidm::optional<OperationalLimitsGroup> getSelectedOperationalLimitsGroup1() const;
    iidm::optional<OperationalLimitsGroup> getSelectedOperationalLimitsGroup2() const;
    std::vector<std::string> getAllSelectedOperationalLimitsGroupIds(TwoSides side) const;
    std::vector<std::string> getAllSelectedOperationalLimitsGroupIdsOrdered(TwoSides side) const;
    std::vector<OperationalLimitsGroup> getAllSelectedOperationalLimitsGroups(TwoSides side) const;
    Self& setSelectedOperationalLimitsGroup1(const std::string& id);
    Self& setSelectedOperationalLimitsGroup2(const std::string& id);
    void addSelectedOperationalLimitsGroups(TwoSides side, const std::vector<std::string>& ids);
    void addSelectedOperationalLimitsGroupByPredicate(TwoSides side, const std::function<bool(const std::string&)>& idPredicate);
    void deselectOperationalLimitsGroups(TwoSides side, const std::vector<std::string>& ids);
    void cancelSelectedOperationalLimitsGroup1();
    void cancelSelectedOperationalLimitsGroup2();
    OperationalLimitsGroup getOrCreateSelectedOperationalLimitsGroup1();
    OperationalLimitsGroup getOrCreateSelectedOperationalLimitsGroup2();
    OperationalLimitsGroup getOrCreateSelectedOperationalLimitsGroup1(const std::string& id);
    OperationalLimitsGroup getOrCreateSelectedOperationalLimitsGroup2(const std::string& id);

    // ── Limits in force: those of the selected group, or of every selected group ──
    iidm::optional<CurrentLimits> getCurrentLimits1() const;
    iidm::optional<CurrentLimits> getCurrentLimits2() const;
    iidm::optional<CurrentLimits> getCurrentLimits(TwoSides side) const;
    std::vector<CurrentLimits> getAllSelectedCurrentLimits(TwoSides side) const;
    iidm::optional<ActivePowerLimits> getActivePowerLimits1() const;
    iidm::optional<ActivePowerLimits> getActivePowerLimits2() const;
    iidm::optional<ActivePowerLimits> getActivePowerLimits(TwoSides side) const;
    std::vector<ActivePowerLimits> getAllSelectedActivePowerLimits(TwoSides side) const;
    iidm::optional<ApparentPowerLimits> getApparentPowerLimits1() const;
    iidm::optional<ApparentPowerLimits> getApparentPowerLimits2() const;
    iidm::optional<ApparentPowerLimits> getApparentPowerLimits(TwoSides side) const;
    std::vector<ApparentPowerLimits> getAllSelectedApparentPowerLimits(TwoSides side) const;
    // Of a type (current, active power or apparent power)
    iidm::optional<LoadingLimits> getLimits(LimitType type, TwoSides side) const;
    std::vector<LoadingLimits> getAllSelectedLimits(LimitType type, TwoSides side) const;

    // ── Overloads: a flow (current, active or apparent power) over the limits in force; with a
    // limit reduction, over these limits multiplied by it ──────────────────────────────────────
    // Whether the current is over the permanent limit, on either side
    bool isOverloaded() const;
    bool isOverloaded(double limitReductionValue) const;
    // The shortest acceptable duration of the overloads of the current (seconds; INT_MAX without any)
    int getOverloadDuration() const;
    bool checkPermanentLimit(TwoSides side, LimitType type) const;
    bool checkPermanentLimit(TwoSides side, double limitReductionValue, LimitType type) const;
    bool checkPermanentLimit1(LimitType type) const;
    bool checkPermanentLimit1(double limitReductionValue, LimitType type) const;
    bool checkPermanentLimit2(LimitType type) const;
    bool checkPermanentLimit2(double limitReductionValue, LimitType type) const;
    // The overload of a side (empty without one), the overloads of every selected group of a side
    iidm::optional<Overload> checkTemporaryLimits(TwoSides side, LimitType type) const;
    iidm::optional<Overload> checkTemporaryLimits(TwoSides side, double limitReductionValue, LimitType type) const;
    iidm::optional<Overload> checkTemporaryLimits1(LimitType type) const;
    iidm::optional<Overload> checkTemporaryLimits1(double limitReductionValue, LimitType type) const;
    iidm::optional<Overload> checkTemporaryLimits2(LimitType type) const;
    iidm::optional<Overload> checkTemporaryLimits2(double limitReductionValue, LimitType type) const;
    std::vector<Overload> checkAllTemporaryLimits(TwoSides side, LimitType type) const;
    std::vector<Overload> checkAllTemporaryLimits(TwoSides side, double limitReductionValue, LimitType type) const;

    // ── Extensions ───────────────────────────────────────────────────────
    // An extension's getter throws ExtensionNotFoundException when it is absent
    bool hasBranchObservability() const;
    BranchObservability getBranchObservability() const;
    BranchObservabilityAdder newBranchObservability();
    void removeBranchObservability();

protected:
    BranchMethods() = default;
    ~BranchMethods() = default;

private:
    const Self& self() const { return static_cast<const Self&>(*this); }
    Self& self() { return static_cast<Self&>(*this); }
    ObjectHandle handle() const { return Access::handle(self()); }
    BackendProvider* backend() const { return Access::backend(self()); }
    // The limits of a side
    BranchSideLimits sideLimits(TwoSides side) const;
};

} // namespace detail

// Any branch (Line, TwoWindingsTransformer): each of them converts to a Branch.
class Branch : public Identifiable, public detail::BranchMethods<Branch> {
public:
    static constexpr int TYPE = prop::BRANCH;

    Branch() = default;
    explicit Branch(ObjectHandle handle, BackendProvider* backend) : Identifiable(handle, backend) {}

    template <class T, typename std::enable_if<std::is_base_of<detail::BranchMethods<T>, T>::value, int>::type = 0>
    Branch(const T& branch) : Identifiable(branch) {}
};

} // namespace iidm
