#include <iidm/HvdcLine.h>
#include <iidm/HvdcAngleDroopActivePowerControl.h>
#include <iidm/HvdcOperatorActivePowerRange.h>
#include <iidm/BackendProvider.h>
#include <iidm/IidmException.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

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

} // namespace iidm
