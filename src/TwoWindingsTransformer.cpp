#include <iidm/TwoWindingsTransformer.h>
#include <iidm/RatioTapChanger.h>
#include <iidm/PhaseTapChanger.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/OperationalLimitsGroup.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

double TwoWindingsTransformer::getR() const {
    return backend_->getDouble(handle_, prop::TWO_WT_R);
}

double TwoWindingsTransformer::getX() const {
    return backend_->getDouble(handle_, prop::TWO_WT_X);
}

double TwoWindingsTransformer::getG() const {
    return backend_->getDouble(handle_, prop::TWO_WT_G);
}

double TwoWindingsTransformer::getB() const {
    return backend_->getDouble(handle_, prop::TWO_WT_B);
}

double TwoWindingsTransformer::getRatedU1() const {
    return backend_->getDouble(handle_, prop::TWO_WT_RATED_U1);
}

double TwoWindingsTransformer::getRatedU2() const {
    return backend_->getDouble(handle_, prop::TWO_WT_RATED_U2);
}

iidm::optional<double> TwoWindingsTransformer::getRatedS() const {
    return backend_->getOptDouble(handle_, prop::TWO_WT_RATED_S);
}

bool TwoWindingsTransformer::hasRatioTapChanger() const {
    return backend_->getBool(handle_, prop::TWO_WT_RTC_EXISTS);
}

iidm::optional<RatioTapChanger> TwoWindingsTransformer::getRatioTapChanger() const {
    if (!hasRatioTapChanger()) return {};
    return RatioTapChanger(handle_, backend_, RatioTapChangerConfig{
        prop::TWO_WT_RTC_EXISTS,
        prop::TWO_WT_RTC_TAP_POSITION,
        prop::TWO_WT_RTC_LOW_TAP,
        prop::TWO_WT_RTC_HIGH_TAP,
        prop::TWO_WT_RTC_REGULATING,
        prop::TWO_WT_RTC_TARGET_V,
        prop::TWO_WT_RTC_STEP,
        prop::TWO_WT_RTC_TARGET_DEADBAND,
        prop::TWO_WT_RTC_REG_TERMINAL_ID,
        prop::REL_TWO_WT_RTC_REG_TERMINAL,
        prop::TWO_WT_RTC_LOAD_TAP_CAP
    });
}

int TwoWindingsTransformer::getRatioTapPosition() const {
    return backend_->getInt(handle_, prop::TWO_WT_RTC_TAP_POSITION);
}

TwoWindingsTransformer& TwoWindingsTransformer::setRatioTapPosition(int pos) {
    backend_->setInt(handle_, prop::TWO_WT_RTC_TAP_POSITION, pos);
    return *this;
}

int TwoWindingsTransformer::getRatioTapLow() const {
    return backend_->getInt(handle_, prop::TWO_WT_RTC_LOW_TAP);
}

int TwoWindingsTransformer::getRatioTapHigh() const {
    return backend_->getInt(handle_, prop::TWO_WT_RTC_HIGH_TAP);
}

bool TwoWindingsTransformer::isRatioTapRegulating() const {
    return backend_->getBool(handle_, prop::TWO_WT_RTC_REGULATING);
}

TwoWindingsTransformer& TwoWindingsTransformer::setRatioTapRegulating(bool regulating) {
    backend_->setBool(handle_, prop::TWO_WT_RTC_REGULATING, regulating);
    return *this;
}

double TwoWindingsTransformer::getRatioTapTargetV() const {
    return backend_->getDouble(handle_, prop::TWO_WT_RTC_TARGET_V);
}

TwoWindingsTransformer& TwoWindingsTransformer::setRatioTapTargetV(double v) {
    backend_->setDouble(handle_, prop::TWO_WT_RTC_TARGET_V, v);
    return *this;
}

bool TwoWindingsTransformer::hasPhaseTapChanger() const {
    return backend_->getBool(handle_, prop::TWO_WT_PTC_EXISTS);
}

iidm::optional<PhaseTapChanger> TwoWindingsTransformer::getPhaseTapChanger() const {
    if (!hasPhaseTapChanger()) return {};
    return PhaseTapChanger(handle_, backend_, PhaseTapChangerConfig{
        prop::TWO_WT_PTC_EXISTS,
        prop::TWO_WT_PTC_TAP_POSITION,
        prop::TWO_WT_PTC_LOW_TAP,
        prop::TWO_WT_PTC_HIGH_TAP,
        prop::TWO_WT_PTC_REGULATING,
        prop::TWO_WT_PTC_REG_MODE,
        prop::TWO_WT_PTC_REG_VALUE,
        prop::TWO_WT_PTC_STEP,
        prop::TWO_WT_PTC_TARGET_DEADBAND,
        prop::TWO_WT_PTC_REG_TERMINAL_ID
    });
}

int TwoWindingsTransformer::getPhaseTapPosition() const {
    return backend_->getInt(handle_, prop::TWO_WT_PTC_TAP_POSITION);
}

TwoWindingsTransformer& TwoWindingsTransformer::setPhaseTapPosition(int pos) {
    backend_->setInt(handle_, prop::TWO_WT_PTC_TAP_POSITION, pos);
    return *this;
}

int TwoWindingsTransformer::getPhaseTapLow() const {
    return backend_->getInt(handle_, prop::TWO_WT_PTC_LOW_TAP);
}

int TwoWindingsTransformer::getPhaseTapHigh() const {
    return backend_->getInt(handle_, prop::TWO_WT_PTC_HIGH_TAP);
}

bool TwoWindingsTransformer::isPhaseTapRegulating() const {
    return backend_->getBool(handle_, prop::TWO_WT_PTC_REGULATING);
}

TwoWindingsTransformer& TwoWindingsTransformer::setPhaseTapRegulating(bool regulating) {
    backend_->setBool(handle_, prop::TWO_WT_PTC_REGULATING, regulating);
    return *this;
}

PhaseTapChangerRegulationMode TwoWindingsTransformer::getPhaseTapRegulationMode() const {
    return static_cast<PhaseTapChangerRegulationMode>(
        backend_->getInt(handle_, prop::TWO_WT_PTC_REG_MODE));
}

TwoWindingsTransformer& TwoWindingsTransformer::setPhaseTapRegulationMode(
        PhaseTapChangerRegulationMode mode) {
    backend_->setInt(handle_, prop::TWO_WT_PTC_REG_MODE, static_cast<int>(mode));
    return *this;
}

double TwoWindingsTransformer::getPhaseTapRegulationValue() const {
    return backend_->getDouble(handle_, prop::TWO_WT_PTC_REG_VALUE);
}

TwoWindingsTransformer& TwoWindingsTransformer::setPhaseTapRegulationValue(double val) {
    backend_->setDouble(handle_, prop::TWO_WT_PTC_REG_VALUE, val);
    return *this;
}

// ── Extensions ───────────────────────────────────────────────────────────────

bool TwoWindingsTransformer::hasTwoWindingsTransformerToBeEstimated() const {
    return backend_->getBool(handle_, prop::EXT_TWO_WT_TBE_EXISTS);
}

TwoWindingsTransformerToBeEstimated TwoWindingsTransformer::getTwoWindingsTransformerToBeEstimated() const {
    return TwoWindingsTransformerToBeEstimated(handle_, backend_);
}

} // namespace iidm
