#pragma once

#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl CoordinatedReactiveControl extension attached to a Generator.
class CoordinatedReactiveControl {
public:
    CoordinatedReactiveControl() = default;
    explicit CoordinatedReactiveControl(ObjectHandle handle, BackendProvider* backend);

    double getQPercent() const;
    CoordinatedReactiveControl& setQPercent(double qPercent);

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const CoordinatedReactiveControl& other) const;
    bool operator!=(const CoordinatedReactiveControl& other) const;

private:
    ObjectHandle     handle_  = INVALID_HANDLE;
    BackendProvider* backend_ = nullptr;
};

} // namespace iidm
