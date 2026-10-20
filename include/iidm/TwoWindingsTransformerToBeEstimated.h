#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The TwoWindingsTransformerToBeEstimated extension of a two windings transformer: which of its
// tap changers a state estimator must estimate. An object of its own, from
// TwoWindingsTransformer::getTwoWindingsTransformerToBeEstimated().
class TwoWindingsTransformerToBeEstimated : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::TWO_WINDINGS_TRANSFORMER_TO_BE_ESTIMATED;

    TwoWindingsTransformerToBeEstimated() = default;
    explicit TwoWindingsTransformerToBeEstimated(ObjectHandle handle, BackendProvider* backend);

    bool shouldEstimateRatioTapChanger() const;
    TwoWindingsTransformerToBeEstimated& shouldEstimateRatioTapChanger(bool toBeEstimated);

    bool shouldEstimatePhaseTapChanger() const;
    TwoWindingsTransformerToBeEstimated& shouldEstimatePhaseTapChanger(bool toBeEstimated);
};

} // namespace iidm
