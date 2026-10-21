#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>

namespace iidm {

class BackendProvider;

// The BusbarSectionPosition extension of a busbar section: where it is drawn in single line
// diagrams. An object of its own, from BusbarSection::getBusbarSectionPosition().
class BusbarSectionPosition : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::BUSBAR_SECTION_POSITION;

    BusbarSectionPosition() = default;
    explicit BusbarSectionPosition(ObjectHandle handle, BackendProvider* backend);

    // The rank of its busbar (from 1)
    int getBusbarIndex() const { return getInt(prop::EXT_BSP_BUSBAR_INDEX); }
    BusbarSectionPosition& setBusbarIndex(int busbarIndex) {
        return setInt(*this, prop::EXT_BSP_BUSBAR_INDEX, busbarIndex);
    }

    // Its rank along the busbar (from 1)
    int getSectionIndex() const { return getInt(prop::EXT_BSP_SECTION_INDEX); }
    BusbarSectionPosition& setSectionIndex(int sectionIndex) {
        return setInt(*this, prop::EXT_BSP_SECTION_INDEX, sectionIndex);
    }
};

} // namespace iidm
