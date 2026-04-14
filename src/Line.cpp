#include <iidm/Line.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

Line::Line(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string Line::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string Line::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

double Line::getR() const {
    return backend_->getDouble(handle_, prop::LINE_R);
}

double Line::getX() const {
    return backend_->getDouble(handle_, prop::LINE_X);
}

double Line::getG1() const {
    return backend_->getDouble(handle_, prop::LINE_G1);
}

double Line::getB1() const {
    return backend_->getDouble(handle_, prop::LINE_B1);
}

double Line::getG2() const {
    return backend_->getDouble(handle_, prop::LINE_G2);
}

double Line::getB2() const {
    return backend_->getDouble(handle_, prop::LINE_B2);
}

Terminal Line::getTerminal1() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL1);
    return Terminal(termHandle, backend_);
}

Terminal Line::getTerminal2() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL2);
    return Terminal(termHandle, backend_);
}

void Line::connect() {
    getTerminal1().connect();
    getTerminal2().connect();
}

void Line::disconnect() {
    getTerminal1().disconnect();
    getTerminal2().disconnect();
}

bool Line::isConnected() const {
    return getTerminal1().isConnected() && getTerminal2().isConnected();
}

bool Line::operator==(const Line& other) const {
    return handle_ == other.handle_;
}

bool Line::operator!=(const Line& other) const {
    return !(*this == other);
}

} // namespace iidm
