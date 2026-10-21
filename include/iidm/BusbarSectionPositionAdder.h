#pragma once

#include <iidm/Adder.h>
#include <iidm/BusbarSectionPosition.h>

namespace iidm {

// New BusbarSectionPosition extension of a busbar section
class BusbarSectionPositionAdder : public detail::AdderBase<BusbarSectionPositionAdder> {
public:
    BusbarSectionPositionAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    BusbarSectionPositionAdder& withBusbarIndex(int busbarIndex) { return intField(prop::EXT_BSP_BUSBAR_INDEX, busbarIndex); }
    BusbarSectionPositionAdder& withSectionIndex(int sectionIndex) { return intField(prop::EXT_BSP_SECTION_INDEX, sectionIndex); }

    BusbarSectionPosition add() { return BusbarSectionPosition(create(), backend_); }
};

} // namespace iidm
