#pragma once

#include <iidm/ObjectHandle.h>
#include <memory>
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
    ~HandleTracker();

    // The backend of a destroyed network, closed: it stays as long as a wrapper holds the tracker
    // (wrappers with an invalid handle included), answering them with InvalidHandleException
    // instead of being freed under them, and goes with the last of them.
    static void keep(std::unique_ptr<BackendProvider> closed);

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
    std::unique_ptr<BackendProvider>         kept_;
};

} } // namespace iidm::detail
