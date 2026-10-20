#pragma once

#include <iidm/CurrentLimits.h>
#include <iidm/ObjectRef.h>
#include <iidm/OperationalLimitsGroup.h>
#include <iidm/stdcxx/optional.hpp>
#include <vector>

namespace iidm {
namespace detail {

// The limits of what has one set of them (powsybl FlowsLimitsHolder): three windings transformer
// legs, dangling lines. A branch has two sets: one per side.
template <class Self>
class FlowsLimitsHolderMethods {
public:
    std::vector<OperationalLimitsGroup> getOperationalLimitsGroups() const;
    // Empty when no group is selected
    iidm::optional<OperationalLimitsGroup> getSelectedOperationalLimitsGroup() const;
    // The current limits of the selected group
    iidm::optional<CurrentLimits> getCurrentLimits() const;

protected:
    FlowsLimitsHolderMethods() = default;
    ~FlowsLimitsHolderMethods() = default;

private:
    const Self& self() const { return static_cast<const Self&>(*this); }
    ObjectHandle handle() const { return Access::handle(self()); }
    BackendProvider* backend() const { return Access::backend(self()); }
};

} // namespace detail
} // namespace iidm
