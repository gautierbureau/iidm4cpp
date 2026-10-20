#include <iidm/Terminal.h>
#include <iidm/BackendProvider.h>
#include <iidm/Connectable.h>
#include <iidm/PropertyCodes.h>
#include <iidm/VoltageLevel.h>

namespace iidm {

// ── BusBreakerView ────────────────────────────────────────────────────────────

Bus Terminal::BusBreakerView::getBus() const {
    return Bus(backend_->getRelated(handle_, prop::REL_BUS), backend_);
}

Bus Terminal::BusBreakerView::getConnectableBus() const {
    return Bus(backend_->getRelated(handle_, prop::REL_CONNECTABLE_BUS), backend_);
}

// ── NodeBreakerView ───────────────────────────────────────────────────────────

int Terminal::NodeBreakerView::getNode() const {
    return backend_->getInt(handle_, prop::TERMINAL_NODE);
}

// ── Terminal ──────────────────────────────────────────────────────────────────

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

bool Terminal::connect() {
    // The terminal itself when this connected it (its handle is released right away)
    return HandleRef(backend_->invoke(handle_, prop::ACT_CONNECT), backend_) != INVALID_HANDLE;
}

bool Terminal::disconnect() {
    return HandleRef(backend_->invoke(handle_, prop::ACT_DISCONNECT), backend_) != INVALID_HANDLE;
}

Bus Terminal::getBusView() const {
    return Bus(backend_->getRelated(handle_, prop::REL_BUS), backend_);
}

std::string Terminal::getBusId() const {
    return backend_->getString(handle_, prop::TERMINAL_BUS_ID);
}

Connectable Terminal::getConnectable() const {
    return Connectable(backend_->getRelated(handle_, prop::REL_CONNECTABLE), backend_);
}

iidm::optional<ThreeSides> Terminal::getSide() const {
    const int side = backend_->getInt(handle_, prop::TERMINAL_SIDE);
    if (side < 0) return {};
    return static_cast<ThreeSides>(side);
}

VoltageLevel Terminal::getVoltageLevel() const {
    return VoltageLevel(backend_->getRelated(handle_, prop::REL_VOLTAGE_LEVEL), backend_);
}

Terminal::BusBreakerView Terminal::getBusBreakerView() const {
    return BusBreakerView(handle_, backend_);
}

Terminal::NodeBreakerView Terminal::getNodeBreakerView() const {
    return NodeBreakerView(handle_, backend_);
}

} // namespace iidm
