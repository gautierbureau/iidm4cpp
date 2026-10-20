#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/Terminal.h>
#include <iidm/VoltageRegulation.h>

namespace iidm {

namespace detail {

// Fields of a voltage regulation: shared by its adder (inside a new holder's adder) and its
// builder (on an existing holder)
template <class Self>
class VoltageRegulationFields : public AdderBase<Self> {
public:
    // Remote regulation only: a local regulation has a NaN target value (see VoltageRegulation)
    Self& withTargetValue(double targetValue)       { return this->doubleField(prop::VR_TARGET_VALUE, targetValue); }
    Self& withTargetDeadband(double targetDeadband) { return this->doubleField(prop::VR_TARGET_DEADBAND, targetDeadband); }
    Self& withSlope(double slope)                   { return this->doubleField(prop::VR_SLOPE, slope); }
    Self& withTerminal(const Terminal& terminal)    { return this->relatedField(prop::REL_TERMINAL, terminal.handle_); }
    Self& withMode(RegulationMode mode)             { return this->intField(prop::VR_MODE, static_cast<int>(mode)); }
    Self& withRegulating(bool regulating)           { return this->boolField(prop::VR_REGULATING, regulating); }

protected:
    VoltageRegulationFields(ObjectHandle adder, BackendProvider* backend) : AdderBase<Self>(adder, backend) {}
};

} // namespace detail

// Voltage regulation of a new holder, inside the holder's adder; add() returns to it:
//   vl.newGenerator().setId("G")...setLocalTargetV(400)
//     .newVoltageRegulation().withMode(RegulationMode::VOLTAGE).withRegulating(true).add()
//     .add();
template <class Parent>
class VoltageRegulationAdder : public detail::VoltageRegulationFields<VoltageRegulationAdder<Parent>> {
public:
    VoltageRegulationAdder(ObjectHandle adder, BackendProvider* backend, Parent& parent)
        : detail::VoltageRegulationFields<VoltageRegulationAdder<Parent>>(adder, backend), parent_(parent) {}

    Parent& add() {
        this->create();
        return parent_;
    }

private:
    Parent& parent_;
};

// New voltage regulation of an existing holder, which replaces the current one: build() returns it
class VoltageRegulationBuilder : public detail::VoltageRegulationFields<VoltageRegulationBuilder> {
public:
    VoltageRegulationBuilder(ObjectHandle builder, BackendProvider* backend)
        : VoltageRegulationFields(builder, backend) {}

    VoltageRegulation build() { return VoltageRegulation(create(), backend_); }
};

} // namespace iidm
