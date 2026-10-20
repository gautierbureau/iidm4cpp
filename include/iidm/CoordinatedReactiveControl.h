#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The CoordinatedReactiveControl extension of a generator: an object of its own, from
// Generator::getCoordinatedReactiveControl().
class CoordinatedReactiveControl : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::COORDINATED_REACTIVE_CONTROL;

    CoordinatedReactiveControl() = default;
    explicit CoordinatedReactiveControl(ObjectHandle handle, BackendProvider* backend);

    // Share (%) of the reactive power the generator takes in the voltage control of its bus
    double getQPercent() const;
    CoordinatedReactiveControl& setQPercent(double qPercent);
};

} // namespace iidm
