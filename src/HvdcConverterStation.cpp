#include <iidm/HvdcConverterStation.h>
#include <iidm/BackendProvider.h>
#include <iidm/HvdcLine.h>
#include "Wrap.h"

namespace iidm {

HvdcType HvdcConverterStation::getHvdcType() const {
    return static_cast<HvdcType>(backend_->getInt(handle_, prop::CONVERTER_STATION_HVDC_TYPE));
}

double HvdcConverterStation::getLossFactor() const {
    return backend_->getDouble(handle_, prop::CONVERTER_STATION_LOSS_FACTOR);
}

HvdcConverterStation& HvdcConverterStation::setLossFactor(double lossFactor) {
    backend_->setDouble(handle_, prop::CONVERTER_STATION_LOSS_FACTOR, lossFactor);
    return *this;
}

iidm::optional<HvdcLine> HvdcConverterStation::getHvdcLine() const {
    return detail::wrapOptional<HvdcLine>(backend_->getRelated(handle_, prop::REL_HVDC_LINE), backend_);
}

iidm::optional<HvdcConverterStation> HvdcConverterStation::getOtherConverterStation() const {
    return detail::wrapOptional<HvdcConverterStation>(backend_->getRelated(handle_, prop::REL_OTHER_CONVERTER_STATION), backend_);
}

} // namespace iidm
