#pragma once

#include <iidm/HandleRef.h>
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
    HandleRef  handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
