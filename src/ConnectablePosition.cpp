#include <iidm/ConnectablePosition.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

iidm::optional<std::string> ConnectablePosition::Feeder::getName() const {
    return backend_->getOptString(handle_, prop::EXT_FEEDER_NAME);
}

ConnectablePosition::Feeder& ConnectablePosition::Feeder::setName(const std::string& name) {
    backend_->setString(handle_, prop::EXT_FEEDER_NAME, name);
    return *this;
}

iidm::optional<int> ConnectablePosition::Feeder::getOrder() const {
    return backend_->getOptInt(handle_, prop::EXT_FEEDER_ORDER);
}

ConnectablePosition::Feeder& ConnectablePosition::Feeder::setOrder(int order) {
    backend_->setInt(handle_, prop::EXT_FEEDER_ORDER, order);
    return *this;
}

ConnectablePosition::Feeder& ConnectablePosition::Feeder::removeOrder() {
    backend_->invoke(handle_, prop::ACT_REMOVE_ORDER);
    return *this;
}

ConnectablePosition::Direction ConnectablePosition::Feeder::getDirection() const {
    return static_cast<Direction>(backend_->getInt(handle_, prop::EXT_FEEDER_DIRECTION));
}

ConnectablePosition::Feeder& ConnectablePosition::Feeder::setDirection(Direction direction) {
    backend_->setInt(handle_, prop::EXT_FEEDER_DIRECTION, static_cast<int>(direction));
    return *this;
}

ConnectablePosition::ConnectablePosition(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

iidm::optional<ConnectablePosition::Feeder> ConnectablePosition::getFeeder() const {
    return detail::wrapOptional<Feeder>(backend_->getRelatedByIndex(handle_, prop::FEEDER, 0), backend_);
}

iidm::optional<ConnectablePosition::Feeder> ConnectablePosition::getFeeder1() const {
    return detail::wrapOptional<Feeder>(backend_->getRelatedByIndex(handle_, prop::FEEDER, 1), backend_);
}

iidm::optional<ConnectablePosition::Feeder> ConnectablePosition::getFeeder2() const {
    return detail::wrapOptional<Feeder>(backend_->getRelatedByIndex(handle_, prop::FEEDER, 2), backend_);
}

iidm::optional<ConnectablePosition::Feeder> ConnectablePosition::getFeeder3() const {
    return detail::wrapOptional<Feeder>(backend_->getRelatedByIndex(handle_, prop::FEEDER, 3), backend_);
}

} // namespace iidm
