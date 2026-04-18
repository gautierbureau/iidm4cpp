#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Terminal.h>
#include <iidm/ShuntCompensatorNonLinearModel.h>
#include <string>

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

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const ShuntCompensator& other) const;
    bool operator!=(const ShuntCompensator& other) const;

private:
    ObjectHandle    handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
