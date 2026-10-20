#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/TemporaryLimit.h>
#include <vector>

namespace iidm {

class BackendProvider;

class CurrentLimits : public detail::ObjectRef {
public:
    CurrentLimits() = default;
    explicit CurrentLimits(ObjectHandle handle, BackendProvider* backend);

    double                    getPermanentLimit()    const;
    std::vector<TemporaryLimit> getTemporaryLimits() const;
};

} // namespace iidm
