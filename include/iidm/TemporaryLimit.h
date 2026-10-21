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

    std::string getName() const { return getString(prop::TL_NAME); }
    double getValue() const { return getDouble(prop::TL_VALUE); }
    // Seconds
    int getAcceptableDuration() const { return getInt(prop::TL_ACCEPTABLE_DURATION); }
    bool isFictitious() const { return getBool(prop::TL_FICTITIOUS); }
};

} // namespace iidm
