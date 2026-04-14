#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Enums.h>
#include <iidm/Terminal.h>
#include <string>

namespace iidm {

class BackendProvider;

class StaticVarCompensator {
public:
    StaticVarCompensator() = default;
    explicit StaticVarCompensator(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getBMin() const;
    double getBMax() const;

    double getVoltageSetpoint() const;
    StaticVarCompensator& setVoltageSetpoint(double value);

    double getReactivePowerSetpoint() const;
    StaticVarCompensator& setReactivePowerSetpoint(double value);

    StaticVarCompensatorRegulationMode getRegulationMode() const;
    StaticVarCompensator& setRegulationMode(StaticVarCompensatorRegulationMode mode);

    Terminal getTerminal() const;

    void connect();
    void disconnect();
    bool isConnected() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const StaticVarCompensator& other) const;
    bool operator!=(const StaticVarCompensator& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
