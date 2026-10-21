#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/PhaseTapChanger.h>
#include <iidm/TapChangerStepsReplacer.h>
#include <iidm/Terminal.h>
#include <string>

namespace iidm {

// New phase tap changer of a two windings transformer or of a three windings transformer leg
// (newPhaseTapChanger()), which replaces the current one: its steps, from the low tap position, its
// tap position and its regulation.
class PhaseTapChangerAdder : public detail::AdderBase<PhaseTapChangerAdder> {
public:
    PhaseTapChangerAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    PhaseTapChangerAdder& setLowTapPosition(int lowTapPosition) { return intField(prop::TC_LOW_TAP_POSITION, lowTapPosition); }
    PhaseTapChangerAdder& setTapPosition(int tapPosition) { return intField(prop::TC_TAP_POSITION, tapPosition); }
    PhaseTapChangerAdder& setSolvedTapPosition(int solvedTapPosition) {
        return intField(prop::TC_SOLVED_TAP_POSITION, solvedTapPosition);
    }
    PhaseTapChangerAdder& setLoadTapChangingCapabilities(bool loadTapChangingCapabilities) {
        return boolField(prop::TC_LOAD_TAP_CHANGING_CAPABILITIES, loadTapChangingCapabilities);
    }
    PhaseTapChangerAdder& setRegulating(bool regulating) { return boolField(prop::REGULATING, regulating); }
    PhaseTapChangerAdder& setRegulationMode(PhaseTapChangerRegulationMode regulationMode) {
        return intField(prop::PTC_REGULATION_MODE, static_cast<int>(regulationMode));
    }
    // Current (A) or active power (MW) target, after the regulation mode
    PhaseTapChangerAdder& setRegulationValue(double regulationValue) { return doubleField(prop::PTC_REGULATION_VALUE, regulationValue); }
    PhaseTapChangerAdder& setTargetDeadband(double targetDeadband) { return doubleField(prop::PTC_TARGET_DEADBAND, targetDeadband); }
    PhaseTapChangerAdder& setRegulationTerminal(const Terminal& regulationTerminal) {
        return relatedField(prop::REL_REGULATION_TERMINAL, detail::Access::handle(regulationTerminal));
    }
    PhaseTapChangerAdder& setProperty(const std::string& name, const std::string& value) {
        return keyedStringField(prop::PROPERTY, name, value);
    }

    PhaseTapChangerStepAdder<PhaseTapChangerAdder> beginStep() {
        return PhaseTapChangerStepAdder<PhaseTapChangerAdder>(backend_->invoke(adder_, prop::PHASE_TAP_CHANGER_STEP), backend_, *this);
    }

    PhaseTapChanger add() { return PhaseTapChanger(create(), backend_); }
};

} // namespace iidm
