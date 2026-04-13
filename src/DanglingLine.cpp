#include <iidm/DanglingLine.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

DanglingLine::DanglingLine(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string DanglingLine::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string DanglingLine::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

double DanglingLine::getP0() const {
    return backend_->getDouble(handle_, prop::DL_P0);
}

DanglingLine& DanglingLine::setP0(double p0) {
    backend_->setDouble(handle_, prop::DL_P0, p0);
    return *this;
}

double DanglingLine::getQ0() const {
    return backend_->getDouble(handle_, prop::DL_Q0);
}

DanglingLine& DanglingLine::setQ0(double q0) {
    backend_->setDouble(handle_, prop::DL_Q0, q0);
    return *this;
}

double DanglingLine::getR() const {
    return backend_->getDouble(handle_, prop::DL_R);
}

double DanglingLine::getX() const {
    return backend_->getDouble(handle_, prop::DL_X);
}

double DanglingLine::getG() const {
    return backend_->getDouble(handle_, prop::DL_G);
}

double DanglingLine::getB() const {
    return backend_->getDouble(handle_, prop::DL_B);
}

Terminal DanglingLine::getTerminal() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL);
    return Terminal(termHandle, backend_);
}

void DanglingLine::connect() {
    getTerminal().connect();
}

void DanglingLine::disconnect() {
    getTerminal().disconnect();
}

bool DanglingLine::isConnected() const {
    return getTerminal().isConnected();
}

bool DanglingLine::operator==(const DanglingLine& other) const {
    return handle_ == other.handle_;
}

bool DanglingLine::operator!=(const DanglingLine& other) const {
    return !(*this == other);
}

} // namespace iidm
