#pragma once

#include <iidm/Adder.h>
#include <string>

namespace iidm {

namespace detail {

// What every new tap changer step sets (powsybl TapChangerStepAdder); endStep() returns to the
// parent: a steps replacer, or the adder of a new tap changer
template <class Self, class Parent>
class TapChangerStepAdderBase : public AdderBase<Self> {
public:
    Self& setRho(double rho) { return this->doubleField(prop::STEP_RHO, rho); }
    Self& setR(double r)     { return this->doubleField(prop::STEP_R, r); }
    Self& setX(double x)     { return this->doubleField(prop::STEP_X, x); }
    Self& setG(double g)     { return this->doubleField(prop::STEP_G, g); }
    Self& setB(double b)     { return this->doubleField(prop::STEP_B, b); }
    Self& setProperty(const std::string& name, const std::string& value) {
        return this->keyedStringField(prop::PROPERTY, name, value);
    }

    Parent& endStep() {
        this->create();
        return parent_;
    }

protected:
    TapChangerStepAdderBase(ObjectHandle adder, BackendProvider* backend, Parent& parent)
        : AdderBase<Self>(adder, backend), parent_(parent) {}

private:
    Parent& parent_;
};

} // namespace detail

// New step of a ratio tap changer
template <class Parent>
class RatioTapChangerStepAdder : public detail::TapChangerStepAdderBase<RatioTapChangerStepAdder<Parent>, Parent> {
public:
    RatioTapChangerStepAdder(ObjectHandle adder, BackendProvider* backend, Parent& parent)
        : detail::TapChangerStepAdderBase<RatioTapChangerStepAdder<Parent>, Parent>(adder, backend, parent) {}
};

// New step of a phase tap changer: with a phase shift too
template <class Parent>
class PhaseTapChangerStepAdder : public detail::TapChangerStepAdderBase<PhaseTapChangerStepAdder<Parent>, Parent> {
public:
    PhaseTapChangerStepAdder(ObjectHandle adder, BackendProvider* backend, Parent& parent)
        : detail::TapChangerStepAdderBase<PhaseTapChangerStepAdder<Parent>, Parent>(adder, backend, parent) {}

    // Degrees
    PhaseTapChangerStepAdder& setAlpha(double alpha) { return this->doubleField(prop::PTC_STEP_ALPHA, alpha); }
};

namespace detail {

// Replaces every step of a tap changer at once (powsybl TapChangerStepsReplacer): the new steps,
// from the low tap position, then replaceSteps(). Nothing changes before that; the tap positions
// must then stay within the new steps (ValidationException).
template <class Self, template <class> class StepAdder, int STEP_TYPE>
class TapChangerStepsReplacerBase : public AdderBase<Self> {
public:
    StepAdder<Self> beginStep() {
        return StepAdder<Self>(this->backend_->invoke(this->adder_, STEP_TYPE), this->backend_, static_cast<Self&>(*this));
    }

    void replaceSteps() { this->create(); }

protected:
    TapChangerStepsReplacerBase(ObjectHandle replacer, BackendProvider* backend) : AdderBase<Self>(replacer, backend) {}
};

} // namespace detail

//   rtc.stepsReplacer()
//      .beginStep().setRho(0.95).setR(0).setX(0).setG(0).setB(0).endStep()
//      .beginStep().setRho(1.05).setR(0).setX(0).setG(0).setB(0).endStep()
//      .replaceSteps();
class RatioTapChangerStepsReplacer
    : public detail::TapChangerStepsReplacerBase<RatioTapChangerStepsReplacer, RatioTapChangerStepAdder,
                                                 prop::RATIO_TAP_CHANGER_STEP> {
public:
    RatioTapChangerStepsReplacer(ObjectHandle replacer, BackendProvider* backend)
        : TapChangerStepsReplacerBase(replacer, backend) {}
};

class PhaseTapChangerStepsReplacer
    : public detail::TapChangerStepsReplacerBase<PhaseTapChangerStepsReplacer, PhaseTapChangerStepAdder,
                                                 prop::PHASE_TAP_CHANGER_STEP> {
public:
    PhaseTapChangerStepsReplacer(ObjectHandle replacer, BackendProvider* backend)
        : TapChangerStepsReplacerBase(replacer, backend) {}
};

} // namespace iidm
