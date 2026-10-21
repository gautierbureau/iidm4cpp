#pragma once

#include <iidm/Adder.h>
#include <iidm/ShuntCompensator.h>
#include <iidm/VoltageRegulationAdder.h>
#include <string>

namespace iidm {

class ShuntCompensatorLinearModelAdder;
class ShuntCompensatorNonLinearModelAdder;

// New shunt compensator of a voltage level (VoltageLevel::newShuntCompensator()): its model, linear
// or non-linear, and its section count:
//   vl.newShuntCompensator().setId("SC").setBus("B").setSectionCount(1)
//       .newLinearModel().setBPerSection(1e-5).setMaximumSectionCount(2).add()
//       .add();
class ShuntCompensatorAdder : public detail::InjectionAdder<ShuntCompensatorAdder> {
public:
    ShuntCompensatorAdder(ObjectHandle adder, BackendProvider* backend) : InjectionAdder(adder, backend) {}

    ShuntCompensatorAdder& setSectionCount(int sectionCount) { return intField(prop::SHUNT_SECTION_COUNT, sectionCount); }
    ShuntCompensatorAdder& setSolvedSectionCount(int solvedSectionCount) {
        return intField(prop::SHUNT_SOLVED_SECTION_COUNT, solvedSectionCount);
    }
    ShuntCompensatorAdder& setLocalTargetV(double localTargetV) { return doubleField(prop::LOCAL_TARGET_V, localTargetV); }
    ShuntCompensatorAdder& setLocalTargetQ(double localTargetQ) { return doubleField(prop::LOCAL_TARGET_Q, localTargetQ); }

    ShuntCompensatorLinearModelAdder newLinearModel();
    ShuntCompensatorNonLinearModelAdder newNonLinearModel();
    VoltageRegulationAdder<ShuntCompensatorAdder> newVoltageRegulation() {
        return VoltageRegulationAdder<ShuntCompensatorAdder>(backend_->invoke(adder_, prop::VOLTAGE_REGULATION), backend_, *this);
    }

    ShuntCompensator add() { return ShuntCompensator(create(), backend_); }
};

// The linear model of a new shunt compensator: add() returns to the shunt's adder
class ShuntCompensatorLinearModelAdder : public detail::AdderBase<ShuntCompensatorLinearModelAdder> {
public:
    ShuntCompensatorLinearModelAdder(ObjectHandle adder, BackendProvider* backend, ShuntCompensatorAdder& parent)
        : AdderBase(adder, backend), parent_(parent) {}

    ShuntCompensatorLinearModelAdder& setBPerSection(double bPerSection) { return doubleField(prop::SHUNT_B_PER_SECTION, bPerSection); }
    ShuntCompensatorLinearModelAdder& setGPerSection(double gPerSection) { return doubleField(prop::SHUNT_G_PER_SECTION, gPerSection); }
    ShuntCompensatorLinearModelAdder& setMaximumSectionCount(int maximumSectionCount) {
        return intField(prop::SHUNT_MAX_SECTION_COUNT, maximumSectionCount);
    }

    ShuntCompensatorAdder& add() {
        create();
        return parent_;
    }

private:
    ShuntCompensatorAdder& parent_;
};

// The non-linear model of a new shunt compensator: its sections, in order; add() returns to the
// shunt's adder
class ShuntCompensatorNonLinearModelAdder : public detail::AdderBase<ShuntCompensatorNonLinearModelAdder> {
public:
    class SectionAdder;

    ShuntCompensatorNonLinearModelAdder(ObjectHandle adder, BackendProvider* backend, ShuntCompensatorAdder& parent)
        : AdderBase(adder, backend), parent_(parent) {}

    SectionAdder beginSection();

    ShuntCompensatorAdder& add() {
        create();
        return parent_;
    }

private:
    ShuntCompensatorAdder& parent_;
};

// A section of a new non-linear model: the susceptance and conductance of the shunt when it is its
// last section in service; endSection() returns to the model's adder
class ShuntCompensatorNonLinearModelAdder::SectionAdder : public detail::AdderBase<SectionAdder> {
public:
    SectionAdder(ObjectHandle adder, BackendProvider* backend, ShuntCompensatorNonLinearModelAdder& parent)
        : AdderBase(adder, backend), parent_(parent) {}

    SectionAdder& setB(double b) { return doubleField(prop::SHUNT_SECTION_B, b); }
    SectionAdder& setG(double g) { return doubleField(prop::SHUNT_SECTION_G, g); }

    ShuntCompensatorNonLinearModelAdder& endSection() {
        create();
        return parent_;
    }

private:
    ShuntCompensatorNonLinearModelAdder& parent_;
};

inline ShuntCompensatorLinearModelAdder ShuntCompensatorAdder::newLinearModel() {
    return ShuntCompensatorLinearModelAdder(backend_->invoke(adder_, prop::SHUNT_COMPENSATOR_LINEAR_MODEL), backend_, *this);
}

inline ShuntCompensatorNonLinearModelAdder ShuntCompensatorAdder::newNonLinearModel() {
    return ShuntCompensatorNonLinearModelAdder(backend_->invoke(adder_, prop::SHUNT_COMPENSATOR_NON_LINEAR_MODEL), backend_, *this);
}

inline ShuntCompensatorNonLinearModelAdder::SectionAdder ShuntCompensatorNonLinearModelAdder::beginSection() {
    return SectionAdder(backend_->invoke(adder_, prop::SHUNT_SECTION), backend_, *this);
}

} // namespace iidm
