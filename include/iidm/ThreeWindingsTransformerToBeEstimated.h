#pragma once

#include <iidm/Enums.h>
#include <iidm/HandleRef.h>
#include <iidm/ObjectHandle.h>

namespace iidm {

class BackendProvider;

// Typed wrapper for the PowSyBl ThreeWindingsTransformerToBeEstimated extension: tells a state
// estimator which tap changers of a ThreeWindingsTransformer must be estimated.
class ThreeWindingsTransformerToBeEstimated {
public:
    ThreeWindingsTransformerToBeEstimated() = default;
    explicit ThreeWindingsTransformerToBeEstimated(ObjectHandle handle, BackendProvider* backend);

    bool shouldEstimateRatioTapChanger1() const;
    bool shouldEstimateRatioTapChanger2() const;
    bool shouldEstimateRatioTapChanger3() const;
    bool shouldEstimateRatioTapChanger(ThreeSides side) const;

    bool shouldEstimatePhaseTapChanger1() const;
    bool shouldEstimatePhaseTapChanger2() const;
    bool shouldEstimatePhaseTapChanger3() const;
    bool shouldEstimatePhaseTapChanger(ThreeSides side) const;

    ThreeWindingsTransformerToBeEstimated& shouldEstimateRatioTapChanger1(bool toBeEstimated);
    ThreeWindingsTransformerToBeEstimated& shouldEstimateRatioTapChanger2(bool toBeEstimated);
    ThreeWindingsTransformerToBeEstimated& shouldEstimateRatioTapChanger3(bool toBeEstimated);
    ThreeWindingsTransformerToBeEstimated& shouldEstimateRatioTapChanger(bool toBeEstimated, ThreeSides side);

    ThreeWindingsTransformerToBeEstimated& shouldEstimatePhaseTapChanger1(bool toBeEstimated);
    ThreeWindingsTransformerToBeEstimated& shouldEstimatePhaseTapChanger2(bool toBeEstimated);
    ThreeWindingsTransformerToBeEstimated& shouldEstimatePhaseTapChanger3(bool toBeEstimated);
    ThreeWindingsTransformerToBeEstimated& shouldEstimatePhaseTapChanger(bool toBeEstimated, ThreeSides side);

    bool isValid() const { return handle_ != INVALID_HANDLE; }

    bool operator==(const ThreeWindingsTransformerToBeEstimated& other) const;
    bool operator!=(const ThreeWindingsTransformerToBeEstimated& other) const;

private:
    HandleRef  handle_;
    BackendProvider* backend_ = detail::nullBackend();
};

} // namespace iidm
