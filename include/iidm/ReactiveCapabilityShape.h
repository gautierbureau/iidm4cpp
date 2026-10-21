#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/PropertyCodes.h>
#include <iidm/ReactiveLimits.h>
#include <vector>

namespace iidm {

// One of the half-spaces that bound a reactive capability shape: an inequality on the reactive
// power, the voltage and the active power
class ReactiveCapabilityShapePlane : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::REACTIVE_CAPABILITY_SHAPE_PLANE;

    ReactiveCapabilityShapePlane() = default;
    explicit ReactiveCapabilityShapePlane(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    double getAlpha() const { return getDouble(prop::PLANE_ALPHA); }
    double getBeta() const { return getDouble(prop::PLANE_BETA); }
    double getGamma() const { return getDouble(prop::PLANE_GAMMA); }
    double getDelta() const { return getDouble(prop::PLANE_DELTA); }
    bool isLessOrEqual() const { return getBool(prop::PLANE_LESS_OR_EQUAL); }
    bool isGreaterOrEqual() const { return getBool(prop::PLANE_GREATER_OR_EQUAL); }
    // The inequality made <= gamma, or >= gamma, in place: the shape changes with it
    ReactiveCapabilityShapePlane& lessOrEqual(double gamma) {
        return setDouble(*this, prop::PLANE_LESS_OR_EQUAL, gamma);
    }
    ReactiveCapabilityShapePlane& greaterOrEqual(double gamma) {
        return setDouble(*this, prop::PLANE_GREATER_OR_EQUAL, gamma);
    }
};

// Reactive limits that depend on the active power and the voltage: a polyhedron in the (q, v, p)
// space, given by the planes that bound it
class ReactiveCapabilityShape : public ReactiveLimits {
public:
    static constexpr int TYPE = prop::REACTIVE_CAPABILITY_SHAPE;

    ReactiveCapabilityShape() = default;
    explicit ReactiveCapabilityShape(ObjectHandle handle, BackendProvider* backend) : ReactiveLimits(handle, backend) {}

    // The limits at an active power and a voltage
    double getMinQ(double p, double v) const;
    double getMaxQ(double p, double v) const;
    using ReactiveLimits::getMinQ;
    using ReactiveLimits::getMaxQ;

    bool isInside(double p, double q, double v) const;
    std::vector<ReactiveCapabilityShapePlane> getPlanes() const;
};

} // namespace iidm
