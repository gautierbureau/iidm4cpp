#pragma once

#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl TwoWindingsTransformerToBeEstimated extension: tells a state
// estimator which tap changers of a TwoWindingsTransformer must be estimated.
class TwoWindingsTransformerToBeEstimated {
public:
    TwoWindingsTransformerToBeEstimated() = default;
    explicit TwoWindingsTransformerToBeEstimated(ObjectHandle handle, BackendProvider* backend);

    bool shouldEstimateRatioTapChanger() const;
    TwoWindingsTransformerToBeEstimated& shouldEstimateRatioTapChanger(bool toBeEstimated);

    bool shouldEstimatePhaseTapChanger() const;
    TwoWindingsTransformerToBeEstimated& shouldEstimatePhaseTapChanger(bool toBeEstimated);

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const TwoWindingsTransformerToBeEstimated& other) const;
    bool operator!=(const TwoWindingsTransformerToBeEstimated& other) const;

private:
    HandleRef  handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
