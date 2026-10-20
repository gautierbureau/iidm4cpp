#pragma once

#include <iidm/Injection.h>
#include <iidm/ShuntCompensatorNonLinearModel.h>
#include <iidm/Terminal.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>

namespace iidm {

class ShuntCompensator : public Injection {
public:
    static constexpr int TYPE = prop::SHUNT_COMPENSATOR;

    ShuntCompensator() = default;
    explicit ShuntCompensator(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    int getSectionCount() const;
    // Empty when the section count is not defined
    iidm::optional<int> findSectionCount() const;
    ShuntCompensator& setSectionCount(int count);

    // Section count of a load flow solution; empty when there is none
    iidm::optional<int> getSolvedSectionCount() const;
    ShuntCompensator& setSolvedSectionCount(int count);

    int getMaximumSectionCount() const;

    bool hasLinearModel() const;
    double getBPerSection() const;
    double getGPerSection() const;
    double getB() const;

    bool hasNonLinearModel() const;
    ShuntCompensatorNonLinearModel getNonLinearModel() const;

    bool isVoltageRegulatorOn() const;
    double getTargetV() const;
    std::string getRegulatingTerminalId() const;
    Terminal getRegulatingTerminal() const;
};

} // namespace iidm
