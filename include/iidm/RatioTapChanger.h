#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/RatioTapChangerStep.h>
#include <iidm/Terminal.h>
#include <iidm/stdcxx/optional.hpp>
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
    int regulationTerminalRelation;
    int loadTapChangingCapabilities; // bool property code
};

class RatioTapChanger : public detail::ObjectRef {
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

    iidm::optional<double> getTargetDeadband() const;
    std::string getRegulationTerminalId() const;
    Terminal getRegulationTerminal() const;

    bool hasLoadTapChangingCapabilities() const;

    std::vector<RatioTapChangerStep> getAllSteps() const;
    RatioTapChangerStep getCurrentStep() const;

    // Same transformer and same tap changer
    bool operator==(const RatioTapChanger& other) const;
    bool operator!=(const RatioTapChanger& other) const { return !(*this == other); }

private:
    RatioTapChangerConfig cfg_     = {};
};

} // namespace iidm
