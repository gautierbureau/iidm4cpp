#include <iidm/BackendProvider.h>
#include "HandleTracker.h"

namespace iidm {

BackendProvider::BackendProvider()
    : tracker_(std::make_shared<detail::HandleTracker>(this)) {}

BackendProvider::~BackendProvider() {
    // Wrappers outliving the backend must not call into it anymore (a backend kept by its tracker
    // goes with the tracker)
    if (tracker_) tracker_->detach();
}

std::shared_ptr<detail::HandleTracker> BackendProvider::handleTracker() const {
    return tracker_ ? tracker_ : keptBy_.lock();
}

std::size_t BackendProvider::liveHandleCount() const {
    std::shared_ptr<detail::HandleTracker> tracker = handleTracker();
    return tracker ? tracker->liveHandleCount() : 0;
}

void BackendProvider::releaseAllHandles() noexcept {
    if (tracker_) tracker_->releaseAll();
}

} // namespace iidm
