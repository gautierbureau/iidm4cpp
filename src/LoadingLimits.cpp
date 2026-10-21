#include <iidm/LoadingLimits.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

// ── TemporaryLimit ────────────────────────────────────────────────────────────

TemporaryLimit::TemporaryLimit(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

// ── LoadingLimits ─────────────────────────────────────────────────────────────

LoadingLimits::LoadingLimits(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

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
