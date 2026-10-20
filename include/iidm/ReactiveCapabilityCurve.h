#pragma once

#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>
#include <vector>

namespace iidm {

class BackendProvider;

class ReactiveCapabilityCurve {
public:
    struct Point {
        double p;
        double minQ;
        double maxQ;
    };

    ReactiveCapabilityCurve() = default;
    explicit ReactiveCapabilityCurve(ObjectHandle handle, BackendProvider* backend);

    std::vector<Point> getPoints() const;

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const ReactiveCapabilityCurve& other) const;
    bool operator!=(const ReactiveCapabilityCurve& other) const;

private:
    HandleRef  handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
