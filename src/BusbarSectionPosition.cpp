#include <iidm/BusbarSectionPosition.h>
#include <iidm/BackendProvider.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

BusbarSectionPosition::BusbarSectionPosition(ObjectHandle handle, BackendProvider* backend)
    : ObjectRef(handle, backend) {}

int BusbarSectionPosition::getBusbarIndex() const {
    return backend_->getInt(handle_, prop::EXT_BSP_BUSBAR_INDEX);
}

BusbarSectionPosition& BusbarSectionPosition::setBusbarIndex(int busbarIndex) {
    backend_->setInt(handle_, prop::EXT_BSP_BUSBAR_INDEX, busbarIndex);
    return *this;
}

int BusbarSectionPosition::getSectionIndex() const {
    return backend_->getInt(handle_, prop::EXT_BSP_SECTION_INDEX);
}

BusbarSectionPosition& BusbarSectionPosition::setSectionIndex(int sectionIndex) {
    backend_->setInt(handle_, prop::EXT_BSP_SECTION_INDEX, sectionIndex);
    return *this;
}

} // namespace iidm
