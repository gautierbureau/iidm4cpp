#pragma once

#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/Terminal.h>
#include <iidm/ShuntCompensatorNonLinearModel.h>
#include <string>
#include <iidm/InjectionObservability.h>
#include <iidm/Measurement.h>
#include <iidm/DiscreteMeasurement.h>

namespace iidm {

class BackendProvider;

class ShuntCompensator {
public:
    ShuntCompensator() = default;
    explicit ShuntCompensator(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    int getSectionCount() const;
    ShuntCompensator& setSectionCount(int count);

    int getMaximumSectionCount() const;

    bool hasLinearModel() const;
    double getBPerSection() const;
    double getGPerSection() const;
    double getB() const;

    bool hasNonLinearModel() const;
    ShuntCompensatorNonLinearModel getNonLinearModel() const;

    Terminal getTerminal() const;

    bool isVoltageRegulatorOn() const;
    double getTargetV() const;
    std::string getRegulatingTerminalId() const;
    Terminal getRegulatingTerminal() const;

    void connect();
    void disconnect();
    bool isConnected() const;

    // ── State estimation extensions ──────────────────────────────────────
    bool hasInjectionObservability() const;
    InjectionObservability getInjectionObservability() const;

    bool hasMeasurements() const;
    Measurements getMeasurements() const;

    bool hasDiscreteMeasurements() const;
    DiscreteMeasurements getDiscreteMeasurements() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const ShuntCompensator& other) const;
    bool operator!=(const ShuntCompensator& other) const;

private:
    HandleRef handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
