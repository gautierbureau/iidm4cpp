#include <iidm/ThreeWindingsTransformerToBeEstimated.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

namespace {

int ratioCode(ThreeSides side) {
    return prop::EXT_THREE_WT_TBE_RTC1 + static_cast<int>(side);
}

int phaseCode(ThreeSides side) {
    return prop::EXT_THREE_WT_TBE_PTC1 + static_cast<int>(side);
}

} // namespace

ThreeWindingsTransformerToBeEstimated::ThreeWindingsTransformerToBeEstimated(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

bool ThreeWindingsTransformerToBeEstimated::shouldEstimateRatioTapChanger1() const {
    return shouldEstimateRatioTapChanger(ThreeSides::ONE);
}

bool ThreeWindingsTransformerToBeEstimated::shouldEstimateRatioTapChanger2() const {
    return shouldEstimateRatioTapChanger(ThreeSides::TWO);
}

bool ThreeWindingsTransformerToBeEstimated::shouldEstimateRatioTapChanger3() const {
    return shouldEstimateRatioTapChanger(ThreeSides::THREE);
}

bool ThreeWindingsTransformerToBeEstimated::shouldEstimateRatioTapChanger(ThreeSides side) const {
    return backend_->getBool(handle_, ratioCode(side));
}

bool ThreeWindingsTransformerToBeEstimated::shouldEstimatePhaseTapChanger1() const {
    return shouldEstimatePhaseTapChanger(ThreeSides::ONE);
}

bool ThreeWindingsTransformerToBeEstimated::shouldEstimatePhaseTapChanger2() const {
    return shouldEstimatePhaseTapChanger(ThreeSides::TWO);
}

bool ThreeWindingsTransformerToBeEstimated::shouldEstimatePhaseTapChanger3() const {
    return shouldEstimatePhaseTapChanger(ThreeSides::THREE);
}

bool ThreeWindingsTransformerToBeEstimated::shouldEstimatePhaseTapChanger(ThreeSides side) const {
    return backend_->getBool(handle_, phaseCode(side));
}

ThreeWindingsTransformerToBeEstimated& ThreeWindingsTransformerToBeEstimated::shouldEstimateRatioTapChanger1(bool toBeEstimated) {
    return shouldEstimateRatioTapChanger(toBeEstimated, ThreeSides::ONE);
}

ThreeWindingsTransformerToBeEstimated& ThreeWindingsTransformerToBeEstimated::shouldEstimateRatioTapChanger2(bool toBeEstimated) {
    return shouldEstimateRatioTapChanger(toBeEstimated, ThreeSides::TWO);
}

ThreeWindingsTransformerToBeEstimated& ThreeWindingsTransformerToBeEstimated::shouldEstimateRatioTapChanger3(bool toBeEstimated) {
    return shouldEstimateRatioTapChanger(toBeEstimated, ThreeSides::THREE);
}

ThreeWindingsTransformerToBeEstimated& ThreeWindingsTransformerToBeEstimated::shouldEstimateRatioTapChanger(bool toBeEstimated, ThreeSides side) {
    backend_->setBool(handle_, ratioCode(side), toBeEstimated);
    return *this;
}

ThreeWindingsTransformerToBeEstimated& ThreeWindingsTransformerToBeEstimated::shouldEstimatePhaseTapChanger1(bool toBeEstimated) {
    return shouldEstimatePhaseTapChanger(toBeEstimated, ThreeSides::ONE);
}

ThreeWindingsTransformerToBeEstimated& ThreeWindingsTransformerToBeEstimated::shouldEstimatePhaseTapChanger2(bool toBeEstimated) {
    return shouldEstimatePhaseTapChanger(toBeEstimated, ThreeSides::TWO);
}

ThreeWindingsTransformerToBeEstimated& ThreeWindingsTransformerToBeEstimated::shouldEstimatePhaseTapChanger3(bool toBeEstimated) {
    return shouldEstimatePhaseTapChanger(toBeEstimated, ThreeSides::THREE);
}

ThreeWindingsTransformerToBeEstimated& ThreeWindingsTransformerToBeEstimated::shouldEstimatePhaseTapChanger(bool toBeEstimated, ThreeSides side) {
    backend_->setBool(handle_, phaseCode(side), toBeEstimated);
    return *this;
}

bool ThreeWindingsTransformerToBeEstimated::operator==(const ThreeWindingsTransformerToBeEstimated& other) const {
    return handle_ == other.handle_;
}

bool ThreeWindingsTransformerToBeEstimated::operator!=(const ThreeWindingsTransformerToBeEstimated& other) const {
    return !(*this == other);
}

} // namespace iidm
