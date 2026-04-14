#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Extension.h>
#include <iidm/Terminal.h>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

class VscConverterStation {
public:
    VscConverterStation() = default;
    explicit VscConverterStation(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    float getLossFactor() const;

    bool isVoltageRegulatorOn() const;
    VscConverterStation& setVoltageRegulatorOn(bool on);

    double getVoltageSetpoint() const;
    VscConverterStation& setVoltageSetpoint(double value);

    double getReactivePowerSetpoint() const;
    VscConverterStation& setReactivePowerSetpoint(double value);

    Terminal getTerminal() const;

    void connect();
    void disconnect();
    bool isConnected() const;

    bool hasExtension(const std::string& name) const;
    Extension getExtension(const std::string& name) const;
    std::vector<Extension> getExtensions() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const VscConverterStation& other) const;
    bool operator!=(const VscConverterStation& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
