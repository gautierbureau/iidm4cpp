#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl TwoWindingsTransformerToBeEstimated extension: tells a state
// estimator which tap changers of a TwoWindingsTransformer must be estimated.
class TwoWindingsTransformerToBeEstimated : public detail::ObjectRef {
public:
    TwoWindingsTransformerToBeEstimated() = default;
    explicit TwoWindingsTransformerToBeEstimated(ObjectHandle handle, BackendProvider* backend);

    bool shouldEstimateRatioTapChanger() const;
    TwoWindingsTransformerToBeEstimated& shouldEstimateRatioTapChanger(bool toBeEstimated);

    bool shouldEstimatePhaseTapChanger() const;
    TwoWindingsTransformerToBeEstimated& shouldEstimatePhaseTapChanger(bool toBeEstimated);
};

} // namespace iidm
