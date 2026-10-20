#pragma once

#include <iidm/ObjectHandle.h>
#include <mutex>
#include <unordered_map>

namespace iidm {

class BackendProvider;

namespace detail {

// Counts the HandleRefs of each handle of one backend. Shared with the HandleRefs, so that
// wrappers outliving their backend (or Network) can still be destroyed safely: the owner is
// cleared when the backend goes away and the remaining releases become no-ops.
class HandleTracker {
public:
    explicit HandleTracker(BackendProvider* owner) : owner_(owner) {}

    void retain(ObjectHandle h);
    void release(ObjectHandle h) noexcept;

    // Releases every handle still referenced (backend closing) and stops tracking.
    void releaseAll() noexcept;
    void detach() noexcept;

    std::size_t liveHandleCount() const;

private:
    mutable std::mutex                       mutex_;
    std::unordered_map<ObjectHandle, long>   counts_;
    BackendProvider*                         owner_;
};

} } // namespace iidm::detail
