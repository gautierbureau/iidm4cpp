#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <vector>

namespace iidm {

class BackendProvider;

class ReactiveCapabilityCurve : public detail::ObjectRef {
public:
    struct Point {
        double p;
        double minQ;
        double maxQ;
    };

    ReactiveCapabilityCurve() = default;
    explicit ReactiveCapabilityCurve(ObjectHandle handle, BackendProvider* backend);

    std::vector<Point> getPoints() const;
};

} // namespace iidm
