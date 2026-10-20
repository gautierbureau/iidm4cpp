#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl CoordinatedReactiveControl extension attached to a Generator.
class CoordinatedReactiveControl : public detail::ObjectRef {
public:
    CoordinatedReactiveControl() = default;
    explicit CoordinatedReactiveControl(ObjectHandle handle, BackendProvider* backend);

    double getQPercent() const;
    CoordinatedReactiveControl& setQPercent(double qPercent);
};

} // namespace iidm
