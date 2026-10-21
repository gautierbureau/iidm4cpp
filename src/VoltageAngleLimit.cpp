#include <iidm/VoltageAngleLimit.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

Terminal VoltageAngleLimit::getTerminalFrom() const {
    return Terminal(backend_->getRelated(handle_, prop::REL_TERMINAL_FROM), backend_);
}

Terminal VoltageAngleLimit::getTerminalTo() const {
    return Terminal(backend_->getRelated(handle_, prop::REL_TERMINAL_TO), backend_);
}

void VoltageAngleLimit::remove() {
    backend_->invoke(handle_, prop::ACT_REMOVE);
}

} // namespace iidm
