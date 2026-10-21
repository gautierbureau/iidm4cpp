#include <iidm/LoadingLimits.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

// ── TemporaryLimit ────────────────────────────────────────────────────────────

TemporaryLimit::TemporaryLimit(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

std::string TemporaryLimit::getName() const {
    return backend_->getString(handle_, prop::TL_NAME);
}

double TemporaryLimit::getValue() const {
    return backend_->getDouble(handle_, prop::TL_VALUE);
}

int TemporaryLimit::getAcceptableDuration() const {
    return backend_->getInt(handle_, prop::TL_ACCEPTABLE_DURATION);
}

bool TemporaryLimit::isFictitious() const {
    return backend_->getBool(handle_, prop::TL_FICTITIOUS);
}

// ── LoadingLimits ─────────────────────────────────────────────────────────────

LoadingLimits::LoadingLimits(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

LimitType LoadingLimits::getLimitType() const {
    return static_cast<LimitType>(backend_->getInt(handle_, prop::LL_LIMIT_TYPE));
}

DetectionKind LoadingLimits::getDetectionKind() const {
    return static_cast<DetectionKind>(backend_->getInt(handle_, prop::LL_DETECTION_KIND));
}

double LoadingLimits::getPermanentLimit() const {
    return backend_->getDouble(handle_, prop::LL_PERMANENT_LIMIT);
}

LoadingLimits& LoadingLimits::setPermanentLimit(double permanentLimit) {
    backend_->setDouble(handle_, prop::LL_PERMANENT_LIMIT, permanentLimit);
    return *this;
}

std::string LoadingLimits::getPermanentLimitName() const {
    return backend_->getString(handle_, prop::LL_PERMANENT_LIMIT_NAME);
}

LoadingLimits& LoadingLimits::setPermanentLimitName(const std::string& permanentLimitName) {
    backend_->setString(handle_, prop::LL_PERMANENT_LIMIT_NAME, permanentLimitName);
    return *this;
}

std::vector<TemporaryLimit> LoadingLimits::getTemporaryLimits() const {
    return detail::wrapAll<TemporaryLimit>(backend_->getChildren(handle_, prop::TEMPORARY_LIMIT), backend_);
}

iidm::optional<TemporaryLimit> LoadingLimits::getTemporaryLimit(int acceptableDuration) const {
    return detail::wrapOptional<TemporaryLimit>(
        backend_->getRelatedByIndex(handle_, prop::TEMPORARY_LIMIT, acceptableDuration), backend_);
}

double LoadingLimits::getTemporaryLimitValue(int acceptableDuration) const {
    return backend_->evaluate(handle_, prop::LL_TEMPORARY_LIMIT_VALUE, {static_cast<double>(acceptableDuration)});
}

LoadingLimits& LoadingLimits::setTemporaryLimitValue(int acceptableDuration, double value) {
    // Two values at once: a change, applied by ACT_ADD
    HandleRef change(backend_->invoke(handle_, prop::LL_TEMPORARY_LIMIT_VALUE), backend_);
    backend_->setInt(change, prop::TL_ACCEPTABLE_DURATION, acceptableDuration);
    backend_->setDouble(change, prop::TL_VALUE, value);
    backend_->invoke(change, prop::ACT_ADD);
    return *this;
}

void LoadingLimits::remove() {
    backend_->invoke(handle_, prop::ACT_REMOVE);
}

} // namespace iidm
