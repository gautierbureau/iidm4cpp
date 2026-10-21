#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The OperatingStatus extension of a busbar section, a line, a tie line, a transformer, an HVDC
// line or a boundary line (ValidationException on another identifiable): an object of its own,
// from Identifiable::getOperatingStatus().
class OperatingStatus : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::OPERATING_STATUS;
    // powsybl OperatingStatus.Status
    enum class Status { IN_OPERATION, PLANNED_OUTAGE, FORCED_OUTAGE };

    OperatingStatus() = default;
    explicit OperatingStatus(ObjectHandle handle, BackendProvider* backend);

    Status getStatus() const;
    OperatingStatus& setStatus(Status status);
};

} // namespace iidm
