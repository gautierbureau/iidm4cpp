#pragma once

#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl ActivePowerControl extension attached to a Generator.
// Uses the generator's handle with EXT_APC_* property codes to dispatch through the backend.
class ActivePowerControl {
public:
    ActivePowerControl() = default;
    explicit ActivePowerControl(ObjectHandle handle, BackendProvider* backend);

    bool isParticipate() const;
    ActivePowerControl& setParticipate(bool participate);

    double getDroop() const;
    ActivePowerControl& setDroop(double droop);

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const ActivePowerControl& other) const;
    bool operator!=(const ActivePowerControl& other) const;

private:
    ObjectHandle     handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
