#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

class MinMaxReactiveLimits : public detail::ObjectRef {
public:
    MinMaxReactiveLimits() = default;
    explicit MinMaxReactiveLimits(ObjectHandle handle, BackendProvider* backend,
                                   int minQProp, int maxQProp);

    double getMinQ() const;
    double getMaxQ() const;

private:
    int              minQProp_ = 0;
    int              maxQProp_ = 0;
};

} // namespace iidm
