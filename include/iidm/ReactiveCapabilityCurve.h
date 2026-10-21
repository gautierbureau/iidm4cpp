#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/PropertiesHolder.h>
#include <iidm/ReactiveLimits.h>
#include <vector>

namespace iidm {

// Reactive limits given at some active powers, interpolated in between
class ReactiveCapabilityCurve : public ReactiveLimits {
public:
    // The limits at one active power
    class Point : public detail::ObjectRef, public detail::PropertiesHolder<Point> {
    public:
        static constexpr int TYPE = prop::REACTIVE_CURVE_POINT;

        Point() = default;
        explicit Point(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

        double getP() const { return getDouble(prop::POINT_P); }
        double getMinQ() const { return getDouble(prop::POINT_MIN_Q); }
        double getMaxQ() const { return getDouble(prop::POINT_MAX_Q); }
    };

    static constexpr int TYPE = prop::REACTIVE_CAPABILITY_CURVE;

    ReactiveCapabilityCurve() = default;
    explicit ReactiveCapabilityCurve(ObjectHandle handle, BackendProvider* backend) : ReactiveLimits(handle, backend) {}

    // By increasing active power
    std::vector<Point> getPoints() const;
    int getPointCount() const { return getInt(prop::CURVE_POINT_COUNT); }
    double getMinP() const { return getDouble(prop::CURVE_MIN_P); }
    double getMaxP() const { return getDouble(prop::CURVE_MAX_P); }

    // Beyond the curve, the limits of its first or last point, or with extrapolateReactiveLimitSlope
    // the slope of its first or last two points
    double getMinQ(double p, bool extrapolateReactiveLimitSlope) const;
    double getMaxQ(double p, bool extrapolateReactiveLimitSlope) const;
    using ReactiveLimits::getMinQ;
    using ReactiveLimits::getMaxQ;
};

} // namespace iidm
