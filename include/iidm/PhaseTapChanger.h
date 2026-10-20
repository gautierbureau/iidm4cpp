#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/Enums.h>
#include <iidm/PhaseTapChangerStep.h>
#include <iidm/stdcxx/optional.hpp>
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

class PhaseTapChanger : public detail::ObjectRef {
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

    iidm::optional<double> getTargetDeadband() const;
    std::string getRegulationTerminalId() const;

    std::vector<PhaseTapChangerStep> getAllSteps() const;
    PhaseTapChangerStep getCurrentStep() const;

    // Same transformer and same tap changer
    bool operator==(const PhaseTapChanger& other) const;
    bool operator!=(const PhaseTapChanger& other) const { return !(*this == other); }

private:
    PhaseTapChangerConfig cfg_     = {};
};

} // namespace iidm
