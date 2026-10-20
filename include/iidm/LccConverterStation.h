#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Terminal.h>
#include <string>
#include <iidm/InjectionObservability.h>
#include <iidm/Measurement.h>

namespace iidm {

class BackendProvider;
class HvdcLine;

class LccConverterStation {
public:
    LccConverterStation() = default;
    explicit LccConverterStation(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getLossFactor() const;
    double getPowerFactor() const;

    Terminal getTerminal() const;
    HvdcLine getHvdcLine() const;

    void connect();
    void disconnect();
    bool isConnected() const;

    // ── State estimation extensions ──────────────────────────────────────
    bool hasInjectionObservability() const;
    InjectionObservability getInjectionObservability() const;

    bool hasMeasurements() const;
    Measurements getMeasurements() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const LccConverterStation& other) const;
    bool operator!=(const LccConverterStation& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
