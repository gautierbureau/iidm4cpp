#include <iidm/HandleRef.h>
#include <iidm/BackendProvider.h>
#include "HandleTracker.h"
#include <utility>
#include <vector>

namespace iidm {

// ── HandleTracker ─────────────────────────────────────────────────────────────

namespace detail {

void HandleTracker::retain(ObjectHandle h) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (owner_) ++counts_[h];
}

void HandleTracker::release(ObjectHandle h) noexcept {
    BackendProvider* owner = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = counts_.find(h);
        if (it == counts_.end() || --it->second > 0) return;
        counts_.erase(it);
        owner = owner_;
    }
    if (owner) {
        try {
            owner->releaseHandle(h);
        } catch (...) {
            // Called from destructors: a failed release only leaks the Java-side reference
        }
    }
}

void HandleTracker::releaseAll() noexcept {
    std::vector<ObjectHandle> handles;
    BackendProvider* owner = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& entry : counts_) handles.push_back(entry.first);
        counts_.clear();
        owner = owner_;
        owner_ = nullptr;
    }
    if (!owner) return;
    for (ObjectHandle h : handles) {
        try {
            owner->releaseHandle(h);
        } catch (...) {
        }
    }
}

void HandleTracker::detach() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    counts_.clear();
    owner_ = nullptr;
}

std::size_t HandleTracker::liveHandleCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return counts_.size();
}

HandleTracker::~HandleTracker() = default;

void HandleTracker::keep(std::unique_ptr<BackendProvider> closed) {
    // The tracker owns the backend now: the backend only keeps a weak reference to it
    std::shared_ptr<HandleTracker> tracker = std::move(closed->tracker_);
    closed->keptBy_ = tracker;
    tracker->kept_ = std::move(closed);
}

} // namespace detail

// ── HandleRef ─────────────────────────────────────────────────────────────────

HandleRef::HandleRef(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle) {
    // An invalid handle holds the tracker too, uncounted: it keeps the backend of a destroyed network
    if (backend) {
        tracker_ = backend->handleTracker();
        retain();
    }
}

HandleRef::HandleRef(const HandleRef& other)
    : handle_(other.handle_), tracker_(other.tracker_) {
    retain();
}

HandleRef::HandleRef(HandleRef&& other) noexcept
    : handle_(other.handle_), tracker_(std::move(other.tracker_)) {
    other.handle_ = INVALID_HANDLE;
}

HandleRef& HandleRef::operator=(const HandleRef& other) {
    if (this != &other) {
        HandleRef copy(other);
        *this = std::move(copy);
    }
    return *this;
}

HandleRef& HandleRef::operator=(HandleRef&& other) noexcept {
    if (this != &other) {
        release();
        handle_  = other.handle_;
        tracker_ = std::move(other.tracker_);
        other.handle_ = INVALID_HANDLE;
    }
    return *this;
}

HandleRef::~HandleRef() {
    release();
}

void HandleRef::retain() {
    if (tracker_ && handle_ != INVALID_HANDLE) tracker_->retain(handle_);
}

void HandleRef::release() noexcept {
    if (tracker_) {
        if (handle_ != INVALID_HANDLE) tracker_->release(handle_);
        tracker_.reset();
    }
}

} // namespace iidm
