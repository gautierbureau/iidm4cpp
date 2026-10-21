#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/ObjectRef.h>
#include <iidm/PropertiesHolder.h>
#include <iidm/PropertyCodes.h>
#include <string>

namespace iidm {

class BackendProvider;

// A temporary limit of loading limits (powsybl LoadingLimits.TemporaryLimit): the value the flow
// may reach for at most its acceptable duration. Read-only: LoadingLimits::setTemporaryLimitValue
// changes a value (and replaces the temporary limit).
class TemporaryLimit : public detail::ObjectRef, public detail::PropertiesHolder<TemporaryLimit> {
public:
    static constexpr int TYPE = prop::TEMPORARY_LIMIT;

    TemporaryLimit() = default;
    explicit TemporaryLimit(ObjectHandle handle, BackendProvider* backend);

    std::string getName() const;
    double getValue() const;
    // Seconds
    int getAcceptableDuration() const;
    bool isFictitious() const;
};

} // namespace iidm
