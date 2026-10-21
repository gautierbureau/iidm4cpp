#pragma once

#include <iidm/Adder.h>
#include <iidm/HandleRef.h>
#include <iidm/ReactiveCapabilityShape.h>
#include <string>

namespace iidm {

// New reactive capability shape of a generator, battery, VSC converter station or boundary line
// generation (newReactiveCapabilityShape()): the planes that bound it, one or more
// (ValidationException from add() otherwise). It replaces the holder's reactive limits.
class ReactiveCapabilityShapeAdder : public detail::AdderBase<ReactiveCapabilityShapeAdder> {
public:
    ReactiveCapabilityShapeAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    // The inequality delta * Q + alpha * U + beta * P <= gamma (>= gamma when isGreaterOrEqual)
    ReactiveCapabilityShapeAdder& addPlane(double alpha, double beta, double delta, bool isGreaterOrEqual, double gamma) {
        HandleRef plane(backend_->invoke(adder_, prop::REACTIVE_CAPABILITY_SHAPE_PLANE), backend_);
        backend_->setDouble(plane, prop::PLANE_ALPHA, alpha);
        backend_->setDouble(plane, prop::PLANE_BETA, beta);
        backend_->setDouble(plane, prop::PLANE_DELTA, delta);
        backend_->setBool(plane, prop::PLANE_GREATER_OR_EQUAL, isGreaterOrEqual);
        backend_->setDouble(plane, prop::PLANE_GAMMA, gamma);
        backend_->invoke(plane, prop::ACT_ADD);
        return *this;
    }
    ReactiveCapabilityShapeAdder& setProperty(const std::string& name, const std::string& value) {
        return keyedStringField(prop::PROPERTY, name, value);
    }

    ReactiveCapabilityShape add() { return ReactiveCapabilityShape(create(), backend_); }
};

} // namespace iidm
