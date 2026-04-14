#include <iidm/Load.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

Load::Load(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string Load::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string Load::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

double Load::getP0() const {
    return backend_->getDouble(handle_, prop::LOAD_P0);
}

Load& Load::setP0(double p0) {
    backend_->setDouble(handle_, prop::LOAD_P0, p0);
    return *this;
}

double Load::getQ0() const {
    return backend_->getDouble(handle_, prop::LOAD_Q0);
}

Load& Load::setQ0(double q0) {
    backend_->setDouble(handle_, prop::LOAD_Q0, q0);
    return *this;
}

LoadType Load::getLoadType() const {
    return static_cast<LoadType>(backend_->getInt(handle_, prop::LOAD_TYPE));
}

Terminal Load::getTerminal() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL);
    return Terminal(termHandle, backend_);
}

void Load::connect() {
    getTerminal().connect();
}

void Load::disconnect() {
    getTerminal().disconnect();
}

bool Load::isConnected() const {
    return getTerminal().isConnected();
}

bool Load::operator==(const Load& other) const {
    return handle_ == other.handle_;
}

bool Load::operator!=(const Load& other) const {
    return !(*this == other);
}

} // namespace iidm
