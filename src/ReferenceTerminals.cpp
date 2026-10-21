#include <iidm/ReferenceTerminals.h>
#include <iidm/BackendProvider.h>
#include <iidm/HandleRef.h>
#include <iidm/Network.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

ReferenceTerminals::ReferenceTerminals(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

std::vector<Terminal> ReferenceTerminals::getReferenceTerminals() const {
    return detail::wrapAll<Terminal>(backend_->getChildren(handle_, prop::TERMINAL), backend_);
}

ReferenceTerminals& ReferenceTerminals::setReferenceTerminals(const std::vector<Terminal>& terminals) {
    HandleRef change(backend_->invoke(handle_, prop::TERMINAL), backend_);
    for (const Terminal& terminal : terminals) {
        backend_->setRelated(change, prop::ADD_REFERENCE_TERMINAL, detail::Access::handle(terminal));
    }
    backend_->invoke(change, prop::ACT_ADD);
    return *this;
}

ReferenceTerminals& ReferenceTerminals::reset() {
    backend_->invoke(handle_, prop::ACT_RESET_REFERENCE_TERMINALS);
    return *this;
}

ReferenceTerminals& ReferenceTerminals::addReferenceTerminal(const Terminal& terminal) {
    backend_->setRelated(handle_, prop::ADD_REFERENCE_TERMINAL, detail::Access::handle(terminal));
    return *this;
}

std::vector<Terminal> ReferenceTerminals::getTerminals(const Network& network) {
    if (!network.hasExtension<ReferenceTerminals>()) return std::vector<Terminal>();
    return network.getExtension<ReferenceTerminals>().getReferenceTerminals();
}

void ReferenceTerminals::reset(const Network& network) {
    detail::Access::backend(network)->invoke(detail::Access::handle(network), prop::ACT_RESET_REFERENCE_TERMINALS);
}

void ReferenceTerminals::addTerminal(const Terminal& terminal) {
    detail::Access::backend(terminal)->invoke(detail::Access::handle(terminal), prop::ACT_ADD_REFERENCE_TERMINAL);
}

} // namespace iidm
