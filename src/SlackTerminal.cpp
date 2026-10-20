#include <iidm/SlackTerminal.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

SlackTerminal::SlackTerminal(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle, backend), backend_(detail::orNullBackend(backend)) {}

Terminal SlackTerminal::getTerminal() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_SLACK_TERMINAL);
    return Terminal(termHandle, backend_);
}

SlackTerminal& SlackTerminal::setTerminal(const Terminal& terminal) {
    backend_->setRelated(handle_, prop::REL_SLACK_TERMINAL, terminal.handle_);
    return *this;
}

bool SlackTerminal::isEmpty() const {
    return backend_->getBool(handle_, prop::EXT_ST_EMPTY);
}

bool SlackTerminal::operator==(const SlackTerminal& other) const {
    return handle_ == other.handle_;
}

bool SlackTerminal::operator!=(const SlackTerminal& other) const {
    return !(*this == other);
}

} // namespace iidm
