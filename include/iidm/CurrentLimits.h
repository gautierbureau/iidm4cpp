#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/TemporaryLimit.h>
#include <vector>

namespace iidm {

class BackendProvider;

class CurrentLimits {
public:
    CurrentLimits() = default;
    explicit CurrentLimits(ObjectHandle handle, BackendProvider* backend);

    double                    getPermanentLimit()    const;
    std::vector<TemporaryLimit> getTemporaryLimits() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

private:
    ObjectHandle     handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
