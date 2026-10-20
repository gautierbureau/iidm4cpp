#pragma once

#include <iidm/BranchObservability.h>
#include <iidm/BranchObservabilityAdder.h>
#include <iidm/CurrentLimits.h>
#include <iidm/Enums.h>
#include <iidm/Identifiable.h>
#include <iidm/OperationalLimitsGroup.h>
#include <iidm/Terminal.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <type_traits>
#include <vector>

namespace iidm {

namespace detail {

// What every branch has (powsybl Branch): two terminals, and limits on each side. A mixin
// rather than a base class, as in powsybl a branch is not always a Connectable (a TieLine).
template <class Self>
class BranchMethods {
public:
    Terminal getTerminal1() const;
    Terminal getTerminal2() const;
    Terminal getTerminal(TwoSides side) const;
    // The terminal in this voltage level
    Terminal getTerminal(const std::string& voltageLevelId) const;

    iidm::optional<CurrentLimits> getCurrentLimits1() const;
    iidm::optional<CurrentLimits> getCurrentLimits2() const;

    std::vector<OperationalLimitsGroup> getOperationalLimitsGroups1() const;
    std::vector<OperationalLimitsGroup> getOperationalLimitsGroups2() const;
    iidm::optional<OperationalLimitsGroup> getSelectedOperationalLimitsGroup1() const;
    iidm::optional<OperationalLimitsGroup> getSelectedOperationalLimitsGroup2() const;

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
    ObjectHandle handle() const { return Access::handle(self()); }
    BackendProvider* backend() const { return Access::backend(self()); }
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
