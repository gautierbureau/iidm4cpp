#include <iidm/BusbarSection.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>
#include "Extensions.h"

namespace iidm {

bool BusbarSection::hasBusbarSectionPosition() const {
    return detail::hasExtension<BusbarSectionPosition>(backend_, handle_);
}

BusbarSectionPosition BusbarSection::getBusbarSectionPosition() const {
    return detail::getExtension<BusbarSectionPosition>(backend_, handle_, "BusbarSectionPosition");
}

BusbarSectionPositionAdder BusbarSection::newBusbarSectionPosition() {
    return detail::newExtension<BusbarSectionPosition, BusbarSectionPositionAdder>(backend_, handle_);
}

void BusbarSection::removeBusbarSectionPosition() {
    detail::removeExtension<BusbarSectionPosition>(backend_, handle_);
}

} // namespace iidm
