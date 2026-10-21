#include <iidm/ConnectablePosition.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

ConnectablePosition::Feeder& ConnectablePosition::Feeder::removeOrder() {
    backend_->invoke(handle_, prop::ACT_REMOVE_ORDER);
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
