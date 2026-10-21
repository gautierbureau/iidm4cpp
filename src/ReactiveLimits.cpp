#include <iidm/ReactiveLimits.h>
#include <iidm/BackendProvider.h>
#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/ReactiveCapabilityCurve.h>
#include <iidm/ReactiveCapabilityShape.h>
#include "Wrap.h"

namespace iidm {

// ── ReactiveLimits ───────────────────────────────────────────────────────────

double ReactiveLimits::getMinQ(double p) const {
    return backend_->evaluate(handle_, prop::RL_MIN_Q, {p});
}

double ReactiveLimits::getMaxQ(double p) const {
    return backend_->evaluate(handle_, prop::RL_MAX_Q, {p});
}

// ── MinMaxReactiveLimits ─────────────────────────────────────────────────────

// ── ReactiveCapabilityCurve ──────────────────────────────────────────────────

std::vector<ReactiveCapabilityCurve::Point> ReactiveCapabilityCurve::getPoints() const {
    return detail::wrapAll<Point>(backend_->getChildren(handle_, Point::TYPE), backend_);
}

double ReactiveCapabilityCurve::getMinQ(double p, bool extrapolateReactiveLimitSlope) const {
    return backend_->evaluate(handle_, prop::RL_MIN_Q, {p, extrapolateReactiveLimitSlope ? 1.0 : 0.0});
}

double ReactiveCapabilityCurve::getMaxQ(double p, bool extrapolateReactiveLimitSlope) const {
    return backend_->evaluate(handle_, prop::RL_MAX_Q, {p, extrapolateReactiveLimitSlope ? 1.0 : 0.0});
}

// ── ReactiveCapabilityShape ──────────────────────────────────────────────────

double ReactiveCapabilityShape::getMinQ(double p, double v) const {
    return backend_->evaluate(handle_, prop::RL_MIN_Q, {p, v});
}

double ReactiveCapabilityShape::getMaxQ(double p, double v) const {
    return backend_->evaluate(handle_, prop::RL_MAX_Q, {p, v});
}

bool ReactiveCapabilityShape::isInside(double p, double q, double v) const {
    return backend_->evaluate(handle_, prop::SHAPE_IS_INSIDE, {p, q, v}) != 0.0;
}

std::vector<ReactiveCapabilityShapePlane> ReactiveCapabilityShape::getPlanes() const {
    return detail::wrapAll<ReactiveCapabilityShapePlane>(
        backend_->getChildren(handle_, ReactiveCapabilityShapePlane::TYPE), backend_);
}

} // namespace iidm
