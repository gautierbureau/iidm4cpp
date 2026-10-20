#pragma once

#include <iidm/Enums.h>
#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The ThreeWindingsTransformerToBeEstimated extension of a three windings transformer: which of
// its tap changers a state estimator must estimate. An object of its own, from
// ThreeWindingsTransformer::getThreeWindingsTransformerToBeEstimated().
class ThreeWindingsTransformerToBeEstimated : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::THREE_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED;

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
};

} // namespace iidm
