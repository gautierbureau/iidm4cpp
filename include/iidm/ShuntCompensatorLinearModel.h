#pragma once

#include <iidm/ShuntCompensatorModel.h>

namespace iidm {

// Every section has the same susceptance and conductance: at n sections, n times those
class ShuntCompensatorLinearModel : public ShuntCompensatorModel {
public:
    static constexpr int TYPE = prop::SHUNT_COMPENSATOR_LINEAR_MODEL;

    ShuntCompensatorLinearModel() = default;
    explicit ShuntCompensatorLinearModel(ObjectHandle handle, BackendProvider* backend)
        : ShuntCompensatorModel(handle, backend) {}

    double getBPerSection() const;
    ShuntCompensatorLinearModel& setBPerSection(double bPerSection);
    double getGPerSection() const;
    ShuntCompensatorLinearModel& setGPerSection(double gPerSection);
    // (ShuntCompensator::getMaximumSectionCount() reads it)
    ShuntCompensatorLinearModel& setMaximumSectionCount(int maximumSectionCount);
};

} // namespace iidm
