#include <iidm/HvdcLine.h>
#include <iidm/HvdcAngleDroopActivePowerControl.h>
#include <iidm/HvdcOperatorActivePowerRange.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>
#include "Extensions.h"

namespace iidm {

double HvdcLine::getR() const {
    return backend_->getDouble(handle_, prop::HVDC_R);
}

HvdcLine& HvdcLine::setR(double r) {
    backend_->setDouble(handle_, prop::HVDC_R, r);
    return *this;
}

double HvdcLine::getNominalV() const {
    return backend_->getDouble(handle_, prop::HVDC_NOMINAL_V);
}

HvdcLine& HvdcLine::setNominalV(double nominalV) {
    backend_->setDouble(handle_, prop::HVDC_NOMINAL_V, nominalV);
    return *this;
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

HvdcLine& HvdcLine::setMaxP(double maxP) {
    backend_->setDouble(handle_, prop::HVDC_MAX_P, maxP);
    return *this;
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

HvdcConverterStation HvdcLine::getConverterStation(TwoSides side) const {
    return HvdcConverterStation(
        backend_->getRelatedByIndex(handle_, HvdcConverterStation::TYPE, static_cast<int>(side)), backend_);
}

bool HvdcLine::connectConverterStations() {
    // The line itself when this connected them (its handle is released right away)
    return HandleRef(backend_->invoke(handle_, prop::ACT_CONNECT), backend_) != INVALID_HANDLE;
}

bool HvdcLine::disconnectConverterStations() {
    return HandleRef(backend_->invoke(handle_, prop::ACT_DISCONNECT), backend_) != INVALID_HANDLE;
}

void HvdcLine::remove() {
    backend_->invoke(handle_, prop::ACT_REMOVE);
}

// ── Extensions ───────────────────────────────────────────────────────────────

bool HvdcLine::hasHvdcAngleDroopActivePowerControl() const {
    return detail::hasExtension<HvdcAngleDroopActivePowerControl>(backend_, handle_);
}

HvdcAngleDroopActivePowerControl HvdcLine::getHvdcAngleDroopActivePowerControl() const {
    return detail::getExtension<HvdcAngleDroopActivePowerControl>(backend_, handle_, "HvdcAngleDroopActivePowerControl");
}

HvdcAngleDroopActivePowerControlAdder HvdcLine::newHvdcAngleDroopActivePowerControl() {
    return detail::newExtension<HvdcAngleDroopActivePowerControl, HvdcAngleDroopActivePowerControlAdder>(backend_, handle_);
}

void HvdcLine::removeHvdcAngleDroopActivePowerControl() {
    detail::removeExtension<HvdcAngleDroopActivePowerControl>(backend_, handle_);
}

bool HvdcLine::hasHvdcOperatorActivePowerRange() const {
    return detail::hasExtension<HvdcOperatorActivePowerRange>(backend_, handle_);
}

HvdcOperatorActivePowerRange HvdcLine::getHvdcOperatorActivePowerRange() const {
    return detail::getExtension<HvdcOperatorActivePowerRange>(backend_, handle_, "HvdcOperatorActivePowerRange");
}

HvdcOperatorActivePowerRangeAdder HvdcLine::newHvdcOperatorActivePowerRange() {
    return detail::newExtension<HvdcOperatorActivePowerRange, HvdcOperatorActivePowerRangeAdder>(backend_, handle_);
}

void HvdcLine::removeHvdcOperatorActivePowerRange() {
    detail::removeExtension<HvdcOperatorActivePowerRange>(backend_, handle_);
}

} // namespace iidm
