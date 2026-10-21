#pragma once

#include <iidm/Adder.h>
#include <iidm/ReactiveCapabilityCurve.h>
#include <string>

namespace iidm {

// New reactive capability curve of a generator, battery, VSC converter station or boundary line
// generation (newReactiveCapabilityCurve()): two points or more, of different active powers
// (ValidationException from add() otherwise). It replaces the holder's reactive limits.
//   generator.newReactiveCapabilityCurve()
//       .beginPoint().setP(0).setMinQ(-100).setMaxQ(100).endPoint()
//       .beginPoint().setP(200).setMinQ(-80).setMaxQ(80).endPoint()
//       .add();
class ReactiveCapabilityCurveAdder : public detail::AdderBase<ReactiveCapabilityCurveAdder> {
public:
    class PointAdder;

    ReactiveCapabilityCurveAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    PointAdder beginPoint();
    ReactiveCapabilityCurveAdder& setProperty(const std::string& name, const std::string& value) {
        return keyedStringField(prop::PROPERTY, name, value);
    }

    ReactiveCapabilityCurve add() { return ReactiveCapabilityCurve(create(), backend_); }
};

// A point of a new curve: endPoint() adds it and returns to the curve's adder
class ReactiveCapabilityCurveAdder::PointAdder : public detail::AdderBase<PointAdder> {
public:
    PointAdder(ObjectHandle adder, BackendProvider* backend, ReactiveCapabilityCurveAdder& parent)
        : AdderBase(adder, backend), parent_(parent) {}

    // MW, MVar
    PointAdder& setP(double p)       { return doubleField(prop::POINT_P, p); }
    PointAdder& setMinQ(double minQ) { return doubleField(prop::POINT_MIN_Q, minQ); }
    PointAdder& setMaxQ(double maxQ) { return doubleField(prop::POINT_MAX_Q, maxQ); }
    PointAdder& setProperty(const std::string& name, const std::string& value) {
        return keyedStringField(prop::PROPERTY, name, value);
    }

    ReactiveCapabilityCurveAdder& endPoint() {
        create();
        return parent_;
    }

private:
    ReactiveCapabilityCurveAdder& parent_;
};

inline ReactiveCapabilityCurveAdder::PointAdder ReactiveCapabilityCurveAdder::beginPoint() {
    return PointAdder(backend_->invoke(adder_, prop::REACTIVE_CURVE_POINT), backend_, *this);
}

} // namespace iidm
