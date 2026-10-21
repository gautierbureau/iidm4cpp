#include <iidm/ReactiveLimits.h>
#include <iidm/BackendProvider.h>
#include <iidm/MinMaxReactiveLimits.h>
#include <iidm/ReactiveCapabilityCurve.h>
#include <iidm/ReactiveCapabilityShape.h>
#include "Wrap.h"

namespace iidm {

// ── ReactiveLimits ───────────────────────────────────────────────────────────

ReactiveLimitsKind ReactiveLimits::getKind() const {
    return static_cast<ReactiveLimitsKind>(backend_->getInt(handle_, prop::RL_KIND));
}

double ReactiveLimits::getMinQ(double p) const {
    return backend_->evaluate(handle_, prop::RL_MIN_Q, {p});
}

double ReactiveLimits::getMaxQ(double p) const {
    return backend_->evaluate(handle_, prop::RL_MAX_Q, {p});
}

// ── MinMaxReactiveLimits ─────────────────────────────────────────────────────

double MinMaxReactiveLimits::getMinQ() const {
    return backend_->getDouble(handle_, prop::RL_MIN_Q);
}

double MinMaxReactiveLimits::getMaxQ() const {
    return backend_->getDouble(handle_, prop::RL_MAX_Q);
}

// ── ReactiveCapabilityCurve ──────────────────────────────────────────────────

double ReactiveCapabilityCurve::Point::getP() const {
    return backend_->getDouble(handle_, prop::POINT_P);
}

double ReactiveCapabilityCurve::Point::getMinQ() const {
    return backend_->getDouble(handle_, prop::POINT_MIN_Q);
}

double ReactiveCapabilityCurve::Point::getMaxQ() const {
    return backend_->getDouble(handle_, prop::POINT_MAX_Q);
}

std::vector<ReactiveCapabilityCurve::Point> ReactiveCapabilityCurve::getPoints() const {
    return detail::wrapAll<Point>(backend_->getChildren(handle_, Point::TYPE), backend_);
}

int ReactiveCapabilityCurve::getPointCount() const {
    return backend_->getInt(handle_, prop::CURVE_POINT_COUNT);
}

double ReactiveCapabilityCurve::getMinP() const {
    return backend_->getDouble(handle_, prop::CURVE_MIN_P);
}

double ReactiveCapabilityCurve::getMaxP() const {
    return backend_->getDouble(handle_, prop::CURVE_MAX_P);
}

double ReactiveCapabilityCurve::getMinQ(double p, bool extrapolateReactiveLimitSlope) const {
    return backend_->evaluate(handle_, prop::RL_MIN_Q, {p, extrapolateReactiveLimitSlope ? 1.0 : 0.0});
}

double ReactiveCapabilityCurve::getMaxQ(double p, bool extrapolateReactiveLimitSlope) const {
    return backend_->evaluate(handle_, prop::RL_MAX_Q, {p, extrapolateReactiveLimitSlope ? 1.0 : 0.0});
}

// ── ReactiveCapabilityShape ──────────────────────────────────────────────────

double ReactiveCapabilityShapePlane::getAlpha() const {
    return backend_->getDouble(handle_, prop::PLANE_ALPHA);
}

double ReactiveCapabilityShapePlane::getBeta() const {
    return backend_->getDouble(handle_, prop::PLANE_BETA);
}

double ReactiveCapabilityShapePlane::getGamma() const {
    return backend_->getDouble(handle_, prop::PLANE_GAMMA);
}

double ReactiveCapabilityShapePlane::getDelta() const {
    return backend_->getDouble(handle_, prop::PLANE_DELTA);
}

bool ReactiveCapabilityShapePlane::isLessOrEqual() const {
    return backend_->getBool(handle_, prop::PLANE_LESS_OR_EQUAL);
}

bool ReactiveCapabilityShapePlane::isGreaterOrEqual() const {
    return backend_->getBool(handle_, prop::PLANE_GREATER_OR_EQUAL);
}

ReactiveCapabilityShapePlane& ReactiveCapabilityShapePlane::lessOrEqual(double gamma) {
    backend_->setDouble(handle_, prop::PLANE_LESS_OR_EQUAL, gamma);
    return *this;
}

ReactiveCapabilityShapePlane& ReactiveCapabilityShapePlane::greaterOrEqual(double gamma) {
    backend_->setDouble(handle_, prop::PLANE_GREATER_OR_EQUAL, gamma);
    return *this;
}

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
