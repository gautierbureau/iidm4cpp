#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Enums.h>
#include <iidm/Extension.h>
#include <iidm/Terminal.h>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

class Generator {
public:
    Generator() = default;
    explicit Generator(ObjectHandle handle, BackendProvider* backend);

    std::string getId() const;
    std::string getName() const;

    double getTargetP() const;
    Generator& setTargetP(double targetP);

    double getTargetQ() const;
    Generator& setTargetQ(double targetQ);

    double getTargetV() const;
    Generator& setTargetV(double targetV);

    double getMinP() const;
    double getMaxP() const;

    bool isVoltageRegulatorOn() const;
    Generator& setVoltageRegulatorOn(bool on);

    EnergySource getEnergySource() const;

    Terminal getTerminal() const;

    void connect();
    void disconnect();
    bool isConnected() const;

    bool hasExtension(const std::string& name) const;
    Extension getExtension(const std::string& name) const;
    std::vector<Extension> getExtensions() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const Generator& other) const;
    bool operator!=(const Generator& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
