#include <iidm/HvdcLine.h>
#include <iidm/HvdcAngleDroopActivePowerControl.h>
#include <iidm/HvdcOperatorActivePowerRange.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

double HvdcLine::getR() const {
    return backend_->getDouble(handle_, prop::HVDC_R);
}

double HvdcLine::getNominalV() const {
    return backend_->getDouble(handle_, prop::HVDC_NOMINAL_V);
}

double HvdcLine::getActivePowerSetpoint() const {
    return backend_->getDouble(handle_, prop::HVDC_ACTIVE_POWER_SETPOINT);
}

HvdcLine& HvdcLine::setActivePowerSetpoint(double value) {
    backend_->setDouble(handle_, prop::HVDC_ACTIVE_POWER_SETPOINT, value);
    return *this;
}

double HvdcLine::getMaxP() const {
    return backend_->getDouble(handle_, prop::HVDC_MAX_P);
}

HvdcConverterStationMode HvdcLine::getConvertersMode() const {
    return static_cast<HvdcConverterStationMode>(backend_->getInt(handle_, prop::HVDC_CONVERTERS_MODE));
}

HvdcLine& HvdcLine::setConvertersMode(HvdcConverterStationMode mode) {
    backend_->setInt(handle_, prop::HVDC_CONVERTERS_MODE, static_cast<int>(mode));
    return *this;
}

HvdcConverterStation HvdcLine::getConverterStation1() const {
    return HvdcConverterStation(backend_->getRelated(handle_, prop::REL_CONVERTER_STATION1), backend_);
}

HvdcConverterStation HvdcLine::getConverterStation2() const {
    return HvdcConverterStation(backend_->getRelated(handle_, prop::REL_CONVERTER_STATION2), backend_);
}

void HvdcLine::remove() {
    backend_->invoke(handle_, prop::ACT_REMOVE);
}

bool HvdcLine::hasHvdcAngleDroopActivePowerControl() const {
    return backend_->getBool(handle_, prop::EXT_HADAPC_EXISTS);
}

HvdcAngleDroopActivePowerControl HvdcLine::getHvdcAngleDroopActivePowerControl() const {
    return HvdcAngleDroopActivePowerControl(handle_, backend_);
}

bool HvdcLine::hasHvdcOperatorActivePowerRange() const {
    return backend_->getBool(handle_, prop::EXT_HOAR_EXISTS);
}

HvdcOperatorActivePowerRange HvdcLine::getHvdcOperatorActivePowerRange() const {
    return HvdcOperatorActivePowerRange(handle_, backend_);
}

} // namespace iidm
