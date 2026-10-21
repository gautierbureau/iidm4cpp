#include <iidm/HvdcConverterStation.h>
#include <iidm/BackendProvider.h>
#include <iidm/HvdcLine.h>
#include "Wrap.h"

namespace iidm {

iidm::optional<HvdcLine> HvdcConverterStation::getHvdcLine() const {
    return detail::wrapOptional<HvdcLine>(backend_->getRelated(handle_, prop::REL_HVDC_LINE), backend_);
}

iidm::optional<HvdcConverterStation> HvdcConverterStation::getOtherConverterStation() const {
    return detail::wrapOptional<HvdcConverterStation>(backend_->getRelated(handle_, prop::REL_OTHER_CONVERTER_STATION), backend_);
}

} // namespace iidm
