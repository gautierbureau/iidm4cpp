#pragma once

#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/Enums.h>
#include <iidm/Terminal.h>
#include <string>
#include <iidm/InjectionObservability.h>
#include <iidm/Measurement.h>

namespace iidm {

class BackendProvider;

class Load {
public:
    Load() = default;
    explicit Load(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getP0() const;
    Load& setP0(double p0);

    double getQ0() const;
    Load& setQ0(double q0);

    LoadType getLoadType() const;
    bool isFictitious() const;

    Terminal getTerminal() const;

    void connect();
    void disconnect();
    bool isConnected() const;

    // Removes the load from the network
    void remove();

    // ── State estimation extensions ──────────────────────────────────────
    bool hasInjectionObservability() const;
    InjectionObservability getInjectionObservability() const;

    bool hasMeasurements() const;
    Measurements getMeasurements() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Load& other) const;
    bool operator!=(const Load& other) const;

private:
    HandleRef handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
