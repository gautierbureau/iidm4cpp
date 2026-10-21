#pragma once

#include <iidm/ObjectRef.h>
#include <iidm/ObjectHandle.h>
#include <iidm/PropertyCodes.h>
#include <iidm/stdcxx/optional.hpp>
#include <string>

namespace iidm {

class BackendProvider;

// The ConnectablePosition extension of a connectable: how its ends are drawn in single line
// diagrams, a feeder per end (the feeder of an injection, or one per side). An object of its own,
// from Connectable::getConnectablePosition().
class ConnectablePosition : public detail::ObjectRef {
public:
    static constexpr int TYPE = prop::CONNECTABLE_POSITION;
    // powsybl ConnectablePosition.Direction
    enum class Direction { TOP, BOTTOM, UNDEFINED };

    // An end of the connectable in a diagram (powsybl ConnectablePosition.Feeder)
    class Feeder : public detail::ObjectRef {
    public:
        static constexpr int TYPE = prop::FEEDER;

        Feeder() = default;
        explicit Feeder(ObjectHandle handle, BackendProvider* backend) : ObjectRef(handle, backend) {}

        iidm::optional<std::string> getName() const;
        Feeder& setName(const std::string& name);
        // Its rank among the feeders of its busbar section (empty when unset)
        iidm::optional<int> getOrder() const;
        Feeder& setOrder(int order);
        Feeder& removeOrder();
        Direction getDirection() const;
        Feeder& setDirection(Direction direction);
    };

    ConnectablePosition() = default;
    explicit ConnectablePosition(ObjectHandle handle, BackendProvider* backend);

    // The feeder of an injection; those of the sides of a branch or a three windings transformer.
    // Empty when absent.
    iidm::optional<Feeder> getFeeder() const;
    iidm::optional<Feeder> getFeeder1() const;
    iidm::optional<Feeder> getFeeder2() const;
    iidm::optional<Feeder> getFeeder3() const;
};

} // namespace iidm
