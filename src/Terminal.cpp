#include <iidm/Terminal.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

// ── BusBreakerView ────────────────────────────────────────────────────────────

Terminal::BusBreakerView::BusBreakerView(ObjectHandle h, BackendProvider* b)
    : handle_(h), backend_(b) {}

Bus Terminal::BusBreakerView::getBus() const {
    return Bus(backend_->getRelated(handle_, prop::REL_BUS), backend_);
}

Bus Terminal::BusBreakerView::getConnectableBus() const {
    return Bus(backend_->getRelated(handle_, prop::REL_CONNECTABLE_BUS), backend_);
}

// ── NodeBreakerView ───────────────────────────────────────────────────────────

Terminal::NodeBreakerView::NodeBreakerView(ObjectHandle h, BackendProvider* b)
    : handle_(h), backend_(b) {}

int Terminal::NodeBreakerView::getNode() const {
    return backend_->getInt(handle_, prop::TERMINAL_NODE);
}

// ── Terminal ──────────────────────────────────────────────────────────────────

Terminal::Terminal(ObjectHandle handle, BackendProvider* backend)
    : handle_(handle), backend_(backend) {}

double Terminal::getP() const {
    return backend_->getDouble(handle_, prop::TERMINAL_P);
}

Terminal& Terminal::setP(double p) {
    backend_->setDouble(handle_, prop::TERMINAL_P, p);
    return *this;
}

double Terminal::getQ() const {
    return backend_->getDouble(handle_, prop::TERMINAL_Q);
}

Terminal& Terminal::setQ(double q) {
    backend_->setDouble(handle_, prop::TERMINAL_Q, q);
    return *this;
}

bool Terminal::isConnected() const {
    return backend_->getBool(handle_, prop::TERMINAL_CONNECTED);
}

void Terminal::connect() {
    backend_->setBool(handle_, prop::TERMINAL_CONNECTED, true);
}

void Terminal::disconnect() {
    backend_->setBool(handle_, prop::TERMINAL_CONNECTED, false);
}

Bus Terminal::getBusView() const {
    ObjectHandle busHandle = backend_->getRelated(handle_, prop::REL_BUS);
    return Bus(busHandle, backend_);
}

std::string Terminal::getBusId() const {
    return backend_->getString(handle_, prop::TERMINAL_BUS_ID);
}

Terminal::BusBreakerView Terminal::getBusBreakerView() const {
    return BusBreakerView(handle_, backend_);
}

Terminal::NodeBreakerView Terminal::getNodeBreakerView() const {
    return NodeBreakerView(handle_, backend_);
}

bool Terminal::operator==(const Terminal& other) const {
    return handle_ == other.handle_;
}

bool Terminal::operator!=(const Terminal& other) const {
    return !(*this == other);
}

} // namespace iidm
