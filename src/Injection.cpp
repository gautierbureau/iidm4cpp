#include <iidm/Injection.h>
#include <iidm/BackendProvider.h>

namespace iidm {

Terminal Injection::getTerminal() const {
    return Terminal(backend_->getRelated(handle_, prop::REL_TERMINAL), backend_);
}

} // namespace iidm
