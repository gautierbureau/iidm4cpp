#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl ActivePowerControl extension.
// Serves both Generator (EXT_APC_*) and Battery (EXT_BAT_APC_*): both code blocks share
// the same layout, so the wrapper only needs the block's EXISTS code as base.
class ActivePowerControl {
public:
    ActivePowerControl() = default;
    explicit ActivePowerControl(ObjectHandle handle, BackendProvider* backend, int codeBase);
    // Former signature, kept for compatibility: the code block is derived from droopCode.
    ActivePowerControl(ObjectHandle handle, BackendProvider* backend, int droopCode, int participateCode);

    bool isParticipate() const;
    ActivePowerControl& setParticipate(bool participate);

    double getDroop() const;
    ActivePowerControl& setDroop(double droop);

    double getParticipationFactor() const;
    ActivePowerControl& setParticipationFactor(double participationFactor);

    // Limits of the active power target used when the injection participates; empty when
    // undefined. A value outside [minP, maxP], or min > max, throws ValidationException.
    iidm::optional<double> getMinTargetP() const;
    ActivePowerControl& setMinTargetP(double minTargetP);
    ActivePowerControl& unsetMinTargetP();

    iidm::optional<double> getMaxTargetP() const;
    ActivePowerControl& setMaxTargetP(double maxTargetP);
    ActivePowerControl& unsetMaxTargetP();

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const ActivePowerControl& other) const;
    bool operator!=(const ActivePowerControl& other) const;

private:
    int code(int generatorCode) const;

    ObjectHandle     handle_   = INVALID_HANDLE;
    BackendProvider* backend_  = detail::nullBackend();
    int              codeBase_ = 0;
};

} // namespace iidm
