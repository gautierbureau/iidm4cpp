#pragma once

#include <iidm/Enums.h>
#include <iidm/ObjectHandle.h>
#include <iidm/ObjectRef.h>
#include <iidm/PropertiesHolder.h>
#include <iidm/PropertyCodes.h>
#include <iidm/TemporaryLimit.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

// Limits on a flow (powsybl LoadingLimits): a permanent limit, and temporary limits that the flow
// may reach for a while. Current, active power and apparent power limits are LoadingLimits, of an
// OperationalLimitsGroup.
class LoadingLimits : public detail::ObjectRef, public detail::PropertiesHolder<LoadingLimits> {
public:
    static constexpr int TYPE = prop::LOADING_LIMITS;

    LoadingLimits() = default;
    explicit LoadingLimits(ObjectHandle handle, BackendProvider* backend);

    LimitType getLimitType() const;
    DetectionKind getDetectionKind() const;

    double getPermanentLimit() const;
    LoadingLimits& setPermanentLimit(double permanentLimit);
    std::string getPermanentLimitName() const;
    LoadingLimits& setPermanentLimitName(const std::string& permanentLimitName);

    // By decreasing acceptable duration
    std::vector<TemporaryLimit> getTemporaryLimits() const;
    // Empty when there is none with this acceptable duration (seconds)
    iidm::optional<TemporaryLimit> getTemporaryLimit(int acceptableDuration) const;
    // NaN when there is none with this acceptable duration
    double getTemporaryLimitValue(int acceptableDuration) const;
    // Replaces the temporary limit with this acceptable duration by one with this value
    LoadingLimits& setTemporaryLimitValue(int acceptableDuration, double value);

    // Removes them from their group
    void remove();
};

// Limits on the current (A)
class CurrentLimits : public LoadingLimits {
public:
    static constexpr int TYPE = prop::CURRENT_LIMITS;

    CurrentLimits() = default;
    explicit CurrentLimits(ObjectHandle handle, BackendProvider* backend) : LoadingLimits(handle, backend) {}
};

// Limits on the active power (MW)
class ActivePowerLimits : public LoadingLimits {
public:
    static constexpr int TYPE = prop::ACTIVE_POWER_LIMITS;

    ActivePowerLimits() = default;
    explicit ActivePowerLimits(ObjectHandle handle, BackendProvider* backend) : LoadingLimits(handle, backend) {}
};

// Limits on the apparent power (MVA)
class ApparentPowerLimits : public LoadingLimits {
public:
    static constexpr int TYPE = prop::APPARENT_POWER_LIMITS;

    ApparentPowerLimits() = default;
    explicit ApparentPowerLimits(ObjectHandle handle, BackendProvider* backend) : LoadingLimits(handle, backend) {}
};

} // namespace iidm
