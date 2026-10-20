#pragma once

#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/CurrentLimits.h>
#include <iidm/Terminal.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>
#include <iidm/InjectionObservability.h>
#include <iidm/Measurement.h>

namespace iidm {

class BackendProvider;

class DanglingLine {
public:
    DanglingLine() = default;
    explicit DanglingLine(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getP0() const;
    DanglingLine& setP0(double p0);

    double getQ0() const;
    DanglingLine& setQ0(double q0);

    double getR() const;
    double getX() const;
    double getG() const;
    double getB() const;

    iidm::optional<CurrentLimits> getCurrentLimits() const;

    Terminal getTerminal() const;

    void connect();
    void disconnect();
    bool isConnected() const;

    // ── State estimation extensions ──────────────────────────────────────
    bool hasInjectionObservability() const;
    InjectionObservability getInjectionObservability() const;

    bool hasMeasurements() const;
    Measurements getMeasurements() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const DanglingLine& other) const;
    bool operator!=(const DanglingLine& other) const;

private:
    HandleRef handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
