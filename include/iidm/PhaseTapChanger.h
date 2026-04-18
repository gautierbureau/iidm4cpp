#pragma once

#include <iidm/ObjectHandle.h>
#include <iidm/Enums.h>
#include <iidm/PhaseTapChangerStep.h>
#include <optional>
#include <string>
#include <vector>

namespace iidm {

class BackendProvider;

struct PhaseTapChangerConfig {
    int exists;
    int tapPosition;
    int lowTap;
    int highTap;
    int regulating;
    int regulationMode;
    int regulationValue;
    int stepChildType;
    int targetDeadband;
    int regulationTerminalId;
};

class PhaseTapChanger {
public:
    PhaseTapChanger() = default;
    explicit PhaseTapChanger(ObjectHandle handle, BackendProvider* backend,
                              const PhaseTapChangerConfig& cfg);

    int getTapPosition() const;
    PhaseTapChanger& setTapPosition(int pos);

    int getLowTapPosition()  const;
    int getHighTapPosition() const;

    bool isRegulating() const;
    PhaseTapChanger& setRegulating(bool reg);

    PhaseTapChangerRegulationMode getRegulationMode() const;
    PhaseTapChanger& setRegulationMode(PhaseTapChangerRegulationMode mode);

    double getRegulationValue() const;
    PhaseTapChanger& setRegulationValue(double val);

    std::optional<double> getTargetDeadband() const;
    std::string getRegulationTerminalId() const;

    std::vector<PhaseTapChangerStep> getAllSteps() const;
    PhaseTapChangerStep getCurrentStep() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const PhaseTapChanger& other) const;
    bool operator!=(const PhaseTapChanger& other) const;

private:
    ObjectHandle          handle_  = INVALID_HANDLE;
    BackendProvider*      backend_ = nullptr;
    PhaseTapChangerConfig cfg_     = {};
};

} // namespace iidm
