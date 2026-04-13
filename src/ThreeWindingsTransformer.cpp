#include <iidm/ThreeWindingsTransformer.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

ThreeWindingsTransformer::ThreeWindingsTransformer(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string ThreeWindingsTransformer::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string ThreeWindingsTransformer::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

Terminal ThreeWindingsTransformer::getTerminal1() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL1);
    return Terminal(termHandle, backend_);
}

Terminal ThreeWindingsTransformer::getTerminal2() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL2);
    return Terminal(termHandle, backend_);
}

Terminal ThreeWindingsTransformer::getTerminal3() const {
    // REL_TERMINAL maps to terminal 3 for the three windings transformer
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL);
    return Terminal(termHandle, backend_);
}

void ThreeWindingsTransformer::connect() {
    getTerminal1().connect();
    getTerminal2().connect();
    getTerminal3().connect();
}

void ThreeWindingsTransformer::disconnect() {
    getTerminal1().disconnect();
    getTerminal2().disconnect();
    getTerminal3().disconnect();
}

bool ThreeWindingsTransformer::isConnected() const {
    return getTerminal1().isConnected() && getTerminal2().isConnected() && getTerminal3().isConnected();
}

bool ThreeWindingsTransformer::operator==(const ThreeWindingsTransformer& other) const {
    return handle_ == other.handle_;
}

bool ThreeWindingsTransformer::operator!=(const ThreeWindingsTransformer& other) const {
    return !(*this == other);
}

} // namespace iidm
