#include <iidm/Battery.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

Battery::Battery(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

std::string Battery::getId() const {
    return backend_->getString(handle_, prop::ID);
}

std::string Battery::getName() const {
    return backend_->getString(handle_, prop::NAME);
}

double Battery::getTargetP() const {
    return backend_->getDouble(handle_, prop::BAT_TARGET_P);
}

Battery& Battery::setTargetP(double targetP) {
    backend_->setDouble(handle_, prop::BAT_TARGET_P, targetP);
    return *this;
}

double Battery::getTargetQ() const {
    return backend_->getDouble(handle_, prop::BAT_TARGET_Q);
}

Battery& Battery::setTargetQ(double targetQ) {
    backend_->setDouble(handle_, prop::BAT_TARGET_Q, targetQ);
    return *this;
}

double Battery::getMinP() const {
    return backend_->getDouble(handle_, prop::BAT_MIN_P);
}

double Battery::getMaxP() const {
    return backend_->getDouble(handle_, prop::BAT_MAX_P);
}

double Battery::getP0() const {
    return backend_->getDouble(handle_, prop::BAT_P0);
}

Battery& Battery::setP0(double p0) {
    backend_->setDouble(handle_, prop::BAT_P0, p0);
    return *this;
}

double Battery::getQ0() const {
    return backend_->getDouble(handle_, prop::BAT_Q0);
}

Battery& Battery::setQ0(double q0) {
    backend_->setDouble(handle_, prop::BAT_Q0, q0);
    return *this;
}

Terminal Battery::getTerminal() const {
    ObjectHandle termHandle = backend_->getRelated(handle_, prop::REL_TERMINAL);
    return Terminal(termHandle, backend_);
}

void Battery::connect() {
    getTerminal().connect();
}

void Battery::disconnect() {
    getTerminal().disconnect();
}

bool Battery::isConnected() const {
    return getTerminal().isConnected();
}

bool Battery::operator==(const Battery& other) const {
    return handle_ == other.handle_;
}

bool Battery::operator!=(const Battery& other) const {
    return !(*this == other);
}

} // namespace iidm
