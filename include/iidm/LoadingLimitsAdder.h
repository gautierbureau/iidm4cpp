#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/LoadingLimits.h>
#include <string>

namespace iidm {

// New temporary limit, inside a limits adder; endTemporaryLimit() returns to it:
//   group.newCurrentLimits().setPermanentLimit(1000)
//     .beginTemporaryLimit().setName("TL20").setAcceptableDuration(1200).setValue(1200).endTemporaryLimit()
//     .add();
template <class Parent>
class TemporaryLimitAdder : public detail::AdderBase<TemporaryLimitAdder<Parent>> {
public:
    TemporaryLimitAdder(ObjectHandle adder, BackendProvider* backend, Parent& parent)
        : detail::AdderBase<TemporaryLimitAdder<Parent>>(adder, backend), parent_(parent) {}

    TemporaryLimitAdder& setName(const std::string& name)        { return this->stringField(prop::TL_NAME, name); }
    TemporaryLimitAdder& setValue(double value)                  { return this->doubleField(prop::TL_VALUE, value); }
    // Seconds
    TemporaryLimitAdder& setAcceptableDuration(int duration)     { return this->intField(prop::TL_ACCEPTABLE_DURATION, duration); }
    TemporaryLimitAdder& setFictitious(bool fictitious)          { return this->boolField(prop::TL_FICTITIOUS, fictitious); }
    // A free name is derived from the given one if another temporary limit has it
    TemporaryLimitAdder& ensureNameUnicity()                     { return this->boolField(prop::ENSURE_ID_UNICITY, true); }
    TemporaryLimitAdder& addProperty(const std::string& name, const std::string& value) {
        return this->keyedStringField(prop::PROPERTY, name, value);
    }

    Parent& endTemporaryLimit() {
        this->create();
        return parent_;
    }

private:
    Parent& parent_;
};

namespace detail {

// What every limits adder sets (powsybl LoadingLimitsAdder); add() returns the new limits, L
template <class Self, class L>
class LoadingLimitsAdderBase : public AdderBase<Self> {
public:
    Self& setPermanentLimit(double permanentLimit)                 { return this->doubleField(prop::LL_PERMANENT_LIMIT, permanentLimit); }
    Self& setPermanentLimitName(const std::string& name)           { return this->stringField(prop::LL_PERMANENT_LIMIT_NAME, name); }
    Self& setDetectionKind(DetectionKind detectionKind)            { return this->intField(prop::LL_DETECTION_KIND, static_cast<int>(detectionKind)); }
    Self& setProperty(const std::string& name, const std::string& value) {
        return this->keyedStringField(prop::PROPERTY, name, value);
    }

    TemporaryLimitAdder<Self> beginTemporaryLimit() {
        return TemporaryLimitAdder<Self>(this->backend_->invoke(this->adder_, prop::TEMPORARY_LIMIT), this->backend_,
                                         static_cast<Self&>(*this));
    }

    L add() { return L(this->create(), this->backend_); }

protected:
    LoadingLimitsAdderBase(ObjectHandle adder, BackendProvider* backend) : AdderBase<Self>(adder, backend) {}
};

} // namespace detail

// New limits of an operational limits group (OperationalLimitsGroup::newCurrentLimits()...)
class CurrentLimitsAdder : public detail::LoadingLimitsAdderBase<CurrentLimitsAdder, CurrentLimits> {
public:
    CurrentLimitsAdder(ObjectHandle adder, BackendProvider* backend) : LoadingLimitsAdderBase(adder, backend) {}
};

class ActivePowerLimitsAdder : public detail::LoadingLimitsAdderBase<ActivePowerLimitsAdder, ActivePowerLimits> {
public:
    ActivePowerLimitsAdder(ObjectHandle adder, BackendProvider* backend) : LoadingLimitsAdderBase(adder, backend) {}
};

class ApparentPowerLimitsAdder : public detail::LoadingLimitsAdderBase<ApparentPowerLimitsAdder, ApparentPowerLimits> {
public:
    ApparentPowerLimitsAdder(ObjectHandle adder, BackendProvider* backend) : LoadingLimitsAdderBase(adder, backend) {}
};

} // namespace iidm
