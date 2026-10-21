#pragma once

#include <iidm/Adder.h>
#include <iidm/RatioTapChanger.h>
#include <iidm/TapChangerStepsReplacer.h>
#include <iidm/VoltageRegulationAdder.h>
#include <string>

namespace iidm {

// New ratio tap changer of a two windings transformer or of a three windings transformer leg
// (newRatioTapChanger()), which replaces the current one: its steps, from the low tap position, and
// its tap position; it regulates through its voltage regulation (newVoltageRegulation()).
//   transformer.newRatioTapChanger().setLowTapPosition(0).setTapPosition(1)
//       .beginStep().setRho(0.95).setR(0).setX(0).setG(0).setB(0).endStep()
//       .beginStep().setRho(1.05).setR(0).setX(0).setG(0).setB(0).endStep()
//       .add();
class RatioTapChangerAdder : public detail::AdderBase<RatioTapChangerAdder> {
public:
    RatioTapChangerAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    RatioTapChangerAdder& setLowTapPosition(int lowTapPosition) { return intField(prop::TC_LOW_TAP_POSITION, lowTapPosition); }
    RatioTapChangerAdder& setTapPosition(int tapPosition) { return intField(prop::TC_TAP_POSITION, tapPosition); }
    RatioTapChangerAdder& setSolvedTapPosition(int solvedTapPosition) {
        return intField(prop::TC_SOLVED_TAP_POSITION, solvedTapPosition);
    }
    RatioTapChangerAdder& setLoadTapChangingCapabilities(bool loadTapChangingCapabilities) {
        return boolField(prop::TC_LOAD_TAP_CHANGING_CAPABILITIES, loadTapChangingCapabilities);
    }
    RatioTapChangerAdder& setProperty(const std::string& name, const std::string& value) {
        return keyedStringField(prop::PROPERTY, name, value);
    }

    VoltageRegulationAdder<RatioTapChangerAdder> newVoltageRegulation() {
        return VoltageRegulationAdder<RatioTapChangerAdder>(backend_->invoke(adder_, prop::VOLTAGE_REGULATION), backend_, *this);
    }
    RatioTapChangerStepAdder<RatioTapChangerAdder> beginStep() {
        return RatioTapChangerStepAdder<RatioTapChangerAdder>(backend_->invoke(adder_, prop::RATIO_TAP_CHANGER_STEP), backend_, *this);
    }

    RatioTapChanger add() { return RatioTapChanger(create(), backend_); }
};

} // namespace iidm
