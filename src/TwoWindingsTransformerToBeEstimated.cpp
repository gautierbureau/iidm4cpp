#include <iidm/TwoWindingsTransformerToBeEstimated.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

TwoWindingsTransformerToBeEstimated::TwoWindingsTransformerToBeEstimated(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

bool TwoWindingsTransformerToBeEstimated::shouldEstimateRatioTapChanger() const {
    return backend_->getBool(handle_, prop::EXT_TWO_WT_TBE_RTC);
}

TwoWindingsTransformerToBeEstimated& TwoWindingsTransformerToBeEstimated::shouldEstimateRatioTapChanger(bool toBeEstimated) {
    backend_->setBool(handle_, prop::EXT_TWO_WT_TBE_RTC, toBeEstimated);
    return *this;
}

bool TwoWindingsTransformerToBeEstimated::shouldEstimatePhaseTapChanger() const {
    return backend_->getBool(handle_, prop::EXT_TWO_WT_TBE_PTC);
}

TwoWindingsTransformerToBeEstimated& TwoWindingsTransformerToBeEstimated::shouldEstimatePhaseTapChanger(bool toBeEstimated) {
    backend_->setBool(handle_, prop::EXT_TWO_WT_TBE_PTC, toBeEstimated);
    return *this;
}

} // namespace iidm
