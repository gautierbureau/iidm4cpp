#pragma once

#include <iidm/ShuntCompensatorModel.h>
#include <vector>

namespace iidm {

// The susceptance and conductance are given for each section count: at n sections, those of
// the nth section
class ShuntCompensatorNonLinearModel : public ShuntCompensatorModel {
public:
    // The susceptance and conductance at a section count
    class Section : public detail::ObjectRef, public detail::PropertiesHolder<Section> {
    public:
        static constexpr int TYPE = prop::SHUNT_SECTION;

        Section() = default;
        explicit Section(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

        double getB() const { return getDouble(prop::SHUNT_SECTION_B); }
        Section& setB(double b) { return setDouble(*this, prop::SHUNT_SECTION_B, b); }
        double getG() const { return getDouble(prop::SHUNT_SECTION_G); }
        Section& setG(double g) { return setDouble(*this, prop::SHUNT_SECTION_G, g); }
    };

    static constexpr int TYPE = prop::SHUNT_COMPENSATOR_NON_LINEAR_MODEL;

    ShuntCompensatorNonLinearModel() = default;
    explicit ShuntCompensatorNonLinearModel(ObjectHandle handle, BackendProvider* backend)
        : ShuntCompensatorModel(handle, backend) {}

    // From the first section (one section in service) to the last one
    std::vector<Section> getAllSections() const;
};

} // namespace iidm
