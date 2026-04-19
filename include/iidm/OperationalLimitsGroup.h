#pragma once
#include <iidm/ObjectHandle.h>
#include <iidm/CurrentLimits.h>
#include <optional>
#include <string>

namespace iidm {
class BackendProvider;
class OperationalLimitsGroup {
public:
    OperationalLimitsGroup() = default;
    explicit OperationalLimitsGroup(ObjectHandle handle, BackendProvider* backend);
    std::string getId() const;
    bool isEmpty() const;
    std::optional<CurrentLimits> getCurrentLimits() const;
    bool isValid() const { return handle_ != INVALID_HANDLE; }
private:
    ObjectHandle handle_ = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};
} // namespace iidm
