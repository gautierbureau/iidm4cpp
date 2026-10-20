#pragma once

#include <iidm/HandleRef.h>
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
    HandleRef  handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
