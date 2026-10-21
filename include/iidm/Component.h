#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/PropertyCodes.h>
#include <vector>

namespace iidm {

class BackendProvider;
class Bus;

// A connected or a synchronous component of the bus view (powsybl Component): buses connected through
// equipment, AC and HVDC (connected component) or AC only (synchronous component). They are numbered
// by size: 0 for the main (biggest) one.
class Component : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::COMPONENT;

    Component() = default;
    explicit Component(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

    int getNum() const { return getInt(prop::COMPONENT_NUM); }
    // Its number of buses
    int getSize() const { return getInt(prop::COMPONENT_SIZE); }
    // Its buses, of the bus view (include <iidm/Bus.h> to use it)
    std::vector<Bus> getBuses() const;
};

} // namespace iidm
