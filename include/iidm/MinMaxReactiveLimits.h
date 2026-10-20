#pragma once

#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

class MinMaxReactiveLimits {
public:
    MinMaxReactiveLimits() = default;
    explicit MinMaxReactiveLimits(ObjectHandle handle, BackendProvider* backend,
                                   int minQProp, int maxQProp);

    double getMinQ() const;
    double getMaxQ() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const MinMaxReactiveLimits& other) const;
    bool operator!=(const MinMaxReactiveLimits& other) const;

private:
    HandleRef  handle_;
    BackendProvider* backend_  = detail::nullBackend();
    int              minQProp_ = 0;
    int              maxQProp_ = 0;
};

} // namespace iidm
