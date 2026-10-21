#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/PropertyCodes.h>
#include <iidm/TemporaryLimit.h>
#include <string>

namespace iidm {

class BackendProvider;

// What a check of the temporary limits of a side of a branch or a three windings transformer finds
// (powsybl Overload): the limit that the flow is over, and the temporary limit above it, whose
// acceptable duration is how long the overload may last.
class Overload : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::OVERLOAD;

    Overload() = default;
    explicit Overload(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    // The temporary limit above the flow; over every limit, a fictitious one named "Unacceptable",
    // with an acceptable duration of 0
    TemporaryLimit getTemporaryLimit() const;
    // The limit the flow is over: the permanent limit, or a temporary limit
    double getPreviousLimit() const { return getDouble(prop::OVERLOAD_PREVIOUS_LIMIT); }
    std::string getPreviousLimitName() const { return getString(prop::OVERLOAD_PREVIOUS_LIMIT_NAME); }
    // The group of the limits
    std::string getOperationalLimitsGroupId() const { return getString(prop::OVERLOAD_OLG_ID); }
    // What the limits were multiplied by (the limit reduction of the check)
    double getLimitScalingCoefficient() const { return getDouble(prop::OVERLOAD_LIMIT_SCALING); }
};

} // namespace iidm
