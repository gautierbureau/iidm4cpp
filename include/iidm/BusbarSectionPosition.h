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
    int getBusbarIndex() const;
    BusbarSectionPosition& setBusbarIndex(int busbarIndex);

    // Its rank along the busbar (from 1)
    int getSectionIndex() const;
    BusbarSectionPosition& setSectionIndex(int sectionIndex);
};

} // namespace iidm
