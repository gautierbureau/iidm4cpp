#pragma once

#include <iidm/Enums.h>
#include <iidm/LoadingLimits.h>
#include <iidm/LoadingLimitsAdder.h>
#include <iidm/ObjectHandle.h>
#include <iidm/ObjectRef.h>
#include <iidm/PropertiesHolder.h>
#include <iidm/PropertyCodes.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>

namespace iidm {

class BackendProvider;

// A set of limits of a flows limits holder (branch side, 3WT leg, boundary line), at most one of
// each type: the limits in force are those of the holder's selected group(s).
class OperationalLimitsGroup : public detail::ObjectRef, public detail::PropertiesHolder<OperationalLimitsGroup> {
public:
    static constexpr int TYPE = prop::OPERATIONAL_LIMITS_GROUP;

    OperationalLimitsGroup() = default;
    explicit OperationalLimitsGroup(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    // Whether it has no limits
    bool isEmpty() const;

    // Empty when it has none of that type
    iidm::optional<CurrentLimits> getCurrentLimits() const;
    iidm::optional<ActivePowerLimits> getActivePowerLimits() const;
    iidm::optional<ApparentPowerLimits> getApparentPowerLimits() const;
    iidm::optional<LoadingLimits> getLoadingLimits(LimitType type) const;

    // New limits replace those of the same type
    CurrentLimitsAdder newCurrentLimits();
    ActivePowerLimitsAdder newActivePowerLimits();
    ApparentPowerLimitsAdder newApparentPowerLimits();
    // The same, with the values of other limits (a copy)
    CurrentLimitsAdder newCurrentLimits(const CurrentLimits& copied);
    ActivePowerLimitsAdder newActivePowerLimits(const ActivePowerLimits& copied);
    ApparentPowerLimitsAdder newApparentPowerLimits(const ApparentPowerLimits& copied);

    void removeCurrentLimits();
    void removeActivePowerLimits();
    void removeApparentPowerLimits();
};

} // namespace iidm
