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

    double getBPerSection() const { return getDouble(prop::SHUNT_B_PER_SECTION); }
    ShuntCompensatorLinearModel& setBPerSection(double bPerSection) {
        return setDouble(*this, prop::SHUNT_B_PER_SECTION, bPerSection);
    }
    double getGPerSection() const { return getDouble(prop::SHUNT_G_PER_SECTION); }
    ShuntCompensatorLinearModel& setGPerSection(double gPerSection) {
        return setDouble(*this, prop::SHUNT_G_PER_SECTION, gPerSection);
    }
    // (ShuntCompensator::getMaximumSectionCount() reads it)
    ShuntCompensatorLinearModel& setMaximumSectionCount(int maximumSectionCount) {
        return setInt(*this, prop::SHUNT_MAX_SECTION_COUNT, maximumSectionCount);
    }
};

} // namespace iidm
