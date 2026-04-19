#pragma once

#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl ActivePowerControl extension.
// Accepts explicit property codes so it can serve both Generator (EXT_APC_*) and
// Battery (EXT_BAT_APC_*) without duplicating the class.
class ActivePowerControl {
public:
    ActivePowerControl() = default;
    explicit ActivePowerControl(ObjectHandle handle, BackendProvider* backend,
                                int droopCode, int participateCode);

    bool isParticipate() const;
    ActivePowerControl& setParticipate(bool participate);

    double getDroop() const;
    ActivePowerControl& setDroop(double droop);

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const ActivePowerControl& other) const;
    bool operator!=(const ActivePowerControl& other) const;

private:
    ObjectHandle     handle_       = INVALID_HANDLE;
    BackendProvider* backend_      = nullptr;
    int              droopCode_    = 0;
    int              participateCode_ = 0;
};

} // namespace iidm
