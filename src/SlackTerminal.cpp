#include <iidm/SlackTerminal.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

SlackTerminal::SlackTerminal(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

Terminal SlackTerminal::getTerminal() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_SLACK_TERMINAL);
    return Terminal(termHandle, backend_);
}

bool SlackTerminal::operator==(const SlackTerminal& other) const {
    return handle_ == other.handle_;
}

bool SlackTerminal::operator!=(const SlackTerminal& other) const {
    return !(*this == other);
}

} // namespace iidm
