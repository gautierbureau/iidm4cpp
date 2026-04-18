#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/BackendProvider.h>
#include <iidm/CurrentLimits.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

// ── Leg ──────────────────────────────────────────────────────────────────────

ThreeWindingsTransformer::Leg::Leg(ObjectHandle twtHandle, BackendProvider* backend,
                                    int legBase, int relTerminal,
                                    int rtcStepChildType, int ptcStepChildType)
    : handle_(twtHandle), backend_(backend), legBase_(legBase),
      relTerminal_(relTerminal),
      rtcStepChildType_(rtcStepChildType), ptcStepChildType_(ptcStepChildType) {}

double ThreeWindingsTransformer::Leg::getR() const {
    return backend_->getDouble(handle_, legBase_ + prop::THREE_WT_LEG_R_OFF);
}
double ThreeWindingsTransformer::Leg::getX() const {
    return backend_->getDouble(handle_, legBase_ + prop::THREE_WT_LEG_X_OFF);
}
double ThreeWindingsTransformer::Leg::getG() const {
    return backend_->getDouble(handle_, legBase_ + prop::THREE_WT_LEG_G_OFF);
}
double ThreeWindingsTransformer::Leg::getB() const {
    return backend_->getDouble(handle_, legBase_ + prop::THREE_WT_LEG_B_OFF);
}
double ThreeWindingsTransformer::Leg::getRatedU() const {
    return backend_->getDouble(handle_, legBase_ + prop::THREE_WT_LEG_RATED_U_OFF);
}
std::optional<double> ThreeWindingsTransformer::Leg::getRatedS() const {
    return backend_->getOptDouble(handle_, legBase_ + prop::THREE_WT_LEG_RATED_S_OFF);
}

bool ThreeWindingsTransformer::Leg::hasRatioTapChanger() const {
    return backend_->getBool(handle_, legBase_ + prop::THREE_WT_LEG_RTC_EXISTS_OFF);
}
RatioTapChanger ThreeWindingsTransformer::Leg::getRatioTapChanger() const {
    int legIdx = (legBase_ - prop::THREE_WT_LEG1_BASE) / 20;
    return RatioTapChanger(handle_, backend_, RatioTapChangerConfig{
        legBase_ + prop::THREE_WT_LEG_RTC_EXISTS_OFF,
        legBase_ + prop::THREE_WT_LEG_RTC_TAP_POS_OFF,
        legBase_ + prop::THREE_WT_LEG_RTC_LOW_TAP_OFF,
        legBase_ + prop::THREE_WT_LEG_RTC_HIGH_TAP_OFF,
        legBase_ + prop::THREE_WT_LEG_RTC_REGULATING_OFF,
        legBase_ + prop::THREE_WT_LEG_RTC_TARGET_V_OFF,
        rtcStepChildType_,
        prop::THREE_WT_LEG1_RTC_TARGET_DEADBAND + legIdx * 2,
        prop::THREE_WT_LEG1_RTC_REG_TERMINAL_ID + legIdx * 2
    });
}

bool ThreeWindingsTransformer::Leg::hasPhaseTapChanger() const {
    return backend_->getBool(handle_, legBase_ + prop::THREE_WT_LEG_PTC_EXISTS_OFF);
}
PhaseTapChanger ThreeWindingsTransformer::Leg::getPhaseTapChanger() const {
    int legIdx = (legBase_ - prop::THREE_WT_LEG1_BASE) / 20;
    return PhaseTapChanger(handle_, backend_, PhaseTapChangerConfig{
        legBase_ + prop::THREE_WT_LEG_PTC_EXISTS_OFF,
        legBase_ + prop::THREE_WT_LEG_PTC_TAP_POS_OFF,
        legBase_ + prop::THREE_WT_LEG_PTC_LOW_TAP_OFF,
        legBase_ + prop::THREE_WT_LEG_PTC_HIGH_TAP_OFF,
        legBase_ + prop::THREE_WT_LEG_PTC_REGULATING_OFF,
        legBase_ + prop::THREE_WT_LEG_PTC_REG_MODE_OFF,
        legBase_ + prop::THREE_WT_LEG_PTC_REG_VALUE_OFF,
        ptcStepChildType_,
        prop::THREE_WT_LEG1_PTC_TARGET_DEADBAND + legIdx * 2,
        prop::THREE_WT_LEG1_PTC_REG_TERMINAL_ID + legIdx * 2
    });
}

Terminal ThreeWindingsTransformer::Leg::getTerminal() const {
    return Terminal(backend_->getRelated(handle_, relTerminal_), backend_);
}

std::optional<CurrentLimits> ThreeWindingsTransformer::Leg::getCurrentLimits() const {
    int legIdx = (legBase_ - prop::THREE_WT_LEG1_BASE) / 20; // 0, 1, or 2
    ObjectHandle h = backend_->getRelated(handle_, prop::REL_CURRENT_LIMITS1 + legIdx);
    if (h == INVALID_HANDLE) return std::nullopt;
    return CurrentLimits(h, backend_);
}

// ── ThreeWindingsTransformer ──────────────────────────────────────────────────

ThreeWindingsTransformer::ThreeWindingsTransformer(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string ThreeWindingsTransformer::getId() const {
    return backend_->getString(handle_, prop::ID);
}
std::string ThreeWindingsTransformer::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

ThreeWindingsTransformer::Leg ThreeWindingsTransformer::getLeg1() const {
    return Leg(handle_, backend_, prop::THREE_WT_LEG1_BASE,
               prop::REL_TERMINAL1,
               prop::THREE_WT_LEG1_RTC_STEP, prop::THREE_WT_LEG1_PTC_STEP);
}
ThreeWindingsTransformer::Leg ThreeWindingsTransformer::getLeg2() const {
    return Leg(handle_, backend_, prop::THREE_WT_LEG2_BASE,
               prop::REL_TERMINAL2,
               prop::THREE_WT_LEG2_RTC_STEP, prop::THREE_WT_LEG2_PTC_STEP);
}
ThreeWindingsTransformer::Leg ThreeWindingsTransformer::getLeg3() const {
    return Leg(handle_, backend_, prop::THREE_WT_LEG3_BASE,
               prop::REL_TERMINAL,
               prop::THREE_WT_LEG3_RTC_STEP, prop::THREE_WT_LEG3_PTC_STEP);
}

Terminal ThreeWindingsTransformer::getTerminal1() const {
    return Terminal(backend_->getRelated(handle_, prop::REL_TERMINAL1), backend_);
}
Terminal ThreeWindingsTransformer::getTerminal2() const {
    return Terminal(backend_->getRelated(handle_, prop::REL_TERMINAL2), backend_);
}
Terminal ThreeWindingsTransformer::getTerminal3() const {
    return Terminal(backend_->getRelated(handle_, prop::REL_TERMINAL), backend_);
}

void ThreeWindingsTransformer::connect() {
    getTerminal1().connect();
    getTerminal2().connect();
    getTerminal3().connect();
}
void ThreeWindingsTransformer::disconnect() {
    getTerminal1().disconnect();
    getTerminal2().disconnect();
    getTerminal3().disconnect();
}
bool ThreeWindingsTransformer::isConnected() const {
    return getTerminal1().isConnected()
        && getTerminal2().isConnected()
        && getTerminal3().isConnected();
}

bool ThreeWindingsTransformer::operator==(const ThreeWindingsTransformer& other) const {
    return handle_ == other.handle_;
}
bool ThreeWindingsTransformer::operator!=(const ThreeWindingsTransformer& other) const {
    return !(*this == other);
}

} // namespace iidm
