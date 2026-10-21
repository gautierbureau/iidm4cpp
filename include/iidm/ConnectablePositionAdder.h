#pragma once

#include <iidm/Adder.h>
#include <iidm/ConnectablePosition.h>
#include <string>

namespace iidm {

// New ConnectablePosition extension of a connectable: a feeder for an injection (newFeeder()), or
// one per side (newFeeder1() to newFeeder3()). ValidationException from add() without a feeder, or
// with both kinds.
class ConnectablePositionAdder : public detail::AdderBase<ConnectablePositionAdder> {
public:
    class FeederAdder;

    ConnectablePositionAdder(ObjectHandle adder, BackendProvider* backend) : AdderBase(adder, backend) {}

    FeederAdder newFeeder();
    FeederAdder newFeeder1();
    FeederAdder newFeeder2();
    FeederAdder newFeeder3();

    ConnectablePosition add() { return ConnectablePosition(create(), backend_); }

private:
    FeederAdder newFeederAt(int slot);
};

// A feeder of a new ConnectablePosition: add() adds it and returns to the extension's adder
class ConnectablePositionAdder::FeederAdder : public detail::AdderBase<FeederAdder> {
public:
    FeederAdder(ObjectHandle adder, BackendProvider* backend, ConnectablePositionAdder& parent)
        : AdderBase(adder, backend), parent_(parent) {}

    FeederAdder& withName(const std::string& name) { return stringField(prop::EXT_FEEDER_NAME, name); }
    FeederAdder& withOrder(int order) { return intField(prop::EXT_FEEDER_ORDER, order); }
    FeederAdder& withDirection(ConnectablePosition::Direction direction) {
        return intField(prop::EXT_FEEDER_DIRECTION, static_cast<int>(direction));
    }

    ConnectablePositionAdder& add() {
        create();
        return parent_;
    }

private:
    friend class ConnectablePositionAdder;
    FeederAdder& atSlot(int slot) { return intField(prop::EXT_FEEDER_SLOT, slot); }

    ConnectablePositionAdder& parent_;
};

inline ConnectablePositionAdder::FeederAdder ConnectablePositionAdder::newFeederAt(int slot) {
    FeederAdder feeder(backend_->invoke(adder_, prop::FEEDER), backend_, *this);
    feeder.atSlot(slot);
    return feeder;
}

inline ConnectablePositionAdder::FeederAdder ConnectablePositionAdder::newFeeder() { return newFeederAt(0); }
inline ConnectablePositionAdder::FeederAdder ConnectablePositionAdder::newFeeder1() { return newFeederAt(1); }
inline ConnectablePositionAdder::FeederAdder ConnectablePositionAdder::newFeeder2() { return newFeederAt(2); }
inline ConnectablePositionAdder::FeederAdder ConnectablePositionAdder::newFeeder3() { return newFeederAt(3); }

} // namespace iidm
