#include <iidm/Connectable.h>
#include <iidm/BackendProvider.h>
#include "Wrap.h"

namespace iidm {

std::vector<Terminal> Connectable::getTerminals() const {
    return detail::wrapAll<Terminal>(backend_->getChildren(handle_, prop::TERMINAL), backend_);
}

bool Connectable::connect() {
    // The connectable itself when this connected it (its handle is released right away)
    return HandleRef(backend_->invoke(handle_, prop::ACT_CONNECT), backend_) != INVALID_HANDLE;
}

bool Connectable::disconnect() {
    return HandleRef(backend_->invoke(handle_, prop::ACT_DISCONNECT), backend_) != INVALID_HANDLE;
}

void Connectable::unsetSolvedValues() {
    backend_->invoke(handle_, prop::ACT_UNSET_SOLVED_VALUES);
}

void Connectable::remove() {
    backend_->invoke(handle_, prop::ACT_REMOVE);
}

} // namespace iidm
