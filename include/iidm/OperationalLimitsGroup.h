#pragma once
#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/CurrentLimits.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>

namespace iidm {
class BackendProvider;
class OperationalLimitsGroup {
public:
    OperationalLimitsGroup() = default;
    explicit OperationalLimitsGroup(ObjectHandle handle, BackendProvider* backend);
    std::string getId() const;
    bool isEmpty() const;
    iidm::optional<CurrentLimits> getCurrentLimits() const;
    bool isValid() const { return handle_ != INVALID_HANDLE; }
private:
    HandleRef handle_;
    BackendProvider* backend_ = detail::nullBackend();
};
} // namespace iidm
