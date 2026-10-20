#pragma once
#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/CurrentLimits.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>

namespace iidm {
class BackendProvider;
class OperationalLimitsGroup : public detail::ObjectRef {
public:
    OperationalLimitsGroup() = default;
    explicit OperationalLimitsGroup(ObjectHandle handle, BackendProvider* backend);
    std::string getId() const;
    bool isEmpty() const;
    iidm::optional<CurrentLimits> getCurrentLimits() const;
};
} // namespace iidm
