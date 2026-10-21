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

    bool shouldEstimateRatioTapChanger() const { return getBool(prop::EXT_TWO_WT_TBE_RTC); }
    TwoWindingsTransformerToBeEstimated& shouldEstimateRatioTapChanger(bool toBeEstimated) {
        return setBool(*this, prop::EXT_TWO_WT_TBE_RTC, toBeEstimated);
    }

    bool shouldEstimatePhaseTapChanger() const { return getBool(prop::EXT_TWO_WT_TBE_PTC); }
    TwoWindingsTransformerToBeEstimated& shouldEstimatePhaseTapChanger(bool toBeEstimated) {
        return setBool(*this, prop::EXT_TWO_WT_TBE_PTC, toBeEstimated);
    }
};

} // namespace iidm
