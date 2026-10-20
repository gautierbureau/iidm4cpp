#include <iidm/BackendProvider.h>
#include "HandleTracker.h"

namespace iidm {

BackendProvider::BackendProvider()
    : tracker_(std::make_shared<detail::HandleTracker>(this)) {}

BackendProvider::~BackendProvider() {
    // Wrappers outliving the backend must not call into it anymore
    tracker_->detach();
}

std::size_t BackendProvider::liveHandleCount() const {
    return tracker_->liveHandleCount();
}

void BackendProvider::releaseAllHandles() noexcept {
    tracker_->releaseAll();
}

} // namespace iidm
