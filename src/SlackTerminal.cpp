#include <iidm/SlackTerminal.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

SlackTerminal::SlackTerminal(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

iidm::optional<Terminal> SlackTerminal::getTerminal() const {
    return detail::wrapOptional<Terminal>(backend_->getRelated(handle_, prop::REL_TERMINAL), backend_);
}

SlackTerminal& SlackTerminal::setTerminal(const Terminal& terminal, bool cleanIfEmpty) {
    backend_->setRelated(handle_, prop::REL_TERMINAL, detail::Access::handle(terminal));
    if (cleanIfEmpty) backend_->invoke(handle_, prop::ACT_CLEAN_IF_EMPTY);
    return *this;
}

bool SlackTerminal::isEmpty() const {
    return backend_->getBool(handle_, prop::EXT_ST_EMPTY);
}

} // namespace iidm
