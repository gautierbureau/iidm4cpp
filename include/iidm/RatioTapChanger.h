#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/RatioTapChangerStep.h>
#include <optional>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

struct RatioTapChangerConfig {
    int exists;
    int tapPosition;
    int lowTap;
    int highTap;
    int regulating;
    int targetV;
    int stepChildType;
    int targetDeadband;
    int regulationTerminalId;
};

class RatioTapChanger {
public:
    RatioTapChanger() = default;
    explicit RatioTapChanger(ObjectHandle handle, BackendProvider* backend,
                              const RatioTapChangerConfig& cfg);

    int getTapPosition() const;
    RatioTapChanger& setTapPosition(int pos);

    int getLowTapPosition()  const;
    int getHighTapPosition() const;

    bool isRegulating() const;
    RatioTapChanger& setRegulating(bool reg);

    double getTargetV() const;
    RatioTapChanger& setTargetV(double v);

    std::optional<double> getTargetDeadband() const;
    std::string getRegulationTerminalId() const;

    std::vector<RatioTapChangerStep> getAllSteps() const;
    RatioTapChangerStep getCurrentStep() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const RatioTapChanger& other) const;
    bool operator!=(const RatioTapChanger& other) const;

private:
    ObjectHandle          handle_  = INVALID_HANDLE;
    BackendProvider*      backend_ = nullptr;
    RatioTapChangerConfig cfg_     = {};
};

} // namespace iidm
