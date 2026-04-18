#include <iidm/TwoWindingsTransformer.h>
#include <iidm/RatioTapChanger.h>
#include <iidm/PhaseTapChanger.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

TwoWindingsTransformer::TwoWindingsTransformer(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string TwoWindingsTransformer::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string TwoWindingsTransformer::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

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

std::optional<double> TwoWindingsTransformer::getRatedS() const {
    return backend_->getOptDouble(handle_, prop::TWO_WT_RATED_S);
}

bool TwoWindingsTransformer::hasRatioTapChanger() const {
    return backend_->getBool(handle_, prop::TWO_WT_RTC_EXISTS);
}

std::optional<RatioTapChanger> TwoWindingsTransformer::getRatioTapChanger() const {
    if (!hasRatioTapChanger()) return std::nullopt;
    return RatioTapChanger(handle_, backend_, RatioTapChangerConfig{
        prop::TWO_WT_RTC_EXISTS,
        prop::TWO_WT_RTC_TAP_POSITION,
        prop::TWO_WT_RTC_LOW_TAP,
        prop::TWO_WT_RTC_HIGH_TAP,
        prop::TWO_WT_RTC_REGULATING,
        prop::TWO_WT_RTC_TARGET_V,
        prop::TWO_WT_RTC_STEP,
        prop::TWO_WT_RTC_TARGET_DEADBAND,
        prop::TWO_WT_RTC_REG_TERMINAL_ID
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

std::optional<PhaseTapChanger> TwoWindingsTransformer::getPhaseTapChanger() const {
    if (!hasPhaseTapChanger()) return std::nullopt;
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

Terminal TwoWindingsTransformer::getTerminal1() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL1);
    return Terminal(termHandle, backend_);
}

Terminal TwoWindingsTransformer::getTerminal2() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL2);
    return Terminal(termHandle, backend_);
}

void TwoWindingsTransformer::connect() {
    getTerminal1().connect();
    getTerminal2().connect();
}

void TwoWindingsTransformer::disconnect() {
    getTerminal1().disconnect();
    getTerminal2().disconnect();
}

bool TwoWindingsTransformer::isConnected() const {
    return getTerminal1().isConnected() && getTerminal2().isConnected();
}

bool TwoWindingsTransformer::operator==(const TwoWindingsTransformer& other) const {
    return handle_ == other.handle_;
}

bool TwoWindingsTransformer::operator!=(const TwoWindingsTransformer& other) const {
    return !(*this == other);
}

} // namespace iidm
