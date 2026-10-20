#pragma once

#include <iidm/HvdcConverterStation.h>
#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/ReactiveCapabilityCurve.h>
#include <iidm/Terminal.h>

namespace iidm {

class VscConverterStation : public HvdcConverterStation {
public:
    static constexpr int TYPE = prop::VSC_CONVERTER_STATION;

    VscConverterStation() = default;
    explicit VscConverterStation(ObjectHandle handle, BackendProvider* backend) : HvdcConverterStation(handle, backend) {}

    bool isVoltageRegulatorOn() const;
    VscConverterStation& setVoltageRegulatorOn(bool on);

    double getVoltageSetpoint() const;
    VscConverterStation& setVoltageSetpoint(double value);

    double getReactivePowerSetpoint() const;
    VscConverterStation& setReactivePowerSetpoint(double value);

    Terminal getRegulatingTerminal() const;

    // ── Reactive limits ──────────────────────────────────────────────────
    bool hasMinMaxReactiveLimits() const;
    MinMaxReactiveLimits getMinMaxReactiveLimits() const;

    bool hasReactiveCapabilityCurve() const;
    ReactiveCapabilityCurve getReactiveCapabilityCurve() const;
};

} // namespace iidm
