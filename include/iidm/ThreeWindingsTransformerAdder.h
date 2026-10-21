#pragma once

#include <iidm/Adder.h>
#include <iidm/Enums.h>
#include <iidm/ThreeWindingsTransformer.h>
#include <string>

namespace iidm {

// New three windings transformer of a substation (Substation::newThreeWindingsTransformer()): its three
// legs, between the star point and three of its voltage levels; their tap changers are added to them
// afterwards
class ThreeWindingsTransformerAdder : public detail::ConnectableAdder<ThreeWindingsTransformerAdder> {
public:
    class LegAdder;

    ThreeWindingsTransformerAdder(ObjectHandle adder, BackendProvider* backend) : ConnectableAdder(adder, backend) {}

    // The voltage of the star point (kV)
    ThreeWindingsTransformerAdder& setRatedU0(double ratedU0) { return doubleField(prop::THREE_WT_RATED_U0, ratedU0); }
    LegAdder newLeg1();
    LegAdder newLeg2();
    LegAdder newLeg3();

    ThreeWindingsTransformer add() { return ThreeWindingsTransformer(create(), backend_); }

private:
    LegAdder newLeg(ThreeSides side);
};

// A leg of a new three windings transformer: add() returns to the transformer's adder
class ThreeWindingsTransformerAdder::LegAdder : public detail::AdderBase<LegAdder> {
public:
    LegAdder(ObjectHandle adder, BackendProvider* backend, ThreeWindingsTransformerAdder& parent)
        : AdderBase(adder, backend), parent_(parent) {}

    LegAdder& setR(double r)              { return doubleField(prop::LEG_R, r); }
    LegAdder& setX(double x)              { return doubleField(prop::LEG_X, x); }
    LegAdder& setG(double g)              { return doubleField(prop::LEG_G, g); }
    LegAdder& setB(double b)              { return doubleField(prop::LEG_B, b); }
    LegAdder& setRatedU(double ratedU)    { return doubleField(prop::LEG_RATED_U, ratedU); }
    LegAdder& setRatedS(double ratedS)    { return doubleField(prop::LEG_RATED_S, ratedS); }
    LegAdder& setVoltageLevel(const std::string& voltageLevelId) { return stringField(prop::TERMINAL_VOLTAGE_LEVEL_ID, voltageLevelId); }
    // Bus-breaker topology: the bus it is connected to, and the bus it can be connected to
    LegAdder& setBus(const std::string& busId)            { return stringField(prop::TERMINAL_BUS_BREAKER_BUS_ID, busId); }
    LegAdder& setConnectableBus(const std::string& busId) { return stringField(prop::TERMINAL_BUS_BREAKER_CONNECTABLE_BUS_ID, busId); }
    // Node-breaker topology
    LegAdder& setNode(int node) { return intField(prop::TERMINAL_NODE, node); }

    ThreeWindingsTransformerAdder& add() {
        create();
        return parent_;
    }

private:
    friend class ThreeWindingsTransformerAdder;
    LegAdder& onSide(ThreeSides side) { return intField(prop::LEG_SIDE, static_cast<int>(side)); }

    ThreeWindingsTransformerAdder& parent_;
};

inline ThreeWindingsTransformerAdder::LegAdder ThreeWindingsTransformerAdder::newLeg(ThreeSides side) {
    LegAdder leg(backend_->invoke(adder_, prop::THREE_WINDINGS_TRANSFORMER_LEG), backend_, *this);
    leg.onSide(side);
    return leg;
}

inline ThreeWindingsTransformerAdder::LegAdder ThreeWindingsTransformerAdder::newLeg1() { return newLeg(ThreeSides::ONE); }
inline ThreeWindingsTransformerAdder::LegAdder ThreeWindingsTransformerAdder::newLeg2() { return newLeg(ThreeSides::TWO); }
inline ThreeWindingsTransformerAdder::LegAdder ThreeWindingsTransformerAdder::newLeg3() { return newLeg(ThreeSides::THREE); }

} // namespace iidm
