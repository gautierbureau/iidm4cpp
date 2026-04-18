#include <iidm/ReactiveCapabilityCurve.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

ReactiveCapabilityCurve::ReactiveCapabilityCurve(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::vector<ReactiveCapabilityCurve::Point> ReactiveCapabilityCurve::getPoints() const {
    auto handles = backend_->getChildren(handle_, prop::REACTIVE_CURVE_POINT);
    std::vector<Point> pts;
    pts.reserve(handles.size());
    for (auto h : handles) {
        pts.push_back({
            backend_->getDouble(h, prop::POINT_P),
            backend_->getDouble(h, prop::POINT_MIN_Q),
            backend_->getDouble(h, prop::POINT_MAX_Q)
        });
    }
    return pts;
}

bool ReactiveCapabilityCurve::operator==(const ReactiveCapabilityCurve& other) const {
    return handle_ == other.handle_;
}

bool ReactiveCapabilityCurve::operator!=(const ReactiveCapabilityCurve& other) const {
    return !(*this == other);
}

} // namespace iidm
