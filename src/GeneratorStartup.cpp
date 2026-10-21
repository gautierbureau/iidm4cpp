#include <iidm/GeneratorStartup.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

GeneratorStartup::GeneratorStartup(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

double GeneratorStartup::getPlannedActivePowerSetpoint() const {
    return backend_->getDouble(handle_, prop::EXT_GS_PLANNED_ACTIVE_POWER_SETPOINT);
}

GeneratorStartup& GeneratorStartup::setPlannedActivePowerSetpoint(double plannedActivePowerSetpoint) {
    backend_->setDouble(handle_, prop::EXT_GS_PLANNED_ACTIVE_POWER_SETPOINT, plannedActivePowerSetpoint);
    return *this;
}

double GeneratorStartup::getStartupCost() const {
    return backend_->getDouble(handle_, prop::EXT_GS_STARTUP_COST);
}

GeneratorStartup& GeneratorStartup::setStartupCost(double startupCost) {
    backend_->setDouble(handle_, prop::EXT_GS_STARTUP_COST, startupCost);
    return *this;
}

double GeneratorStartup::getMarginalCost() const {
    return backend_->getDouble(handle_, prop::EXT_GS_MARGINAL_COST);
}

GeneratorStartup& GeneratorStartup::setMarginalCost(double marginalCost) {
    backend_->setDouble(handle_, prop::EXT_GS_MARGINAL_COST, marginalCost);
    return *this;
}

double GeneratorStartup::getPlannedOutageRate() const {
    return backend_->getDouble(handle_, prop::EXT_GS_PLANNED_OUTAGE_RATE);
}

GeneratorStartup& GeneratorStartup::setPlannedOutageRate(double plannedOutageRate) {
    backend_->setDouble(handle_, prop::EXT_GS_PLANNED_OUTAGE_RATE, plannedOutageRate);
    return *this;
}

double GeneratorStartup::getForcedOutageRate() const {
    return backend_->getDouble(handle_, prop::EXT_GS_FORCED_OUTAGE_RATE);
}

GeneratorStartup& GeneratorStartup::setForcedOutageRate(double forcedOutageRate) {
    backend_->setDouble(handle_, prop::EXT_GS_FORCED_OUTAGE_RATE, forcedOutageRate);
    return *this;
}

} // namespace iidm
