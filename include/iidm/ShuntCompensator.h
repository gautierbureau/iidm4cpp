#pragma once

#include <iidm/BackendProvider.h>
#include <iidm/Enums.h>
#include <iidm/Injection.h>
#include <iidm/ShuntCompensatorLinearModel.h>
#include <iidm/ShuntCompensatorModel.h>
#include <iidm/ShuntCompensatorNonLinearModel.h>
#include <iidm/VoltageRegulationHolder.h>
#include <iidm/stdcxx/optional.hpp>

namespace iidm {

// Sections of capacitors or reactors, of which some are in service. Its model (linear or
// non-linear) gives its susceptance and conductance at each section count.
class ShuntCompensator : public Injection, public detail::VoltageRegulationHolderMethods<ShuntCompensator> {
public:
    static constexpr int TYPE = prop::SHUNT_COMPENSATOR;

    ShuntCompensator() = default;
    explicit ShuntCompensator(ObjectHandle handle, BackendProvider* backend) : Injection(handle, backend) {}

    // ── Section counts ───────────────────────────────────────────────────
    // The number of sections in service: ValidationException when it is undefined, which only a
    // network at the EQUIPMENT validation level accepts (findSectionCount gives it as empty then)
    int getSectionCount() const { return getInt(prop::SHUNT_SECTION_COUNT); }
    iidm::optional<int> findSectionCount() const { return getOptInt(prop::SHUNT_SECTION_COUNT); }
    ShuntCompensator& setSectionCount(int count) { return setInt(*this, prop::SHUNT_SECTION_COUNT, count); }
    ShuntCompensator& unsetSectionCount();
    // That of a load flow solution: empty when there is none
    iidm::optional<int> getSolvedSectionCount() const { return getOptInt(prop::SHUNT_SOLVED_SECTION_COUNT); }
    iidm::optional<int> findSolvedSectionCount() const;
    ShuntCompensator& setSolvedSectionCount(int count) {
        return setInt(*this, prop::SHUNT_SOLVED_SECTION_COUNT, count);
    }
    ShuntCompensator& unsetSolvedSectionCount();
    int getMaximumSectionCount() const { return getInt(prop::SHUNT_MAX_SECTION_COUNT); }

    // ── Susceptance and conductance (S) ─────────────────────────────────
    // At the section count (which must be defined); at a section count from 0 to the maximum
    // (ValidationException outside)
    double getB() const { return getDouble(prop::SHUNT_B); }
    double getG() const { return getDouble(prop::SHUNT_G); }
    double getB(int sectionCount) const;
    double getG(int sectionCount) const;

    // ── Model ────────────────────────────────────────────────────────────
    ShuntCompensatorModelType getModelType() const {
        return static_cast<ShuntCompensatorModelType>(getInt(prop::SHUNT_MODEL_TYPE));
    }
    ShuntCompensatorModel getModel() const;
    // As a ShuntCompensatorLinearModel or ShuntCompensatorNonLinearModel: ValidationException when
    // it is of the other type
    template <class M> M getModel() const {
        return M(backend_->getRelated(handle_, M::TYPE), backend_);
    }

    // ── Solved values ────────────────────────────────────────────────────
    // The solved section count, when there is one, becomes the section count
    void applySolvedValues();
    void setSectionCountToSolvedSectionCount();
    // (Connectable::unsetSolvedValues unsets the solved section count too)
};

} // namespace iidm
