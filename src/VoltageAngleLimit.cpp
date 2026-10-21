#include <iidm/VoltageAngleLimit.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

std::string VoltageAngleLimit::getId() const {
    return backend_->getString(handle_, prop::VAL_ID);
}

LimitType VoltageAngleLimit::getLimitType() const {
    return static_cast<LimitType>(backend_->getInt(handle_, prop::LL_LIMIT_TYPE));
}

Terminal VoltageAngleLimit::getTerminalFrom() const {
    return Terminal(backend_->getRelated(handle_, prop::REL_TERMINAL_FROM), backend_);
}

Terminal VoltageAngleLimit::getTerminalTo() const {
    return Terminal(backend_->getRelated(handle_, prop::REL_TERMINAL_TO), backend_);
}

iidm::optional<double> VoltageAngleLimit::getLowLimit() const {
    return backend_->getOptDouble(handle_, prop::VAL_LOW_LIMIT);
}

iidm::optional<double> VoltageAngleLimit::getHighLimit() const {
    return backend_->getOptDouble(handle_, prop::VAL_HIGH_LIMIT);
}

void VoltageAngleLimit::remove() {
    backend_->invoke(handle_, prop::ACT_REMOVE);
}

} // namespace iidm
