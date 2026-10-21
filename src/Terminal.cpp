#include <iidm/Terminal.h>
#include <iidm/BackendProvider.h>
#include <iidm/Connectable.h>
#include <iidm/HandleRef.h>
#include <iidm/PropertyCodes.h>
#include <iidm/VoltageLevel.h>
#include "Wrap.h"

namespace iidm {

// ── BusBreakerView ────────────────────────────────────────────────────────────

iidm::optional<Bus> Terminal::BusBreakerView::getBus() const {
    return detail::wrapOptional<Bus>(backend_->getRelated(handle_, prop::REL_BUS), backend_);
}

iidm::optional<Bus> Terminal::BusBreakerView::getConnectableBus() const {
    return detail::wrapOptional<Bus>(backend_->getRelated(handle_, prop::REL_CONNECTABLE_BUS), backend_);
}

void Terminal::BusBreakerView::setConnectableBus(const std::string& busId) {
    backend_->setString(handle_, prop::TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID, busId);
}

void Terminal::BusBreakerView::moveConnectable(const std::string& busId, bool connected) {
    HandleRef move(backend_->invoke(handle_, prop::MOVE_CONNECTABLE), backend_);
    backend_->setString(move, prop::TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID, busId);
    backend_->setBool(move, prop::TERMINAL_CONNECTED, connected);
    backend_->invoke(move, prop::ACT_ADD);
}

// ── BusView ───────────────────────────────────────────────────────────────────

iidm::optional<Bus> Terminal::BusView::getBus() const {
    return detail::wrapOptional<Bus>(backend_->getRelated(handle_, prop::REL_BV_BUS), backend_);
}

iidm::optional<Bus> Terminal::BusView::getConnectableBus() const {
    return detail::wrapOptional<Bus>(backend_->getRelated(handle_, prop::REL_BV_CONNECTABLE_BUS), backend_);
}

// ── NodeBreakerView ───────────────────────────────────────────────────────────

int Terminal::NodeBreakerView::getNode() const {
    return backend_->getInt(handle_, prop::TERMINAL_NODE);
}

void Terminal::NodeBreakerView::moveConnectable(int node, const std::string& voltageLevelId) {
    HandleRef move(backend_->invoke(handle_, prop::MOVE_CONNECTABLE), backend_);
    backend_->setInt(move, prop::TERMINAL_NODE, node);
    backend_->setString(move, prop::TERMINAL_VOLTAGE_LEVEL_ID, voltageLevelId);
    backend_->invoke(move, prop::ACT_ADD);
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

double Terminal::getI() const {
    return backend_->getDouble(handle_, prop::TERMINAL_I);
}

void Terminal::unsetSolvedValues() {
    backend_->invoke(handle_, prop::ACT_UNSET_SOLVED_VALUES);
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

Terminal::BusView Terminal::getBusView() const {
    return BusView(handle_, backend_);
}

Terminal::NodeBreakerView Terminal::getNodeBreakerView() const {
    return NodeBreakerView(handle_, backend_);
}

} // namespace iidm
