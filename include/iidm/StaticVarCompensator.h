#pragma once

#include <iidm/Enums.h>
#include <iidm/Injection.h>
#include <iidm/Terminal.h>
#include <iidm/VoltagePerReactivePowerControl.h>

namespace iidm {

class StaticVarCompensator : public Injection {
public:
    static constexpr int TYPE = prop::STATIC_VAR_COMPENSATOR;

    StaticVarCompensator() = default;
    explicit StaticVarCompensator(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    double getBMin() const;
    double getBMax() const;

    double getVoltageSetpoint() const;
    StaticVarCompensator& setVoltageSetpoint(double value);

    double getReactivePowerSetpoint() const;
    StaticVarCompensator& setReactivePowerSetpoint(double value);

    StaticVarCompensatorRegulationMode getRegulationMode() const;
    StaticVarCompensator& setRegulationMode(StaticVarCompensatorRegulationMode mode);

    Terminal getRegulatingTerminal() const;

    bool hasVoltagePerReactivePowerControl() const;
    VoltagePerReactivePowerControl getVoltagePerReactivePowerControl() const;
};

} // namespace iidm
