#include <iidm/SecondaryVoltageControl.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "Wrap.h"

namespace iidm {

std::vector<std::string> PilotPoint::getBusbarSectionsOrBusesIds() const {
    return backend_->getStringList(handle_, prop::EXT_SVC_PILOT_POINT_IDS);
}

PilotPoint ControlZone::getPilotPoint() const {
    return PilotPoint(backend_->getRelated(handle_, prop::PILOT_POINT), backend_);
}

std::vector<ControlUnit> ControlZone::getControlUnits() const {
    return detail::wrapAll<ControlUnit>(backend_->getChildren(handle_, prop::CONTROL_UNIT), backend_);
}

iidm::optional<ControlUnit> ControlZone::getControlUnit(const std::string& id) const {
    return detail::wrapOptional<ControlUnit>(backend_->getRelatedByString(handle_, prop::CONTROL_UNIT, id), backend_);
}

SecondaryVoltageControl::SecondaryVoltageControl(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

std::vector<ControlZone> SecondaryVoltageControl::getControlZones() const {
    return detail::wrapAll<ControlZone>(backend_->getChildren(handle_, prop::CONTROL_ZONE), backend_);
}

iidm::optional<ControlZone> SecondaryVoltageControl::getControlZone(const std::string& name) const {
    return detail::wrapOptional<ControlZone>(backend_->getRelatedByString(handle_, prop::CONTROL_ZONE, name), backend_);
}

} // namespace iidm
